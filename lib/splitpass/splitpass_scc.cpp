#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"

#include "llvm/Transforms/Utils/Cloning.h" // inorder to include "ValueToValueMapTy"
#include "llvm/Transforms/Utils/BasicBlockUtils.h" // splitEdge, etc
#include "llvm/Transforms/Utils/Local.h"

#include "DG.h"
#include <stack>


/* Loop is a child class of LoopBase
 * There are many useful methods defined in LoopBase
 */

using namespace llvm;

namespace {
    class SplitPassSCC: public FunctionPass {
        private:
            ProfileInfo* PI;
            LoopInfo *LI;
            DG *depmap;
            

        public:
            static char ID;
            SplitPassSCC() : FunctionPass(ID) {}

            bool runOnFunction(Function &F);
            void getAnalysisUsage(AnalysisUsage &) const;
            void remapInstruction(Instruction *I,  ValueToValueMapTy &VMap);
            void fillOrder(Instruction* Inst, std::map<Instruction*, bool> visited, 
                    std::stack<Instruction*> &Stack, 
                    std::map<Instruction*, std::set<Instruction*> > &Map);
            void DFSUtil(Instruction* Inst, 
                    std::map<Instruction*, bool> visited, 
                    std::map<Instruction*, std::set<Instruction*> > &Map);

            
            // clone the given loop, L 
            Loop *CreateOneLoop(Loop *L);
    };
}

// Add it to command line 
char SplitPassSCC::ID = 0;
static RegisterPass<SplitPassSCC> X("splitpass_scc", "loop fission project", 
        false,    // only looks at CFG?
        true);    // Transform Pass should be ture 


// CreateOneLoop should take the current Loop 
Loop * SplitPassSCC::CreateOneLoop(Loop *L) {
    DEBUG(dbgs() << "Running CreateOneLoop\n");
    // TODO check if only one exit block.
    // TODO check if save to clone.

    BasicBlock *Header = L->getHeader();
    std::vector< BasicBlock * > body = L->getBlocks();
    DEBUG(dbgs() << "Test \n");
    BasicBlock *PreHeader = L->getLoopPredecessor(); 

    BasicBlock *Exit = L->getExitBlock(); 
    ValueToValueMapTy LastValueMap;
    std::vector<BasicBlock *> new_body;

    for(std::vector<BasicBlock * >::iterator it = body.begin(); it != body.end(); ++it) {
        ValueToValueMapTy VMap;
        BasicBlock *New = CloneBasicBlock(*it, VMap, ".copied");
        Header->getParent()->getBasicBlockList().push_back(New);
        new_body.push_back(New);

        // Update our running map of newest clones
        LastValueMap[*it] = New;
        for (ValueToValueMapTy::iterator VI = VMap.begin(), VE = VMap.end(); VI != VE; ++VI) {
            LastValueMap[VI->first] = VI->second;
        }
    }
    // new_body[0] is Header new_body(new_body.size()-1) is Latch
    
    // change vars
    for(unsigned i=0; i<new_body.size(); ++i) {
        for (BasicBlock::iterator I = new_body[i]->begin(); I != new_body[i]->end(); ++I) {
            remapInstruction(I, LastValueMap);
        }
    }
    DEBUG(dbgs() << "Cloned loop's register name changed ok\n");

    // fix the phi node in cloned Loops Header  
    for (BasicBlock::iterator I = new_body[0]->begin(); isa<PHINode>(I); ++I) {
        PHINode *NewPHI = cast<PHINode>(I); 

        for (unsigned i=0; i<NewPHI->getNumIncomingValues(); ++i) {
            if (NewPHI->getIncomingBlock(i) == PreHeader) {
                NewPHI->setIncomingBlock(i, Header);
            }
        }
    }
    DEBUG(dbgs() << "Start working on exit block \n");
    // fix the phi node in original Loop's exit BB. Because the cloned loop points to it.
    for (BasicBlock::iterator I = Exit->begin(); isa<PHINode>(I); ++I) {
        PHINode *NewPHI = cast<PHINode>(I);

        for (unsigned i=0; i<NewPHI->getNumIncomingValues(); ++i) {
            if (NewPHI->getIncomingBlock(i) == Header) {
                NewPHI->setIncomingBlock(i, new_body[0]); // cloned loop's Header
            }
        }
        DEBUG(dbgs() << "Exit block, Done phi node: "<<  NewPHI << "\n");
    }

    DEBUG(dbgs() << "All Phi Node done \n");



    BranchInst *new_back_edge = cast<BranchInst>(new_body[new_body.size()-1]->getTerminator());   // Latch's last inst is branch
    new_back_edge->setSuccessor(0, new_body[0]); // Set to branch to new Cond (Header) 1st BB

    // link new loop body together
    for(unsigned i=0; i<new_body.size()-1; i++) {
        BranchInst *next = cast<BranchInst>(new_body[i]->getTerminator());   
        next->setSuccessor(0, new_body[i+1]);
    }

    // first cond exit points to second cond block
    BranchInst *first_loop_to_next_loop = cast<BranchInst>(Header->getTerminator());
    for(unsigned i=0; i< first_loop_to_next_loop->getNumSuccessors(); ++i) { 
        if (first_loop_to_next_loop->getSuccessor(i) == Exit) {
            first_loop_to_next_loop->setSuccessor(i, new_body[0]);
        }
    }

    Loop *new_loop = new Loop();

    for (unsigned i=0; i<new_body.size();i++) {
        new_loop->addBlockEntry(new_body[i]);
    }
    new_loop->moveToHeader(new_body[0]); // set Header for new loop 
    
    return new_loop;
}

