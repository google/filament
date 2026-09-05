//===--- CodeGenOptions.h ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CodeGenOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_CODEGENOPTIONS_H
#define LLVM_CLANG_FRONTEND_CODEGENOPTIONS_H

#include "clang/Basic/Sanitizers.h"
#include "llvm/Support/Regex.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "dxc/HLSL/HLSLExtensionsCodegenHelper.h" // HLSL change
#include "dxc/Support/SPIRVOptions.h" // SPIR-V Change
#include "dxc/DxcBindingTable/DxcBindingTable.h" // HLSL change
#include "dxc/Support/DxcOptToggles.h" // HLSL change

namespace clang {

/// \brief Bitfields of CodeGenOptions, split out from CodeGenOptions to ensure
/// that this large collection of bitfields is a trivial class type.
class CodeGenOptionsBase {
public:
#define CODEGENOPT(Name, Bits, Default) unsigned Name : Bits;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default)
#include "clang/Frontend/CodeGenOptions.def"

protected:
#define CODEGENOPT(Name, Bits, Default)
#define ENUM_CODEGENOPT(Name, Type, Bits, Default) unsigned Name : Bits;
#include "clang/Frontend/CodeGenOptions.def"
};

/// CodeGenOptions - Track various options which control how the code
/// is optimized and passed to the backend.
class CodeGenOptions : public CodeGenOptionsBase {
public:
  enum InliningMethod {
    NoInlining,         // Perform no inlining whatsoever.
    NormalInlining,     // Use the standard function inlining pass.
    OnlyAlwaysInlining  // Only run the always inlining pass.
  };

  enum VectorLibrary {
    NoLibrary, // Don't use any vector library.
    Accelerate // Use the Accelerate framework.
  };

  enum ObjCDispatchMethodKind {
    Legacy = 0,
    NonLegacy = 1,
    Mixed = 2
  };

  enum DebugInfoKind {
    NoDebugInfo,          /// Don't generate debug info.

    LocTrackingOnly,      /// Emit location information but do not generate
                          /// debug info in the output. This is useful in
                          /// cases where the backend wants to track source
                          /// locations for instructions without actually
                          /// emitting debug info for them (e.g., when -Rpass
                          /// is used).

    DebugLineTablesOnly,  /// Emit only debug info necessary for generating
                          /// line number tables (-gline-tables-only).

    LimitedDebugInfo,     /// Limit generated debug info to reduce size
                          /// (-fno-standalone-debug). This emits
                          /// forward decls for types that could be
                          /// replaced with forward decls in the source
                          /// code. For dynamic C++ classes type info
                          /// is only emitted int the module that
                          /// contains the classe's vtable.

    FullDebugInfo         /// Generate complete debug info.
  };

  enum TLSModel {
    GeneralDynamicTLSModel,
    LocalDynamicTLSModel,
    InitialExecTLSModel,
    LocalExecTLSModel
  };

  enum FPContractModeKind {
    FPC_Off,        // Form fused FP ops only where result will not be affected.
    FPC_On,         // Form fused FP ops according to FP_CONTRACT rules.
    FPC_Fast        // Aggressively fuse FP ops (E.g. FMA).
  };

  enum StructReturnConventionKind {
    SRCK_Default,  // No special option was passed.
    SRCK_OnStack,  // Small structs on the stack (-fpcc-struct-return).
    SRCK_InRegs    // Small structs in registers (-freg-struct-return).
  };

  /// The code model to use (-mcmodel).
  std::string CodeModel;

  /// The filename with path we use for coverage files. The extension will be
  /// replaced.
  std::string CoverageFile;

  /// The version string to put into coverage files.
  char CoverageVersion[4];

  /// Enable additional debugging information.
  std::string DebugPass;

  /// The string to embed in debug information as the current working directory.
  std::string DebugCompilationDir;

  /// The string to embed in the debug information for the compile unit, if
  /// non-empty.
  std::string DwarfDebugFlags;

  /// The ABI to use for passing floating point arguments.
  std::string FloatABI;

