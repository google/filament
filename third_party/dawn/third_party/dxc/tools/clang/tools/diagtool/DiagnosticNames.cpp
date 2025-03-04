//===- DiagnosticNames.cpp - Defines a table of all builtin diagnostics ----==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DiagnosticNames.h"
#include "clang/Basic/AllDiagnostics.h"
#include "llvm/ADT/STLExtras.h"

using namespace clang;
using namespace diagtool;

static const DiagnosticRecord BuiltinDiagnosticsByName[] = {
#define DIAG_NAME_INDEX(ENUM) { #ENUM, diag::ENUM, STR_SIZE(#ENUM, uint8_t) },
#include "clang/Basic/DiagnosticIndexName.inc"
#undef DIAG_NAME_INDEX
};

llvm::ArrayRef<DiagnosticRecord> diagtool::getBuiltinDiagnosticsByName() {
  return llvm::makeArrayRef(BuiltinDiagnosticsByName);
}


// FIXME: Is it worth having two tables, especially when this one can get
// out of sync easily?
static const DiagnosticRecord BuiltinDiagnosticsByID[] = {
#define DIAG(ENUM,CLASS,DEFAULT_MAPPING,DESC,GROUP,               \
             SFINAE,NOWERROR,SHOWINSYSHEADER,CATEGORY)            \
  { #ENUM, diag::ENUM, STR_SIZE(#ENUM, uint8_t) },
#include "clang/Basic/DiagnosticCommonKinds.inc"
#include "clang/Basic/DiagnosticDriverKinds.inc"
#include "clang/Basic/DiagnosticFrontendKinds.inc"
#include "clang/Basic/DiagnosticSerializationKinds.inc"
#include "clang/Basic/DiagnosticLexKinds.inc"
#include "clang/Basic/DiagnosticParseKinds.inc"
#include "clang/Basic/DiagnosticASTKinds.inc"
#include "clang/Basic/DiagnosticCommentKinds.inc"
#include "clang/Basic/DiagnosticSemaKinds.inc"
#include "clang/Basic/DiagnosticAnalysisKinds.inc"
#undef DIAG
};

static bool orderByID(const DiagnosticRecord &Left,
                      const DiagnosticRecord &Right) {
  return Left.DiagID < Right.DiagID;
}

const DiagnosticRecord &diagtool::getDiagnosticForID(short DiagID) {
  DiagnosticRecord Key = {nullptr, DiagID, 0};

  const DiagnosticRecord *Result =
    std::lower_bound(std::begin(BuiltinDiagnosticsByID),
                     std::end(BuiltinDiagnosticsByID),
                     Key, orderByID);
  assert(Result && "diagnostic not found; table may be out of date");
  return *Result;
}


#define GET_DIAG_ARRAYS
#include "clang/Basic/DiagnosticGroups.inc"
#undef GET_DIAG_ARRAYS

// Second the table of options, sorted by name for fast binary lookup.
static const GroupRecord OptionTable[] = {
#define GET_DIAG_TABLE
#include "clang/Basic/DiagnosticGroups.inc"
#undef GET_DIAG_TABLE
};

llvm::StringRef GroupRecord::getName() const {
  return StringRef(DiagGroupNames + NameOffset + 1, DiagGroupNames[NameOffset]);
}

GroupRecord::subgroup_iterator GroupRecord::subgroup_begin() const {
  return DiagSubGroups + SubGroups;
}

GroupRecord::subgroup_iterator GroupRecord::subgroup_end() const {
  return nullptr;
}

GroupRecord::diagnostics_iterator GroupRecord::diagnostics_begin() const {
  return DiagArrays + Members;
}

GroupRecord::diagnostics_iterator GroupRecord::diagnostics_end() const {
  return nullptr;
}

llvm::ArrayRef<GroupRecord> diagtool::getDiagnosticGroups() {
  return llvm::makeArrayRef(OptionTable);
}
