#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "BP.h"

using namespace llvm;

char BP::ID = 0;
static RegisterPass<BP> X("BP", "BP Pass", false, false);

void BP::OutputBP(Loop *L) {
	
      std::vector<Loop*> subLoops = L->getSubLoops();
	  if (subLoops.empty()) { // analysis only applies to innermost loops, so check for that
      errs() << "scc: printing the data from DG pass for ";
      errs() <<*L << '\n';
      if (depmap->dgOfLoops.count(L) == 0) {
        errs() << "scc: No analysis found\n";
      } else {
//        std::map<Instruction*, std::set<Instruction*> > dg_temp = depmap->dgOfLoops[L];
              inst_map_set dg_instr_map = depmap->dgOfLoops[L];
        if (dg_instr_map.empty()) {
          errs() << "Hello: Dependence graph is empty\n";
        } else {
	////////////NOTICE//////////
          build_partition(L,dg_instr_map); 
        }
      }
    } else {
	  for (std::vector<Loop*>::iterator it = subLoops.begin() ; it != subLoops.end(); ++it) {
        OutputBP(*it);
      }
	}

}


bool BP:: runOnFunction(Function &F) {
	  LI = &getAnalysis<LoopInfo>();
	  depmap = &getAnalysis<DG>();
	  Loop *curLoop;
	  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
	    curLoop = *i;
		if (curLoop->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration
		OutputBP(curLoop);
      }
      return false;
    }

void BP::getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
      AU.addRequired<DG>();
    }


void BP::build_partition(Loop *CurL, inst_map_set CurInstMapSet){
int NumOfNode = depmap->numOfNodes[CurL];
inst_visit visit_flag;
for(int Iidx=0;Iidx<NumOfNode;Iidx++)
  visit_flag[Iidx]=false;

for(int Iidx=0;Iidx<NumOfNode;Iidx++)
{
  Instruction* curInstr = 
}

}
