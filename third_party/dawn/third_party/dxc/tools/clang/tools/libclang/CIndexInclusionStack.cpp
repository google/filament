//===- CIndexInclusionStack.cpp - Clang-C Source Indexing Library ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a callback mechanism for clients to get the inclusion
// stack from a translation unit.
//
//===----------------------------------------------------------------------===//

#include "CIndexer.h"
#include "CXSourceLocation.h"
#include "CXTranslationUnit.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/Frontend/ASTUnit.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

// extern "C" {                // HLSL Change -Don't use c linkage.
void clang_getInclusions(CXTranslationUnit TU, CXInclusionVisitor CB,
                         CXClientData clientData) {
  if (cxtu::isNotUsableTU(TU)) {
    LOG_BAD_TU(TU);
    return;
  }

  ASTUnit *CXXUnit = cxtu::getASTUnit(TU);
  SourceManager &SM = CXXUnit->getSourceManager();
  ASTContext &Ctx = CXXUnit->getASTContext();

  SmallVector<CXSourceLocation, 10> InclusionStack;
  unsigned n =  SM.local_sloc_entry_size();

  // In the case where all the SLocEntries are in an external source, traverse
  // those SLocEntries as well.  This is the case where we are looking
  // at the inclusion stack of an AST/PCH file.
  const SrcMgr::SLocEntry &(SourceManager::*Getter)(unsigned, bool*) const;
  if (n == 1) {
    Getter = &SourceManager::getLoadedSLocEntry;
    n = SM.loaded_sloc_entry_size();
  } else
    Getter = &SourceManager::getLocalSLocEntry;

  for (unsigned i = 0 ; i < n ; ++i) {
    bool Invalid = false;
    const SrcMgr::SLocEntry &SL = (SM.*Getter)(i, &Invalid);
    
    if (!SL.isFile() || Invalid)
      continue;

    const SrcMgr::FileInfo &FI = SL.getFile();
    if (!FI.getContentCache()->OrigEntry)
      continue;
    
    // Build the inclusion stack.
    SourceLocation L = FI.getIncludeLoc();
    InclusionStack.clear();
    while (L.isValid()) {
      PresumedLoc PLoc = SM.getPresumedLoc(L);
      InclusionStack.push_back(cxloc::translateSourceLocation(Ctx, L));
      L = PLoc.isValid()? PLoc.getIncludeLoc() : SourceLocation();
    }
            
    // Callback to the client.
    // FIXME: We should have a function to construct CXFiles.
    CB(static_cast<CXFile>(
         const_cast<FileEntry *>(FI.getContentCache()->OrigEntry)), 
       InclusionStack.data(), InclusionStack.size(), clientData);
  }    
}
// } // end extern C     // HLSL Change -Don't use c linkage.
