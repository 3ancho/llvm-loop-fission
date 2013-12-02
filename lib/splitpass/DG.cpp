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
using namespace llvm;

namespace {

  class DG : public FunctionPass {
  private:
    AliasAnalysis *AA;
    Function *F;
    DependenceAnalysis *DA;
  std::map<Instruction*, std::vector<Instruction*> > adjacentInsts;

  public:
    static char ID; 
    DG() : FunctionPass(ID) {};
    bool runOnFunction(Function &F);
    void getAnalysisUsage(AnalysisUsage &) const;
    void printAdj(std::map<Instruction*, std::vector<Instruction*> > );
  }; // class DG
} // namespace llvm


static RegisterPass<DG> X("dg", "583 final dependence graph");
char DG::ID = 0;
bool DG::runOnFunction(Function &F) {
  this->F = &F;
  AA = &getAnalysis<AliasAnalysis>();
  DA = &getAnalysis<DependenceAnalysis>();
  // Build an adjacent list
  std::vector<Instruction*> vect;
  for (inst_iterator SrcI = inst_begin(F), SrcE = inst_end(F); SrcI != SrcE; ++SrcI) {
    if (isa<StoreInst>(*SrcI) || isa<LoadInst>(*SrcI)) {
    vect.clear();
      for (inst_iterator DstI = inst_begin(F), DstE = inst_end(F); DstI != DstE; ++DstI) {
      if (DstI == SrcI) continue;
        if (isa<StoreInst>(*DstI) || isa<LoadInst>(*DstI)) {
          if (DA->depends(&*SrcI, &*DstI, true)) {
            //errs() << "dependence found\n";
      vect.push_back(&*DstI);
          }
        }
      }
    adjacentInsts[&*SrcI] = vect;
    }
  }
  // print the adjacency information
  printAdj(adjacentInsts);
  return false;
}
void DG::printAdj(std::map<Instruction*, std::vector<Instruction*> > m) {
  errs() << "print the built adjacent instruction lists:\n";
  std::map<Instruction*, std::vector<Instruction*> >::iterator mapit;
  for (mapit = adjacentInsts.begin(); mapit != adjacentInsts.end(); ++mapit) {
    Instruction *inst = mapit->first;
  errs() << "Instruction " << *inst << " has dependence to:\n";
  std::vector<Instruction*> vect = mapit->second;
  std::vector<Instruction*>::iterator vit;
  for (vit = vect.begin(); vit != vect.end(); ++vit) {
    errs() << "\t" << *(*vit) << "  --  ";
    Dependence *D = DA->depends(inst, *vit, true);
    if (D->isConfused()) {
      errs() << "confused\n";
    } else {
        if (D->isConsistent())
          errs() << "consistent ";
        if (D->isFlow())
          errs() << "flow ";
        else if (D->isOutput())
          errs() << "output ";
        else if (D->isAnti())
          errs() << "anti ";
        else if (D->isInput())
          errs() << "input ";
    const SCEV *Distance = D->getDistance(D->getLevels()); //only check at the highest level for now
    errs() << *(cast<SCEVConstant>(*Distance).getValue())<< " ";
    unsigned Direction = D->getDirection(D->getLevels()); //only check at the highest level for now
    if (Direction == Dependence::DVEntry::ALL)
      errs() << "*\n";
    else {
      if (Direction & Dependence::DVEntry::LT)
      errs() << "<\n";
      if (Direction & Dependence::DVEntry::EQ)
      errs() << "=\n";
      if (Direction & Dependence::DVEntry::GT)
      errs() << ">\n";
    }
    }
  }
  }
}



void DG::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<AliasAnalysis>();
  AU.addRequiredTransitive<DependenceAnalysis>();
}







