#include <algorithm>
#include "DG.h"

using namespace llvm;

static RegisterPass<DG> X("dg", "583 - data dependence graph of loops in a function");
char DG::ID = 0;
void DG::buildDG(Loop *L) {
  std::vector<Loop*> subLoops = L->getSubLoops();
  if (subLoops.empty()) {
    // this is the innermost loop. Do the analysis.
	// first get all the instructions in the loop body
    std::vector<Instruction*> bodyInsts;
	bodyInsts.clear();
	BasicBlock *Header = L->getHeader();
	//BasicBlock *Latch = L->getLoopLatch();
	//errs() << "Latch ---------------- " << *Latch << "\n";
    for (unsigned j = 0, k = L->getBlocks().size(); j != k; ++j) {
      BasicBlock *BB = L->getBlocks()[j];
      if (BB != Header) {
	    errs() << "BB is in the body-------------------------------------: " << *BB << "\n";
	    for (BasicBlock::iterator i = BB->begin(), e = BB->end(); i != e; ++i) {
		  Instruction *Inst = i;
		  bodyInsts.push_back(Inst);
		}
	  }
	}
	errs() << "bodyInsts size is " << bodyInsts.size() << "\n";
    DA = &getAnalysis<DependenceAnalysis>();
    std::map<Instruction*, std::set<Instruction*> > daMap;
    daMap.clear();
    numOfNodes[L] = 0;
    numOfDeps[L] = 0;
    std::set<Instruction*> daSet;
	  for (std::vector<Instruction*>::iterator vi2 = bodyInsts.begin(); vi2 != bodyInsts.end(); ++vi2) {
        Instruction *SrcI = *vi2;
        daSet.clear();
        numOfNodes[L]++;
          for (std::vector<Instruction*>::iterator vi3 = bodyInsts.begin(); vi3 != bodyInsts.end(); ++vi3) {
            Instruction *DstI = *vi3;
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
                  //do nothing
                } else {
                  //consider double direction dependence
                  daSet.insert(DstI);
                }
              }
            } 
			// for all instructions, run the register dependence analysis through def-use chain
			// def-use chain
            for (Value::use_iterator ui = SrcI->use_begin(), ue = SrcI->use_end(); ui != ue; ++ui) {
              if (Instruction *inst = dyn_cast<Instruction>(*ui)) {
			    if(std::find(bodyInsts.begin(), bodyInsts.end(), inst)!=bodyInsts.end()){
                  // the use inst is in the body blocks as well
                  daSet.insert(inst);
                }
              }
            }
          }
        daMap[SrcI] = daSet;
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
    int count = 0;
    errs() << "\tPrint the dependences for " << *mapit1->first << "\n";
    std::map<Instruction*, std::set<Instruction*> > dg_temp = mapit1->second;
    errs() << "There are " << dg_temp.size() << " nodes in the loop\n";
    errs() << "There are " << numOfNodes[mapit1->first] << " nodes in the loop, to confirm\n";
    std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
    for (mapit2 = dg_temp.begin(); mapit2 != dg_temp.end(); ++mapit2) {
      Instruction *inst = mapit2->first;
      errs() << "\t\tInstruction " << *inst << "\n";
      std::set<Instruction*> set_temp = mapit2->second;
      count+=set_temp.size();
      std::set<Instruction*>::iterator setit;
      for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
        errs() << "\t\t\t\t -------->" << *(*setit) << "\n";
      }
    }
	numOfDeps[mapit1->first] = count;
    errs() << "There are " << numOfDeps[mapit1->first] << " data dependencies in the loop\n ";
  }
} 

void DG::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<AliasAnalysis>();
  AU.addRequiredTransitive<DependenceAnalysis>();
  AU.addRequired<LoopInfo>();
  //AU.addPreserved<LoopInfo>();
}
