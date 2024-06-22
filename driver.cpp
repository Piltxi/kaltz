#include "driver.hpp"
#include "parser.hpp"

LLVMContext *context = new LLVMContext;
Module *module = new Module("Kaleidoscope", *context);
IRBuilder<> *builder = new IRBuilder(*context);

Value *LogErrorV(const std::string Str)
{
  std::cerr << Str << std::endl;
  return nullptr;
}

/* Il codice seguente sulle prime non è semplice da comprendere.
   Esso definisce una utility (funzione C++) con due parametri:
   1) la rappresentazione di una funzione llvm IR, e
   2) il nome per un registro SSA
   La chiamata di questa utility restituisce un'istruzione IR che alloca un double
   in memoria e ne memorizza il puntatore in un registro SSA cui viene attribuito
   il nome passato come secondo parametro. L'istruzione verrà scritta all'inizio
   dell'entry block della funzione passata come primo parametro.
   Si ricordi che le istruzioni sono generate da un builder. Per non
   interferire con il builder globale, la generazione viene dunque effettuata
   con un builder temporaneo TmpB
*/
static AllocaInst *CreateEntryBlockAlloca(Function *fun, StringRef VarName, Type *type = Type::getDoubleTy(*context))
{
  IRBuilder<> TmpB(&fun->getEntryBlock(), fun->getEntryBlock().begin());
  return TmpB.CreateAlloca(type, nullptr, VarName);
}

driver::driver() : trace_parsing(false), trace_scanning(false) {};

int driver::parse(const std::string &f)
{
  file = f;
  location.initialize(&file);
  scan_begin();
  yy::parser parser(*this);
  parser.set_debug_level(trace_parsing);
  int res = parser.parse();
  scan_end();
  return res;
}

void driver::codegen()
{
  // inizia il processo di generazione del codice chiamando il metodo codegen sul nodo radice dell'AST
  root->codegen(*this);
};

/************************* Sequence tree **************************/
SeqAST::SeqAST(RootAST *first, RootAST *continuation) : first(first), continuation(continuation) {};

// La classe SeqAST rappresenta una sequenza di istruzioni nell'AST
// Il metodo codegen per SeqAST chiama ricorsivamente codegen su ciascun nodo figlio (first e continuation)

Value *SeqAST::codegen(driver &drv)
{
  if (first != nullptr)
  {
    Value *f = first->codegen(drv);
  }
  else
  {
    if (continuation == nullptr)
      return nullptr;
  }
  Value *c = continuation->codegen(drv);
  return nullptr;
};

/** Number Expression Tree
 *  NumberExprAST rappresenta un nodo dell'AST per espressioni numeriche (numeri costanti).
 *  Il costruttore della classe NumberExprAST prende un valore di tipo double come parametro (Val)
 *  e lo assegna al membro della classe Val. Questo permette di creare un nodo che
 *  rappresenta un numero costante nell'AST.
 *
 *  Il metodo codegen genera il codice LLVM IR corrispondente a questo nodo dell'AST.
 *  Nel caso di NumberExprAST, genera una costante floating-point (ConstantFP)
 *  che rappresenta il valore memorizzato in Val.
 *
 *  Quando memorizzate in questo modo, le costanti appartengono al contesto, che in LLVM viene gestito in modo
 *  "condiviso" con tutte le lib
 */
NumberExprAST::NumberExprAST(double Val) : Val(Val) {};

lexval NumberExprAST::getLexVal() const
{
  // Non utilizzata, Inserita per continuità con versione precedente [!]
  lexval lval = Val;
  return lval;
};

Value *NumberExprAST::codegen(driver &drv)
{
  return ConstantFP::get(*context, APFloat(Val));
};

/** Variable Expression Tree
 *  costruisce la classe VariableExprAST con il nome della variabile (Name)
 *  e un puntatore a un'espressione (Exp).
 *  La variabile Exp viene essere utilizzata per rappresentare un'inizializzazione o un'espressione associata alla variabile.
 *
 *  Si cerca dapprima una variabile globale già definita, e se non si trova si crea un nuovo nodo.
 */
