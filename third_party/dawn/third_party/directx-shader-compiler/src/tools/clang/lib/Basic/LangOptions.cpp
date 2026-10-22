//===--- LangOptions.cpp - C Language Family Language Options ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the LangOptions class.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/LangOptions.h"
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
using namespace clang;

#ifdef LLVM_ON_UNIX
#ifndef MS_SUPPORT_VARIABLE_LANGOPTS
#define LANGOPT(Name, Bits, Default, Description) const unsigned LangOptionsBase::Name;
#define LANGOPT_BOOL(Name, Default, Description) const bool LangOptionsBase::Name;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "clang/Basic/LangOptions.fixed.def"
#endif // MS_SUPPORT_VARIABLE_LANGOPTS
#endif // LLVM_ON_UNIX

LangOptions::LangOptions() 
    : HLSLVersion(hlsl::LangStd::vLatest) {
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
#define LANGOPT(Name, Bits, Default, Description) Name = Default;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) set##Name(Default);
#include "clang/Basic/LangOptions.def"
#endif
}

void LangOptions::resetNonModularOptions() {
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
#define LANGOPT(Name, Bits, Default, Description)
#define BENIGN_LANGOPT(Name, Bits, Default, Description) Name = Default;
#define BENIGN_ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Name = Default;
#include "clang/Basic/LangOptions.def"
#endif

  // FIXME: This should not be reset; modules can be different with different
  // sanitizer options (this affects __has_feature(address_sanitizer) etc).
  Sanitize.clear();
  SanitizerBlacklistFiles.clear();

  CurrentModule.clear();
  ImplementationOfModule.clear();
}

