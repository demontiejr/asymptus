#ifndef DEPGRAPH_H_
#define DEPGRAPH_H_

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "depgraph"
#endif

#define USE_ALIAS_SETS true

#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/raw_ostream.h"
#include "LoopInfoEx.h"
#include "AliasSets.h"
#include "GenericGraph.h"
#include <list>
#include <map>
#include <set>
#include <stack>
#include <sstream>
#include <stdio.h>

using namespace std;

namespace llvm {
STATISTIC(NrOpNodes, "Number of operation nodes");
STATISTIC(NrVarNodes, "Number of variable nodes");
STATISTIC(NrMemNodes, "Number of memory nodes");
STATISTIC(NrBackNodes, "Number of back nodes");
STATISTIC(NrEdges, "Number of edges");

typedef enum {
        etData = 0, etControl = 1
} edgeType;

/*
 * Class GraphNode
 *
 * This abstract class can do everything a simple graph node can do:
 *              - It knows the nodes that points to it
 *              - It knows the nodes who are ponted by it
 *              - It has a unique ID that can be used to identify the node
 *              - It knows how to connect itself to another GraphNode
 *
 * This class provides virtual methods that makes possible printing the graph
 * in a fancy .dot file, providing for each node:
 *              - Label
 *              - Shape
 *              - Style
 *
 */
class GraphNode {
private:
        std::map<GraphNode*, edgeType> successors;
        std::map<GraphNode*, edgeType> predecessors;

        static int currentID;
        int ID;

protected:
        int Class_ID;
public:
        GraphNode();
        GraphNode(GraphNode &G);

        virtual ~GraphNode();

        static inline bool classof(const GraphNode *N) {
                return true;
        }
        ;
        std::map<GraphNode*, edgeType> getSuccessors();
        bool hasSuccessor(GraphNode* succ);

        std::map<GraphNode*, edgeType> getPredecessors();
        bool hasPredecessor(GraphNode* pred);

        void connect(GraphNode* dst, edgeType type = etData);
        void disconnect(GraphNode* dst);

        int getClass_Id() const;
        int getId() const;
        std::string getName();
        virtual std::string getLabel() = 0;
        virtual std::string getShape() = 0;
        virtual std::string getStyle();

        virtual llvm::raw_ostream& dump(llvm::raw_ostream &strm);

        virtual GraphNode* clone() = 0;
};

//extern llvm::raw_ostream& operator<<(llvm::raw_ostream &strm, GraphNode &a);

//llvm::raw_ostream& operator<<(llvm::raw_ostream &strm, GraphNode &a) {
//  return a.dump(strm);
//}

/*
 * Class OpNode
 *
 * This class represents the operation nodes:
 *              - It has a OpCode that is compatible with llvm::Instruction OpCodes
 *              - It may or may not store a value, that is the variable defined by the operation
 */
class OpNode: public GraphNode {
private:
        unsigned int OpCode;
        Value* value;
public:
        OpNode(int OpCode) :
                GraphNode(), OpCode(OpCode), value(NULL) {
                this->Class_ID = 1;
                NrOpNodes++;
        }
        ;
        OpNode(int OpCode, Value* v) :
                GraphNode(), OpCode(OpCode), value(v) {
                this->Class_ID = 1;
                NrOpNodes++;
        }
        ;
        ~OpNode() {
                NrOpNodes--;
        }
        ;
        static inline bool classof(const GraphNode *N) {
                return N->getClass_Id() == 1 || N->getClass_Id() == 3;
        }
        ;
        unsigned int getOpCode() const;
        void setOpCode(unsigned int opCode);
        Value* getValue();

        std::string getLabel();
        std::string getShape();

        GraphNode* clone();
};

/*
 * Class CallNode
 *
 * This class represents operation nodes of llvm::Call instructions:
 *              - It stores the pointer to the called function
 */
class CallNode: public OpNode {
private:
        CallInst* CI;
public:
        CallNode(CallInst* CI) :
                OpNode(Instruction::Call, CI), CI(CI) {
                this->Class_ID = 3;
        }
        ;
        static inline bool classof(const GraphNode *N) {
                return N->getClass_Id() == 3;
        }
        ;
        Function* getCalledFunction() const;