VariableExprAST::VariableExprAST(const std::string &Name, ExprAST *Exp) : Name(Name), Exp(Exp) {};

lexval VariableExprAST::getLexVal() const
{
  lexval lval = Name;
  return lval;
};

Value *VariableExprAST::codegen(driver &drv)
{
  AllocaInst *A = drv.NamedValues[Name];
  if (!A)
  {
    GlobalVariable *globVar = module->getNamedGlobal(Name);
    if (!globVar)
      return LogErrorV("undefined variable: " + Name);

    return builder->CreateLoad(globVar->getValueType(), globVar, Name.c_str());
  }
  return builder->CreateLoad(A->getAllocatedType(), A, Name.c_str());
}

/** Binary Expression Tree
 *  Le righe sotto implementano la gestione degli operatori richiesti dalla prima e dalla terza grammatica.
 *  Per comodità tutti gli operatori sono stati inseriti qui: gli operatori aritmetici e gli altri operatori booleani
 *
 *  Tutti gli operatori richiedono l'analisi di LHS e di un RHS; almeno tranne l'operatore unario NOT.
 *  Per l'operatore unario NOT si valuta soltanto il RHS; si costruisce la rappresentazione intermedia del RHS,
 *  e quando non fallita si procede a creare la ramificazione dell'albero sintattico.
 */
BinaryExprAST::BinaryExprAST(char Op, ExprAST *LHS, ExprAST *RHS) : Op(Op), LHS(LHS), RHS(RHS) {};

Value *BinaryExprAST::codegen(driver &drv)
{
  if (Op == 'n')
  {
    Value *R = RHS->codegen(drv);
    if (!R)
      return nullptr;
    return builder->CreateNot(R, "notres");
  }
  Value *L = LHS->codegen(drv);
  Value *R = RHS->codegen(drv);
  if (!L || !R)
    return nullptr;
  switch (Op)
  {
  case '+':
    return builder->CreateFAdd(L, R, "addres");
  case '-':
    return builder->CreateFSub(L, R, "subres");
  case '*':
    return builder->CreateFMul(L, R, "mulres");
  case '/':
    return builder->CreateFDiv(L, R, "addres");
  case '<':
    return builder->CreateFCmpULT(L, R, "lttest");
  case '>':
    return builder->CreateFCmpUGT(L, R, "gttest");
  case '=':
    return builder->CreateFCmpUEQ(L, R, "eqtest");
  case 'a':
    return builder->CreateLogicalAnd(L, R, "andres");
  case 'o':
    return builder->CreateLogicalOr(L, R, "orres");
  default:
    std::cout << Op << std::endl;
    return LogErrorV("binary operator not supported");
  }
};

/** Call [Procedure Calling] in AST
 *  Con questa classe si costruisce un AST sulle istruzioni di chiamata.
 *  Per costruirlo servono logicamente due informazioni: il nome della funzione e
 *  l'elenco degli argomenti che questa deve ricevere
 *
 *  La prima cosa da fare è la ricerca della funzione mediante l'handle a LLVM.
 *  Se l'handle esiste, si procede, viceversa ci si blocca e si genera un errore.
 *
 *  La seconda cosa da fare è verificare se la chiamata comprende tutti i parametri necessari;
 *  questo si fa mediante arg_size
 *
 *  Una volta verificate i "requisiti" della CC, si avvia la costruzione dei nodi di AST dei nodi argomenti,
 *  memorizzati in un vector.
 */
CallExprAST::CallExprAST(std::string Callee, std::vector<ExprAST *> Args) : Callee(Callee), Args(std::move(Args)) {};

lexval CallExprAST::getLexVal() const
{
  lexval lval = Callee;
  return lval;
};

