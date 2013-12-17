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
            Partitions[L] = build_scc(L, prdg, find_edges(prdg, dg_mem_map)); 
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

inst_vec BP::find_prdg(Instruction *inst, inst_vec_vec prdg){
  inst_vec_vec::iterator idx;
  for (idx = prdg.begin(); idx != prdg.end(); idx++) {
    inst_vec rdg = *idx;
    if ((std::find(rdg.begin(), rdg.end(), inst)) != rdg.end()){
      return rdg;
    }
  }
  errs() << "instruction not found in find_prdg!!!!\n";
//  return rdg;
}

// find the edges between partitions (prdg)
inst_map_vec_set BP::find_edges(inst_vec_vec prdg, inst_map_set dg_mem_map){
  inst_map_set::iterator it;
  inst_set::iterator idx;
  inst_map_vec_set edges;
  for(it = dg_mem_map.begin(); it != dg_mem_map.end(); it++){   //iterate mem_map
    if (it->second.empty()) continue;
    Instruction* first_inst = it->first;
//  errs() << "first inst:: " << *first_inst << "\n";
    inst_vec first_rdg = find_prdg(first_inst, prdg);
    inst_set first_set = it->second;
//  errs() << "first inst_vec\n";
//  dumpinst_vec(first_rdg);
// find rdg of first_ins    
    for(idx = first_set.begin(); idx != first_set.end(); idx++){
      Instruction* second_inst = *idx;
//  errs() << "second inst: " << *second_inst << "\n";
      inst_vec second_rdg = find_prdg(second_inst, prdg);
//  errs() << "second inst_vec\n";
//  dumpinst_vec(second_rdg);
      edges[first_rdg].insert(second_rdg);
    }
  }
  return edges;
}

inst_vec_vec BP::build_scc(Loop *CurL, inst_vec_vec prdg, inst_map_vec_set mem_map){
  inst_vec_vec scc; //result
  inst_vec_visit visited;
  inst_vec_visit visited_tmp; //used for dfs: visited cannot be changed in dfs
  inst_vec_vec::iterator prdg_iter;

//  errs() << "build_scc\n";
  //init
  for (prdg_iter = prdg.begin(); prdg_iter != prdg.end(); prdg_iter++){
    inst_vec rdg0 = *prdg_iter; //rdg0: main rdg
    visited[rdg0] = false;
  }
  visited_tmp = visited;

  for (prdg_iter = prdg.begin(); prdg_iter != prdg.end(); prdg_iter++){
    inst_vec rdg0 = *prdg_iter; //rdg0: main rdg
//  errs() << "first inst_vec\n";
//  dumpinst_vec(rdg0);
    if (!visited[rdg0]){ // not visited
//  errs() << "new rdg...\n";
//  dumpinst_vec(rdg0);
      inst_vec merged;
      std::map<inst_vec, bool> valid;
      valid[rdg0] = true;
      inst_vec_vec prdg_vec0 = dfs_scc(rdg0, prdg, mem_map, &visited_tmp); // find all reachable rdgs
//  errs() << "end of dfs_scc...\n";
      visited_tmp = visited; //reset
      inst_vec_vec::iterator prdg_vec_iter0;     // do dfs for all reachable rdgs to find out any un-reachable rdg
      for (prdg_vec_iter0 = prdg_vec0.begin(); prdg_vec_iter0 != prdg_vec0.end(); prdg_vec_iter0++){
        inst_vec rdg1 = *prdg_vec_iter0; // rdg1: checked rdg in prdg_vec0 (main vec)
//  errs() << "checked inst_vec\n";
//  dumpinst_vec(rdg1);
        valid[rdg1] = true; //at first assume it is part of SCC
        if (rdg1 == rdg0) continue;
        inst_vec_vec prdg_vec1 = dfs_scc(rdg1, prdg, mem_map, &visited_tmp); // dfs rdg1
        visited_tmp = visited; //reset
        inst_vec_vec::iterator prdg_vec_iter1;
        for (prdg_vec_iter1 = prdg_vec0.begin(); prdg_vec_iter1 != prdg_vec0.end(); prdg_vec_iter1++){ // checking all rdgs in prdg_vec0
          inst_vec rdg2 = *prdg_vec_iter1; // rdg2: rdg being checked in prdg_vec_iter0
//  errs() << "inst_vec being checked\n";
//  dumpinst_vec(rdg1);
          if (std::find(prdg_vec1.begin(), prdg_vec1.end(), rdg2) == prdg_vec1.end()) { // cannot find this rdg in prdg_vec1
             valid[rdg1] = false; // rdg1 is not a part of SCC because it cannot reach part of the prdg_vec0
            break;
          } //end if
        } //end prdg_vec_iter1
      } //end prdg_vec_iter0
      for (prdg_vec_iter0 = prdg_vec0.begin(); prdg_vec_iter0 != prdg_vec0.end(); prdg_vec_iter0++){  //erasing, merging, and mark visited
        inst_vec rdg1 = *prdg_vec_iter0;
        if (valid[rdg1]){
          visited[rdg1] = true;
//          errs() << "inserting rdg1\n";
//          dumpinst_vec(rdg1);
          merged.insert(merged.end(), rdg1.begin(), rdg1.end());
        }
      }
      scc.push_back(merged);
    } //end if visited
  }

  // apply heurstics
  if (HEURSTICS)
    return check_partition(scc, CurL);
  else 
    return scc;
}

