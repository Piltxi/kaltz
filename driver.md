# introduzione a <i>Driver</i>

Nel modulo <a href="driver.cpp">driver.cpp</a> è implementata la classe centrale che gestisce il processo di compilazione. 

```cpp
class driver
{
public:
  driver();
  std::map<std::string, AllocaInst *> NamedValues; 
  int parse(const std::string &f);
  std::string file;
  bool trace_parsing;  
  void scan_begin();
  void scan_end();
  bool trace_scanning;
  yy::location location;
  void codegen();
};
```

In particolare: 
- i metodi `scan_begin()` e `scan_end()` si riferiscono a quanto implementato nello <a href="scanner.ll">scanner</a>
- `NamedValues` indica la tabella in cui si associano nomi di variabili (rappresentati da stringhe) a istruzioni di allocazione (AllocaInst*) che rappresentano la memoria allocata per queste variabili. `NamedValues` è utile in più casi, poiché consente di tenere traccia delle variabili realmente allocate; ad esempio sotto. 
    ```cpp
    Value *VariableExprAST::codegen(driver &drv) {
        AllocaInst *A = drv.NamedValues[Name];
        if (!A) {
            GlobalVariable* globVar = module->getNamedGlobal(Name);
            if (!globVar)
                return LogErrorV("Variabile non definita: " + Name);
            return builder->CreateLoad(globVar->getValueType(), globVar, Name.c_str());
        }
        return builder->CreateLoad(A->getAllocatedType(), A, Name.c_str());
    }
    ```
    Quando viene generato il codice per una variabile `VariableExprAST::codegen`, il driver cerca la variabile nella map `NamedValues`.
    Dunque, se la variabile non è trovata, viene cercata come variabile globale, viceversa, viene costruita un'istruzione di `load` per caricare il valore della variabile dalla memoria.

L'obiettivo del modulo <a href="driver.cpp">driver.cpp</a>, ad ogni modo è quello di costruire l'albero sintattico di un programma sorgente. 

# cose interessanti nel <i>Driver</i>

### Collegamento con il Parser Bison

la definizione di una funzione e di un prototipo è gestita tramite le regole della grammatica. 
Sotto, l'estratto della grammatica Bison che gestisce le definizioni di funzione e i prototipi:

```
definition:
  "def" proto block { $$ = new FunctionAST($2, $3); $2->noemit(); };

external:
  "extern" proto { $$ = $2; };

proto:
  "id" "(" idseq ")" { $$ = new PrototypeAST($1, $3); };
```

> "def" proto block { $$ = new FunctionAST($2, $3); $2->noemit(); };
  - `"def"`: Parola chiave che indica l'inizio di una definizione di funzione.
  - `proto`: Questa è la regola che produce un prototipo di funzione (`PrototypeAST`).
  - `block`: Questa è la regola che produce il corpo della funzione (`BlockAST`).
  - `{ $$ = new FunctionAST($2, $3); $2->noemit(); }`: Questa azione crea un nuovo nodo AST per la funzione (`FunctionAST`) utilizzando il prototipo (`$2`) e il corpo della funzione (`$3`). Chiama anche il metodo `noemit` sul prototipo per prevenire la doppia emissione del codice.

> "extern" proto { $$ = $2; };
  
  - `"extern"`: Parola chiave che indica l'inizio di una dichiarazione di funzione esterna.
  - `proto`: Questa è la regola che produce un prototipo di funzione (`PrototypeAST`).
  - `{ $$ = $2; }`: Questa azione restituisce direttamente il prototipo prodotto dalla regola `proto`.


> "id" "(" idseq ")" { $$ = new PrototypeAST($1, $3); };
  - `"id"`: Identificatore che rappresenta il nome della funzione.
  - `"(" idseq ")"`: Sequenza di argomenti della funzione.
  - `{ $$ = new PrototypeAST($1, $3); }`: Questa azione crea un nuovo nodo AST per il prototipo (`PrototypeAST`) utilizzando il nome della funzione (`$1`) e la sequenza di argomenti (`$3`).

### controllo delle emissioni

quando viene definita una funzione, il codice del prototipo non deve essere emesso due volte: una volta come dichiarazione `extern` e una volta come parte della definizione della funzione. Questo è gestito nel metodo `codegen` della classe `PrototypeAST`.

#### Implementazione della Prevenzione

Ecco una spiegazione passo-passo di come si previene la doppia emissione:

1. **Definizione della Funzione nel Parser**:
   Quando il parser riconosce una definizione di funzione, viene creato un oggetto `FunctionAST` utilizzando il prototipo e il corpo della funzione.
   ```bison
   definition:
     "def" proto block { $$ = new FunctionAST($2, $3); $2->noemit(); };
   ```

2. **Chiamata al Metodo `noemit`**:
   Dopo aver creato l'oggetto `FunctionAST`, il parser chiama il metodo `noemit` sul prototipo (`$2`).
   ```cpp
   $2->noemit();
   ```
   Questo imposta il flag `emitcode` a `false` nel prototipo, indicando che il codice del prototipo non deve essere emesso nuovamente.

