#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/GlobalVariable.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <variant>

#include "parser.hpp"

using namespace llvm;

#define YY_DECL \
  yy::parser::symbol_type yylex(driver &drv)

YY_DECL;


class driver
{
public:
  driver();
  std::map<std::string, AllocaInst *> NamedValues; 
  RootAST* root; 
  int parse(const std::string &f);
  std::string file;
  bool trace_parsing;   
  void scan_begin();     
  void scan_end();       
  bool trace_scanning;   
  yy::location location;
  void codegen();
};

typedef std::variant<std::string, double> lexval;
const lexval NONE = 0.0;


enum initType
{
  ASSIGNMENT,
  BINDING,
  INIT
};


class RootAST
{
public:
  virtual ~RootAST() {};
  virtual lexval getLexVal() const { return NONE; };
  virtual Value *codegen(driver &drv) { return nullptr; };
};


class SeqAST : public RootAST
{
private:
  RootAST *first;
  RootAST *continuation;

public:
  SeqAST(RootAST *first, RootAST *continuation);
  Value *codegen(driver &drv) override;
};


class StmtAST : public RootAST
{
};


class InitAST : public StmtAST
{
private:
  std::string Name;

public:
  virtual std::string &getName();
  virtual initType getType();
};


class ExprAST : public StmtAST
{
};


class NumberExprAST : public ExprAST
{
private:
  double Val;

public:
  NumberExprAST(double Val);
  lexval getLexVal() const override;
  Value *codegen(driver &drv) override;
};


class VariableExprAST : public ExprAST
{
private:
  std::string Name;
  ExprAST *Exp;

public:
  VariableExprAST(const std::string &Name, ExprAST *Exp = nullptr);
  lexval getLexVal() const override;
  Value *codegen(driver &drv) override;
};


class BinaryExprAST : public ExprAST
{
private:
  char Op;
  ExprAST *LHS;
  ExprAST *RHS;

public:
  BinaryExprAST(char Op, ExprAST *LHS, ExprAST *RHS);
  Value *codegen(driver &drv) override;
};


class CallExprAST : public ExprAST
{
private:
  std::string Callee;
  std::vector<ExprAST *> Args; 

public:
  CallExprAST(std::string Callee, std::vector<ExprAST *> Args);
  lexval getLexVal() const override;
  Value *codegen(driver &drv) override;
};

class IfExprAST : public ExprAST
{
private:
  ExprAST *cond;
  ExprAST *trueexp;
  ExprAST *falseexp;

public:
  IfExprAST(ExprAST *cond, ExprAST *trueexp, ExprAST *falseexp);
  Value *codegen(driver &drv) override;
};


class BlockAST : public ExprAST
{
private:
  std::vector<InitAST *> Def;
  std::vector<StmtAST *> Stmts;

public:
  BlockAST(std::vector<InitAST *> Def, std::vector<StmtAST *> Stmts);
  BlockAST(std::vector<StmtAST *> Stmts);
  Value *codegen(driver &drv) override;
};


class VarBindingsAST : public InitAST
{
private:
  std::string Name;
  ExprAST *Val;

public:
  VarBindingsAST(std::string Name, ExprAST *Val);
  AllocaInst *codegen(driver &drv) override;
  initType getType() override;
  std::string &getName() override;
};


class AssignmentExprAST : public InitAST
{
private:
  std::string Name;
  ExprAST *Val;

public:
  AssignmentExprAST(std::string Name, ExprAST *Val);
  Value *codegen(driver &drv) override;
  initType getType() override;
  std::string &getName() override;
};


class GlobalVariableAST : public RootAST
{
private:
  std::string Name;
  double Size;

public:
  GlobalVariableAST(std::string Name, double Size = -1);
  Value *codegen(driver &drv) override;
  std::string &getName();
};


class IfStmtAST : public StmtAST
{
private:
  ExprAST *cond;
  StmtAST *trueblock;
  StmtAST *falseblock;

public:
  IfStmtAST(ExprAST *cond, StmtAST *trueblock, StmtAST *falseblock);
  IfStmtAST(ExprAST *cond, StmtAST *trueblock);
  Value *codegen(driver &drv) override;
};


class ForStmtAST : public StmtAST
{
private:
  InitAST *init;
  ExprAST *cond;
  AssignmentExprAST *step;
  StmtAST *body;

public:
  ForStmtAST(InitAST *init, ExprAST *cond, AssignmentExprAST *step, StmtAST *body);
  Value *codegen(driver &drv) override;
};


class PrototypeAST : public RootAST
{
private:
  std::string Name;
  std::vector<std::string> Args;
  bool emitcode;

public:
  PrototypeAST(std::string Name, std::vector<std::string> Args);
  const std::vector<std::string> &getArgs() const;
  lexval getLexVal() const override;
  Function *codegen(driver &drv) override;
  void noemit();
};


class FunctionAST : public RootAST
{
private:
  PrototypeAST *Proto;
  ExprAST *Body;
  bool external;

public:
  FunctionAST(PrototypeAST *Proto, ExprAST *Body);
  Function *codegen(driver &drv) override;
};

#endif // ! DRIVER_HH
