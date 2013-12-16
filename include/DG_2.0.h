#ifndef DG_H
#define DG_H
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

namespace llvm {
  class AliasAnalysis;
  class DependenceAnalysis;
  class Value;
  class raw_ostream;
  
  class DG : public FunctionPass {
  private:
    AliasAnalysis *AA;
    Function *F;
    DependenceAnalysis *DA;
    LoopInfo *LI;

  public:
    static char ID; 
    DG() : FunctionPass(ID) {};
	std::map<Loop*, bool> ifLoopDist;
	std::map<Loop*, std::map<Instruction*, std::set<Instruction*> > > dgOfLoops;
	std::map<Loop*, std::map<Instruction*, std::set<Instruction*> > > dgOfLoopsMem;
    std::map<Loop*, int> numOfNodes;
    std::map<Loop*, int> numOfDeps;
    bool runOnFunction(Function &F);
    void getAnalysisUsage(AnalysisUsage &) const;
    void printDG();
    bool buildDG(Loop *);
  }; // class DG

} // namespace llvm
#endif