void SplitPassSCC::remapInstruction(Instruction *I,  ValueToValueMapTy &VMap) {
    for (unsigned op = 0, E = I->getNumOperands(); op != E; ++op) {
        Value *Op = I->getOperand(op);
        ValueToValueMapTy::iterator It = VMap.find(Op);
        if (It != VMap.end()) {
            I->setOperand(op, It->second);
        }
    }

    if (PHINode *PN = dyn_cast<PHINode>(I)) {
        for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
            ValueToValueMapTy::iterator It = VMap.find(PN->getIncomingBlock(i));
            if (It != VMap.end())
                PN->setIncomingBlock(i, cast<BasicBlock>(It->second));
        }
    }
}

bool SplitPassSCC::runOnFunction(Function &F) {
    PI = &getAnalysis<ProfileInfo>();
    LI = &getAnalysis<LoopInfo>();
    depmap = &getAnalysis<DG>();

    int loopnum;

    loopnum = 0;
    for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
        loopnum++;
    }
    DEBUG(dbgs() << "Loop num: " << loopnum << "\n") ;


    for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
        Loop *L = *i;
        if (L->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration


        // scc
        std::map<Instruction*, std::set<Instruction*> > dg_temp = depmap->dgOfLoops[L];
        std::vector<std::vector<Instruction*> > scc;

        std::stack<Instruction*> Stack;
        unsigned V = scc.size(); // hard code TODO use SCC map 

        std::map<Instruction*, bool> visited;

        for (std::map<Instruction*, std::set<Instruction*> >::iterator it=dg_temp.begin();
                it!=dg_temp.end(); ++it) {

            visited[it->first] = false;
        }
        DEBUG(dbgs() << "---------- fillOrder ------------\n") ;
        for (std::map<Instruction*, std::set<Instruction*> >::iterator it=dg_temp.begin();
                it!=dg_temp.end(); ++it) {

            if (visited[it->first] == false) {
                fillOrder(it->first, visited, Stack, dg_temp);
            }
        }

        DEBUG(dbgs() << "---------- reverse ------------\n") ;
        std::map<Instruction*, std::set<Instruction*> > dg_reverse;

        for (std::map<Instruction*, std::set<Instruction*> >::iterator it=dg_temp.begin();
                it!=dg_temp.end(); ++it) {
            
            Instruction * Inst = it->first;
            std::set<Instruction*> temp_set = it->second;
            for (std::set<Instruction*>::iterator i=temp_set.begin(); 
                    i!=temp_set.end(); ++i) {
                dg_reverse[*i].insert(Inst);
            }
        }

        for (std::map<Instruction*, std::set<Instruction*> >::iterator it=dg_temp.begin();
                it!=dg_temp.end(); ++it) {

            visited[it->first] = false;
        }

        while (Stack.empty() == false) {
            Instruction* Inst = Stack.top();
            Stack.pop();

            if (visited[Inst] == false) {
                DFSUtil(Inst, visited, dg_reverse);
                DEBUG(dbgs() << "---------- SSC ------------\n") ;
            }
        }


        //// split
        //for (unsigned i=0; i<V; ++i) {
        //    Loop* lp = CreateOneLoop(L);
        //    if (lp) {
        //        LI->addTopLevelLoop(lp); 
        //    }
        //}
        //// break; // TODO work on first loop of a function only
    }


    loopnum = 0;
    for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
        loopnum++;
    }
    DEBUG(dbgs() << "Loop num: " << loopnum << "\n") ;
    return true; // if code is chaged, should return true
}

void SplitPassSCC::DFSUtil(Instruction* Inst, std::map<Instruction*, bool> visited, 
                    std::map<Instruction*, std::set<Instruction*> > &Map)
{
    // Mark the current node as visited and print it
    visited[Inst] = true;

    // add Inst to SSC 1
    DEBUG(dbgs() << "Inst: ");
    Inst->dump();
 
    // Recur for all the vertices adjacent to this vertex
    for (std::set<Instruction*>::iterator it=Map[Inst].begin(); 
            it!=Map[Inst].end(); ++it) {
        if(!visited[*it]) {
            DFSUtil(*it, visited, Map);
        }
    }
}

void SplitPassSCC::fillOrder(Instruction* Inst, 
        std::map<Instruction*, bool> visited, 
        std::stack<Instruction*> &Stack, std::map<Instruction*, std::set<Instruction*> > &Map)
{
    // Mark the current node as visited and print it
    visited[Inst] = true;
 
    // Recur for all the vertices adjacent to this vertex
    for (std::set<Instruction*>::iterator it=Map[Inst].begin(); 
            it!=Map[Inst].end(); ++it) {
        if(!visited[*it]) {
            fillOrder(*it, visited, Stack, Map);
        }
    }
    // All vertices reachable from v are processed by now, push v to Stack
    Stack.push(Inst);
}

// Load other analysis
void SplitPassSCC::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfo>();
    AU.addRequired<ProfileInfo>();
    AU.addRequired<DG>();
} 


