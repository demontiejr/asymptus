#include "llvm/Analysis/LoopPass.h"  
#include "llvm/DebugInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "../util/DepGraph.h"
#include "../util/LoopInfoEx.h"
#include <string>
#include <fstream>
#include "LoopInstrumentation.h"

#ifndef _lcomp_h
#define _lcomp_h

using std::cout;
using std::cin;
using std::string;

/*
* todo: passar pra privado depois
*/


class LoopComplexity : public FunctionPass {
private:
  std::stringstream expr;

  void verifyInnerLoops(std::vector<Loop*> subLoops, BasicBlock *HeaderOuter, DominatorTree &DT, PostDominatorTree &PDT, Graph *depGraph, bool isParentConstant);

  void addstr(string str){
    expr << str;
  }

  /*
  void addstr(Twine str){
    expr << str.str();
  }
  */

  void dumpstr(){
    errs() << expr.str();
  }

  string getstr(){
    return expr.str();
  }

  void strclear(){
    expr.str(std::string());
    expr.clear();
  }

public:
  static char ID; 
  LoopComplexity() ;
  void getAnalysisUsage(AnalysisUsage &AU) const ;
  virtual bool runOnFunction(Function &F) ;
  bool isMultipliable(Instruction *InstHeaderOuter, BasicBlock *HeaderOuter, BasicBlock *Header, BasicBlock *BodyOuter, DominatorTree &DT, PostDominatorTree &PDT) ;
};


string avaliate(string expr);
string getLineNumber(Instruction *I);
string cleanName(string name);

#endif
