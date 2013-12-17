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
      errs() << "BP: printing the data from DG pass for ";
      errs() <<*L << '\n';
      if (depmap->dgOfLoops.count(L) == 0) {
        errs() << "BP: No analysis found\n";
      } else {
        inst_map_set dg_instr_map = depmap->dgOfLoops[L];
        inst_map_set dg_mem_map = depmap->dgOfLoopsMem[L];
        if (dg_instr_map.empty()) {
          errs() << "BP: Dependence graph is empty\n";
        } else {
//          if (depmap->ifLoopDist[L]){ // split
            inst_map_set inst_map = dual_dg_map(dg_instr_map);
            inst_vec_vec prdg = build_partition(L, inst_map);
            errs() << "PRDG: \n";
            dumpBP(prdg);
            Partitions[L] = build_scc(L, find_dual(dg_mem_map), prdg); 
//          }
//          else
//            Partitions[L] = convert(L); // keep all info, just convert data structure to loop_sccs and get rid of the last two instructions (%inc and br)
          errs() << "\nscc: \n";
          dumpBP(Partitions[L]);
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
}











inst_pair_set BP::find_dual(inst_map_set dg_mem_map){
  inst_pair_set duals;            //final result
  inst_map_set::iterator it, it1;
  inst_set::iterator idx0, idx1;
  for(it = dg_mem_map.begin(); it != dg_mem_map.end(); it++){ //iterate every inst in mem_map
    Instruction* first_inst = it->first;  
    errs() << "first_inst: ";
    errs() << *first_inst << "\n"; 
    inst_set first_set = it->second;    //set for that instruction
    for(idx0 = first_set.begin(); idx0 != first_set.end(); idx0++){
      Instruction* second_inst = *idx0;
      errs() << "second_inst: ";
      errs() << *second_inst << "\n"; 
      it1 = dg_mem_map.find(second_inst);  //find the instruction being checked in map
      if (it1 == dg_mem_map.end())
        continue;
      else  {
        inst_set second_set = it1->second;  //set of the checked instruction
        if (second_set.find(first_inst) != second_set.end()) {  //found the first inst in the set of the checked instruction: that's a dual link
    errs() << "found pair \n";
          duals.insert(std::make_pair(first_inst, second_inst));
          continue;
        }
      }
    }
  }
  return duals;
}

inst_vec_vec BP::build_scc(Loop *CurL, inst_pair_set dg_mem, inst_vec_vec prdg){
  inst_vec_vec scc = prdg;  //result
  inst_vec vec0, vec1;
  inst_pair_set::iterator it;
  inst_vec_vec::iterator idx0, idx1;
  std::set<std::set<inst_vec> > sets_of_merged_vecs;
  for(it = dg_mem.begin(); it != dg_mem.end(); it++){       //iterate pairs
    inst_pair pair = *it;
// find dual vertex
    for(idx0 = prdg.begin(); idx0 != prdg.end(); idx0++){   //iterate prdg
      inst_vec vec0 = *idx0;
      if (std::find(vec0.begin(), vec0.end(), pair.first) != vec0.end()) {
    //dual mem_dep in this vec(scc), find the second and record to merge
        for(idx1 = prdg.begin(); idx1 != prdg.end(); idx1++){ //iterate prdg
          inst_vec vec1 = *idx1;
          if (std::find(vec1.begin(), vec1.end(), pair.second) != vec1.end()) { //in this scc
            std::set<std::set<inst_vec> >::iterator set_iter;
            for (set_iter = sets_of_merged_vecs.begin(); set_iter != sets_of_merged_vecs.end(); set_iter++) {
              std::set<inst_vec> vec0_set = *set_iter; 
              if (vec0_set.find(vec0) != vec0_set.end()){  //if vec0 already has a dual link to others
                vec0_set.insert(vec1);   // insert vec1 to the set that contains vec0
                continue;
              }
              std::set<inst_vec> vec1_set = *set_iter; 
              if (vec1_set.find(vec1) != vec1_set.end()){  //if vec1 already has a dual link to others
                vec1_set.insert(vec0);
                continue;
              }
              std::set<inst_vec> merged_vecs;               // it's a new set 
              merged_vecs.insert(vec0);
              merged_vecs.insert(vec1);
              sets_of_merged_vecs.insert(merged_vecs);
            }
          }
        break;
        }
      }
    }
  }

  //merge
  std::set<std::set<inst_vec> >::iterator set_iter;
  for (set_iter = sets_of_merged_vecs.begin(); set_iter != sets_of_merged_vecs.end(); set_iter++) {
 errs() << "debug\n";
    std::set<inst_vec> merged_vecs = *set_iter; 
    std::set<inst_vec>::iterator idx;
    inst_vec new_vec;
    for (idx = merged_vecs.begin(); idx != merged_vecs.end(); idx++) {
      new_vec.insert(new_vec.end(), idx->begin(), idx->end());     //merge the vccs
      inst_vec to_delete = *idx;
      scc.erase(std::find(scc.begin(), scc.end(), to_delete));     //remove the original separated vccs
    }
    scc.push_back(new_vec);       //add the merged vcc
  }

  // apply heurstics
  if (HEURSTICS)
    return check_partition(scc, CurL);
  else 
    return scc;
}

inst_vec BP::dfs(Instruction *start_inst, inst_map_set dg_of_loop, inst_set all_insts, inst_visit *visited){
  inst_vec group;
  inst_set dep_insts = dg_of_loop[start_inst];  //all insts that start_inst related to
  inst_set::iterator it;
  //checking if duplicate
  group.push_back(start_inst);
//  errs() << "start_inst: " << *start_inst << "\n";
  for (it = dep_insts.begin(); it != dep_insts.end(); it++){
    Instruction *inst = *it;
//    errs() << "visited_inst: " << *inst << "\t";
//    errs() << !(*visited)[inst] << "\n";
    if (!(*visited)[inst]) { // not visited
      (*visited)[inst] = true;
      inst_vec new_insts = dfs(inst, dg_of_loop, all_insts, visited);
      //remove duplicates
      for (inst_vec::iterator iter0 = new_insts.begin(); iter0 != new_insts.end(); iter0++){
        for (inst_vec::iterator iter1 = group.begin(); iter1 != group.end(); iter1++){
          if (*iter0 == *iter1){
//            errs() << "duplicate_inst: " << *iter0 << "\n";
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

void BP::dumpBP(inst_vec_vec scc){
//  inst_vec_vec scc = sccs[L];
  for (unsigned int i = 0; i < scc.size(); i++){
    errs() << "scc No. :" << i << "\n";
    for (unsigned int j =0; j < scc[i].size(); j++)
      errs() << *scc[i][j] << "\n";
  }
}

inst_vec_vec BP::check_partition(inst_vec_vec old_scc, Loop* L){
  inst_vec_vec new_sec;
  int scc_no = old_scc.size()-2; // subtract the last two :inc br 
  int all_scc_no = pow(2, (scc_no-1));
  int *size = new int[scc_no];   // no_inst for sccs
  double *Scores = new double[all_scc_no];

  double *IcacheScore=new double[all_scc_no]; //IcacheScore new here!
  double *iterationScore=new double[all_scc_no]; //IcacheScore new here!
  double *extrainstScore=new double[all_scc_no]; //IcacheScore new here!

  int min = 0;
  double min_score = 0;


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
  
  // calculating scores
  /////////////calc Icache score//////////////
  int scc_cut_point = scc_no-1; //////////cut point = scc_no - 1
  
  std::vector<std::vector<int> > ParPlanSizeGroup;
 
  for(int idx=0;idx<all_scc_no;idx++){
    int *pCut = new int[scc_cut_point];
    int cur=idx;
    for(int i=0;i<scc_cut_point;i++){
    pCut[scc_cut_point-1-i]=cur>>(scc_cut_point-1-i);
    cur-=(pCut[scc_cut_point-1-i])<<(scc_cut_point-1-i);
    }

    std::vector<int> CurSizeGroup;   
    int cursize=size[0];
    for(int p=0;p<scc_cut_point;p++)
    {
     if(!pCut[p]){cursize+=size[p+1];}
     else{
            CurSizeGroup.push_back(cursize);
            cursize=size[p+1];
     }
    }
    CurSizeGroup.push_back(cursize);
    delete[] pCut;
    ParPlanSizeGroup.push_back(CurSizeGroup);
  }

  for(unsigned long i=0;i<ParPlanSizeGroup.size();i++){
     double tmpPS = 0.0;
     double maxPS = 0.0;
     double sumPS = 0.0;
    for(unsigned long j=0;j<(ParPlanSizeGroup.at(i)).size();j++){
       double cursize = (double)((ParPlanSizeGroup.at(i)).at(j));
       if(cursize<=I_CACHE_SIZE){tmpPS=((double)I_CACHE_SIZE-cursize);}
       else{tmpPS=((cursize-(double)I_CACHE_SIZE)*((double)OVERFLOW_PENALTY));}
       if(tmpPS>maxPS){maxPS=tmpPS;}      
       sumPS += tmpPS;
    }
    double avgPS = sumPS/((ParPlanSizeGroup.at(i)).size());
    *(IcacheScore+i)=(avgPS+maxPS)/2.0;
  }

  ///////////END calc Icahce score//////////// 

  /////////iteration Score//////////////// 
  for (int i = 0; i < all_scc_no; i++){
    iterationScore[i] = 1.0 / NUM_OF_CORES;
  }
  //////////END iteration score///////////
  //////////////extrainstrScore////////////
  for (int i = 0; i < all_scc_no; i++){
    extrainstScore[i] = ParPlanSizeGroup.at(i).size() * NumHeaderInst(L);
  }
  ///////////END extrainstrScore/////////////

///////////////// find the min penalty partition plan //////////////

  for (int i = 0; i < all_scc_no; i++){
    Scores[i] = IcacheScore[i]*WEIGHT_CACHE + 
                iterationScore[i]*WEIGHT_ITERATION + 
                extrainstScore[i]*WEIGHT_INSTRUCTIONS; 
  }

  min_score=Scores[0];
  for (int i = 0; i < all_scc_no; i++)
    if (min_score > Scores[i]) {
      min = i;
      min_score = Scores[i];
    }

/////////////////END find the min penalty partition plan //////////////

/////////////// Form the new partition plan with minimum penalty /////////////
  int *pCut=new int[scc_cut_point];
  int cur=min; 
  for(int i=0;i<scc_cut_point;i++){
    pCut[scc_cut_point-1-i]=cur>>(scc_cut_point-1-i);
    cur-=(pCut[scc_cut_point-1-i])<<(scc_cut_point-1-i);
  }

    inst_vec tmp;
    for(unsigned long j=0;j<old_scc[0].size();j++){tmp.push_back(old_scc[0].at(j));}
    for(int p=0;p<scc_cut_point;p++){
       if(!pCut[p]){for(unsigned long j=0;j<old_scc[p+1].size();j++){tmp.push_back(old_scc[p+1].at(j));}}
       else{
         new_sec.push_back(tmp);
         tmp.clear();
         for(unsigned long j=0;j<old_scc[p+1].size();j++){tmp.push_back(old_scc[p+1].at(j));}
       }
    }
    new_sec.push_back(tmp);
  delete[] pCut;
///////////// END Form the new partition plan with minimum penalty ///////////

///////////// Add back the last two instrs//////////
  int last=old_scc.size()-2;
  tmp.clear();
  for(unsigned long j=0;j<old_scc[last].size();j++){tmp.push_back(old_scc[last].at(j));}
  new_sec.push_back(tmp);

  last=old_scc.size()-1;
  tmp.clear();
  for(unsigned long j=0;j<old_scc[last].size();j++){tmp.push_back(old_scc[last].at(j));}
  new_sec.push_back(tmp);  

delete[] IcacheScore; ///////////IcacheScore delete here!
delete[] iterationScore; 
delete[] extrainstScore;
delete[] size;        //////////size delete here!
delete[] Scores;      ///////////score delete here!

//  errs() << "debug";
//  errs() << new_sec.size() << "\n";
//  errs() << new_sec[0].size() << "\n";
  return new_sec;
} 

int BP::NumHeaderInst(Loop *L)
{
 BasicBlock *HB = L->getHeader(); //Header Block
 int HeaderInstrCnt=0;
 for(BasicBlock::iterator it=HB->begin();it!=HB->end();++it)
     HeaderInstrCnt++;
 
 return HeaderInstrCnt; 
}