inst_vec_vec BP::dfs_scc(inst_vec rdg0, inst_vec_vec prdg, inst_map_vec_set mem_map, inst_vec_visit *visited){
  inst_vec_vec group;
  inst_vec_set dep_insts = mem_map[rdg0];  //all insts that start_inst related to
  inst_vec_set::iterator it;
  group.push_back(rdg0);
  (*visited)[rdg0] = true;
//  errs() << "dfs_scc...\n";
//  dumpinst_vec(rdg0);
  for (it = dep_insts.begin(); it != dep_insts.end(); it++){
    inst_vec inst = *it;
//  errs() << "checked inst_vec\n";
//  dumpinst_vec(inst);
    if (!(*visited)[inst]) { // not visited
      (*visited)[inst] = true;
      inst_vec_vec new_insts = dfs_scc(inst, prdg, mem_map, visited);
      //remove duplicates
      std::set<inst_vec_vec::iterator> to_remove;
      for (inst_vec_vec::iterator iter0 = new_insts.begin(); iter0 != new_insts.end(); iter0++){
        for (inst_vec_vec::iterator iter1 = group.begin(); iter1 != group.end(); iter1++){
          if (*iter0 == *iter1){
            to_remove.insert(iter0);
            break;
          }
        }
      }
      for (std::set<inst_vec_vec::iterator>::iterator iter = to_remove.begin(); iter != to_remove.end(); iter++)
        new_insts.erase(*iter);

      group.insert(group.end(), new_insts.begin(), new_insts.end());
    }
  }
  return group;
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
      std::set<inst_vec::iterator> to_remove;
      for (inst_vec::iterator iter0 = new_insts.begin(); iter0 != new_insts.end(); iter0++){
        for (inst_vec::iterator iter1 = group.begin(); iter1 != group.end(); iter1++){
          if (*iter0 == *iter1){
//            errs() << "duplicate_inst: " << *iter0 << "\n";
            to_remove.insert(iter0);
            break;
          }
        }
      }

      for (std::set<inst_vec::iterator>::iterator iter0 = to_remove.begin(); iter0 != to_remove.end(); iter0++){
        inst_vec::iterator inst_vec_iter = *iter0;
        new_insts.erase(inst_vec_iter);
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
//  errs()<<"check partition using heuristic\n";
  errs() << "old scc no "<<old_scc.size()<<"\n";
  inst_vec_vec new_sec;
  int scc_no = old_scc.size()-2; // subtract the last two :inc br 
  int all_scc_no = pow(2, (scc_no-1));
  int *size = new int[scc_no];   // no_inst for sccs
  double *Scores = new double[all_scc_no];

  double *IcacheScore=new double[all_scc_no]; //IcacheScore new here!
  double *iterationScore=new double[all_scc_no]; //IcacheScore new here!
  double *extrainstScore=new double[all_scc_no]; //IcacheScore new here!
  double *multiCoreScore=new double[all_scc_no]; //IcacheScore new here!

  int min = 0;
  double min_score = 0;

//  errs()<<"old scc built\n";
//  for(unsigned long i=0;i<scc_no;++i)
//  {
//    unsigned long J=old_scc[i].size();
//    errs()<<"scc no"<<i<<"\n";
 //   for(unsigned long j=0;j<J;++j)
//    {errs()<<*(old_scc[i][j])<<"\t";}
//    errs()<<"\n";
//  }
//  errs()<<"end old scc built\n";

  /*
    sequence of iterating all possible sccs:
    suppose we have scc_no of 5, which means scc 0, 1, 2, 3, and 4.
    scc_no [0] = cut all (original);
    scc_no [1->4] = merge 0->1, 1->2, ...
    scc_no [5->10] = merge 0->1&1->2, 0->1&2->3, ...
    scc_no [11->14] = merge 0->1&1->2&2->3, 0->1&1->2&3->4, ...
    scc_no 15 = merge all
  */
  int total_instr_cnt=0;
//  errs()<<"old scc size\n";
  for (int i = 0; i < scc_no; i++){
    size[i] = old_scc[i].size();
    total_instr_cnt+=size[i];
 //    errs()<<size[i]<<"\n";
  }
//  errs()<<"\n";
//  errs()<<"header_instr_cnt="<<NumHeaderInst(L)<<"\n";
//  errs()<<"total_instr_cnt="<<total_instr_cnt<<"\n";  
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
    for(unsigned long j=0;j<(ParPlanSizeGroup.at(i)).size();j++){
      ((ParPlanSizeGroup.at(i)).at(j))+=2;
    }
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

  //////////normalize IcacheScore////////////
  double minIscore=*(IcacheScore+0);
  double maxIscore=*(IcacheScore+0);
  for(int i=0;i<all_scc_no;i++){
     if(*(IcacheScore+i)>maxIscore){maxIscore=*(IcacheScore+i);}
     if(*(IcacheScore+i)<minIscore){minIscore=*(IcacheScore+i);}
  }
  for(int i=0;i<all_scc_no;i++){
     *(IcacheScore+i) = (*(IcacheScore+i))/maxIscore;
  }
//  errs()<<"maxIscore="<<maxIscore<<"\n";
//  errs()<<"minIscore="<<minIscore<<"\n";
//  errs()<<"max-min Iscore="<<maxIscore-minIscore<<"\n";
//  errs()<<"normalize max-min Iscore="<<(maxIscore-minIscore)/maxIscore<<"\n";
  ///////////END calc Icahce score//////////// 

  /////////iteration Score//////////////// 
  for (int i = 0; i < all_scc_no; i++){
    iterationScore[i] = 1.0 / NUM_OF_CORES;
  }
  //////////END iteration score///////////
  //////////////extrainstrScore////////////
  for (int i = 0; i < all_scc_no; i++){
    extrainstScore[i] = ((double)(ParPlanSizeGroup.at(i).size() * NumHeaderInst(L)))/(100.0*(double)total_instr_cnt);
  }
  ///////////END extrainstrScore/////////////
  ///////////////normalize extrainstrScore////////////////
  double maxEIscore=*(extrainstScore+0);
  double minEIscore=*(extrainstScore+0);
  for (int i = 0; i < all_scc_no; i++){
     if(*(extrainstScore+i)>maxEIscore){maxEIscore=*(extrainstScore+i);}
     if(*(extrainstScore+i)<minEIscore){minEIscore=*(extrainstScore+i);}
  }  
//  for (int i = 0; i < all_scc_no; i++){
//     *(extrainstScore+i)=*(extrainstScore+i)/maxEIscore;
//  } 
//  errs()<<"max extra="<< maxEIscore<<"\n";
//  errs()<<"min extra="<< minEIscore<<"\n";
//  errs()<<"max-min extra="<<maxEIscore-minEIscore<<"\n";
//  errs()<<"normalize max-min extra="<<(maxEIscore-minEIscore)/maxEIscore<<"\n";
  /////////////END normalize extrainstrScore//////////////

  ///////////////multiCoreScore////////////////
  for(int i=0;i<all_scc_no;i++){
//     if(ParPlanSizeGroup.at(i).size()>=NUM_OF_CORES){*(multiCoreScore+i)=1.0;}
//     else{*(multiCoreScore+i)=((double)NUM_OF_CORES)/((double)ParPlanSizeGroup.at(i).size());}
       *(multiCoreScore+i)=((double)NUM_OF_CORES)/((double)ParPlanSizeGroup.at(i).size());
  }
  double maxMCscore=*(multiCoreScore+0);
  for(int i=0;i<all_scc_no;i++){
    if(*(multiCoreScore+i)>maxMCscore) {maxMCscore=*(multiCoreScore+i);}
  } 
  for(int i=0;i<all_scc_no;i++){
     *(multiCoreScore+i)=*(multiCoreScore+i)/maxMCscore;
  } 
  ////////////END multiCoreScore///////////////

///////////////// find the min penalty partition plan //////////////

  for (int i = 0; i < all_scc_no; i++){
    Scores[i] = IcacheScore[i]*WEIGHT_CACHE + 
//                iterationScore[i]*WEIGHT_ITERATION + 
                extrainstScore[i]*WEIGHT_INSTRUCTIONS; //+
//                multiCoreScore[i]*WEIGHT_MULTICORES; 
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
//  errs()<<"new scc built\n";
//  for(unsigned long i=0;i<new_sec.size();++i)
//  {
 //   unsigned long J=new_sec[i].size();
//    errs()<<"scc no"<<i<<"\n";
//    for(unsigned long j=0;j<J;++j)
 //   {errs()<<*(new_sec[i][j])<<"\t";}
 //   errs()<<"\n";
//  }
//  errs()<<"end new scc built\n";
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
delete[] multiCoreScore;
delete[] size;        //////////size delete here!
delete[] Scores;      ///////////score delete here!

  errs() << "new_sec scc no ";
  errs() << new_sec.size()<< "\n";
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

void BP::dumpinst_vec(inst_vec insts)
{
  errs() << "dump inst_vec..\n";
  for (inst_vec::iterator inst = insts.begin(); inst != insts.end(); inst++){
    Instruction* in = *inst;
    errs() << *in << "\n";
  }
}
