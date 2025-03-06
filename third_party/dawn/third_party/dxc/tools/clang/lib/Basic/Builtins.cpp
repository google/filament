//===--- Builtins.cpp - Builtin function implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements various things for builtin functions.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/Builtins.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
using namespace clang;

static const Builtin::Info BuiltinInfo[] = {
  { "not a builtin function", nullptr, nullptr, nullptr, ALL_LANGUAGES},
#define BUILTIN(ID, TYPE, ATTRS) { #ID, TYPE, ATTRS, 0, ALL_LANGUAGES },
#define LANGBUILTIN(ID, TYPE, ATTRS, BUILTIN_LANG) { #ID, TYPE, ATTRS, 0, BUILTIN_LANG },
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER, BUILTIN_LANG) { #ID, TYPE, ATTRS, HEADER,\
                                                            BUILTIN_LANG },
#include "clang/Basic/Builtins.def"
};

const Builtin::Info &Builtin::Context::GetRecord(unsigned ID) const {
  if (ID < Builtin::FirstTSBuiltin)
    return BuiltinInfo[ID];
  assert(ID - Builtin::FirstTSBuiltin < NumTSRecords && "Invalid builtin ID!");
  return TSRecords[ID - Builtin::FirstTSBuiltin];
}

Builtin::Context::Context() {
  // Get the target specific builtins from the target.
  TSRecords = nullptr;
  NumTSRecords = 0;
}

void Builtin::Context::InitializeTarget(const TargetInfo &Target) {
  assert(NumTSRecords == 0 && "Already initialized target?");
  Target.getTargetBuiltins(TSRecords, NumTSRecords);  
}

bool Builtin::Context::BuiltinIsSupported(const Builtin::Info &BuiltinInfo,
                                          const LangOptions &LangOpts) {
  bool BuiltinsUnsupported = LangOpts.NoBuiltin &&
                             strchr(BuiltinInfo.Attributes, 'f');
  bool MathBuiltinsUnsupported =
    LangOpts.NoMathBuiltin && BuiltinInfo.HeaderName &&      
    llvm::StringRef(BuiltinInfo.HeaderName).equals("math.h");
  bool GnuModeUnsupported = !LangOpts.GNUMode &&
                            (BuiltinInfo.builtin_lang & GNU_LANG);
  bool MSModeUnsupported = !LangOpts.MicrosoftExt &&
                           (BuiltinInfo.builtin_lang & MS_LANG);
  bool ObjCUnsupported = !LangOpts.ObjC1 &&
                         BuiltinInfo.builtin_lang == OBJC_LANG;
  return !BuiltinsUnsupported && !MathBuiltinsUnsupported &&
         !GnuModeUnsupported && !MSModeUnsupported && !ObjCUnsupported;
}

/// InitializeBuiltins - Mark the identifiers for all the builtins with their
/// appropriate builtin ID # and mark any non-portable builtin identifiers as
/// such.
void Builtin::Context::InitializeBuiltins(IdentifierTable &Table,
                                          const LangOptions& LangOpts) {
  // Step #1: mark all target-independent builtins with their ID's.
  for (unsigned i = Builtin::NotBuiltin+1; i != Builtin::FirstTSBuiltin; ++i)
    if (BuiltinIsSupported(BuiltinInfo[i], LangOpts)) {
      Table.get(BuiltinInfo[i].Name).setBuiltinID(i);
    }

  // Step #2: Register target-specific builtins.
  for (unsigned i = 0, e = NumTSRecords; i != e; ++i)
    if (BuiltinIsSupported(TSRecords[i], LangOpts))
      Table.get(TSRecords[i].Name).setBuiltinID(i+Builtin::FirstTSBuiltin);
}

void
Builtin::Context::GetBuiltinNames(SmallVectorImpl<const char *> &Names) {
  // Final all target-independent names
  for (unsigned i = Builtin::NotBuiltin+1; i != Builtin::FirstTSBuiltin; ++i)
    if (!strchr(BuiltinInfo[i].Attributes, 'f'))
      Names.push_back(BuiltinInfo[i].Name);

  // Find target-specific names.
  for (unsigned i = 0, e = NumTSRecords; i != e; ++i)
    if (!strchr(TSRecords[i].Attributes, 'f'))
      Names.push_back(TSRecords[i].Name);
}

void Builtin::Context::ForgetBuiltin(unsigned ID, IdentifierTable &Table) {
  Table.get(GetRecord(ID).Name).setBuiltinID(0);
}

bool Builtin::Context::isLike(unsigned ID, unsigned &FormatIdx,
                              bool &HasVAListArg, const char *Fmt) const {
  assert(Fmt && "Not passed a format string");
  assert(::strlen(Fmt) == 2 &&
         "Format string needs to be two characters long");
  assert(::toupper(Fmt[0]) == Fmt[1] &&
         "Format string is not in the form \"xX\"");

  const char *Like = ::strpbrk(GetRecord(ID).Attributes, Fmt);
  if (!Like)
    return false;

  HasVAListArg = (*Like == Fmt[1]);

  ++Like;
  assert(*Like == ':' && "Format specifier must be followed by a ':'");
  ++Like;

  assert(::strchr(Like, ':') && "Format specifier must end with a ':'");
  FormatIdx = ::strtol(Like, nullptr, 10);
  return true;
}

bool Builtin::Context::isPrintfLike(unsigned ID, unsigned &FormatIdx,
                                    bool &HasVAListArg) {
  return isLike(ID, FormatIdx, HasVAListArg, "pP");
}

bool Builtin::Context::isScanfLike(unsigned ID, unsigned &FormatIdx,
                                   bool &HasVAListArg) {
  return isLike(ID, FormatIdx, HasVAListArg, "sS");
}