        CallInst* getCallInst() const;

        std::string getLabel();
        std::string getShape();

        GraphNode* clone();
};

/*
 * Class VarNode
 *
 * This class represents variables and constants which are not pointers:
 *              - It stores the pointer to the corresponding Value*
 */
class VarNode: public GraphNode {
private:
        Value* value;
public:
        VarNode(Value* value) :
                GraphNode(), value(value) {
                this->Class_ID = 2;
                NrVarNodes++;
        }
        ;
        ~VarNode() {
                NrVarNodes--;
        }
        static inline bool classof(const GraphNode *N) {
                return N->getClass_Id() == 2;
        }
        ;
        Value* getValue();

        std::string getLabel();
        std::string getShape();

        GraphNode* clone();
};




/*
 * Class MemNode
 *
 * This class represents memory as AliasSets of pointer values:
 *              - It stores the ID of the AliasSet
 *              - It provides a method to get access to all the Values contained in the AliasSet
 */
class MemNode: public GraphNode {
private:
        int aliasSetID;
        AliasSets *AS;
public:
        MemNode(int aliasSetID, AliasSets *AS) :
                aliasSetID(aliasSetID), AS(AS) {
                this->Class_ID = 4;
                NrMemNodes++;
        }
        ;
        ~MemNode() {
                NrMemNodes--;
        }
        ;
        static inline bool classof(const GraphNode *N) {
                return N->getClass_Id() == 4;
        }
        ;
        std::set<Value*> getAliases();

        std::string getLabel();
        std::string getShape();
        GraphNode* clone();
        std::string getStyle();

        int getAliasSetId() const;
};


/*
 * Class BackNode
 *
 * This class represents nodes that explicit back edges:
 *              - It stores the ID of the AliasSet
 *              - It provides a method to get access to all the Values contained in the AliasSet
 */
class BackNode: public GraphNode {
private:
	GraphNode* next;
public:
		BackNode(GraphNode* next) :
			next(next) {
                this->Class_ID = 5;
                NrBackNodes++;
        }
        ;
        ~BackNode() {
        	NrBackNodes--;
        }
        ;
        static inline bool classof(const GraphNode *N) {
                return N->getClass_Id() == 5;
        };


        std::string getLabel();
        std::string getShape();
        GraphNode* clone();

        bool operator== (BackNode &rhs){return next == rhs.next;}
        bool operator!= (BackNode &rhs){return next != rhs.next;}

        bool operator>= (BackNode &rhs){return next >= rhs.next;}
        bool operator<= (BackNode &rhs){return next <= rhs.next;}

        bool operator> (BackNode &rhs){return next > rhs.next;}
        bool operator< (BackNode &rhs){return next < rhs.next;}

		GraphNode* getNext() {
			return next;
		}
};



/*
 * Class Graph
 *
 * Stores a set of nodes. Each node knows how to go to other nodes.
 *
 * The class provides methods to:
 *              - Find specific nodes
 *              - Delete specific nodes
 *              - Print the graph
 *
 */
//Dependence Graph
class Graph {
private:
		//Graph nodes
		std::set<GraphNode*> nodes;							//List of nodes of the graph
		llvm::DenseMap<Value*, GraphNode*> opNodes;			//Subset of nodes
        llvm::DenseMap<Value*, GraphNode*> callNodes;		//Subset of opnodes
        llvm::DenseMap<Value*, GraphNode*> varNodes;		//Subset of nodes
        llvm::DenseMap<int, GraphNode*> memNodes;			//Subset of nodes
        llvm::DenseMap<GraphNode*, BackNode*> backNodes;	//Subset of nodes

		//Navigation through subgraphs
		Graph* parentGraph;							//Graph that has originated this graph
		std::map<GraphNode*, GraphNode*> nodeMap;	//Correspondence of nodes between graphs

