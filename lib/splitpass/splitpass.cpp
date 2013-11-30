#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <ios>  
#include <fstream>
#include <iomanip>

/* Loop is a child class of LoopBase
 * There are many useful methods defined in LoopBase
 */

using namespace llvm;

namespace {
    struct SplitPass: public LoopPass {
        // init
        static char ID;

        //ProfileInfo* PI;
        //ProfileInfo::EdgeWeights edge_weights;

        SplitPass() : LoopPass(ID) {}

        // main run on every loop (start from inner to outter)
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {


            errs() <<"------------ preheader -----------\n";
            BasicBlock *preheader = L->getLoopPreheader();
            preheader->dump(); 
           

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

            // This will fetch Body, Condition, Inc (loop body basic blocks).
            errs() <<"\n------------ body -----------\n";
            std::vector< BasicBlock * > body = L->getBlocks();
            for(std::vector<BasicBlock * >::iterator it = body.begin(); it != body.end(); ++it) {
                    /* std::cout << *it; ... */
                dyn_cast<BasicBlock>(*it)->dump();
            }

            // L->getExitBlock will return exit BB

            return false; // TODO if code is chaged, should return true
        }

        // Load other analysis
        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<LoopInfo>();
        } 
    };
}

char SplitPass::ID = 0;
static RegisterPass<SplitPass> X("splitpass", "loop fission project", 
                                false,    // only looks at CFG?
                                false);   // TODO Transform Pass should be ture 
