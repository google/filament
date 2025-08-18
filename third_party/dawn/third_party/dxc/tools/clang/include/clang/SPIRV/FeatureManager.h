//===------ FeatureManager.h - SPIR-V Version/Extension Manager -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
//
//  This file defines a SPIR-V version and extension manager.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_FEATUREMANAGER_H
#define LLVM_CLANG_LIB_SPIRV_FEATUREMANAGER_H

#include <string>

#include "spirv-tools/libspirv.h"

#include "dxc/Support/SPIRVOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/VersionTuple.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace spirv {

/// A list of SPIR-V extensions known to our CodeGen.
enum class Extension {
  KHR = 0,
  KHR_16bit_storage,
  KHR_device_group,
  KHR_fragment_shading_rate,
  KHR_non_semantic_info,
  KHR_multiview,
  KHR_shader_draw_parameters,
  KHR_post_depth_coverage,
  KHR_ray_tracing,
  KHR_shader_clock,
  EXT_demote_to_helper_invocation,
  EXT_descriptor_indexing,
  EXT_fragment_fully_covered,
  EXT_fragment_invocation_density,
  EXT_fragment_shader_interlock,
  EXT_mesh_shader,
  EXT_shader_stencil_export,
  EXT_shader_viewport_index_layer,
  AMD_gpu_shader_half_float,
  AMD_shader_early_and_late_fragment_tests,
  GOOGLE_hlsl_functionality1,
  GOOGLE_user_type,
  NV_ray_tracing,
  NV_mesh_shader,
  KHR_ray_query,
  EXT_shader_image_int64,
  KHR_physical_storage_buffer,
  AMD_shader_enqueue,
  KHR_vulkan_memory_model,
  NV_compute_shader_derivatives,
  KHR_compute_shader_derivatives,
  KHR_fragment_shader_barycentric,
  KHR_maximal_reconvergence,
  KHR_float_controls,
  NV_shader_subgroup_partitioned,
  KHR_quad_control,
  Unknown,
};

/// The class for handling SPIR-V version and extension requests.
class FeatureManager {
public:
  FeatureManager(DiagnosticsEngine &de, const SpirvCodeGenOptions &);

  /// Allows the given extension to be used in CodeGen.
  bool allowExtension(llvm::StringRef);

  /// Allows all extensions to be used in CodeGen.
  void allowAllKnownExtensions();

  /// Requests the given extension for translating the given target feature at
  /// the given source location. Emits an error if the given extension is not
  /// permitted to use.
  bool requestExtension(Extension, llvm::StringRef target, SourceLocation);

  /// Translates extension name to symbol.
  Extension getExtensionSymbol(llvm::StringRef name);

  /// Translates extension symbol to name.
  const char *getExtensionName(Extension symbol);

  /// Returns true if the given extension is a KHR extension.
  bool isKHRExtension(llvm::StringRef name);

  /// Returns the names of all known extensions as a string.
  std::string getKnownExtensions(const char *delimiter, const char *prefix = "",
                                 const char *postfix = "");

  /// Request the given target environment for translating the given feature at
  /// the given source location. Emits an error if the requested target
  /// environment does not match user's target environemnt.
  bool requestTargetEnv(spv_target_env, llvm::StringRef target, SourceLocation);

  /// Returns the target environment corresponding to the target environment
  /// that was specified as command line option. If no option is specified, the
  /// default (Vulkan 1.0) is returned.
  spv_target_env getTargetEnv() const { return targetEnv; }

  /// Returns true if the given extension is not part of the core of the target
  /// environment.
  bool isExtensionRequiredForTargetEnv(Extension);

  /// Returns true if the given extension is set in allowedExtensions
  bool isExtensionEnabled(Extension);

  /// Returns true if the target environment is Vulkan 1.1 or above.
  /// Returns false otherwise.
  bool isTargetEnvVulkan1p1OrAbove();

  /// Returns true if the target environment is SPIR-V 1.4 or above.
  /// Returns false otherwise.
  bool isTargetEnvSpirv1p4OrAbove();

  /// Returns true if the target environment is Vulkan 1.1 with SPIR-V 1.4 or
  /// above. Returns false otherwise.
  bool isTargetEnvVulkan1p1Spirv1p4OrAbove();

  /// Returns true if the target environment is Vulkan 1.2 or above.
  /// Returns false otherwise.
  bool isTargetEnvVulkan1p2OrAbove();

  /// Returns true if the target environment is Vulkan 1.3 or above.
  /// Returns false otherwise.
  bool isTargetEnvVulkan1p3OrAbove();

  /// Return true if the target environment is a Vulkan environment.
  bool isTargetEnvVulkan();

  /// Returns the spv_target_env matching the input string if possible.
  /// This functions matches the spv_target_env with the command-line version
  /// of the name ('vulkan1.1', not 'Vulkan 1.1').
  /// Returns an empty Optional if no matching env is found.
  static llvm::Optional<spv_target_env>
  stringToSpvEnvironment(const std::string &target_env);

  // Returns the SPIR-V version used for the target environment.
  static clang::VersionTuple getSpirvVersion(spv_target_env env);

  /// Returns the equivalent to spv_target_env in pretty, human readable form.
  /// (SPV_ENV_VULKAN_1_0 -> "Vulkan 1.0").
  /// Returns an empty Optional if the name cannot be matched.
  static llvm::Optional<std::string>
  spvEnvironmentToPrettyName(spv_target_env target_env);

private:
  /// Returns whether codegen should allow usage of this extension by default.
  bool enabledByDefault(Extension);

  /// \brief Wrapper method to create an error message and report it
  /// in the diagnostic engine associated with this object.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Error, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create an note message and report it
  /// in the diagnostic engine associated with this object.
  template <unsigned N>
  DiagnosticBuilder emitNote(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Note, message);
    return diags.Report(loc, diagId);
  }

  DiagnosticsEngine &diags;

  llvm::SmallBitVector allowedExtensions;
  spv_target_env targetEnv;
  std::string targetEnvStr;
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_FEATUREMANAGER_H