3. **Generazione del Codice del Prototipo**:
   Nel metodo `codegen` della classe `PrototypeAST`, il codice del prototipo viene emesso solo se `emitcode` è `true`.
   ```cpp
   if (emitcode)
   {
     F->print(errs());
     fprintf(stderr, "\n");
   }
   ```
   Quando `noemit` viene chiamato, `emitcode` è impostato a `false`, quindi questa parte del codice viene saltata, prevenendo la doppia emissione.

Supponiamo di avere il seguente input:

```
def foo(x, y) {
  x + y;
}
```

Il parser riconosce la definizione di funzione e applica la regola `definition`: in questo modo si crea un `PrototypeAST` per `foo` con argomenti `x` e `y`, e un `BlockAST` per il corpo della funzione.

1. Il parser chiama `PrototypeAST::codegen` per generare il codice del prototipo.
2. Il parser chiama `noemit` sul prototipo, impostando `emitcode` a `false`.
3. Il parser chiama `FunctionAST::codegen` per generare il codice della funzione.
4. `FunctionAST::codegen` chiama nuovamente `PrototypeAST::codegen`, ma questa volta `emitcode` è `false`, quindi il codice del prototipo non viene emesso di nuovo.

la prevenzione della doppia emissione è gestita impostando un flag (`emitcode`) e controllandolo nel metodo `codegen` del prototipo. Il flag viene impostato a `false` quando il prototipo è parte di una definizione di funzione completa, disattivando la duplicazione del codice.

#### `PrototypeAST::codegen(driver &drv)`

Questo metodo genera il codice per il prototipo di una funzione.

```cpp
Function *PrototypeAST::codegen(driver &drv)
{
  // Costruisce un vettore di tipi per gli argomenti della funzione. Nel nostro caso, tutti i tipi sono 'double'.
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*context));

  // Crea il tipo della funzione (ritorna un 'double' e accetta un vettore di 'double').
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(*context), Doubles, false);

  // Crea la funzione con il tipo definito, il nome del prototipo e la linkage type ExternalLinkage.
  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, *module);

  // Assegna i nomi ai parametri della funzione come specificato nel prototipo.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  // Se 'emitcode' è true, emette il codice per il prototipo della funzione.
  if (emitcode)
  {
    F->print(errs());
    fprintf(stderr, "\n");
  }

  return F;
}
```
1. Tipi Argomenti:
   - Viene creato un vettore `Doubles` che contiene il tipo `double` per ogni argomento della funzione.

2. Tipo della Funzione:
   - `FunctionType::get` viene utilizzato per creare un tipo di funzione che restituisce un `double` e accetta un vettore di `double`.

3. Corpo della Funzione:
   - `Function::Create` crea una nuova funzione con il tipo definito (`FT`), linkage type `ExternalLinkage` (che indica che la funzione può essere chiamata da altri moduli) e il nome del prototipo (`Name`).

4. Nomi Argomenti:
   - Viene assegnato un nome a ciascun argomento della funzione come specificato nel prototipo.

5. Emissione eventuale:
   - Se `emitcode` è `true`, il codice del prototipo viene emesso.

#### `FunctionAST::codegen(driver &drv)`

Questo metodo genera il codice per la definizione di una funzione, utilizzando il prototipo e il corpo della funzione.

```cpp
FunctionAST::FunctionAST(PrototypeAST *Proto, ExprAST *Body) : Proto(Proto), Body(Body) {}

Function *FunctionAST::codegen(driver &drv)
{
  // Recupera la funzione dal modulo utilizzando il nome dal prototipo.
  Function *function = module->getFunction(std::get<std::string>(Proto->getLexVal()));

  // Se la funzione non esiste, genera il codice del prototipo.
  if (!function)
    function = Proto->codegen(drv);
  else
    return nullptr;

  if (!function)
    return nullptr;

  // Crea un blocco di base 'entry' e imposta il builder per inserirvi istruzioni.
  BasicBlock *BB = BasicBlock::Create(*context, "entry", function);
  builder->SetInsertPoint(BB);

  // Alloca spazio per ogni argomento della funzione nel blocco di base e memorizza i valori degli argomenti in queste allocazioni.
  for (auto &Arg : function->args())
  {
    AllocaInst *Alloca = CreateEntryBlockAlloca(function, Arg.getName());
    builder->CreateStore(&Arg, Alloca);
    drv.NamedValues[std::string(Arg.getName())] = Alloca;
  }

  // Genera il codice per il corpo della funzione e crea l'istruzione di ritorno.
  if (Value *RetVal = Body->codegen(drv))
  {
    builder->CreateRet(RetVal);

    // Verifica la correttezza della funzione.
    verifyFunction(*function);

    // Emissione del codice della funzione.
    function->print(errs());
    fprintf(stderr, "\n");
    return function;
  }

  // Se la generazione del codice fallisce, rimuove la funzione.
  function->eraseFromParent();
  return nullptr;
}
```