#include "LComp.h"
#include "llvm/Support/CommandLine.h"

static cl::opt<bool> usenames("use-names", cl::desc("Use names"));
static cl::opt<bool> send2file("toFile", cl::desc("toFile"));
cl::opt<string> OutputFilename("eqname", cl::desc("Specify output filename"), cl::value_desc("filename"));

//Fix the technique to get the filename.

#undef  DEBUG_TYPE
#define DEBUG_TYPE "lcomp"

using namespace llvm; 
using std::string;


LoopComplexity::LoopComplexity() : FunctionPass(ID) {} 

void LoopComplexity::getAnalysisUsage(AnalysisUsage &AU) const {       
  AU.addRequired<functionDepGraph>();    
  AU.addRequired<LoopInfoWrapperPass>(); 
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTree>();
  AU.setPreservesAll(); 
} 
          

bool LoopComplexity::runOnFunction(Function &F) { 
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree(); 
  PostDominatorTree &PDT = getAnalysis<PostDominatorTree>();    
  functionDepGraph& DepGraph = getAnalysis<functionDepGraph> ();
  Graph* depGraph = DepGraph.depGraph;

  bool isConstant = false;
  bool prevConstant = false;    
  int aux = 0 ; //Fix It    

  //clear the stringstream when pass run in other function
  strclear();
  
  addstr("\nFunction " + F.getName().str() + "\n");

  Instruction *Instr;
  BasicBlock *BBold;
  for (LoopInfo::reverse_iterator i = LI.rbegin(), e = LI.rend(); i != e; ++i) {
    bool isLoad = false;
    isConstant = false;    
    Loop *L = *i;
    std::set<Value*> loopInputs = getLoopInputs(L, depGraph);
    std::set<Value*>::iterator first = loopInputs.begin();
    Value *input = *first;
    Value *ptr = NULL;

    BasicBlock *Header = L->getHeader();
    aux++;

    if(loopInputs.size() > 1)  first++;
    Value *secIn = *first;

    if(loopInputs.size() > 1)
    if(loopInputs.size() != 0){
      if ((isa<ConstantInt>(input) || isa<ConstantFP>(input))
        && loopInputs.size() == 1){
        isConstant = true;
      }else if (LoadInst *load = dyn_cast<LoadInst>(input)) {
        ptr = load->getPointerOperand();
        input->setName(ptr->getName() + "..");
        isLoad=true;
      }else if(loopInputs.size() > 1){
        if (LoadInst *load = dyn_cast<LoadInst>(secIn)) {
          secIn->setName(load->getPointerOperand()->getName() + "..");
        }
      }
    }

   
    if(aux==2){
      if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold) && ! isConstant){          
        addstr(" + ");
      }else if ( ! isConstant){
        addstr(" @ ");          
      }
        aux = 1;
    }

    std::vector<Loop*> subLoops = L->getSubLoops();	    
	    
    if(! isConstant){              
      string varName = ((loopInputs.size() == 0)? getLineNumber(Header->getFirstInsertionPt()) : 
        (loopInputs.size() > 1)? "diff[" + cleanName(input->getName()) + "," + cleanName(secIn->getName()) + "]" : cleanName(input->getName()));

      string loopID = (usenames? varName : getLineNumber(Header->getFirstInsertionPt()));
      addstr(loopID);
    }

    verifyInnerLoops(subLoops, Header, DT, PDT, depGraph, isConstant);
        
  	  
    BBold = Header;
    Instr = BBold->getFirstInsertionPt();
   prevConstant = isConstant;
  }


  /*Cleaner. The final expression may have wrong sentences such as Line233 + 
  That may occurs because we are not printing loops O(1) in the expression.
  */
  verifystr();
  


  send2file ? toFile(OutputFilename) : dumpstr();
  

  
  return(false);
}



/*Verify some conditions that allow us multiply loops*/
bool LoopComplexity::isMultipliable(Instruction *InstHeaderOuter, BasicBlock *HeaderOuter, BasicBlock *Header, BasicBlock *BodyOuter, DominatorTree &DT, PostDominatorTree &PDT){       
  if((DT.dominates(InstHeaderOuter,Header)) && (PDT.dominates(HeaderOuter,Header)))
    if(PDT.dominates(Header,BodyOuter) && (DT.dominates(InstHeaderOuter,BodyOuter)) && (PDT.dominates(HeaderOuter,BodyOuter)))
      return true;
  return false;
}


