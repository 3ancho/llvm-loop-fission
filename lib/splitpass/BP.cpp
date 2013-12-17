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
  
  // apply heurstics
  if (HEURSTICS)
    return check_partition(partition, CurL);
  else 
    return partition;
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

void BP::dumpBP(Loop *L){
  inst_vec_vec sccs = Partitions[L];
  for (unsigned int i = 0; i < sccs.size(); i++){
    errs() << "scc No. :" << i << "\n";
    for (unsigned int j =0; j < sccs[i].size(); j++)
      errs() << *sccs[i][j] << "\n";
  }
}

inst_vec_vec BP::check_partition(inst_vec_vec old_scc, Loop* L){
  errs()<<"check partition using heuristic\n";
  errs() << "old scc no "<<old_scc.size()-2<<"\n";
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
  for (int i = 0; i < scc_no; i++){
    size[i] = old_scc[i].size();
    total_instr_cnt+=size[i];
  }
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
  double maxIscore=*(IcacheScore+0);
  for(int i=0;i<all_scc_no;i++){
     if(*(IcacheScore+i)>maxIscore){maxIscore=*(IcacheScore+i);}
  }
  for(int i=0;i<all_scc_no;i++){
     *(IcacheScore+i) = (*(IcacheScore+i))/maxIscore;
  }
//  errs()<<"maxIscore="<<maxIscore<<"\n";
  ///////////END calc Icahce score//////////// 

  /////////iteration Score//////////////// 
  for (int i = 0; i < all_scc_no; i++){
    iterationScore[i] = 1.0 / NUM_OF_CORES;
  }
  //////////END iteration score///////////
  //////////////extrainstrScore////////////
  for (int i = 0; i < all_scc_no; i++){
    extrainstScore[i] = ((double)(ParPlanSizeGroup.at(i).size() * NumHeaderInst(L)))/(10.0*(double)total_instr_cnt);
  }
  ///////////END extrainstrScore/////////////
  ///////////////normalize extrainstrScore////////////////
  double maxEIscore=*(extrainstScore+0);
  for (int i = 0; i < all_scc_no; i++){
     if(*(extrainstScore+i)>maxEIscore){maxEIscore=*(extrainstScore+i);}
  }  
  for (int i = 0; i < all_scc_no; i++){
     *(extrainstScore+i)=*(extrainstScore+i)/maxEIscore;
  } 
//  errs()<<"max extra="<< maxEIscore<<"\n";
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
  errs() << new_sec.size()-2<< "\n";
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
