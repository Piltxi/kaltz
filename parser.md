# introduzione a <i>Bison</i>

<a href="https://www.gnu.org/software/bison/manual/html_node/Calc_002b_002b-Parser.html"> per il manuale </a>

<i>Bison</i> è una versione moderna di Yacc, uno strumento per generare parser LR in C/C++. 

<i>Bison</i> elabora su un flusso di token -tipicamente ricevuto da <i>flex</i>. 
Il sorgente <i>flex</i> contiene le definizioni dei token da riconoscere, fornita mediante espressioni regolari, e alcune frammenti di codice da eseguire nel momento in cui i vari token vengono rilevati. <i>flex</i> lanciato sul file produce in output codice C/C++ compilabile. 
Per altre informazioni su <i>flex</i> e sullo scanner, premi qui. 

Il funzionamento di <i>Bison</i> è simile al funzionamento di <i>flex</i>; mediante la scrittura di un file  si specificano i nomi dei token e i simboli non terminali della grammatica: 
1. si specifica l’assioma
2. si specificano le produzioni
3. si descrivono frammenti di codice da eseguire quando si esegue una riduzione

Quando l’applicazione prevede scanning e parsing, occorre definire tre meccanismi fondamentali: 
1. i token e la corrispondente condivisione tra <i>flex</i> e <i>Bison</i>
2. il flusso dei token da <i>flex</i> a <i>Bison</i>
3. le informazioni da mostrare in caso di errore.
 
All’interno del <i>parser</i> vengono forniti i nomi dei token, e il corrispondente semantic / lexical value che ne identifica il corrispondente oggetto C/C++. Le definizioni dei token presenti nel parser vengono scritte da <i>Bison</i> in un header generato durante il processo di compilazione. 

Dunque, il file di input per <i>Bison</i> -`parser.y` o `parser.yy`- deve contenere: 
- i nomi dei token e i simboli non terminali della grammatica, indicando quale sia l’assioma
- le produzioni della grammatica
- eventuale codice da eseguire quando si effettua una riduzione

## parser.yy
Di seguito si entra nel merito del file `parser.yy` rilasciato con questo progetto. 

---
Nelle prime righe si specificano alcune direttive utili alla generazioni del sorgente C/C++ del programma di parsing. 

```
%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines

%define api.token.constructor
%define api.location.file none
%define api.value.type variant
%define parse.assert
```

- `%skeleton "lalr1.cc"` specifica lo scheletro che <i>Bison</i>  userà per costruire il parser. 
<br> lalr1.cc è uno dei scheletri forniti da <i>Bison</i>  che implementa un parser LALR(1) in C++. La direttiva `%require "3.2"` definisce la versione di <i>Bison</i> da utilizzare, mentre `%defines` attiva la generazione di un header file. 

- `%define api.token.constructor` attiva la descrizione di una funzione del tipo `make_{some}` utile allo <i>scanner</i> per la generazione di un oggetto <i>complete_symbol</i>, ossia un token con tutte le informazioni richieste per il processo di parsing, inclusi tipo, valore semantico e posizione. Questo approccio facilita la comunicazione tra lo scanner e il parser, permettendo allo scanner di passare token al parser che sono già completamente formati e pronti per essere processati.

- `%define api.location.file none` disattiva la funzionalità di localizzazione automatica di <i>Bison</i>, senza disattivare tuttavia ciò che è definito nello <i>scanner</i>. Mediante la direttiva <i>none</i> <i>Bison</i> non manterrà traccia del nome del file da cui ogni token proviene.

Per il tracciamento degli errori, <i>Bison</i> mette a disposizione la classe location che permette di localizzare i token. Ogni localizzazione consiste di due infomazioni, `begin` e `end` a cui corrispondono ciascuna un indice di riga e di colonna: 
    - `begin` la posizione di inizio del token nel testo sorgente
    - `end` la posizione di fine del token nel testo sorgente

Il tracciamento degli errori viene attivato mediante la direttiva `%locations` alcune righe più sotto. 

---
```
%code requires {
  # include <string>
  # include <exception>
  class driver;
  class RootAST;
  class ExprAST;
  class NumberExprAST;
  class VariableExprAST;
  class CallExprAST;
  class FunctionAST;
  class SeqAST;
  class PrototypeAST;
  class BlockAST;
  class VarBindingsAST;
  class GlobalVariableAST;
  class AssignmentExprAST;
  class StmtAST; 
  class IfStmtAST;
  class InitAST;
  class ForStmtAST;
}
```

