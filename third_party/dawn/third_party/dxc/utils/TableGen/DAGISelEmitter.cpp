//===- DAGISelEmitter.cpp - Generate an instruction selector --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This tablegen backend emits a DAG instruction selector.
//
//===----------------------------------------------------------------------===//

#include "CodeGenDAGPatterns.h"
#include "DAGISelMatcher.h"
#include "llvm/Support/Debug.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
using namespace llvm;

#define DEBUG_TYPE "dag-isel-emitter"

namespace {
/// DAGISelEmitter - The top-level class which coordinates construction
/// and emission of the instruction selector.
class DAGISelEmitter {
  CodeGenDAGPatterns CGP;
public:
  explicit DAGISelEmitter(RecordKeeper &R) : CGP(R) {}
  void run(raw_ostream &OS);
};
} // End anonymous namespace

//===----------------------------------------------------------------------===//
// DAGISelEmitter Helper methods
//

/// getResultPatternCost - Compute the number of instructions for this pattern.
/// This is a temporary hack.  We should really include the instruction
/// latencies in this calculation.
static unsigned getResultPatternCost(TreePatternNode *P,
                                     CodeGenDAGPatterns &CGP) {
  if (P->isLeaf()) return 0;

  unsigned Cost = 0;
  Record *Op = P->getOperator();
  if (Op->isSubClassOf("Instruction")) {
    Cost++;
    CodeGenInstruction &II = CGP.getTargetInfo().getInstruction(Op);
    if (II.usesCustomInserter)
      Cost += 10;
  }
  for (unsigned i = 0, e = P->getNumChildren(); i != e; ++i)
    Cost += getResultPatternCost(P->getChild(i), CGP);
  return Cost;
}

/// getResultPatternCodeSize - Compute the code size of instructions for this
/// pattern.
static unsigned getResultPatternSize(TreePatternNode *P,
                                     CodeGenDAGPatterns &CGP) {
  if (P->isLeaf()) return 0;

  unsigned Cost = 0;
  Record *Op = P->getOperator();
  if (Op->isSubClassOf("Instruction")) {
    Cost += Op->getValueAsInt("CodeSize");
  }
  for (unsigned i = 0, e = P->getNumChildren(); i != e; ++i)
    Cost += getResultPatternSize(P->getChild(i), CGP);
  return Cost;
}

namespace {
// PatternSortingPredicate - return true if we prefer to match LHS before RHS.
// In particular, we want to match maximal patterns first and lowest cost within
// a particular complexity first.
struct PatternSortingPredicate {
  PatternSortingPredicate(CodeGenDAGPatterns &cgp) : CGP(cgp) {}
  CodeGenDAGPatterns &CGP;

  bool operator()(const PatternToMatch *LHS, const PatternToMatch *RHS) {
    const TreePatternNode *LHSSrc = LHS->getSrcPattern();
    const TreePatternNode *RHSSrc = RHS->getSrcPattern();

    MVT LHSVT = (LHSSrc->getNumTypes() != 0 ? LHSSrc->getType(0) : MVT::Other);
    MVT RHSVT = (RHSSrc->getNumTypes() != 0 ? RHSSrc->getType(0) : MVT::Other);
    if (LHSVT.isVector() != RHSVT.isVector())
      return RHSVT.isVector();

    if (LHSVT.isFloatingPoint() != RHSVT.isFloatingPoint())
      return RHSVT.isFloatingPoint();

    // Otherwise, if the patterns might both match, sort based on complexity,
    // which means that we prefer to match patterns that cover more nodes in the
    // input over nodes that cover fewer.
    int LHSSize = LHS->getPatternComplexity(CGP);
    int RHSSize = RHS->getPatternComplexity(CGP);
    if (LHSSize > RHSSize) return true;   // LHS -> bigger -> less cost
    if (LHSSize < RHSSize) return false;

    // If the patterns have equal complexity, compare generated instruction cost
    unsigned LHSCost = getResultPatternCost(LHS->getDstPattern(), CGP);
    unsigned RHSCost = getResultPatternCost(RHS->getDstPattern(), CGP);
    if (LHSCost < RHSCost) return true;
    if (LHSCost > RHSCost) return false;

    unsigned LHSPatSize = getResultPatternSize(LHS->getDstPattern(), CGP);
    unsigned RHSPatSize = getResultPatternSize(RHS->getDstPattern(), CGP);
    if (LHSPatSize < RHSPatSize) return true;
    if (LHSPatSize > RHSPatSize) return false;

    // Sort based on the UID of the pattern, giving us a deterministic ordering
    // if all other sorting conditions fail.
    assert(LHS == RHS || LHS->ID != RHS->ID);
    return LHS->ID < RHS->ID;
  }
};
} // End anonymous namespace


void DAGISelEmitter::run(raw_ostream &OS) {
  emitSourceFileHeader("DAG Instruction Selector for the " +
                       CGP.getTargetInfo().getName() + " target", OS);

  OS << "// *** NOTE: This file is #included into the middle of the target\n"
     << "// *** instruction selector class.  These functions are really "
     << "methods.\n\n";

  DEBUG(errs() << "\n\nALL PATTERNS TO MATCH:\n\n";
        for (CodeGenDAGPatterns::ptm_iterator I = CGP.ptm_begin(),
             E = CGP.ptm_end(); I != E; ++I) {
          errs() << "PATTERN: ";   I->getSrcPattern()->dump();
          errs() << "\nRESULT:  "; I->getDstPattern()->dump();
          errs() << "\n";
        });

  // Add all the patterns to a temporary list so we can sort them.
  std::vector<const PatternToMatch*> Patterns;
  for (CodeGenDAGPatterns::ptm_iterator I = CGP.ptm_begin(), E = CGP.ptm_end();
       I != E; ++I)
    Patterns.push_back(&*I);

  // We want to process the matches in order of minimal cost.  Sort the patterns
  // so the least cost one is at the start.
  std::sort(Patterns.begin(), Patterns.end(), PatternSortingPredicate(CGP));


  // Convert each variant of each pattern into a Matcher.
  std::vector<Matcher*> PatternMatchers;
  for (unsigned i = 0, e = Patterns.size(); i != e; ++i) {
    for (unsigned Variant = 0; ; ++Variant) {
      if (Matcher *M = ConvertPatternToMatcher(*Patterns[i], Variant, CGP))
        PatternMatchers.push_back(M);
      else
        break;
    }
  }

  std::unique_ptr<Matcher> TheMatcher =
    llvm::make_unique<ScopeMatcher>(PatternMatchers);

  OptimizeMatcher(TheMatcher, CGP);
  //Matcher->dump();
  EmitMatcherTable(TheMatcher.get(), CGP, OS);
}

namespace llvm {

void EmitDAGISel(RecordKeeper &RK, raw_ostream &OS) {
  DAGISelEmitter(RK).run(OS);
}

} // End llvm namespace
