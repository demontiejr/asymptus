//
//  RegionAnalysis.h
//  
//
//  Created by Demontiê Junior on 2/3/15.
//
//

#ifndef ____RegionAnalysis__
#define ____RegionAnalysis__

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

using namespace llvm;

class RegionAnalysis : public ModulePass {
public:
    static char ID;
    
    RegionAnalysis();
    
    void getAnalysisUsage(AnalysisUsage &AU) const;
    
    virtual bool runOnModule(Module &M);
    
};

#endif /* defined(____RegionAnalysis__) */