void LoopComplexity::verifyInnerLoops(std::vector<Loop*> subLoops, BasicBlock *HeaderOuter, DominatorTree &DT, PostDominatorTree &PDT, Graph *depGraph, bool isParentConstant=false) {
  int aux = 0 ;
  Instruction *Instr;
  BasicBlock *BBold;
  bool isConstant = false;

  if (subLoops.empty())
    return ;

    
  string putPar = (subLoops.size() > 1? "(" : "");
  addstr(" * " + putPar);   
    
  for (std::vector<Loop*>::iterator li = subLoops.begin(); li != subLoops.end(); li++) {
    bool isLoad = false;
    isConstant = false;
    Value *ptr = NULL;
    Loop *L = *li;
    BasicBlock *Header = L->getHeader();

    std::set<Value*> loopInputs = getLoopInputs(L, depGraph);
    std::set<Value*>::iterator first = loopInputs.begin();
    Value *input = *first;

    if(loopInputs.size() > 1)  first++;
    Value *secIn = *first;

    if(loopInputs.size() > 1)
    if(loopInputs.size() != 0){
      if ((isa<ConstantInt>(input) || isa<ConstantFP>(input))
        && loopInputs.size() == 1){
        isConstant = true;
      }else if (LoadInst *load = dyn_cast<LoadInst>(input)) {
        ptr = load->getPointerOperand();
        input->setName(ptr->getName() + "..");
        isLoad=true;
      }else if(loopInputs.size() > 1){
        if (LoadInst *load = dyn_cast<LoadInst>(secIn)) {
          secIn->setName(load->getPointerOperand()->getName() + "..");
        }
      }
    }


    // =========== debug =============== //
    DEBUG(errs() << "\n->Num Total Inputs: " << loopInputs.size());
    if(loopInputs.size() > 1){            
      if(isLoad)
        DEBUG(errs() << "\n    " <<  ptr->getName());
      else
        DEBUG(errs() << "\n    " <<  input->getName());

      DEBUG(errs() << "\n    " <<  secIn->getName());
            
      DEBUG(errs() << "\n\n");
    }
    // ================================= //


    //Verify Multipliable
    /*if (isMultipliable(InstHeaderOuter,HeaderOuter, Header, BodyOuter , DT, PDT)){                
      errs() << getLineNumber(InstHeaderOuter) << " x" << getLineNumber(Header->getFirstInsertionPt()) << " at Depth " << L->getLoopDepth() <<"\n";                
      verifyInnerLoops(L, Header, DT, PDT);
    }        */                

        

    //Verify Sumable
    aux++;
    if(aux==2){
      if(DT.dominates(Instr,Header) && PDT.dominates(Header,BBold)){
        addstr(" + ");
      }else{          
        addstr(" @ ");
      }
      aux = 1;
    }

    std::vector<Loop*> _subLoops = L->getSubLoops();
    if(! isConstant){              
      string varName = ((loopInputs.size() == 0)? getLineNumber(Header->getFirstInsertionPt()) : 
        (loopInputs.size() > 1)? "diff[" + cleanName(input->getName()) + "," + cleanName(secIn->getName()) + "]" : cleanName(input->getName()));
      string loopID = (usenames? varName : getLineNumber(Header->getFirstInsertionPt()));

      addstr(loopID);
    }
    verifyInnerLoops(_subLoops, Header, DT, PDT,depGraph, isConstant);

    BBold = Header;
    Instr = BBold->getFirstInsertionPt();
  }

  if(subLoops.size() > 1)
    addstr(")");    
}

string avaliate(string expr){
  string prev = "";
  string prevprev = "";
  string succ = "";
  std::string::size_type i = 0;
  std::string::size_type blankEraser = 0;
  for(std::string::iterator it = expr.begin(); it != expr.end(); ++it, ++i){
    blankEraser = i;
    if (*it != ' '){
      //cout << expr << "\n";
      std::string::iterator prox = it+1;
      while(*prox == ' '){
         ++prox;
      }
      succ = *prox;
      
      if(*it == '+' && prev == "+" ){ //Remover sequencia de iguais
        expr.erase(i,1);
      }else if(*it == '*' && prev == "*" ){
        expr.erase(i,1);
      }else if(*it == '@' && prev == "@" ){
        expr.erase(i,1);
      }else if(*it == '*' && ( succ == "\0" || succ == ")" || prev == "(" )){ // Remover operacoes mortas no final ou começo
        expr.erase(i,1);
      }else if(*it == '+' && ( succ == "\0" || succ == ")" || prev == "(" )){
        expr.erase(i,1);
      }else if(*it == '@' && ( succ == "\0" || succ == ")" || prev == "(" )){
        expr.erase(i,1);
      }else if(*it == '*' && ( succ == "\0" || succ == "+" || succ == "@")){ // Remover casos de menor prior, ex rm * em + * +
        expr.erase(i,1);
      }else if(*it == '+' && succ == "@" ){
        expr.erase(i,1);
      }else if(*it == '*' && (prev == "+" || prev == "@") ){
        expr.erase(i,1);
      }else if(*it == '+' && prev == "@" ){
        expr.erase(i,1);
      }else if((*it == '+' || *it == '*' || *it == '@' || *it == '\\')
        && prev.empty()) {
        expr.erase(i,1);
      }
      prevprev = prev;
      prev = *it;
   }else{
      std::string::iterator prox = it+1;
      while(*prox == ' '){
        expr.erase(blankEraser,1);
        prox++;
        blankEraser++;
      }
    }
  }
  //cout << "\n\n";
  return expr;
}

string getLineNumber(Instruction *I) {
  if (DILocation *Loc = I->getDebugLoc()) {
    Twine Line = Twine(Loc->getLine());
    Twine result = Twine("Line") + Line;
    return result.str();
  }
  return "";
}

string cleanName(string name){   
  std::size_t size_ = name.size();
  for(std::size_t i = 0; i < size_ ; i++){
    if(name.at(i) == '.'){
      for(std::size_t j = i; j < size_ ; j++){
        if(name.at(j) == ',' || name.at(j) == '[' || name.at(j) == ']'
          || name.at(j) == '\0' || name.at(j) == '\n' || name.at(j) == ' '
          || name.at(j) == '+' || name.at(j) == '*' || name.at(j) == '-' || name.at(j) == '\\' ){
          
          name.erase(i, j-i);
          size_ = name.size();
          break;
        }else if (j+1 >= size_){
          name.erase(i, j-i+1);
          size_ = name.size();
          break;      
        }
      }
    }
  }
  return name;
}


char LoopComplexity::ID = 0; 
static RegisterPass<LoopComplexity> X("lcomp", 
    "Verify the possibility of multiply or add up Complexities."); 

