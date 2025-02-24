///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNestInfo.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements ScopeNestInfo class to hold the results of the scope           //
// nest analysis.                                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxilConvPasses/ScopeNestInfo.h"
#include "DxilConvPasses/ScopeNestIterator.h"

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//----------------------- Scope Nest Info Implementation ---------------------//
void ScopeNestInfo::print(raw_ostream &out) const {
  out << "ScopeNestInfo:\n";
  int level = 0;
  for (const ScopeNestEvent &element : m_scopeElements) {
    if (element.IsEndScope())
      --level;

    if (element.IsBeginScope()) {
      if (element.Block)
        indent(out, level, element.Block->getName()) << "\n";
      indent(out, level, "@") << element.GetElementTypeName() << "\n";
    } else if (element.IsEndScope()) {
      indent(out, level, "@") << element.GetElementTypeName() << "\n";
      if (element.Block)
        indent(out, level, element.Block->getName()) << "\n";
    } else {
      if (element.Block)
        indent(out, level, element.Block->getName()) << "\n";

      if (element.ElementType == ScopeNestEvent::Type::If_Else ||
          element.ElementType == ScopeNestEvent::Type::Switch_Case)
        indent(out, level - 1, "@") << element.GetElementTypeName() << "\n";
      else if (element.ElementType != ScopeNestEvent::Type::Body)
        indent(out, level, "@") << element.GetElementTypeName() << "\n";
    }

    if (element.IsBeginScope())
      ++level;
  }
}

raw_ostream &ScopeNestInfo::indent(raw_ostream &out, int level,
                                   StringRef str) const {
  for (int i = 0; i < level; ++i)
    out << "    ";
  out << str;
  return out;
}

void ScopeNestInfo::releaseMemory() { m_scopeElements.clear(); }

void ScopeNestInfo::Analyze(Function &F) {
  for (ScopeNestIterator I = ScopeNestIterator::begin(F),
                         E = ScopeNestIterator::end();
       I != E; ++I) {
    ScopeNestEvent element = *I;
    m_scopeElements.push_back(element);
  }
}

//----------------------- Wrapper Pass Implementation ------------------------//
char ScopeNestInfoWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(ScopeNestInfoWrapperPass, "scopenestinfo",
                      "Scope nest info pass", true, true)
INITIALIZE_PASS_END(ScopeNestInfoWrapperPass, "scopenestinfo",
                    "Scope nest info pass", true, true)

FunctionPass *llvm::createScopeNestInfoWrapperPass() {
  return new ScopeNestInfoWrapperPass();
}

bool ScopeNestInfoWrapperPass::runOnFunction(Function &F) {
  releaseMemory();
  SI.Analyze(F);
  return false;
}

void ScopeNestInfoWrapperPass::releaseMemory() { SI.releaseMemory(); }

void ScopeNestInfoWrapperPass::print(raw_ostream &O, const Module *M) const {
  SI.print(O);
}

void ScopeNestInfoWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}