La sezione `%code requires {}` definisce i tipi e le strutture di dati necessari che devono essere conosciuti dal codice generato da <i>Bison</i> prima della compilazione: ciò assicura che tutte le dipendenze siano risolte e che il codice compili senza difficoltà.

All'interno del programma di parsing generato da <i>Bison</i> è necessario definire alcune dichiarazioni anticipate: queste informano dell'esistenza di classi AST che saranno usate dal parser; la dichiarazione anticipata permette al codice generato da <i>Bison</i> di compilare correttamente anche se le definizioni complete delle classi non sono ancora visibili nel file <i>Bison</i> stesso.

---

```
// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
# include "driver.hpp"
}
```

- `%param { driver& drv }` attiva il passaggio per riferimento dell'istanza del driver allo scanner e al parser, senza utilizzare variabili globali. A questo scopo, viene incluso `driver.hpp`. 
Utilizzando questa direttiva, il prototipo dell'handle al paser dallo scanner diviene simile al successivo: `int yyparse(driver& drv);`
N.B. all'interno dell'implementazione <a href="driver.cpp">driver.cpp</a> il punto di chiamata del parsing è definito nel metodo `.parse()`

- `%locations` attiva il monitoraggio delle posizioni come descritto sopra. 
Le ulteriori direttive di <i>parse.trace</i> e <i>parse.error</i> sono utili ai fini di debug. 

---

In questa sezione sono definiti i simboli terminali e non terminali della grammatica del parser. 

```
%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  SEMICOLON  ";"
  COMMA      ","
  DMINUS     "--"
  MINUS      "-"
  DPLUS      "++"
  PLUS       "+"
  STAR       "*"
  SLASH      "/"
  LPAREN     "("
  RPAREN     ")"
  QMARK      "?"
  COLON      ":"
  LT         "<"
  GT         ">"
  EQ         "=="
  ASSIGN     "="
  LSQRBR     "["
  RSQRBR     "]"
  LBRACE     "{"
  RBRACE     "}"
  AND        "and"
  OR         "or"
  NOT        "not"
  EXTERN     "extern"
  DEF        "def"
  VAR        "var"
  GLOBAL     "global"
  IF         "if"
  ELSE       "else"
  FOR        "for"
;

%token <std::string> IDENTIFIER "id"
%token <double> NUMBER "number"
%type <ExprAST*> exp
%type <ExprAST*> idexp
%type <ExprAST*> expif 
%type <ExprAST*> relexp
%type <ExprAST*> initexp
%type <ExprAST*> condexp
%type <std::vector<ExprAST*>> optexp
%type <std::vector<ExprAST*>> explist
%type <RootAST*> program
%type <RootAST*> top
%type <FunctionAST*> definition
%type <PrototypeAST*> external
%type <PrototypeAST*> proto
%type <std::vector<std::string>> idseq
%type <BlockAST*> block
%type <std::vector<InitAST*>> vardefs;
%type <std::vector<StmtAST*>> stmts;
%type <StmtAST*> stmt;
%type <IfStmtAST*> ifstmt;
%type <InitAST*> binding;
%type <GlobalVariableAST*> globalvar;
%type <AssignmentExprAST*> assignment;
%type <InitAST*> init;
%type <ForStmtAST*> forstmt;
%%
%start startsymb;
```
La direttiva `%define api.token.prefix {TOK_}` configura il parser affinché ogni simbolo token generato abbia un prefisso "TOK_"; ciò è utile per evitare conflitti di nomi tra i token e altri identificatori. 

I simboli sopra sono terminali presenti nella grammatica, a cui corrispondono dei valori tra doppi apici. I simboli sono considerat terminali poiché coincidono con dei simboli che non possono essere scissi o formati mediante seconde e terze derivazioni, perché non compaiono a sinistra in nessuna produzione. 

Sono casi particolari i seguenti: 
> %token <std::string> IDENTIFIER "id" 

IDENTIFIER è associato a std::string <br> il lexer fornirà una stringa per ogni identificatore riconosciuto. 
> %token <double> NUMBER "number"

NUMBER è associato a double <br> indicando che il lexer fornirà un valore numerico per ogni numero riconosciuto.

Le direttive `%type` associano i simboli non terminali ad un corrispettivo tipo di dato da elaborare. All'interno del progetto del parser, la maggior parte dei nomi dei tipi corrispondono alle classi definite nel file <a href="driver.cpp">driver.cpp</a> in modo che i simboli non terminali vengano associati alla costruzione dei diversi rami dell'albero sintattico. 

---

> %start startsymb;

