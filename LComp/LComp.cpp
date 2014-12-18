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
        Twine result = Twine(" (Line ") + Line + Twine(")");
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


    void verifyInnerLoops(Loop *L1, BasicBlock *HeaderOuter, DominatorTree &DT, PostDominatorTree &PDT) {           
      unsigned depth = L1->getLoopDepth();
      int aux = 0 ; //Fix It      
      Instruction *Instr;
      BasicBlock *BBold;          
      Instruction *InstHeaderOuter = HeaderOuter->getFirstInsertionPt();
      for (Loop::iterator i = L1->begin(), e = L1->end(); i != e; ++i) {
        Loop *L = *i;
        BasicBlock *Header = L->getHeader();        

        //Verify Multipliable
        /*if (isMultipliable(InstHeaderOuter,HeaderOuter, Header, BodyOuter , DT, PDT)){        
          for(unsigned int i = 0; i<depth; ++i) errs() <<"--"; //Alinhar saidas
          errs() << getLineNumber(InstHeaderOuter) << " x" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";                
          verifyInnerLoops(L, Header, DT, PDT);              
        }        */

        for(unsigned int i = 0; i<depth; ++i) errs() <<"  "; //Alinhar saidas
        errs() << getLineNumber(InstHeaderOuter) << " x" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";                
        verifyInnerLoops(L, Header, DT, PDT);              
                
        
        //Verify Sumable
        aux++;
        if(aux==2){
          if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold)){
            for(unsigned int i = 0; i<depth; ++i) errs() <<"  "; //Pode tirar, é só pra alinhar saida
            errs() << getLineNumber(BBold->getFirstInsertionPt()) << " +" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";
          }          
          aux = 1;          
        }          
        BBold = Header;
        Instr = BBold->getFirstInsertionPt();
      }       
    } 

    virtual bool runOnFunction(Function &F) { 
      LoopInfo &LI = getAnalysis<LoopInfo>(); 
      DominatorTree &DT = getAnalysis<DominatorTree>();       
      PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();    

      int aux = 0 ; //Fix It      
      errs() << "Function " << F.getName() + "\n"; 
      Instruction *Instr;
      BasicBlock *BBold;    
      for (LoopInfo::reverse_iterator i = LI.rbegin(), e = LI.rend(); i != e; ++i) {     
        Loop *L = *i;
        BasicBlock *Header = L->getHeader();        
        verifyInnerLoops(L, Header, DT, PDT);
        aux++;
          if(aux==2){
            if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold)){
              errs() << getLineNumber(BBold->getFirstInsertionPt()) << " +" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";
            }            
            aux = 1;            
          }          
          BBold = Header;
          Instr = BBold->getFirstInsertionPt();
      } 
      return(false);
    } 
  };
}


char LoopComplexity::ID = 0; 
static RegisterPass<LoopComplexity> X("LComp", 
    "Verify the possibility of multiply or add up Complexities."); 

