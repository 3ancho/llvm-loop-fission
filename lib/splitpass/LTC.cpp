/*
This is a LTC pass that demonstrates how to read the analysis results from
the DG pass. There are a few things:
1. #include "DG.h"
2. declare DG *depmap;
3. set depmap = &getAnalysis<DG>();
4. add AU.addRequired<DG>();
5. add hello pass after the dg pass when running the llvm optimization, see dg_test/test.sh.
   opt -load ../Release+Asserts/lib/splitpass.so -basicaa -da -dg -hello < INPUTFILE > /dev/null
*/

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ScalarEvolution.h"

using namespace llvm;

namespace {
  class LTC : public FunctionPass {
  private:
    LoopInfo *LI;
    ScalarEvolution *SE;
    
  public:
    static char ID;
    LTC() : FunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &) const; 
    bool runOnFunction(Function &F);
    void GetLoopTripCount(Loop *L);


  };
}

char LTC::ID = 0;
static RegisterPass<LTC> X("ltc", "loop trip count Pass", false, false);

void LTC::getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
      AU.addRequired<ScalarEvolution>();
    }


bool LTC::runOnFunction(Function &F) {
	  LI = &getAnalysis<LoopInfo>();
          SE = &getAnalysis<ScalarEvolution>();
	  Loop *curLoop;
	  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
	    curLoop = *i;
            if (curLoop->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration
	    GetLoopTripCount(curLoop);
      }
      return false;
     }

void LTC::GetLoopTripCount(Loop *L){
     if(L!=NULL)
     errs()<<"trip count is"<<(*SE->getBackedgeTakenCount(L))<<"\n";
}
