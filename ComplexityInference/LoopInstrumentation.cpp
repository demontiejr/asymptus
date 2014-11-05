#include "LoopInstrumentation.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/DebugInfo.h"

#define END_NODE 9999

LoopInstrumentation::LoopInstrumentation() : FunctionPass(ID) {
    this->printf = NULL;
}

void LoopInstrumentation::getAnalysisUsage(AnalysisUsage &AU) const{
    AU.addRequired<functionDepGraph> ();
    AU.addRequiredTransitive<LoopInfo> ();
}

bool LoopInstrumentation::runOnFunction(Function &F) {    
    //Get the complete dependence graph
    functionDepGraph& DepGraph = getAnalysis<functionDepGraph> ();
    Graph* depGraph = DepGraph.depGraph;
    
    LoopInfo& li = getAnalysis<LoopInfo>();
    
    bool changed = false;
    int counter = 0;
    for (LoopInfo::iterator lit = li.begin(), lend = li.end(); lit != lend; lit++) {
        Loop* l = *lit;
        
        BasicBlock *loopHeader = l->getHeader();
        
        std::string dbgInfo = getLineNumber(loopHeader->getFirstInsertionPt());
        std::replace(dbgInfo.begin(), dbgInfo.end(), '/', '.');
        std::replace(dbgInfo.begin(), dbgInfo.end(), '-', '_');
        
        //Get or create a loop preheader
        BasicBlock *preHeader;
        
        if (BasicBlock *block = l->getLoopPreheader()) {
            preHeader = block;
        } else {
            std::vector<BasicBlock*> preHeaders;
            for (pred_iterator PI = pred_begin(loopHeader); PI != pred_end(loopHeader); ++PI) {
                BasicBlock *pred = *PI;
                if (!l->contains(pred)) {
                    preHeaders.push_back(pred);
                }
            }
            
            preHeader = SplitBlockPredecessors(loopHeader, ArrayRef<BasicBlock*>(preHeaders), "preHeader");
        }
        
        Instruction *lastInst = preHeader->getTerminator();
        
        //Get the loop exit predicates
        std::set<Value*> loopInputs = getLoopInputs(l, depGraph);
        
        //Insert printf calls
        for (std::set<Value*>::iterator it = loopInputs.begin(); it != loopInputs.end(); it++) {
            createPrintfCall(F.getParent(), lastInst, *it, Twine(dbgInfo));
        }
        
        //Create trip counter
        std::string varName = (F.getName() + Twine(".loopCounter.") + Twine(counter++)).str();
        Value *counter = createCounter(l, varName+Twine(".addr"), F);
        
        SmallVector<BasicBlock*, 4> exitBlocks;
        l->getExitBlocks(exitBlocks);
        for (SmallVectorImpl<BasicBlock*>::iterator it = exitBlocks.begin(); it != exitBlocks.end(); it++) {
            BasicBlock *exBB = *it;
            IRBuilder<> builder(exBB->getFirstInsertionPt());
            LoadInst* load = builder.CreateLoad(counter, Twine(varName));
            createPrintfCall(F.getParent(), exBB->getFirstInsertionPt(), load, Twine(dbgInfo));
            changed = true;
        }
    }
    return changed;
}

//Fix point algoritm to get the variables defined outside the loop
set<Value*> getLoopInputs(Loop *L, Graph *depGraph) {
    std::set<Value*> loopExitPredicates = getLoopExitPredicates(L);
    
    std::set<GraphNode*> visitedNodes;
    std::set<GraphNode*> workList;
    
    for(std::set<Value*>::iterator v = loopExitPredicates.begin(); v != loopExitPredicates.end(); v++){
        if (GraphNode* valueNode = depGraph->findNode(*v))
            workList.insert(valueNode);
        else
            errs() << "Value not found in the graph : " << **v << "\n";
    }
    
    std::set<Value*> loopInputs;
    
    while (!workList.empty()) {
        GraphNode* n = *workList.begin();
        workList.erase(n);
        visitedNodes.insert(n);
        
        std::map<GraphNode*, edgeType> preds = n->getPredecessors();
        
        for (std::map<GraphNode*, edgeType>::iterator pred = preds.begin(), s_end =
             preds.end(); pred != s_end; pred++) {
            
            Value* value = NULL;
            if (OpNode* opNode = dyn_cast<OpNode>(pred->first)) {
                value = opNode->getValue();
            } else if (VarNode* varNode = dyn_cast<VarNode>(pred->first)) {
                value = varNode->getValue();
            } else {
                continue;
            }
            
            if (dyn_cast<Constant>(value) || visitedNodes.count(pred->first) != 0)
                continue;
            
            if (L->isLoopInvariant(value)) {
                loopInputs.insert(value);
            } else {
                workList.insert(pred->first);
            }
        }
    }
    
    return loopInputs;
}