```
%start startsymb;

startsymb:
program                 { drv.root = $1; }

program:
  %empty                { $$ = new SeqAST(nullptr,nullptr); }
|  top ";" program      { $$ = new SeqAST($1,$3); };

top:
%empty                  { $$ = nullptr; }
| definition            { $$ = $1; }
| external              { $$ = $1; }
| globalvar   
```

La direttiva <i>%start</i> stabilisce <i>startsymb</i> come il simbolo di partenza per il parsing. Il parsing inizierà esaminando il testo di input e applicando le regole associate a questo simbolo.

Quando il parser riconosce il non-terminale program, esegue l'azione tra graffe: assegna il valore del primo simbolo del lato destro ($1, che rappresenta il valore semantico di program) alla variabile root dell'oggetto `drv`. 

- `%empty { $$ = new SeqAST(nullptr,nullptr); }`
Questa regola permette che program sia vuoto (non contiene elementi). Se ciò avviene, viene creato un nuovo nodo SeqAST con entrambi i parametri settati a nullptr, indicando che non ci sono sottoparti o continuazioni. $$ è la variabile che rappresenta il valore semantico di program.
- `top ";" program { $$ = new SeqAST($1,$3); }`
Se program è composto da un top seguito da un punto e virgola e da un altro program, questa regola costruisce un nuovo SeqAST. Il nuovo SeqAST prende due parametri: il valore di top ($1) e il valore del program seguente ($3).

Arrivati alla sezione del parser, si osservano tutte le regole di produzione per i simboli definiti nella sezione precedente. Ogni regola di produzione consiste di: 

1. una testa di produzione: il simbolo a sinistra - simbolo <i>non</i> terminale.
2. un corpo di produzione: una sequenza di simboli generici α terminali e non terminali presenti nelle regole di produzione

Ogni regola di produzione contribuisce a costruire l'AST rappresentando strutture sintattiche specifiche, dichiarate mediante le classi in driver.cpp. 

---

All'interno della sezione delle produzioni, risaltano alcune righe particolari.

```
%left ":" "?";
%left "<" ">" "==";
%left "+" "-";
%left "not";
%left "and" "or";
%left "*" "/";
```

Queste righe definiscono le precedenze e l'associatività degli operatori nel linguaggio: sono utili al parser per risolvere le ambiguità che potrebbero sorgere nell'analisi delle espressioni che coinvolgono operatori.

La direttiva `%left` specifica che gli operatori elencati hanno associatività sinistra. 
L'associatività sinistra significa che, in caso di operatori con la stessa precedenza, l'espressione viene valutata da sinistra verso destra. 

Senza queste definizioni, non si può garantire che il parser sia in grado di valutare correttamente tutte le possibili espressioni presenti in una istruzione. 

L'ordine di scrittura delle regole di associatività è importante, poiché <i>Bison</i> considera di una prorità più alta regole più in basso -successive. 

> %right "then" "else" ;

Con questa direttiva, che sfrutta associatività a destra, si definisce la precedenza e l'associtatività degli operatori `then` e `else`; in questo caso, la direttiva suggerisce che in caso di operatori consecutivi dello stesso tipo, l'analisi avviene da destra verso sinistra. Questo meccanismo è utile nelle espressioni condizionali per associare correttamente un `else` con il suo `if` più vicino in costrutti nidificati.

```
void
yy::parser::error (const location_type& l, const std::string& m)
{
  std::cerr << l << ": " << m << '\n';
}
```

Questa procedura implementa una semplice gestione degli errori, visualizzando la location di eventuali errori e il messaggio associato. 

--- 

Potenzialmente interessante è anche la gestione di istruzioni "multiple".
All'interno del parser vengono implementate queste righe di codice: 

```
stmts:
  stmt                  {std::vector<StmtAST*> statemets; statemets.insert(statemets.begin(),$1); $$ = statemets; }
| stmt ";" stmts        {$3.insert($3.begin(),$1); $$ = $3;};
```
Nelle righe successive si illustra una spiegazione: 
> stmt { std::vector<StmtAST*> statemets; statemets.insert(statemets.begin(), $1); $$ = statemets; }

1. quando il parser riconosce un oggetto `stmt` istanzia un vettore per contenere tanti oggetti di tipo `StmtAST*`. 
2. inserisce poi l'oggetto `StmtAST*` (rappresentato da $1, il valore semantico del simbolo non terminale `stmt`) all'inizio del vettore. 

La seconda produzione 
> stmt ";" stmts        {$3.insert($3.begin(),$1); $$ = $3;};

fa riferimento alla parsing di più istruzioni in fila: 
inserisce il StmtAST* all'inizio del vettore esistente, e assegna il vettore modificato (che ora include la nuova istruzione all'inizio) al valore semantico della produzione.