Value *CallExprAST::codegen(driver &drv)
{
  Function *CalleeF = module->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("undefined function");

  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("incorrect number of arguments");

  std::vector<Value *> ArgsV;
  for (auto arg : Args)
  {
    ArgsV.push_back(arg->codegen(drv));
    if (!ArgsV.back())
      return nullptr;
  }
  return builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

/** IF BLOCK for expression
 *  si inizializzano i membri cond, trueexp e falseexp con i valori passati come parametri.
 *  questi tre membri sono sempre ExprAST.
 *
 *  * ____ PREPARAZIONE ____
 *  la generazione del codice intermedio per la condizione.
 *
 *  * ____ COSTRUZIONE ____
 *  vengono creati tre blocchi di base:
 *    1. TrueBB: per la parte del codice eseguita se la c. è vera
 *    2. FalseBB: per la parte del codice eseguita se la c. è falsa
 *    3. MergeBB: dove si uniscono i flussi
 *
 *  * FASE 1
 *  viene poi settato il builder per scrivere sul blocco true,
 *  e si scrive il blocco di codice da eseguire quando la condizione è rilevata vera
 *  una volta inserito il blocco true, si prepara l'albero il blocco per il merge.
 *  si crea pertanto l'istruzione di salto al blocco i merge, e si posizionano correttamente i blocchi base dentro fun.
 *
 *  * FASR 2
 *  si setta il builder per scrivere sul blocco falso,
 *  e si scrive il blocco di codice da eseguire quando la condizione è rilevata falsa
 *  una volta inserito il blocco false, si prepara l'albero il blocco per il merge.
 *  si crea pertanto l'istruzione di salto al blocco i merge, e si posizionano correttamente i blocchi base dentro fun.
 *
 *  * FASE 3
 *  dopo aver generato il codice per i rami true e false,
 *  il builder deve sapere dove inserire le istruzioni successive; si imposta quindi MergeBB come punto di partenza.
 *
 *  una volta definita la funzione PHI di convergenza, si aggiungono semplicemente i due punti di ingresso.
 *  Type::getDoubleTy
 */
IfExprAST::IfExprAST(ExprAST *cond, ExprAST *trueexp, ExprAST *falseexp) : cond(cond), trueexp(trueexp), falseexp(falseexp) {};

Value *IfExprAST::codegen(driver &drv)
{
  Value *CondV = cond->codegen(drv);
  if (!CondV)
    return nullptr;

  Function *fun = builder->GetInsertBlock()->getParent();
  BasicBlock *TrueBB = BasicBlock::Create(*context, "trueblock", fun);
  BasicBlock *FalseBB = BasicBlock::Create(*context, "falseblock");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergeblock");
  builder->CreateCondBr(CondV, TrueBB, FalseBB);

  //* FASE 1 - blocco vero
  builder->SetInsertPoint(TrueBB);
  Value *trueV = trueexp->codegen(drv);
  if (!trueV)
    return nullptr;
  TrueBB = builder->GetInsertBlock();
  builder->CreateBr(MergeBB);
  fun->insert(fun->end(), FalseBB);

  //* FASE 2 - blocco falso
  builder->SetInsertPoint(FalseBB);
  Value *falseV = falseexp->codegen(drv);
  if (!falseV)
    return nullptr;
  FalseBB = builder->GetInsertBlock();
  builder->CreateBr(MergeBB);
  fun->insert(fun->end(), MergeBB);

  //* FASE 3 - convergenza
  builder->SetInsertPoint(MergeBB);
  PHINode *P = builder->CreatePHI(Type::getDoubleTy(*context), 2);
  P->addIncoming(trueV, TrueBB);
  P->addIncoming(falseV, FalseBB);

  return P;
};

/** BlockAST::codegen
 *  il metodo è utile a generare del codice per un blocco ast,
 *  che può contenere sia dichiarazioni di variabili sia istruzioni
 *  per sua natura, come parametro prende un vettore StmtAST
 *
 *  Il codice viene attivato nel riconoscimento di un blocco di codice
 *  e semplicemente alloca i nuovi statement.
 *
 *  All'inizio si crea un vettore temporaneo tmp per salvare lo stato corrente della symbol table:
 *  necessario perché le dichiarazioni di variabili all'interno del blocco potrebbero mascherare variabili con
 *  lo stesso nome dichiarate all'esterno del blocco.
 *
 *  Una volta memorizzata la simbol table si cicla sulle definizioni presenti nel blocco, e si genera il nuovo codice
 *  Si genera prima il codice per le definizioni, poi il codice per gli staments veri e propri.
 *
 *  Risalta l'utilizzo della simbol table -la tabella associativa, che viene temporaneamente svuotata e modificata.
 */
BlockAST::BlockAST(std::vector<InitAST *> Def, std::vector<StmtAST *> Stmts) : Def(std::move(Def)), Stmts(std::move(Stmts)) {};
BlockAST::BlockAST(std::vector<StmtAST *> Stmts) : Stmts(std::move(Stmts)) {};

Value *BlockAST::codegen(driver &drv)
{
  std::vector<AllocaInst *> tmp;
  for (int i = 0; i < Def.size(); i++)
  {
    AllocaInst *boundval = (AllocaInst *)Def[i]->codegen(drv);
    if (!boundval)
      return nullptr;
    tmp.push_back(drv.NamedValues[Def[i]->getName()]);
    drv.NamedValues[Def[i]->getName()] = boundval;
  }
  Value *blockvalue;
  for (int i = 0; i < Stmts.size(); i++)
  {
    blockvalue = Stmts[i]->codegen(drv);
    if (!blockvalue)
      return nullptr;
  }
  for (int i = 0; i < Def.size(); i++)
    drv.NamedValues[Def[i]->getName()] = tmp[i];
  return blockvalue;
};

std::string &InitAST::getName() { return Name; };
initType InitAST::getType() { return INIT; };

/** VarBindingsAST
 *  la classe gestisce la dichiarazione/assegnamento di una variabile
 *   si valuta l'espressione di inizializzazione
 *   si crea l'allocazione di memoria per la variabile
 *   si memorizza l'istruzione di assegnazione
 *
 *
 *  per prima cosa, si capisce lo "scope" o almeno l'ambiente di scrittura corrente,
 *  mediante la direttiva < Function *fun = builder->GetInsertBlock()->getParent(); >
 *
 *  la classe, in caso il registor del valore non sia esplitato, gli assegna zero.
 */
VarBindingsAST::VarBindingsAST(std::string Name, ExprAST *Val) : Name(Name), Val(Val) {};
std::string &VarBindingsAST::getName() { return Name; };
initType VarBindingsAST::getType() { return BINDING; };

AllocaInst *VarBindingsAST::codegen(driver &drv)
{
  Function *fun = builder->GetInsertBlock()->getParent();
  Value *boundval;
  if (Val)
    boundval = Val->codegen(drv);
  else
  {
    NumberExprAST *defaultVal = new NumberExprAST(0.0);
    boundval = defaultVal->codegen(drv);
  }
  AllocaInst *Alloca = CreateEntryBlockAlloca(fun, Name);
  builder->CreateStore(boundval, Alloca);
  return Alloca;
};

/** AssignmentExprAST
 *  Le righe sotto costruiscono a tutti gli effetti un'istanza della nuova classe AssignmentExprAST
 *  i parametri sono Name e Val: rappresentano rispettivamente il nome della variabile e il valore da assegnare
 *  
 *  vengono poi definiti nome e tipo di assegnamento
 *  in modo simile a quanto fatto per distinguere variabili locali o globali, si cerca la variabile nella simbol table
 *  se la variabile è presente (nome presente in tabella), signfica che si lavora con una variabile locale, 
 *  viceversa si lavora con una variabile globale
 * 
 *  la dichiarazione di una nuova variabile viene fatta in modo uguale, ma con "contesto" diverso
 */
AssignmentExprAST::AssignmentExprAST(std::string Name, ExprAST *Val) : Name(Name), Val(Val) {};
std::string &AssignmentExprAST::getName() { return Name; };
initType AssignmentExprAST::getType() { return ASSIGNMENT; };
Value *AssignmentExprAST::codegen(driver &drv)
{
  AllocaInst *Variable = drv.NamedValues[Name];
  Value *boundval = Val->codegen(drv);
  
  if (!boundval)
    return nullptr;
  
  if (!Variable)
  {
    GlobalVariable *globVar = module->getNamedGlobal(Name);
    if (!globVar)
      return nullptr;

    builder->CreateStore(boundval, globVar);
    return boundval;
  }

  builder->CreateStore(boundval, Variable);
  return boundval;
};

/** GlobalVariableAST 
 * la classe implementa la dichiarazione di una variabile globale; 
 * sfrutta interamente la firma llvm GlobalVariable
 */
GlobalVariableAST::GlobalVariableAST(std::string Name, double Size) : Name(Name), Size(Size) {}
std::string &GlobalVariableAST::getName() { return Name; };
Value *GlobalVariableAST::codegen(driver &drv)
{
  GlobalVariable *globVar;
  globVar = new GlobalVariable(*module, Type::getDoubleTy(*context), false, GlobalValue::CommonLinkage, ConstantFP::getNullValue(Type::getDoubleTy(*context)), Name);
  
  globVar->print(errs());
  fprintf(stderr, "\n");
  return globVar;
}

/** IF BLOCK for statements
 *  si inizializzano i membri cond, trueexp e falseexp con i valori passati come parametri.
 *  questi tre membri sono sempre ExprAST.
 *
 *  * ____ PREPARAZIONE ____
 *  la generazione del codice intermedio per la condizione.
 *
 *  * ____ COSTRUZIONE ____
 *  vengono creati tre blocchi di base:
 *    1. TrueBB: per la parte del codice eseguita se la c. è vera
 *    2. FalseBB: per la parte del codice eseguita se la c. è falsa
 *    3. MergeBB: dove si uniscono i flussi
 *
 *  * FASE 1
 *  viene poi settato il builder per scrivere sul blocco true,
 *  e si scrive il blocco di codice da eseguire quando la condizione è rilevata vera
 *  una volta inserito il blocco true, si prepara l'albero il blocco per il merge.
 *  si crea pertanto l'istruzione di salto al blocco i merge, e si posizionano correttamente i blocchi base dentro fun.
 *
 *  * FASR 2
 *  si setta il builder per scrivere sul blocco falso,
 *  e si scrive il blocco di codice da eseguire quando la condizione è rilevata falsa
 *  una volta inserito il blocco false, si prepara l'albero il blocco per il merge.
 *  si crea pertanto l'istruzione di salto al blocco i merge, e si posizionano correttamente i blocchi base dentro fun.
 *
 *  * FASE 3
 *  dopo aver generato il codice per i rami true e false,
 *  il builder deve sapere dove inserire le istruzioni successive; si imposta quindi MergeBB come punto di partenza.
 *
 *  una volta definita la funzione PHI di convergenza, si aggiungono semplicemente i due punti di ingresso.
 *  Type::getDoubleTy

 */
IfStmtAST::IfStmtAST(ExprAST *cond, StmtAST *trueblock, StmtAST *falseblock) : cond(cond), trueblock(trueblock), falseblock(falseblock) {};

IfStmtAST::IfStmtAST(ExprAST *cond, StmtAST *trueblock) : cond(cond), trueblock(trueblock) {};

Value *IfStmtAST::codegen(driver &drv)
{
  Value *CondV = cond->codegen(drv);
  if (!CondV)
    return nullptr;

  Function *fun = builder->GetInsertBlock()->getParent();
  BasicBlock *TrueBB = BasicBlock::Create(*context, "trueblock", fun);
  BasicBlock *FalseBB = BasicBlock::Create(*context, "falseblock");
  BasicBlock *MergeBB = BasicBlock::Create(*context, "mergeblock");
  builder->CreateCondBr(CondV, TrueBB, FalseBB);

  //* FASE 1 - blocco vero
  builder->SetInsertPoint(TrueBB);
  Value *trueV = trueblock->codegen(drv);
  if (!trueV)
    return nullptr;
  TrueBB = builder->GetInsertBlock();
  builder->CreateBr(MergeBB);

  //* FASE 2 - blocco falso
  builder->SetInsertPoint(FalseBB);
  Value *falseV;
  fun->insert(fun->end(), FalseBB);
  builder->SetInsertPoint(FalseBB);
  if (falseblock)
  {
    falseV = falseblock->codegen(drv);
    if (!falseV)
      return nullptr;
    FalseBB = builder->GetInsertBlock();
  }
  builder->CreateBr(MergeBB);
  fun->insert(fun->end(), MergeBB);
  builder->SetInsertPoint(MergeBB);

  //* FASE 3 - convergenza
  PHINode *P = builder->CreatePHI(Type::getDoubleTy(*context), 2);
  P->addIncoming(ConstantFP::getNullValue(Type::getDoubleTy(*context)), TrueBB);
  P->addIncoming(ConstantFP::getNullValue(Type::getDoubleTy(*context)), FalseBB);
  return P;
};


/**  FOR in AST
 *  la prima cosa che si implementa è il costruttore con le tre informazioni tipiche di un cilo for: 
 *  init, cond, step, e body.
 * 
 *  si prepara poi il blocco di inizializzazione e di condizione, e si descrive un'operazione di salto non condizionato:
 *  dopo aver eseguito l'inizializzazione, il flusso salta sempre a CondBB per valutare la condizione del ciclo.
 *  
 *  * FASE 0 - preparativi 
 *  si configurano tutti i bb e l'istruzione di salto iniziale
 * 
 *  * FASE 1: inizializzazione ciclo
 *  si dichiara/riassegna la variabile di inizializzazione
 *  in caso la variabile esista già si fa un salvataggio di backup
 *  (che poi viene ripristinato)
 * 
 *  * FASE 2: scrittura del corpo principale
 *  si scrive il flusso principale body->codegen(drv);
 *  e si impostano i salti: 
 *  . condizionati nel caso di valutazione condizione di ripetizione
 *  . non condizionati nel caso di ritorno alla condizione per verificare la possibilità di nuova iterazione
 *  
 *  * FASE 3: uscita da flusso for [si torna a casa]
 *  viene impostato il punto di inserimento al termine del ciclo for, 
 *  viene creato un nodo PHI per gestire i valori che provengono dai blocchi precedenti, 
 *  viene ripristinata la symbol table per ripristinare il valore precedente della variabile iniziale ev.
 */
ForStmtAST::ForStmtAST(InitAST *init, ExprAST *cond, AssignmentExprAST *step, StmtAST *body) : init(init), cond(cond), step(step), body(body) {};
Value *ForStmtAST::codegen(driver &drv)
{
  // * FASE 0 - preparativi 
  Function *fun = builder->GetInsertBlock()->getParent();

  BasicBlock *InitBB = BasicBlock::Create(*context, "init", fun);
  builder->CreateBr(InitBB);
  
  BasicBlock *CondBB = BasicBlock::Create(*context, "cond", fun);
  BasicBlock *LoopBB = BasicBlock::Create(*context, "loop", fun);
  BasicBlock *EndLoop = BasicBlock::Create(*context, "endloop", fun);

  builder->SetInsertPoint(InitBB);

  // * FASE 1 - inizializzazione ciclo
  std::string varName = init->getName();
  AllocaInst *oldVar;
  Value *initVal = init->codegen(drv);
  ;
  if (!initVal)
    return nullptr;
  if (init->getType() == BINDING)
  {
    oldVar = drv.NamedValues[varName];
    drv.NamedValues[varName] = (AllocaInst *)initVal;
  }
  builder->CreateBr(CondBB);

  // * FASE 2 - scrittura del corpo principale
  builder->SetInsertPoint(CondBB);
  Value *condVal = cond->codegen(drv);
  if (!condVal)
    return nullptr;
  
  // si salta alla condizione (obv salto condizionato)
  builder->CreateCondBr(condVal, LoopBB, EndLoop);

  // si scrive veramente il body
  builder->SetInsertPoint(LoopBB);
  Value *bodyVal = body->codegen(drv);
  if (!bodyVal)
    return nullptr;

  // scrittura codice per avanzare -> nextIterat
  Value *stepVal = step->codegen(drv);
  if (!stepVal)
    return nullptr;

  // si torna indietro alla condizione (anche questa volta, non cond.)
  builder->CreateBr(CondBB);

  // * FASE 3 - uscita da flusso for [si torna a casa]
  builder->SetInsertPoint(EndLoop);
  PHINode *P = builder->CreatePHI(Type::getDoubleTy(*context), 1);
  P->addIncoming(ConstantFP::getNullValue(Type::getDoubleTy(*context)), CondBB);

  if (init->getType() == BINDING)
  {
    drv.NamedValues[varName] = oldVar;
  }

  return P;
};

/** Prototype, Function in AST
 *   
 *  nelle prima righe si descrive il costruttore, che inizializza il nome della funzione (Name), 
 *  gli argomenti della funzione (Args), e imposta emitcode a true 
 *  per indicare che di default il codice della funzione verrà emesso
 *  più sotto si definiscono altri metodi accessori. 
 *  
 *  si gestisce la scrittura del prototipo di una funzione. 
 *  si organizza una struttura così composta: 
 *    1. il "tipo" di funzione FunctionType *FT = FunctionType::get(Type::getDoubleTy(*context), Doubles, false);
 *    2. vettore (qui chiamato Doubles) con tipo argomenti std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*context));
 *    3. vettore con i parametri
 *  
 *  il cuore della classe risiede probabilmente nella direttiva Function *F = Function::Create(FT, Function::ExternalLinkage, Name, *module);
 *  che crea un oggetto Function (F) con il tipo FT, con visibilità esterna (ExternalLinkage), e il nome specificato. La funzione viene inserita nel modulo module.
 *  l'emissione del codice della funzione viene fatto solo quando si ha una definizione completa, quindi di prototipo+body, viceversa si scrive doppiamente
 *
 *  Il meccanismo viene spiegato approfonditamente nel readme esterno dedicato. 
 */
PrototypeAST::PrototypeAST(std::string Name, std::vector<std::string> Args) : Name(Name), Args(std::move(Args)), emitcode(true) {}; // Di regola il codice viene emesso

lexval PrototypeAST::getLexVal() const
{
  lexval lval = Name;
  return lval;
};

const std::vector<std::string> &PrototypeAST::getArgs() const
{
  return Args;
};

void PrototypeAST::noemit()
{
  emitcode = false;
};

Function *PrototypeAST::codegen(driver &drv)
{
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*context));
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(*context), Doubles, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, *module);

  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  if (emitcode)
  {
    F->print(errs());
    fprintf(stderr, "\n");
  };

  return F;
}


FunctionAST::FunctionAST(PrototypeAST *Proto, ExprAST *Body) : Proto(Proto), Body(Body) {};

Function *FunctionAST::codegen(driver &drv)
{
  Function *function = module->getFunction(std::get<std::string>(Proto->getLexVal()));

  if (!function)
    function = Proto->codegen(drv);
  else
    return nullptr;

  if (!function)
    return nullptr;

  BasicBlock *BB = BasicBlock::Create(*context, "entry", function);
  builder->SetInsertPoint(BB);

  for (auto &Arg : function->args())
  {
    AllocaInst *Alloca = CreateEntryBlockAlloca(function, Arg.getName());
    builder->CreateStore(&Arg, Alloca);
    drv.NamedValues[std::string(Arg.getName())] = Alloca;
  }

  if (Value *RetVal = Body->codegen(drv))
  {
    builder->CreateRet(RetVal);

    verifyFunction(*function);

    function->print(errs());
    fprintf(stderr, "\n");
    return function;
  }

  function->eraseFromParent();
  return nullptr;
};
