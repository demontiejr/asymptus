//
//  CountTamableLoops.h
//  
//
//  Created by Demontiê Junior on 9/23/14.
//
//

#ifndef ____CountTamableLoops__
#define ____CountTamableLoops__

#include <LoopInstrumentation.h>

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

using namespace llvm;


class CountTamableLoops : public FunctionPass {
public:
    static char ID;
    
    CountTamableLoops();
    
    void getAnalysisUsage(AnalysisUsage &AU) const;
    
    virtual bool runOnFunction(Function &F);

};

#endif /* defined(____CountTamableLoops__) */