		//Graph analysis - Strongly connected components
		std::map<int, std::set<GraphNode*> > sCCs;
		std::map<GraphNode*, int> reverseSCCMap;
		std::list<int> topologicalOrderedSCCs;



		AliasSets *AS;

        bool isValidInst(Value *v); //Return true if the instruction is valid for dependence graph construction
        bool isMemoryPointer(Value *v); //Return true if the value is a memory pointer

public:
        typedef std::set<GraphNode*>::iterator iterator;

        std::set<GraphNode*>::iterator begin();
        std::set<GraphNode*>::iterator end();

        Graph(AliasSets *AS) :
        	parentGraph(NULL), AS(AS){
            NrEdges = 0;
        }
        ; //Constructor
        ~Graph(); //Destructor - Free adjacent matrix's memory
        GraphNode* addInst(Value *v); //Add an instruction into Dependence Graph

        void removeNode(GraphNode* target);

        void addEdge(GraphNode* src, GraphNode* dst, edgeType type = etData);
        void removeEdge(GraphNode* src, GraphNode* dst);

        GraphNode* findNode(Value *op); //Return the pointer to the node or NULL if it is not in the graph
        GraphNode* findNode(GraphNode* node); //Return the pointer to the node or NULL if it is not in the graph

        std::set<GraphNode*> findNodes(std::set<Value*> values);

        OpNode* findOpNode(Value *op); //Return the pointer to the node or NULL if it is not in the graph

        //print graph in dot format
        class Guider {
        public:
                std::string getNodeAttrs(GraphNode* n);
                std::string getEdgeAttrs(GraphNode* u, GraphNode* v);
                void setNodeAttrs(GraphNode* n, std::string attrs);
                void setEdgeAttrs(GraphNode* u, GraphNode* v, std::string attrs);
                void clear();
        private:
                DenseMap<GraphNode*, std::string> nodeAttrs;
                DenseMap<std::pair<GraphNode*, GraphNode*>, std::string> edgeAttrs;
        };

        void toDot(std::string s); //print in stdErr
        void toDot(std::string s, std::string fileName); //print in a file
        void toDot(std::string s, raw_ostream *stream); //print in any stream
        void toDot(std::string s, raw_ostream *stream, llvm::Graph::Guider* g);



        //Creates an entirely new graph, with equivalent nodes and edges
        Graph* clone();

        //Creates a sub-graph that contains only the selected nodes given as argument
        Graph makeSubGraph(std::set<GraphNode*> nodeList);

        Graph generateSubGraph(Value *src, Value *dst); //Take a source value and a destination value and find a Connecting Subgraph from source to destination
        Graph generateSubGraph(GraphNode* src, GraphNode* dst);
        Graph generateSubGraph(int SCCID); //Generate sub graph containing only the selected SCC


        //Depth-First visiting methods (used for deciding reachability)
        void dfsVisit(GraphNode* u, std::set<GraphNode*> &visitedNodes);
        void dfsVisitBack(GraphNode* u, std::set<GraphNode*> &visitedNodes);
        void dfsVisitBack_ext(GraphNode* u, std::set<GraphNode*> &visitedNodes, std::map<int, GraphNode*> &firstNodeVisitedPerSCC);



        void deleteCallNodes(Function* F);

        /*
         * Function getNearestDependence
         *
         * Given a sink, returns the nearest source in the graph and the distance to the nearest source
         */
        std::pair<GraphNode*, int> getNearestDependency(Value* sink,
                        std::set<Value*> sources, bool skipMemoryNodes);

        /*
         * Function getEveryDependency
         *
         * Given a sink, returns shortest path to each source (if it exists)
         */
        std::map<GraphNode*, std::vector<GraphNode*> > getEveryDependency(
                        llvm::Value* sink, std::set<llvm::Value*> sources,
                        bool skipMemoryNodes);