  /// The float precision limit to use, if non-empty.
  std::string LimitFloatPrecision;

  /// The name of the bitcode file to link before optzns.
  std::string LinkBitcodeFile;

  /// The user provided name for the "main file", if non-empty. This is useful
  /// in situations where the input file name does not match the original input
  /// file, for example with -save-temps.
  std::string MainFileName;

  /// The name for the split debug info file that we'll break out. This is used
  /// in the backend for setting the name in the skeleton cu.
  std::string SplitDwarfFile;

  /// The name of the relocation model to use.
  std::string RelocationModel;

  /// The thread model to use
  std::string ThreadModel;

  /// If not an empty string, trap intrinsics are lowered to calls to this
  /// function instead of to trap instructions.
  std::string TrapFuncName;

  /// A list of command-line options to forward to the LLVM backend.
  std::vector<std::string> BackendOptions;

  /// A list of dependent libraries.
  std::vector<std::string> DependentLibraries;

  /// Name of the profile file to use as output for -fprofile-instr-generate
  /// and -fprofile-generate.
  std::string InstrProfileOutput;

  /// Name of the profile file to use with -fprofile-sample-use.
  std::string SampleProfileFile;

  /// Name of the profile file to use as input for -fprofile-instr-use
  std::string InstrProfileInput;

  /// A list of file names passed with -fcuda-include-gpubinary options to
  /// forward to CUDA runtime back-end for incorporating them into host-side
  /// object file.
  std::vector<std::string> CudaGpuBinaryFileNames;
  // HLSL Change Starts
  /// Entry point of hlsl shader passed with -hlsl-entry.
  std::string HLSLEntryFunction;
  /// Shader model to target for hlsl shader passed with -hlsl-profile.
  std::string HLSLProfile;
  /// Whether to target high-level DXIL.
  bool HLSLHighLevel = false;
  /// Whether we allow preserve intermediate values
  bool HLSLAllowPreserveValues = false;
  /// Whether we fail compilation if loop fails to unroll
  bool HLSLOnlyWarnOnUnrollFail = false;
  /// Whether use legacy resource reservation.
  bool HLSLLegacyResourceReservation = false;
  /// Set [branch] on every if.
  bool HLSLPreferControlFlow = false;
  /// Set [flatten] on every if.
  bool HLSLAvoidControlFlow = false;
  /// Force [flatten] on every if.
  bool HLSLAllResourcesBound = false;
  /// Skip adding optional semantics defines except ones which are required for correctness.
  bool HLSLIgnoreOptSemDefs = false;
  /// List of semantic defines that must be ignored.
  std::set<std::string> HLSLIgnoreSemDefs;
  /// List of semantic defines that must be overridden with user-provided values.
  std::map<std::string, std::string> HLSLOverrideSemDefs;
  /// Major version of validator to run.
  unsigned HLSLValidatorMajorVer = 0;
  /// Minor version of validator to run.
  unsigned HLSLValidatorMinorVer = 0;
  /// Define macros passed in from command line
  std::vector<std::string> HLSLDefines;
  /// Precise output passed in from command line
  std::vector<std::string> HLSLPreciseOutputs;
  /// Arguments passed in from command line
  std::vector<std::string> HLSLArguments;
  /// Helper for generating llvm bitcode for hlsl extensions.
  std::shared_ptr<hlsl::HLSLExtensionsCodegenHelper> HLSLExtensionsCodegen;
  /// Signature packing mode (0 == default for target)
  unsigned HLSLSignaturePackingStrategy = 0;
  /// denormalized number mode ("ieee" for default)
  hlsl::DXIL::Float32DenormMode HLSLFloat32DenormMode;
  /// HLSLDefaultSpace also enables automatic binding for libraries if set. UINT_MAX == unset
  unsigned HLSLDefaultSpace = UINT_MAX;
  /// HLSLLibraryExports specifies desired exports, with optional renaming
  std::vector<std::string> HLSLLibraryExports;
  /// ExportShadersOnly limits library export functions to shaders
  bool ExportShadersOnly = false;
  /// DefaultLinkage Internal, External, or Default.  If Default, default
  /// function linkage is determined by library target.
  hlsl::DXIL::DefaultLinkage DefaultLinkage = hlsl::DXIL::DefaultLinkage::Default;
  /// Assume UAVs/SRVs may alias.
  bool HLSLResMayAlias = false;
  /// Lookback scan limit for memory dependencies
  unsigned ScanLimit = 0;
  /// Optimization pass enables, disables and selects
  hlsl::options::OptimizationToggles HLSLOptimizationToggles;
  /// Debug option to print IR before every pass
  bool HLSLPrintBeforeAll = false;
  /// Debug option to print IR before specific pass
  std::set<std::string> HLSLPrintBefore;
  /// Debug option to print IR after every pass
  bool HLSLPrintAfterAll = false;
  /// Debug option to print IR after specific pass
  std::set<std::string> HLSLPrintAfter;
  /// Force-replace lifetime intrinsics by zeroinitializer stores.
  bool HLSLForceZeroStoreLifetimes = false;
  /// Enable lifetime marker generation
  bool HLSLEnableLifetimeMarkers = false;
  /// Put shader sources and options in the module
  bool HLSLEmbedSourcesInModule = false;
  /// Enable generation of payload access qualifier metadata. 
  bool HLSLEnablePayloadAccessQualifiers = false;
  /// Binding table for HLSL resources
  hlsl::DxcBindingTable HLSLBindingTable;
  /// Binding table #define
  struct BindingTableParserType {
    virtual ~BindingTableParserType() {};
    virtual bool Parse(llvm::raw_ostream &os, hlsl::DxcBindingTable *outBindingTable) = 0;
  };
  std::shared_ptr<BindingTableParserType> BindingTableParser;
  // HLSL Change Ends

