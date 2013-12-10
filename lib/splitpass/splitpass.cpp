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
#include <ios>  
#include <fstream>
#include <iomanip>

#include "llvm/Transforms/Utils/Cloning.h" // inorder to include "ValueToValueMapTy"
#include "llvm/Transforms/Utils/BasicBlockUtils.h" // splitEdge, etc


/* Loop is a child class of LoopBase
 * There are many useful methods defined in LoopBase
 */

using namespace llvm;

namespace {
    /* Note about LLPassManager::insertLoop(Loop *L, Loop *ParentLoop)
     *
     * If ParentLoop is NULL, it will be inserted as a TopLevelLoop
     */

    class SplitPass: public LoopPass {
        private:
            ProfileInfo* PI;
            LoopInfo *LI;
            ValueToValueMapTy VMap;  // store old bb -> new bb  TODO maybe init out of this func

        public:
            static int count;
            static char ID;
            SplitPass() : LoopPass(ID) {}

            // main run on every loop (start from inner to outter)
            bool runOnLoop(Loop *L, LPPassManager &LPM);
            bool doInitialization(Loop *, LPPassManager &LPM);
            bool doFinalization();
            void getAnalysisUsage(AnalysisUsage &) const;
            void printLoop(Loop *L);
            void remapInstruction(Instruction *I,  ValueToValueMapTy &VMap);
            Loop *CreateOneLoop(Loop *L, LPPassManager *LPM);
    };
}

int SplitPass::count = 0;

// Add it to command line 
char SplitPass::ID = 0;
static RegisterPass<SplitPass> X("splitpass", "loop fission project", 
        false,    // only looks at CFG?
        false);   // TODO Transform Pass should be ture 


// CreateOneLoop should take the current Loop and a SSC List, and will create loop based on that
Loop * SplitPass::CreateOneLoop(Loop *L, LPPassManager *LPM) {
    DEBUG(dbgs() << "running createOneLoop\n");

    std::vector< BasicBlock * > body = L->getBlocks();
    BasicBlock *Header = L->getHeader();
    BasicBlock *PreHeader = L->getLoopPreheader(); 
    BasicBlock *LatchBlock = L->getLoopLatch();

    ValueToValueMapTy LastValueMap;

    std::vector<BasicBlock *> new_body;

    std::vector<PHINode*> OrigPHINode;

    // Testing PhiNode Methods
    //for (BasicBlock::iterator I = Header->begin(); isa<PHINode>(I); ++I) {
    //    PHINode *phi = cast<PHINode>(I);

    //    phi->dump();
    //    DEBUG(dbgs() << "\n");

    //    phi->getIncomingValueForBlock(LatchBlock)->dump();
    //    DEBUG(dbgs() << "\n");

    //    phi->getIncomingValueForBlock(PreHeader)->dump();
    //    DEBUG(dbgs() << "\n");


    //}
    
    //body.push_back(Header);
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
    DEBUG(dbgs() << "Change Var Done \n");


    for (BasicBlock::iterator I = new_body[0]->begin(); isa<PHINode>(I); ++I) {

        
        PHINode *NewPHI = cast<PHINode>(I); 
        DEBUG(dbgs() << "About to fix phi node: "<<  NewPHI << "\n");

        for (unsigned i=0; i<NewPHI->getNumIncomingValues(); ++i) {
            if (NewPHI->getIncomingBlock(i) == PreHeader) {
                NewPHI->setIncomingBlock(i, Header);
            }
        }
        DEBUG(dbgs() << "Done phi node: "<<  NewPHI << "\n");
        
        //for (unsigned phi_i = 0, e = OrigPHINode.size(); phi_i != e; ++phi_i) {
        //    PHINode *NewPHI = cast<PHINode>(VMap[OrigPHINode[phi_i]]);
        //    DEBUG(dbgs() << "About to fix phi node: "<<  NewPHI << "\n");
        //    NewPHI->setIncomingBlock(0, new_body[new_body.size()-1]);
        //    NewPHI->setIncomingBlock(1, Header);
        //}
    }
    DEBUG(dbgs() << "Phi Node loop done \n");


    BranchInst *new_back_edge = cast<BranchInst>(new_body[new_body.size()-1]->getTerminator());   // Latch's last inst is branch
    new_back_edge->setSuccessor(0, new_body[0]); // Set to branch to new Cond (Header) 1st BB

    // link new loop body together
    for(unsigned i=0; i<new_body.size()-1; i++) {
        BranchInst *next = cast<BranchInst>(new_body[i]->getTerminator());   
        next->setSuccessor(0, new_body[i+1]);
    }

    // first cond exit points to second cond block
    BranchInst *first_loop_to_next_loop = cast<BranchInst>(Header->getTerminator());
    DEBUG(dbgs() << "branch successors " << first_loop_to_next_loop->getNumSuccessors() << "\n");
    first_loop_to_next_loop->setSuccessor(1, new_body[0]);

    // first is last, last is first?
    Header->dump();

    for (unsigned i=0; i<new_body.size();i++) {
        new_body[i]->dump();
    }

    Loop *new_loop = new Loop();


    //LPM->insertLoop(new_loop, NULL); // second parameter means insert as TopLevelLoop
    return new_loop;
}

void SplitPass::remapInstruction(Instruction *I,  ValueToValueMapTy &VMap) {
    DEBUG(dbgs() << "Remapping an inst\n");
    for (unsigned op = 0, E = I->getNumOperands(); op != E; ++op) {
        Value *Op = I->getOperand(op);
        ValueToValueMapTy::iterator It = VMap.find(Op);
        if (It != VMap.end()) {
            DEBUG(dbgs() << "replacing register \n");
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

bool SplitPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    PI = &getAnalysis<ProfileInfo>();
    LI = &getAnalysis<LoopInfo>();

    if (count++ == 0) {
        return false;
    }

    Loop* lp = CreateOneLoop(L, &LPM);
    return true; // TODO if code is chaged, should return true
}

bool SplitPass::doInitialization(Loop *, LPPassManager &LPM) {
    return false;
}

bool SplitPass::doFinalization() {
    return true;
}

// Load other analysis
void SplitPass::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfo>();
    AU.addRequired<ProfileInfo>();
} 