        // Statistics of the Graph
        int getNumOpNodes();
        int getNumCallNodes();
        int getNumMemNodes();
        int getNumVarNodes();
        int getNumBackNodes();
        int getNumDataEdges();
        int getNumControlEdges();
        int getNumEdges(edgeType type);

        std::list<GraphNode*> getNodesWithoutPredecessors();
        std::set<std::pair<GraphNode*, GraphNode*> > getBackEdges();

        void unifyBackEdges();
        void removeBackNodes();

        Graph* getParentGraph();

        llvm::DenseMap<GraphNode*, BackNode*> getBackNodes() {return backNodes;};

        void strongconnect(GraphNode* node,
        		           std::map<GraphNode*, int> &index,
        		           std::map<GraphNode*, int> &lowlink,
        		           int &currentIndex,
        		           std::stack<GraphNode*> &S,
        		           std::set<GraphNode*> &S2,
        		           std::map<int, std::set<GraphNode*> > &SCCs);


        /*
         * Methods to handle SCCs in the Dependence Graph
         */
        void recomputeSCCs();
        std::map<int, std::set<GraphNode*> > getSCCs();
        std::list<int> getSCCTopologicalOrder();

        int getSCCID(GraphNode* node);
        std::set<GraphNode*> getSCC(int ID);

        void dumpSCCs();

        bool acyclicPathExists(GraphNode* src,
        		GraphNode* dst,
                std::set<GraphNode*> alreadyVisitedNodes,
                int SCCID);

        bool hasNestedLoop(int SCCID);
        bool hasNestedLoop(GraphNode* first);

        void getAcyclicPaths_rec(GraphNode* dst,
        		                 std::set<GraphNode*> &visitedNodes,
        		                 std::stack<GraphNode*> &path,
        		                 std::set<std::stack<GraphNode*> > &result,
        		                 int SCCID);

        std::set<std::stack<GraphNode*> > getAcyclicPaths(GraphNode* src, GraphNode* dst);
        std::set<std::stack<GraphNode*> > getAcyclicPathsInsideSCC(GraphNode* src, GraphNode* dst);

};

/*
 * Class functionDepGraph
 *
 * Function pass that provides an intraprocedural dependency graph
 *
 */
class functionDepGraph: public FunctionPass {
public:
        static char ID; // Pass identification, replacement for typeid.
        functionDepGraph() :
                FunctionPass(ID), depGraph(NULL) {
        }
        void getAnalysisUsage(AnalysisUsage &AU) const;
        bool runOnFunction(Function&);

        Graph* depGraph;
};

/*
 * Class moduleDepGraph
 *
 * Module pass that provides a context-insensitive interprocedural dependency graph
 *
 */
class moduleDepGraph: public ModulePass {
public:
        static char ID; // Pass identification, replacement for typeid.
        moduleDepGraph() :
                ModulePass(ID), depGraph(NULL) {
        }
        void getAnalysisUsage(AnalysisUsage &AU) const;
        bool runOnModule(Module&);

        void matchParametersAndReturnValues(Function &F);
        void deleteCallNodes(Function* F);

        Graph* depGraph;
};


class ViewModuleDepGraph: public ModulePass {
public:
        static char ID; // Pass identification, replacement for typeid.
        ViewModuleDepGraph() :
                ModulePass(ID) {
        }

        void getAnalysisUsage(AnalysisUsage &AU) const {
                AU.addRequired<moduleDepGraph> ();
                AU.setPreservesAll();
        }

        bool runOnModule(Module& M) {

                moduleDepGraph& DepGraph = getAnalysis<moduleDepGraph> ();
                Graph *g = DepGraph.depGraph;

                std::string tmp = M.getModuleIdentifier();
                replace(tmp.begin(), tmp.end(), '\\', '_');

                std::string Filename = "/tmp/" + tmp + ".dot";

                //Print dependency graph (in dot format)
                g->toDot(M.getModuleIdentifier(), Filename);

                DisplayGraph(Filename, true, GraphProgram::DOT);

                return false;
        }
};





}

#endif //DEPGRAPH_H_
