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

#include "BP.h"
#include <stack>


/* Loop is a child class of LoopBase
 * There are many useful methods defined in LoopBase
 */

using namespace llvm;

namespace {
    class SplitPass2: public FunctionPass {
        private:
            ProfileInfo* PI;
            LoopInfo *LI;
            BP *bpmap;
            

        public:
            static char ID;
            SplitPass2() : FunctionPass(ID) {}

            bool runOnFunction(Function &F);
            void getAnalysisUsage(AnalysisUsage &) const;
            void remapInstruction(Instruction *I,  ValueToValueMapTy &VMap);
            void fillOrder(Instruction* Inst, std::map<Instruction*, bool> visited, 
                    std::stack<Instruction*> &Stack, 
                    std::map<Instruction*, std::set<Instruction*> > &Map);
            void DFSUtil(Instruction* Inst, 
                    std::map<Instruction*, bool> visited, 
                    std::map<Instruction*, std::set<Instruction*> > &Map,
                    std::vector<Instruction*> new_scc);
            bool scc_contains(std::vector<Instruction*> &list, Instruction *Inst);

            
            // clone the given loop, L 
            Loop *CreateOneLoop(Loop *L, std::map<Instruction*, Instruction*> &instMap);
    };
}

// Add it to command line 
char SplitPass2::ID = 0;
static RegisterPass<SplitPass2> X("splitpass2", "loop fission project", 
        false,    // only looks at CFG?
        true);    // Transform Pass should be ture 


// CreateOneLoop should take the current Loop 
Loop * SplitPass2::CreateOneLoop(Loop *L, std::map<Instruction*, Instruction*>  &instMap) {
    DEBUG(dbgs() << "Running CreateOneLoop\n");
    // TODO check if only one exit block.
    // TODO check if save to clone.

    BasicBlock *Header = L->getHeader();
    std::vector< BasicBlock * > body = L->getBlocks();
    BasicBlock *PreHeader = L->getLoopPredecessor(); 

    BasicBlock *Exit = L->getExitBlock(); 
    ValueToValueMapTy LastValueMap;
    std::vector<BasicBlock *> new_body;

    for(std::vector<BasicBlock * >::iterator it = body.begin(); it != body.end(); ++it) {
        ValueToValueMapTy VMap;
        BasicBlock *New = CloneBasicBlock(*it, VMap, ".copied");
        Header->getParent()->getBasicBlockList().push_back(New);
        new_body.push_back(New);

        // add to instMap
        std::vector<Instruction *> new_insts;
        for (BasicBlock::iterator I = New->begin(); I != New->end(); ++I) {
            new_insts.push_back(I);
        }
        std::vector<Instruction *> old_insts;
        for (BasicBlock::iterator I = (*it)->begin(); I != (*it)->end(); ++I) {
            old_insts.push_back(I);
        }

        for (unsigned i=0; i< new_insts.size(); ++i) {
            instMap[new_insts[i]] = old_insts[i];
        }

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

void SplitPass2::remapInstruction(Instruction *I,  ValueToValueMapTy &VMap) {
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

bool SplitPass2::runOnFunction(Function &F) {
    PI = &getAnalysis<ProfileInfo>();
    LI = &getAnalysis<LoopInfo>();
    bpmap = &getAnalysis<BP>();

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
        std::vector<std::vector<Instruction*> > sccs = bpmap->Partitions[L];
        std::vector<Loop*> loops; 
        std::vector<std::map<Instruction*, Instruction*> > loops_map;

        int sccs_size = sccs.size()-2; 
        if (sccs_size <= 1) continue;

        DEBUG(dbgs() << " SSC count: " << sccs_size <<" \n" );

        // split 

        loops.push_back(L);
        std::map<Instruction*, Instruction*> temp_map;
        // copy sccs_size - 1 times, we will use original loop 
        for (int i=0; i<sccs_size-1; ++i) {
            temp_map.clear();
            Loop* lp = CreateOneLoop(L, temp_map);
            if (lp) {
                LI->addTopLevelLoop(lp); 

                loops.push_back(lp);
                loops_map.push_back(temp_map);
            }
        }



        DEBUG(dbgs() << " copy done\n" );
        DEBUG(dbgs() << " loops_Map size " << loops_map.size() << "\n" );
        DEBUG(dbgs() << " loops_Map[1] size " << loops_map[1].size() << "\n" );
//        for( std::map<Instruction*, Instruction*>::iterator iterator = loops_map[1].begin(); iterator != loops_map[1].end(); iterator++) {
//            iterator->first->dump();
//            DEBUG(dbgs() << " ----  \n" );
//            iterator->second->dump();
//            // Repeat if you also want to iterate through the second map.
//        }
        
        std::vector< BasicBlock * > body;
        BasicBlock * Head;
        BasicBlock * Latch;

        Instruction * tempInst;
      
        // start form index 1 because original loop(scc) is processed
        for (unsigned i=0; i<sccs_size; ++i) {
            DEBUG(dbgs() << " scc NO. " << i << " Started \n" );
            body = loops[i]->getBlocks();
            Head = loops[i]->getHeader();
            Latch = loops[i]->getLoopLatch();

            // loop throught BBs in this loop
            for(int body_i=body.size()-1; body_i>=0; --body_i) {
                if (body[body_i] == Head) continue; // skip header and latch insts


                BasicBlock::iterator BBI = body[body_i]->begin(); 
                BasicBlock::iterator BBIE = body[body_i]->end();
                if (body_i == body.size()-1) {
                    // Latch block
                    BBIE--;    
                    BBIE--;    
                }

                std::vector<Instruction *> insts;
                for ( ; BBI != BBIE; ++BBI)  {
                    insts.push_back(BBI);
                }
                for (int j=insts.size()-1; j>=0; --j) {
                    // if sccs[i] doens't have this inst, remove it from loop[i]
                    if (i>0) {
                        // if i > 0
                        tempInst = loops_map[i-1][insts[j]];
                    } else {
                        // if i == 0
                        tempInst = insts[j];
                    }
                    if (!scc_contains(sccs[i], tempInst)) {
                        DEBUG(dbgs() << " scc NO. " << i << " erasing \n" );
                        insts[j]->dump();
                        insts[j]->eraseFromParent();
                    }
                }
            }
            DEBUG(dbgs() << " scc NO. " << i << " Finished -------  \n" );
        }
    }

    loopnum = 0;
    for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
        loopnum++;
    }
    DEBUG(dbgs() << "Loop num: " << loopnum << "\n") ;
    return true; // if code is chaged, should return true
}

bool SplitPass2::scc_contains(std::vector<Instruction*> &list, Instruction *Inst) {
    for(std::vector<Instruction*>::iterator it = list.begin(); it != list.end(); ++it) {
        if (*it == Inst) {
            return true;
        } 
    }
    return false;
}

// Load other analysis
void SplitPass2::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfo>();
    AU.addRequired<ProfileInfo>();
    AU.addRequired<BP>();
} 


