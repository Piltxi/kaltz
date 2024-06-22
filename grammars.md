# First Level

```
%start startsymb;

globalvar: 
    "global" "id";

startsymb:
    program;

idseq:
    %empty
    | "id" idseq;

program:
    %empty
    | top ";" program;

top:
    %empty
    | definition
    | external
    | globalvar
    | stmt ";" stmts;

definition:
    "def" proto block;

external:
    "extern" proto;

proto:
    "id" "(" idseq ")";

block:
    "{" stmts "}"
    | "{" vardefs ";" stmts "}";

vardefs:
    binding
    | vardefs ";" binding;

binding:
    "var" "id" initexp;

initexp:
    %empty
    | "=" exp;

stmt:
    assignment;

assignment:
    "id" "=" exp;

exp:
    exp "+" exp
    | exp "-" exp
    | exp "*" exp
    | exp "/" exp
    | idexp
    | "(" exp ")"
    | "number"
    | expif;

expif:
    condexp "?" exp ":" exp;

condexp:
    exp "<" exp
    | exp "==" exp;

idexp:
    "id"
    | "id" "(" optexp ")";

optexp:
    %empty
    | explist;

explist:
    exp
    | exp "," explist;
```
matches in the classes implemented in <a href="driver.cpp">driver.cpp</a>:
- RootAST: Represents the main syntax tree, the root of all ASTs.
- SeqAST: Manages sequences of statements or expressions.
- GlobalVariableAST: Manages global variable declarations.
- PrototypeAST: Represents function signatures (used in external and definition).
- FunctionAST: Represents a defined function.
- ExprAST: Base class for all expressions: 
    - NumberExprAST: Represents a number.
    - VariableExprAST: Represents a variable.
    - CallExprAST: Represents a function call.
    - BinaryExprAST: Represents a binary operation (+, -, *, /).
- StmtAST: Base class for all statements.
- AssignmentExprAST: Represents an assignment expression.
- BlockAST: Represents a block of code.
- VarBindingsAST: Manages local variable declarations.
- InitAST: Base class for initializations (used in vardefs and binding).

# Second Level 
```
stmt:
    assignment
    | block
    | ifstmt
    | forstmt
    | exp;

ifstmt:
    "if" "(" condexp ")" stmt
    | "if" "(" condexp ")" stmt "else" stmt;

forstmt:
    "for" "(" init ";" condexp ";" assignment ")" stmt;

init:
    binding
    | assignment;
```

matches in the classes implemented in <a href="driver.cpp">driver.cpp</a>:
- IfStmtAST: Manages conditional statements (if, else).
- ForStmtAST: Manages loop statements (for).

# Third Level
```
condexp:
    relexp
    | relexp "and" condexp
    | relexp "or" condexp
    | "not" condexp
    | "(" condexp ")";

relexp:
    exp "<" exp
    | exp "==" exp;
```

matches in the classes implemented in <a href="driver.cpp">driver.cpp</a>:
- BinaryExprAST: Also used to represent logical operators (and, or, not).
- IfExprAST: Represents conditional expressions (ternary, condexp ? exp : exp).