//Get the list of values that control the loop exit
std::set<Value*> getLoopExitPredicates(Loop* L) {
    std::set<Value*> loopExitPredicates;
    
    SmallVector<BasicBlock*, 4> loopExitingBlocks;
    L->getExitingBlocks(loopExitingBlocks);
    
    for (SmallVectorImpl<BasicBlock*>::iterator BB = loopExitingBlocks.begin(); BB != loopExitingBlocks.end(); BB++){
        if (BranchInst* BI = dyn_cast<BranchInst>((*BB)->getTerminator())) {
            if (!BI->isConditional()) continue;
            loopExitPredicates.insert(BI->getCondition());
        } else if (SwitchInst* SI = dyn_cast<SwitchInst>((*BB)->getTerminator())) {
            loopExitPredicates.insert(SI->getCondition());
        } else if (IndirectBrInst* IBI = dyn_cast<IndirectBrInst>((*BB)->getTerminator())) {
            loopExitPredicates.insert(IBI->getAddress());
        } else if (InvokeInst* II = dyn_cast<InvokeInst>((*BB)->getTerminator())) {
            loopExitPredicates.insert(II);
        }
    }
    
    return loopExitPredicates;
}

static void generateAdd(IRBuilder<> builder, AllocaInst* ptr, LLVMContext &ctx, int value) {
    LoadInst* load = builder.CreateLoad(ptr);
    Value *inc = builder.CreateAdd(load, ConstantInt::get(Type::getInt64Ty(ctx), value));
    builder.CreateStore(inc, ptr);
}

Value *LoopInstrumentation::createCounter(Loop *L, Twine varName, Function &F) {
    IRBuilder<> builder(F.getEntryBlock().getFirstInsertionPt());
    
    LLVMContext& ctx = F.getParent()->getContext();
    
    AllocaInst* counter = builder.CreateAlloca(Type::getInt64Ty(ctx), NULL, varName);
    
    //builder.SetInsertPoint(&(*F.getEntryBlock().rbegin()));
    builder.CreateStore(ConstantInt::get(Type::getInt64Ty(ctx), 0), counter);
    
    recoursiveInc(L, counter, ctx);
    
    return counter;
}

// bellman-ford
int longestPath(std::map<long, std::vector<std::pair<long, int> > > graph, BasicBlock* start) {
    std::map<long, int> d;
    
    std::vector<long> nodes;
    
    typedef std::map<long, std::vector<std::pair<long, int> > >::iterator it_type;
    for (it_type iterator = graph.begin(); iterator != graph.end(); iterator++) {
        d[iterator->first] = -1;
        nodes.push_back(iterator->first);
    }
    
    d[(long)start] = 0;
        
    for (size_t i=0; i < nodes.size(); i++) {
        for (std::vector<long>::iterator ui = nodes.begin(); ui != nodes.end(); ui++) {
            long u = *ui;
            
            std::vector<std::pair<long, int> > neighbors = graph[u];
            for (std::vector<std::pair<long, int> >::iterator vi = neighbors.begin(); vi != neighbors.end(); vi++) {
                std::pair<long, int> v = *vi;
                if (d[v.first] < d[u] + v.second) {
                    d[v.first] = d[u] + v.second;
                }
            }
        }
    }
        
    return d[END_NODE];
}

