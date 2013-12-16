#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "BP.h"
#include <cmath>

using namespace llvm;

char BP::ID = 0;
static RegisterPass<BP> X("bp", "BP Pass", false, false);

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
          inst_map_set inst_map = dual_dg_map(dg_instr_map);
          Partitions[L] = build_partition(L, inst_map); 
          dumpBP(L);
//          Partitions.insert(std::pair<Loop*, inst_vec_vec> (L, build_partition(L, dg_instr_map))); 
        }
      }
    } else {
	  for (std::vector<Loop*>::iterator it = subLoops.begin() ; it != subLoops.end(); ++it) {
        OutputBP(*it);
      }
	}
}

bool BP::runOnFunction(Function &F) {
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

inst_map_set BP::dual_dg_map(inst_map_set dg_inst_map){
  inst_map_set dual_map = dg_inst_map;
  inst_set instset;
  inst_map_set::iterator it;
  inst_set::iterator idx;
  
  for (it = dg_inst_map.begin(); it != dg_inst_map.end(); it++){
    instset = it->second;
    for (idx = instset.begin(); idx != instset.end(); idx++){
      Instruction *inst = *idx;
      dg_inst_map[inst].insert (it->first);
    }
  }
  
  return dg_inst_map;
  
}
inst_vec_vec BP::build_partition(Loop *CurL, inst_map_set CurInstMapSet){
//  int NumOfNode = depmap->numOfNodes[CurL];
//  std::vector<inst_visit> visit_flag;
  inst_set all_insts;
  inst_visit tmp_vf;
  inst_visit *visited = &tmp_vf;
  inst_vec_vec partition;
  std::map<Instruction*, std::set<Instruction*> >::iterator it, idx;
   
  //initialization
  for(it = CurInstMapSet.begin(); it != CurInstMapSet.end(); ++it){
    Instruction* curInstr = it->first;
    tmp_vf.insert (std::pair<Instruction*, bool> (curInstr, false));
    all_insts.insert(curInstr);
  }
  
  //recursive dfs
  for(idx = CurInstMapSet.begin(); idx != CurInstMapSet.end(); ++idx){
    Instruction* curInstr = idx->first;
    if(tmp_vf[curInstr]) continue;
    partition.push_back(dfs(curInstr, CurInstMapSet, all_insts, visited));
  } 
  
  return partition;
  // apply heurstics
//  return check_partition(partition);
}

inst_vec BP::dfs(Instruction *start_inst, inst_map_set dg_of_loop, inst_set all_insts, inst_visit *visited){
  inst_vec group;
  inst_set dep_insts = dg_of_loop[start_inst];  //all insts that start_inst related to
  inst_set::iterator it;
  //checking if duplicate
  group.push_back(start_inst);
  errs() << "start_inst: " << *start_inst << "\n";
  for (it = dep_insts.begin(); it != dep_insts.end(); it++){
    Instruction *inst = *it;
    errs() << "visited_inst: " << *inst << "\t";
    errs() << !(*visited)[inst] << "\n";
    if (!(*visited)[inst]) { // not visited
      (*visited)[inst] = true;
      inst_vec new_insts = dfs(inst, dg_of_loop, all_insts, visited);
      //remove duplicates
      for (inst_vec::iterator iter0 = new_insts.begin(); iter0 != new_insts.end(); iter0++){
        for (inst_vec::iterator iter1 = group.begin(); iter1 != group.end(); iter1++){
          if (*iter0 == *iter1){
            errs() << "duplicate_inst: " << *iter0 << "\n";
            new_insts.erase(iter0);
            break;
          }
        }
      }
      group.insert(group.end(), new_insts.begin(), new_insts.end());
    }
  }
  return group;
} 

void BP::dumpBP(Loop *L){
  inst_vec_vec sccs = Partitions[L];
  for (unsigned int i = 0; i < sccs.size(); i++){
    errs() << "scc No. :" << i << "\n";
    for (unsigned int j =0; j < sccs[i].size(); j++)
      errs() << *sccs[i][j] << "\n";
  }
}

inst_vec_vec BP::check_partition(inst_vec_vec old_scc){
  inst_vec_vec new_sec;
  int scc_no = old_scc.size(); 
  int all_scc_no = pow(2, scc_no);
//  int *size = new (int) (sizeof(int)*scc_no);   // no_inst for sccs
  int size[scc_no];   // no_inst for sccs
  double scores[all_scc_no];
  double max_score = 0;
  int best_scc;
  int i;
  /*
    sequence of iterating all possible sccs:
    suppose we have scc_no of 5, which means scc 0, 1, 2, 3, and 4.
    scc_no [0] = cut all (original);
    scc_no [1->4] = merge 0->1, 1->2, ...
    scc_no [5->10] = merge 0->1&1->2, 0->1&2->3, ...
    scc_no [11->14] = merge 0->1&1->2&2->3, 0->1&1->2&3->4, ...
    scc_no 15 = merge all
  */
  for (int i = 0; i < scc_no; i++)
    size[i] = old_scc[i].size();
} 

double cache_score (std::vector<int>); 


int NumHeaderInst(Loop *L)
{
 BasicBlock *HB = L->getHeader(); //Header Block
 int HeaderInstrCnt=0;
 for(BasicBlock::iterator it=HB->begin();it!=HB->end();++it)
     HeaderInstrCnt++;
 
 return HeaderInstrCnt; 
}
