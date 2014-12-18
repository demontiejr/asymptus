//
//  LoopInstrumentation.h
//
//
//  Created by Demontiê Junior on 9/1/14.
//
//

#ifndef _LoopInstrumentation_h
#define _LoopInstrumentation_h

#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"

#include "../util/DepGraph.h"
#include "../util/LoopInfoEx.h"

using namespace llvm;


class LoopInstrumentation : public FunctionPass {
public:
    static char ID; // Pass identification, replacement for typeid
    
    LoopInstrumentation();
    
    void getAnalysisUsage(AnalysisUsage &AU) const;
    
    virtual bool runOnFunction(Function &F);
    
private:
    Function *printf;
    
    Value *createCounter(Loop *L, Twine varName, Function &F);
    
    void increment(Loop *L, AllocaInst *ptr, LLVMContext& ctx);

    CallInst *createPrintfCall(Module *module, Instruction *insertPoint, Twine name, Twine value, Twine dbg);    

    CallInst *createPrintfCall(Module *module, Instruction *insertPoint, Value *param, Twine dbg);
    
    Function *getPrintf(Module *module);
    
    std::string getDbgInfo(Function &F, Instruction *I);
    
    static GlobalVariable *getFormat(Module *module, Type *Ty) {
        if (!Ty)
            return NULL;
        
        Twine format;
        Twine formatTy;
        
        if (Ty->isFloatingPointTy()) {
            format = Twine("%f");
            formatTy = "Float";
        } else if (Ty->isIntegerTy(64)) {
            format = Twine("%ld");
            formatTy = "Long";
        } else if (Ty->isIntegerTy()) {
            format = Twine("%d");
            formatTy = "Int";
        } else {
            format = Twine("%s");
            formatTy = "";
        }
        return getConstString(module, Twine("format")+formatTy,
                              Twine("\n<LoopInstr> %s: %s = ") + format + Twine("\n"));
    }
    
    static GlobalVariable *getConstString(Module *module, Twine name, Twine str) {
        Twine gvName = name + Twine(".str");
        if (GlobalVariable *gv = module->getNamedGlobal(gvName.str()))
            return gv;
        
        LLVMContext& ctx = module->getContext();
        Constant *format_const = ConstantDataArray::getString(ctx, str.str());
        GlobalVariable *var
            = new GlobalVariable(*module, ArrayType::get(IntegerType::get(ctx, 8), str.str().size()+1),
                                 true, GlobalValue::PrivateLinkage, format_const, gvName);
        return var;
    }
    
};

//Fix point algoritm to get the variables defined outside the loop
set<Value*> getLoopInputs(Loop *L, Graph *depGraph);

//Get the list of values that control the loop exit
std::set<Value*> getLoopExitPredicates(Loop* L);

#endif
