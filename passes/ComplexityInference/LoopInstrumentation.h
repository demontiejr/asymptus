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
#include "llvm/IR/Type.h"

#include "../DepGraph/DepGraph.h"
#include "../DepGraph/LoopInfoEx.h"

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
    
    Value *createCounter2(Loop *L, Twine varName, Function &F);
    
    void increment(Loop *L, AllocaInst *ptr, LLVMContext& ctx);

    CallInst *createPrintfCall(Module *module, Instruction *insertPoint, Twine name, Twine value, Twine dbg);    

    CallInst *createPrintfCall(Module *module, Instruction *insertPoint, Value *param, Twine dbg, bool isIndVar=false);
    
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
        std::string nameStr = gvName.str();
        for (int i=0; i < nameStr.size(); i++) {
            if (nameStr[i] == '.' || nameStr[i] == '_')
                continue;
            if (nameStr[i] < 48 || (nameStr[i] > 57 && nameStr[i] < 65) || (nameStr[i] > 90 && nameStr[i] < 97) || nameStr[i] > 122)
                nameStr[i] = '_';
        }
        /*std::replace(nameStr.begin(), nameStr.end(), '#', '_');
        std::replace(nameStr.begin(), nameStr.end(), ' ', '_');
        std::replace(nameStr.begin(), nameStr.end(), '[', '_');
        std::replace(nameStr.begin(), nameStr.end(), ']', '_');
        std::replace(nameStr.begin(), nameStr.end(), ' ', '_');
        std::replace(nameStr.begin(), nameStr.end(), ' ', '_');
        std::replace(nameStr.begin(), nameStr.end(), ' ', '_');
        std::replace(nameStr.begin(), nameStr.end(), ' ', '_');*/
        if (GlobalVariable *gv = module->getNamedGlobal(nameStr))
            return gv;
        
        LLVMContext& ctx = module->getContext();
        Constant *format_const = ConstantDataArray::getString(ctx, str.str());
        GlobalVariable *var
            = new GlobalVariable(*module, ArrayType::get(IntegerType::get(ctx, 8), str.str().size()+1),
                                 true, GlobalValue::PrivateLinkage, format_const, nameStr);
        return var;
    }
    
};

//Fix point algoritm to get the variables defined outside the loop
set<Value*> getLoopInputs(Loop *L, Graph *depGraph);

bool hasGEP(Value *V, Instruction *I, Graph *depGraph, DominatorTree *DT);
bool usesGEP(Value *V, Instruction *,Graph *depGraph, DominatorTree *DT);

//Get the list of values that control the loop exit
std::set<Value*> getLoopExitPredicates(Loop* L);

#endif
