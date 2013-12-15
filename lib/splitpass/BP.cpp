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
  std::vector<inst_visit> visit_flag;
  inst_set all_insts;

  inst_visit tmp_vf;
  inst_map_set::iterator it;
  for(it=CurInstMapSet.begin();it!=CurInstMapSet.end();++it){
    Instruction* curInstr = it->first;
    tmp_vf->first = curInstr;
    tmp_vf->second = false;
    visit_flag.push_back(tmp_vf);
    all_insts.insert(curInstr);
  }

  int idx=0;
  for(it=CurInstMapSet.begin();it!=CurInstMapSet.end();++it,idx++){
    Instruction* curInstr = it->first;
    tmp_vf = visit_flag.at(idx);
    if(tmp_vf->second) continue;
    dfs(curInstr, CurInstMapSet, all_insts, &visit_flag);
  }  

inst_vec BP::dfs(Instruction *start_inst, inst_map_set dg_of_loop, inst_set all_insts, inst_visit *visited){
  inst_vec group;
  inst_set dep_insts = dg_of_loop[start_inst];  //all insts that start_inst related to
  group.push_back(start_inst);
  for (inst_set::iterator it = dep_insts.begin(); it != dep_insts.end(); it++)
    if (!(*visited)[it]) { // not visited
      (*visited)[it] = True;
      group.push_back(inst_vec(it, dg_of_loop, all_insts, *visited));
    }
  return group;
} 

void BP::dumpBP(*Loop L){
  inst_vec_vec sccs = Partitions[L];
  for (int i = sccs.begin(); i < sccs.end(); i++){
    errs() << "scc No. :" << i << "\n";
    for (int j = sccs[i].begin(); j < sccs[i].end(); j++)
      errs() << sccs[i] << "\n";
  }
}
