#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
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
     *
     */
     
    
    struct SplitPass: public LoopPass {
        // init
        static char ID;

        ProfileInfo* PI;
        LoopInfo *LI;
        //ProfileInfo::EdgeWeights edge_weights;
        
        ValueToValueMapTy VMap;  // store old bb -> new bb  TODO maybe init out of this func

        SplitPass() : LoopPass(ID) {}


        // CreateOneLoop should take the current Loop and a SSC List, and will create loop based on that
        static Loop *CreateOneLoop(Loop *L, LPPassManager *LPM) {

            Loop *new_loop = new Loop();

            BasicBlock *preheader_bb = L->getPreheader(); 
            BasicBlock *header_bb = L->getHeader(); // a reference to previous loop's cond (header) bb
            BasicBlock *exit_bb = L->getExitBlock(); // previous loop's exit

            // TODO update previous loop's exit to ? 
            //
            //
            //   Old          New 
            //    
            //   Preheader          Header -> Exit Branch
            //   Header             Copy of Old.header
            //   Body               Copy of Body
            //   Inc                Copy of Inc
            //   Exit new.header    Old.Exit

            std::vector< BasicBlock * > body = L->getBlocks();

            BranchInst *BI = dyn_cast<BranchInst>(header->getTerminator());
            BasicBlock BI->getSuccessor(1); // exit 

            BranchInst *new_entry_bb_bi = dyn_cast<BranchInst>(preheader->getTerminator());

             
            BasicBlock *new_bb = CloneBasicBlock(dyn_cast<BasicBlock>(*test_bb), VMap);
            BasicBlock *new_exit_bb = CloneBasicBlock(dyn_cast<BasicBlock>(*exit_bb), VMap);

            new_bb->getSuccessor(0) = new_exit_bb;


            // BasicBlock *testBB = SplitEdge(entrybb, entrybb->getSinglePredecessor, this);

            // addBlockEntry means add a basic block reference
            // new_loop->addBlockEntry(new_entrybb);
            // new_loop->addBlockEntry(new_exitbb);

            new_loop->addBlockEntry(new_bb);

            new_loop->addBlockEntry(new_exit_bb);

            
            for(std::vector<BasicBlock * >::iterator it = body.begin(); it != body.end(); ++it) {
             
                BasicBlock *new_bb = CloneBasicBlock(dyn_cast<BasicBlock>(*it), VMap);

                // cond, body, inc
                // cond taken points to new body.
                // cond exit points to original exit.
                // body branch to new inc.
                // inc branch to new cond

                // new_loop->addBlockEntry(new_bb);
            }
            


            LPM->insertLoop(new_loop, NULL); // second parameter means insert as TopLevelLoop
            return new_loop;
        }


        // main run on every loop (start from inner to outter)
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {
            PI = &getAnalysis<ProfileInfo>();
            LI = &getAnalysis<LoopInfo>();

            errs() <<"------------ preheader -----------\n";
            BasicBlock *preheader = L->getLoopPreheader();
            preheader->dump(); 

            errs() <<"\n------------ header -----------\n";
            BasicBlock *header = L->getHeader();
            header->dump(); 
           

            errs() <<"\n------------ parent -----------\n";
            Loop *parent = L->getParentLoop();
            if (parent == NULL) {
                errs() << "parent is null"; 
            }

            errs() <<"\n------------ subloop/children -----------\n";
            std::vector<Loop *> children_vec = L->getSubLoops();

            if (children_vec.empty()) {
                errs() << "children is null"; 
            }

            // latch is the basic block testing if loop ends
            errs() <<"\n------------ latch -----------\n";
            BasicBlock *latch = L->getLoopLatch();
            latch->dump();

            errs() <<"\n------------ header's/ for.cond successor (loop body)-----------\n";
            BranchInst *BI = dyn_cast<BranchInst>(header->getTerminator());
            BI->getSuccessor(0)->dump(); // taken, 

            errs() <<"\n------------ header's/ for.cond successor (loop exit)-----------\n";
            BI->getSuccessor(1)->dump(); // exit 
            
            // errs() <<"----------  num of successors of header's branch inst BI " << BI->getNumSuccessors() << "\n";


            errs() <<"\n------------ header's terminator inst -----------\n";
            BI->dump();


            // This will fetch Body, Condition, Inc (loop body basic blocks).
            errs() <<"\n------------ body -----------\n";
            std::vector< BasicBlock * > body = L->getBlocks();
            for(std::vector<BasicBlock * >::iterator it = body.begin(); it != body.end(); ++it) {
                    /* std::cout << *it; ... */
                dyn_cast<BasicBlock>(*it)->dump();
            }

            // L->getExitBlock will return exit BB
            errs() <<"\n------------ exit -----------\n";
            BasicBlock *exitbb = L->getExitBlock();
            exitbb->dump();

            //BasicBlock *testBB = llvm::SplitEdge(preheader, latch, this);


            Loop* lp = CreateOneLoop(L, &LPM);
            return false; // TODO if code is chaged, should return true
        }

        bool doFinalization() {
            //BasicBlock *preheader = L->getLoopPreheader();
            //BasicBlock *latch = L->getLoopLatch();

            //BasicBlock *testBB = SplitEdge(preheader, latch, this);
            return true;
        }

        // Load other analysis
        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<LoopInfo>();
            AU.addRequired<ProfileInfo>();
        } 
    };
}

char SplitPass::ID = 0;
static RegisterPass<SplitPass> X("splitpass", "loop fission project", 
                                false,    // only looks at CFG?
                                false);   // TODO Transform Pass should be ture 
