//
//  RegionAnalysis.cpp
//  
//
//  Created by Demontiê Junior on 2/3/15.
//
//

#include "RegionAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "../../RangeAnalysis/RangeAnalysis.h"

RegionAnalysis::RegionAnalysis() : ModulePass(ID) {}

void RegionAnalysis::getAnalysisUsage(AnalysisUsage &AU) const{
    AU.setPreservesAll();
    AU.addRequired<IntraProceduralRA<Cousot> >();
}

bool RegionAnalysis::runOnModule(Module &M) {
    Function *F = M.getFunction("main");
    if (!F) //error
        return false;
    
    IntraProceduralRA<Cousot> &ra = getAnalysis<IntraProceduralRA<Cousot> >(*F);
    
    GetElementPtrInst *GEP;
    Range r = Range(Max, Min);
    for (Function::iterator bb = F->begin(), e = F->end(); bb != e; ++bb) {
        for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
            if (!(GEP = dyn_cast<GetElementPtrInst>(i)) || GEP->getPointerOperand()->getName() != "argv")
                continue;
            for (GetElementPtrInst::op_iterator index = GEP->idx_begin(), idx_end = GEP->idx_end(); index != idx_end; index++) {
                Value *v = *index;
                //if (r)
                //    r = ra.getRange(v);
                //else
                    r = r.unionWith(ra.getRange(v));
                
                //if (isa<ConstantInt>(*index))
                //    errs() << "constante\n";
            }
        }
    }
    errs() << "Number of arguments: " << r.getUpper() << "\n";
    //r.print(errs());
    //errs() << "\n";
    return false;
}

char RegionAnalysis::ID = 0;
static RegisterPass<RegionAnalysis> X("region-analysis", "Determine the size of argv.");