void LoopInstrumentation::recoursiveInc(Loop *L, AllocaInst *ptr, LLVMContext& ctx) {
    BasicBlock* header = L->getHeader();
    
    std::vector<Loop*> subLoops = L->getSubLoops();
    std::vector<BasicBlock*> blocks = L->getBlocks();
    
    std::map<long, std::vector<std::pair<long, int> > > graph;
    // create end node
    graph[END_NODE] = std::vector<std::pair<long, int> >();
    
    for (std::vector<BasicBlock*>::iterator i = blocks.begin(); i != blocks.end(); i++) {
        BasicBlock* BB = *i;
        
        int isInnerBlock = false;
        for (std::vector<Loop*>::iterator li = subLoops.begin(); li != subLoops.end(); li++) {
            Loop* innerLoop = *li;
            if (innerLoop->contains(BB)) {
                isInnerBlock = true;
            }
        }
        
        if (isInnerBlock) continue;
        
        if (!graph.count((long)BB)) {
            graph[(long)BB] = std::vector<std::pair<long, int> >();
        }
        
        // count instructions
        int nInst = BB->getInstList().size();
        // get successors
        for (succ_iterator PI = succ_begin(BB), E = succ_end(BB); PI != E; ++PI) {
            BasicBlock* succ = *PI;
            
            long succAddr = (long) succ;
            
            // ignore back edges
            if (succ == header || !L->contains(succ)) {
                succAddr = END_NODE;
            } else {
                for (std::vector<Loop*>::iterator li = subLoops.begin(); li != subLoops.end(); li++) {
                    Loop* innerLoop = *li;
                    if (innerLoop->contains(succ)) {
                        BasicBlock* exBB = innerLoop->getExitBlock();
                        if (!exBB)
                            break;
                        succAddr = (long) exBB;
                    }
                }
            }
            
            // add to the graph
            graph[(long)BB].push_back(std::pair<long, int>(succAddr, nInst));
        }
    }
    
    int pathCost = longestPath(graph, header);
    
    IRBuilder<> builder(header->getFirstInsertionPt());
    generateAdd(builder, ptr, ctx, pathCost);
    
    for (std::vector<Loop*>::iterator li = subLoops.begin(); li != subLoops.end(); li++) {
        Loop* innerLoop = *li;
        recoursiveInc(innerLoop, ptr, ctx);
    }
}

CallInst *LoopInstrumentation::createPrintfCall(Module *module, Instruction *insertPt, Value *param, Twine dbg) {
    LLVMContext& ctx = module->getContext();
    IRBuilder<> builder(ctx);
    builder.SetInsertPoint(insertPt);
    
    Constant *zero = Constant::getNullValue(IntegerType::getInt32Ty(ctx));
    
    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    Constant *var_ref = ConstantExpr::getGetElementPtr(getFormat(module, param->getType()), indices);
    
    GlobalVariable *varName = getConstString(module, param->getName(), param->getName());
    Constant *varName_ref = ConstantExpr::getGetElementPtr(varName, indices);
    
    GlobalVariable *dbgInfo = getConstString(module, dbg, dbg);
    Constant *dbgInfo_ref = ConstantExpr::getGetElementPtr(dbgInfo, indices);
    
    CallInst *call = builder.CreateCall4(getPrintf(module), var_ref, dbgInfo_ref, varName_ref, param, "printf");
    
    return call;
}

Function *LoopInstrumentation::getPrintf(Module *module) {
    if (!this->printf) {
        LLVMContext& ctx = module->getContext();
        std::vector<Type*> argTypes;
        argTypes.push_back(Type::getInt8PtrTy(ctx));
        FunctionType *MTy = FunctionType::get(Type::getInt32Ty(ctx), argTypes, true);
        
        Constant *funcConst = module->getOrInsertFunction("printf", MTy);
        this->printf = cast<Function>(funcConst);
    }
    return this->printf;
}

std::string LoopInstrumentation::getLineNumber(Instruction *I) {
    if (MDNode *N = I->getMetadata("dbg")) {
        DILocation Loc(N);
        Twine Line = Twine(Loc.getLineNumber());
        StringRef File = Loc.getFilename();
        Twine result = File + Twine("_") + Line;
        return result.str();
    }
    return "";
}

char LoopInstrumentation::ID = 0;
static RegisterPass<LoopInstrumentation> X("instr-loop", "Instrument the loops of a function to print the loop exit predicates.");
