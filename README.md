<h1 align="center">kaltz - Languages Project</h1>

*kaltz* implements a compiler for a simple programming language, compiling the source code into LLVM intermediate code. The compiler consists of a lexer, a parser, a code generator, and various classes representing the abstract syntax tree (AST).

## ⛺ Prerequisites and setup
- LLVM 16.06
- flex 2.6.4
- bison (GNU Bison) 3.8.2

To compile and use *kaltz* you need to have LLVM installed and running in release version 16.06. 
Once this requirement is met, you can clone the repository.

```sh
git clone https://github.com/Piltxi/kaltz
```
After you clone the repo, you will almost certainly edit the paths in the complete project's Makefile and edit the entry:
> CPPFLAGS="-I/home/debian/Scrivania/LingWork/INSTALL/include"

by writing the local path of the include LLVM folder. 
Than, to finally compile the *kaltz* compiler, you can't go wrong: just do *make*.
```sh
cd kaltz-main
make
```
At this point, you can use the compiler to do what it's designed to do: generate *intermediate representations*; so, you can run:
```sh
kcomp <some>
```

inside <a href="test_progetto">test_progetto</a> you will then find some test files. To verify the correct functioning of *kaltz*, you can run `make` again: if the folder fills up with files, you must be overjoyed. 

🚑 otherwise something has certainly gone very wrong 🚑

## 📡 Overview

![Repo Diagram](diagram.svg)

Inside the repo, in the main directory, you will not find many files: 
- <a href="scanner.ll"> scanner.ll</a>: flex file for defining regular expressions that match keywords, identifiers, numbers, operators and some
- <a href="parser.yy"> parser.yy</a>:  bison file to define the grammar rules of the language and how they combine to form valid expressions, statements, and program structures
- <a href="driver.cpp"> driver.cpp </a> [and <a href="driver.hpp"> driver.hpp </a>]: central part of the compiler that orchestrates the overall compilation process; it includes the necessary LLVM headers and defines several key components and functions essential for generating LLVM IR code from the AST
- <a href="kcomp.cpp"> kcomp.cpp</a>: entry point for the compiler; it handles command-line arguments, initiates the parsing process. It's the main client in the project: **story begins here**.

of course, once you run `make`, if everything went well, you will find a few more files. 
but don't worry, everything is going great!

the other `.md` available are in Italian, and are my personal notes.

## 🎭 Authors <a name = "authors"></a>

- <a href="https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html">LLVM</a> framework designers
- other students in my course
- [@piltxi](https://github.com/Piltxi/) student of the languages ​​and compilers course - hopefully graduating


Don't pay attention to the last line: it's only for me.
```sh
export PATH=/home/debian/Scrivania/LingWork/INSTALL/bin:$PATH
```
