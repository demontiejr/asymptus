#include "llvm/Analysis/LoopPass.h"  
#include "llvm/DebugInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "../util/DepGraph.h"
#include "../util/LoopInfoEx.h"
#include "LoopInstrumentation.h"
#include <string>
#include <fstream>
#include <sstream>





#ifndef _lcomp_h
#define _lcomp_h

using std::cout;
using std::cin;
using std::string;

string avaliate(string expr);
string getLineNumber(Instruction *I);
string cleanName(string name);


class LoopComplexity : public FunctionPass {
private:
  std::stringstream expr;
  string filename;

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
    expr << "";
    expr.str(std::string());
    expr.clear();
  }

  void verifystr(){
    string eq = getstr();
    string result = " ";
    string temp = ".";
    while(result != temp){
      temp = result;
      result = avaliate(eq);
      eq = result;
    }
    strclear();
    addstr(eq);
  }

  void toFile(string OutputFilename){        
    std::ofstream file;
    string nameout = OutputFilename + ".LCompOut";
    file.open(nameout.c_str(),ios_base::app);
    file << expr.str();
    file.close();
  }


  void setFileName(Function &F){
    Instruction *I = F.getEntryBlock().getFirstInsertionPt();
    if (MDNode *N = I->getMetadata("dbg")) {
      DILocation Loc(N);
      filename = Loc.getFilename();            
    }
  }

public:
  static char ID; 
  LoopComplexity() ;
  void getAnalysisUsage(AnalysisUsage &AU) const ;
  virtual bool runOnFunction(Function &F) ;
  bool isMultipliable(Instruction *InstHeaderOuter, BasicBlock *HeaderOuter, BasicBlock *Header, BasicBlock *BodyOuter, DominatorTree &DT, PostDominatorTree &PDT) ;
};



#endif
