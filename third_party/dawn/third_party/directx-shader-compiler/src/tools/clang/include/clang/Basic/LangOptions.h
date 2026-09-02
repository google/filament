//===--- LangOptions.h - C Language Family Language Options -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the clang::LangOptions interface.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_LANGOPTIONS_H
#define LLVM_CLANG_BASIC_LANGOPTIONS_H

#include "dxc/DXIL/DxilConstants.h" // For DXIL:: default values.
#include "dxc/Support/HLSLVersion.h"
#include "clang/Basic/CommentOptions.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/ObjCRuntime.h"
#include "clang/Basic/Sanitizers.h"
#include "clang/Basic/Visibility.h"
#include <string>
#include <vector>

namespace clang {

/// Bitfields of LangOptions, split out from LangOptions in order to ensure that
/// this large collection of bitfields is a trivial class type.
class LangOptionsBase {
public:
  // Define simple language options (with no accessors).
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS

#define LANGOPT(Name, Bits, Default, Description) unsigned Name : Bits;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "clang/Basic/LangOptions.def"

#else

#define LANGOPT(Name, Bits, Default, Description) static const unsigned Name = Default;
#define LANGOPT_BOOL(Name, Default, Description) static const bool Name = static_cast<bool>( Default );
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "clang/Basic/LangOptions.fixed.def"

#endif

protected:
  // Define language options of enumeration type. These are private, and will
  // have accessors (below).
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS
#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  unsigned Name : Bits;
#include "clang/Basic/LangOptions.def"
#endif
};

// #ifndef MS_SUPPORT_VARIABLE_LANGOPTS
// #define LANGOPT(Name, Bits, Default, Description) __declspec(selectany) unsigned LangOptionsBase::#Name = Default;
// #define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
// #include "clang/Basic/LangOptions.fixed.def"
// #endif

/// \brief Keeps track of the various options that can be
/// enabled, which controls the dialect of C or C++ that is accepted.
class LangOptions : public LangOptionsBase {
public:
  typedef clang::Visibility Visibility;
  
  enum GCMode { NonGC, GCOnly, HybridGC };
  enum StackProtectorMode { SSPOff, SSPOn, SSPStrong, SSPReq };
  
  enum SignedOverflowBehaviorTy {
    SOB_Undefined,  // Default C standard behavior.
    SOB_Defined,    // -fwrapv
    SOB_Trapping    // -ftrapv
  };

  enum PragmaMSPointersToMembersKind {
    PPTMK_BestCase,
    PPTMK_FullGeneralitySingleInheritance,
    PPTMK_FullGeneralityMultipleInheritance,
    PPTMK_FullGeneralityVirtualInheritance
  };

  enum AddrSpaceMapMangling { ASMM_Target, ASMM_On, ASMM_Off };

  enum MSVCMajorVersion {
    MSVC2010 = 16,
    MSVC2012 = 17,
    MSVC2013 = 18,
    MSVC2015 = 19
  };

public:
  /// \brief Set of enabled sanitizers.
  SanitizerSet Sanitize;

  /// \brief Paths to blacklist files specifying which objects
  /// (files, functions, variables) should not be instrumented.
  std::vector<std::string> SanitizerBlacklistFiles;

  clang::ObjCRuntime ObjCRuntime;

  std::string ObjCConstantStringClass;
  
  /// \brief The name of the handler function to be called when -ftrapv is
  /// specified.
  ///
  /// If none is specified, abort (GCC-compatible behaviour).
  std::string OverflowHandler;

  /// \brief The name of the current module.
  std::string CurrentModule;

  /// \brief The name of the module that the translation unit is an
  /// implementation of. Prevents semantic imports, but does not otherwise
  /// treat this as the CurrentModule.
  std::string ImplementationOfModule;

  /// \brief The names of any features to enable in module 'requires' decls
  /// in addition to the hard-coded list in Module.cpp and the target features.
  ///
  /// This list is sorted.
  std::vector<std::string> ModuleFeatures;

  /// \brief Options for parsing comments.
  CommentOptions CommentOpts;
  
  LangOptions();

  // Define accessors/mutators for language options of enumeration type.
#ifdef MS_SUPPORT_VARIABLE_LANGOPTS

#define LANGOPT(Name, Bits, Default, Description) 
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  void set##Name(Type Value) { Name = static_cast<unsigned>(Value); }  
#include "clang/Basic/LangOptions.def"
  
#else

#define LANGOPT(Name, Bits, Default, Description) 
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Type get##Name() const { return static_cast<Type>(Default); } \
  void set##Name(Type Value) { assert(Value == Default); }  
#include "clang/Basic/LangOptions.fixed.def"

#endif

  // HLSL Change Starts
  hlsl::LangStd HLSLVersion = hlsl::LangStd::vLatest;
  std::string HLSLEntryFunction;
  std::string HLSLProfile;
  unsigned RootSigMajor = 1;
  unsigned RootSigMinor = 1;
  bool IsHLSLLibrary = false;
  bool UseMinPrecision = true; // use min precision, not native precision.
  bool EnableDX9CompatMode = false;
  bool EnableFXCCompatMode = false;
  bool EnablePayloadAccessQualifiers = false;
  bool DumpImplicitTopLevelDecls = true;
  bool ExportShadersOnly = false;
  hlsl::DXIL::DefaultLinkage DefaultLinkage =
      hlsl::DXIL::DefaultLinkage::Default;
  /// Whether use row major as default matrix major.
  bool HLSLDefaultRowMajor = false;
  unsigned MaxHLSLVectorLength = hlsl::DXIL::kDefaultMaxVectorLength;
  // HLSL Change Ends

  bool SPIRV = false;  // SPIRV Change
  unsigned SpirvMajorVersion; // SPIRV Change
  unsigned SpirvMinorVersion; // SPIRV Change

  bool isSignedOverflowDefined() const {
    return getSignedOverflowBehavior() == SOB_Defined;
  }
  
  bool isSubscriptPointerArithmetic() const {
    return ObjCRuntime.isSubscriptPointerArithmetic() &&
           !ObjCSubscriptingLegacyRuntime;
  }

  bool isCompatibleWithMSVC(MSVCMajorVersion MajorVersion) const {
    return MSCompatibilityVersion >= MajorVersion * 10000000U;
  }

  /// \brief Reset all of the options that are not considered when building a
  /// module.
  void resetNonModularOptions();
};

/// \brief Floating point control options
class FPOptions {
public:
  unsigned fp_contract : 1;

  FPOptions() : fp_contract(0) {}

  FPOptions(const LangOptions &LangOpts) :
    fp_contract(LangOpts.DefaultFPContract) {}
};

/// \brief OpenCL volatile options
class OpenCLOptions {
public:
#define OPENCLEXT(nm)  unsigned nm : 1;
#include "clang/Basic/OpenCLExtensions.def"

  OpenCLOptions() {
#define OPENCLEXT(nm)   nm = 0;
#include "clang/Basic/OpenCLExtensions.def"
  }
};

/// \brief Describes the kind of translation unit being processed.
enum TranslationUnitKind {
  /// \brief The translation unit is a complete translation unit.
  TU_Complete,
  /// \brief The translation unit is a prefix to a translation unit, and is
  /// not complete.
  TU_Prefix,
  /// \brief The translation unit is a module.
  TU_Module
};
  
}  // end namespace clang

#endif
