//
//  CountTamableLoops.cpp
//  
//
//  Created by Demontiê Junior on 9/23/14.
//
//

#include "CountTamableLoops.h"
#include "llvm/ADT/Statistic.h"
#include <vector>

STATISTIC(Loops, "Number of loops");
STATISTIC(TamableLoops, "Tamable Loops");

CountTamableLoops::CountTamableLoops() : FunctionPass(ID) {}

void CountTamableLoops::getAnalysisUsage(AnalysisUsage &AU) const{
    AU.addRequired<functionDepGraph>();
    AU.addRequiredTransitive<LoopInfoWrapperPass>();
}

bool CountTamableLoops::runOnFunction(Function &F) {    
    //Get the complete dependence graph
    functionDepGraph& DepGraph = getAnalysis<functionDepGraph> ();
    Graph* depGraph = DepGraph.depGraph;
    
    LoopInfo& li = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    
    for (LoopInfo::iterator lit = li.begin(), lend = li.end(); lit != lend; lit++) {
        Loop* l = *lit;
        
        Loops++;
        
        //Get the loop exit predicates
        std::set<Value*> loopInputs = getLoopInputs(l, depGraph);
        
        if (loopInputs.size() == 1) {
            Value *input = *loopInputs.begin();
            Type *Ty = input->getType();
            if (Ty->isIntegerTy() || Ty->isFloatingPointTy())
                TamableLoops++;
        }
    }
    return false;
}

char CountTamableLoops::ID = 0;
static RegisterPass<CountTamableLoops> X("count-loops", "Count loops with a single integer or floating point input.");