  // SPIRV Change Starts
#ifdef ENABLE_SPIRV_CODEGEN
  clang::spirv::SpirvCodeGenOptions SpirvOptions;
#endif
  // SPIRV Change Ends

  /// Regular expression to select optimizations for which we should enable
  /// optimization remarks. Transformation passes whose name matches this
  /// expression (and support this feature), will emit a diagnostic
  /// whenever they perform a transformation. This is enabled by the
  /// -Rpass=regexp flag.
  std::shared_ptr<llvm::Regex> OptimizationRemarkPattern;

  /// Regular expression to select optimizations for which we should enable
  /// missed optimization remarks. Transformation passes whose name matches this
  /// expression (and support this feature), will emit a diagnostic
  /// whenever they tried but failed to perform a transformation. This is
  /// enabled by the -Rpass-missed=regexp flag.
  std::shared_ptr<llvm::Regex> OptimizationRemarkMissedPattern;

  /// Regular expression to select optimizations for which we should enable
  /// optimization analyses. Transformation passes whose name matches this
  /// expression (and support this feature), will emit a diagnostic
  /// whenever they want to explain why they decided to apply or not apply
  /// a given transformation. This is enabled by the -Rpass-analysis=regexp
  /// flag.
  std::shared_ptr<llvm::Regex> OptimizationRemarkAnalysisPattern;

  /// Set of files definining the rules for the symbol rewriting.
  std::vector<std::string> RewriteMapFiles;

  /// Set of sanitizer checks that are non-fatal (i.e. execution should be
  /// continued when possible).
  SanitizerSet SanitizeRecover;

  /// Set of sanitizer checks that trap rather than diagnose.
  SanitizerSet SanitizeTrap;

public:
  // Define accessors/mutators for code generation options of enumeration type.
#define CODEGENOPT(Name, Bits, Default)
#define ENUM_CODEGENOPT(Name, Type, Bits, Default) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  void set##Name(Type Value) { Name = static_cast<unsigned>(Value); }
#include "clang/Frontend/CodeGenOptions.def"

  CodeGenOptions();
};

}  // end namespace clang

#endif
