#ifndef BP_H
#define BP_H
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "DG.h"
#include <stdio.h>


namespace llvm{
typedef std::vector<Instruction*> inst_vec;
typedef std::set<Instruction*> inst_set;
typedef std::map<Instruction*, bool> inst_visit;
typedef std::map<Instruction*, std::set<Instruction*> > inst_map_set;

  class AliasAnalysis;
  class DependenceAnalysis;
  class Value;
  class raw_ostream;

  class BP::public FunctionPass{
	public:
    AliasAnalysis *AA;
    Function *F;
    DependenceAnalysis *DA;
    LoopInfo *LI;
    DG *depmap;
  
    void dfs();
    void build_partition(Loop *CurL, inst_map_set CurInstMapSet);

    std::map<Loop*, int> NumOfPartitions;
    std::map<Loop*, std::vector<Instruction*> >	Partitions; 

    static char ID; 
    scc() : FunctionPass(ID) {};
    bool runOnFunction(Function &F);
    void getAnalysisUsage(AnalysisUsage &) const;
    void OutputBP(Loop *L);
	
};

}//end llvm namespace
#endif
