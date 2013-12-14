/*
This is a Hello pass that demonstrates how to read the analysis results from
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
#include "DG.h"

using namespace llvm;

namespace {
  struct Hello : public FunctionPass {
    static char ID;
	LoopInfo *LI;
	DG *depmap;
    Hello() : FunctionPass(ID) {}
    virtual void outputDG(Loop *L) {
      std::vector<Loop*> subLoops = L->getSubLoops();
	  if (subLoops.empty()) { // analysis only applies to innermost loops, so check for that
      errs() << "Hello: printing the data from DG pass for ";
      errs() <<*L << '\n';
      if (depmap->dgOfLoops.count(L) == 0) {
        errs() << "Hello: No analysis found\n";
      } else {
        std::map<Instruction*, std::set<Instruction*> > dg_temp = depmap->dgOfLoops[L];
        if (dg_temp.empty()) {
          errs() << "Hello: Dependence graph is empty\n";
        } else {
          std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
          for (mapit2 = dg_temp.begin(); mapit2 != dg_temp.end(); ++mapit2) {
            Instruction *inst = mapit2->first;
            errs() << "\t\tHello: Instruction " << *inst << "\n";
            std::set<Instruction*> set_temp = mapit2->second;
            std::set<Instruction*>::iterator setit;
            for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
              errs() << "\t\t\t\t Hello: -------->" << *(*setit) << "\n";
            }
          }
          errs() << "Hello: There are " << depmap->numOfNodes[L] << " nodes in the loop\n";
          errs() << "Hello: There are " << depmap->numOfDeps[L] << " data dependencies in the loop\n";
        }
      }
    } else {
	  for (std::vector<Loop*>::iterator it = subLoops.begin() ; it != subLoops.end(); ++it) {
        outputDG(*it);
      }
	}
	}

    virtual bool runOnFunction(Function &F) {
	  LI = &getAnalysis<LoopInfo>();
	  depmap = &getAnalysis<DG>();
	  Loop *curLoop;
	  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
	    curLoop = *i;
		if (curLoop->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration
		outputDG(curLoop);
      }
      return false;
    }
	void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
	  AU.addRequired<DG>();
      //AU.addPreserved<LoopInfo>();
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass", false, false);

