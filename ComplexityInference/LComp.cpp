#include "llvm/Analysis/LoopPass.h"  
#include "llvm/DebugInfo.h"
#include "llvm/Analysis/PostDominators.h"  

//TODO: Intraprocedural Analysis


using namespace llvm; 
using namespace std; 

namespace {
  struct LoopComplexity : public FunctionPass { 
  static char ID; 
  LoopComplexity() : FunctionPass(ID) {} 

    void getAnalysisUsage(AnalysisUsage &AU) const { 
      AU.addRequired<DominatorTree>();
      AU.addRequired<PostDominatorTree>();
      AU.addRequired<LoopInfo>(); 
      AU.setPreservesAll(); 
    } 

    string getLineNumber(Instruction *I) {
      if (MDNode *N = I->getMetadata("dbg")) {
        DILocation Loc(N);
        Twine Line = Twine(Loc.getLineNumber());       
        Twine result = Twine("Line") + Line;
        return result.str();
      }
      return "";
    }
            

    //Verify if the outer loop's header dominates and postdominates the inner loop's header.
    //Verify if the inner loop's header postdominates the outer loop's body 
    //Veridy if the outer loop's header dominates and postdominates its own body    
    bool isMultipliable(Instruction *InstHeaderOuter, BasicBlock *HeaderOuter, BasicBlock *Header, BasicBlock *BodyOuter, DominatorTree &DT, PostDominatorTree &PDT){       
      if((DT.dominates(InstHeaderOuter,Header)) && (PDT.dominates(HeaderOuter,Header)))
        if(PDT.dominates(Header,BodyOuter) && (DT.dominates(InstHeaderOuter,BodyOuter)) && (PDT.dominates(HeaderOuter,BodyOuter)))
          return true;
      return false;
    }


    void verifyInnerLoops(std::vector<Loop*> subLoops, BasicBlock *HeaderOuter, DominatorTree &DT, PostDominatorTree &PDT) {
        int aux = 0 ; //Fix It
        Instruction *Instr;
        BasicBlock *BBold;
        //Instruction *InstHeaderOuter = HeaderOuter->getFirstInsertionPt();

        if (subLoops.empty())
            return;
        
        errs() << " * " << (subLoops.size() > 1? "(" : "");
        for (std::vector<Loop*>::iterator li = subLoops.begin(); li != subLoops.end(); li++) {
            Loop *L = *li;
            BasicBlock *Header = L->getHeader();        

        //Verify Multipliable
        /*if (isMultipliable(InstHeaderOuter,HeaderOuter, Header, BodyOuter , DT, PDT)){        
          for(unsigned int i = 0; i<depth; ++i) errs() <<"--"; //Alinhar saidas
          errs() << getLineNumber(InstHeaderOuter) << " x" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";                
          verifyInnerLoops(L, Header, DT, PDT);
        }        */        
        
        //Verify Sumable
            aux++;
            if(aux==2){
                if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold)){
                    errs() << " + ";
                }
                aux = 1;
            }
            std::vector<Loop*> subLoops = L->getSubLoops();
            errs() << (subLoops.size() > 1? "(" : "") << getLineNumber(Header->getFirstInsertionPt());
            verifyInnerLoops(subLoops, Header, DT, PDT);
            errs() << (subLoops.size() > 1? ")" : "");
            BBold = Header;
            Instr = BBold->getFirstInsertionPt();
        }
        errs() << (subLoops.size() > 1? ")" : "");
    } 

    virtual bool runOnFunction(Function &F) { 
      LoopInfo &LI = getAnalysis<LoopInfo>(); 
      DominatorTree &DT = getAnalysis<DominatorTree>();       
      PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();    

      int aux = 0 ; //Fix It      
      errs() << "Function " << F.getName() + "\n"; 
      Instruction *Instr;
      BasicBlock *BBold;
      for (LoopInfo::iterator i = LI.begin(), e = LI.end(); i != e; ++i) {     
        Loop *L = *i;
        BasicBlock *Header = L->getHeader();
        /*errs() << "(" << getLineNumber(Header->getFirstInsertionPt());
        verifyInnerLoops(L, Header, DT, PDT);
        errs() << ")";*/
        aux++;
        if(aux==2){
            if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold)){
                errs() << " + ";
            } else {
                errs() << " @ ";
            }
            aux = 1;
        }
        std::vector<Loop*> subLoops = L->getSubLoops();
        errs() << (subLoops.size() > 1? "(" : "") << getLineNumber(Header->getFirstInsertionPt());
        verifyInnerLoops(subLoops, Header, DT, PDT);
        errs() << (subLoops.size() > 1? ")" : "");
        BBold = Header;
        Instr = BBold->getFirstInsertionPt();
      }
      errs() << "\n";
      return(false);
    } 
  };
}


char LoopComplexity::ID = 0; 
static RegisterPass<LoopComplexity> X("LComp", 
    "Verify the possibility of multiply or add up Complexities."); 

