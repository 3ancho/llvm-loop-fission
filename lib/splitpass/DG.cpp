#define DEBUG_TYPE "dg"
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
using namespace llvm;

namespace {

  class DG : public FunctionPass {
  private:
    AliasAnalysis *AA;
    Function *F;
    DependenceAnalysis *DA;
    LoopInfo *LI;
    std::map<Loop*, std::map<Instruction*, std::set<Instruction*> > > dgOfLoops;

  public:
    static char ID; 
    DG() : FunctionPass(ID) {};
    bool runOnFunction(Function &F);
    void getAnalysisUsage(AnalysisUsage &) const;
    void printDG();
    void buildDG(Loop *);
  }; // class DG
} // namespace llvm


static RegisterPass<DG> X("dg", "583 - data dependence graph of loops in a function");
char DG::ID = 0;
void DG::buildDG(Loop *L) {
  std::vector<Loop*> subLoops = L->getSubLoops();
  if (subLoops.empty()) {
    // this is the innermost loop. Do the analysis.
    DA = &getAnalysis<DependenceAnalysis>();
    std::map<Instruction*, std::set<Instruction*> > daMap;
    daMap.clear();
    std::set<Instruction*> daSet;
    for (Loop::block_iterator LB = L->block_begin(), LBE = L->block_end(); LB != LBE; ++LB) {
	  for (BasicBlock::iterator BI = (*LB)->begin(), BIE = (*LB)->end(); BI != BIE; ++BI) {
        Instruction *SrcI = BI;
        daSet.clear();
        for (Loop::block_iterator LBB = L->block_begin(), LBBE = L->block_end(); LBB != LBBE; ++LBB) {
          for (BasicBlock::iterator BBI = (*LBB)->begin(), BBIE = (*LBB)->end(); BBI != BBIE; ++BBI) {
            Instruction *DstI = BBI;
            if (DstI == SrcI) continue;
            if ((isa<StoreInst>(*DstI) || isa<LoadInst>(*DstI)) 
               && (isa<StoreInst>(*SrcI) || isa<LoadInst>(*SrcI))) {
              // memory dependence analysis
              if (Dependence *D = DA->depends(&*SrcI, &*DstI, true)) {
                unsigned Direction = D->getDirection(D->getLevels());
                if (Direction & Dependence::DVEntry::LT) {
                  // SrcI -> DstI case
                  daSet.insert(DstI);
                } else if (Direction & Dependence::DVEntry::EQ) {
                  if (daMap.count(DstI) == 0 || daMap[DstI].count(SrcI) == 0) {
                    daSet.insert(DstI);
                  }
                } else if (Direction & Dependence::DVEntry::GT) {
                  
                } else {
                  
                }
              }
            } else {
              // register dependence analysis
            }
          }
        }
        daMap[SrcI] = daSet;
      }
    }
    dgOfLoops[L] = daMap;
  } else {
    for (std::vector<Loop*>::iterator it = subLoops.begin() ; it != subLoops.end(); ++it) {
      buildDG(*it);
    }
  }
}

bool DG::runOnFunction(Function &F) {
  this->F = &F;
  AA = &getAnalysis<AliasAnalysis>();
  //DA = &getAnalysis<DependenceAnalysis>();
  LI = &getAnalysis<LoopInfo>();
  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
    Loop *curLoop = *i;
    if (curLoop->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration
    buildDG(curLoop);
  }
  printDG();
  return false;
}

void DG::printDG() {
  errs() << "Print the built dependence graph for each loop: \n";
  std::map<Loop*, std::map<Instruction*, std::set<Instruction*> > >::iterator mapit1;
  for (mapit1 = dgOfLoops.begin(); mapit1 != dgOfLoops.end(); ++mapit1) {
    errs() << "\tPrint the dependences for " << *mapit1->first << "\n";
    std::map<Instruction*, std::set<Instruction*> > dg_temp = mapit1->second;
    std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
    for (mapit2 = dg_temp.begin(); mapit2 != dg_temp.end(); ++mapit2) {
      Instruction *inst = mapit2->first;
      errs() << "\t\tInstruction " << *inst << "\n";
      std::set<Instruction*> set_temp = mapit2->second;
      std::set<Instruction*>::iterator setit;
      for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
        errs() << "\t\t\t\t -------->" << *(*setit) << "\n";
      }
    }
  }
} 

void DG::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<AliasAnalysis>();
  AU.addRequiredTransitive<DependenceAnalysis>();
  AU.addRequired<LoopInfo>();
  AU.addPreserved<LoopInfo>();
}







