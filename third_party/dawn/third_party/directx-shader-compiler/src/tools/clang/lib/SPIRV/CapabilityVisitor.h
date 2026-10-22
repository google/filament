//===--- CapabilityVisitor.h - Capability Visitor ----------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_CAPABILITYVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_CAPABILITYVISITOR_H

#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class SpirvBuilder;

class CapabilityVisitor : public Visitor {
public:
  CapabilityVisitor(ASTContext &astCtx, SpirvContext &spvCtx,
                    const SpirvCodeGenOptions &opts, SpirvBuilder &builder,
                    FeatureManager &featureMgr)
      : Visitor(opts, spvCtx), spvBuilder(builder),
        shaderModel(spv::ExecutionModel::Max), featureManager(featureMgr) {}

  bool visit(SpirvModule *, Phase) override;

  bool visit(SpirvDecoration *decor) override;
  bool visit(SpirvEntryPoint *) override;
  bool visit(SpirvExecutionModeBase *execMode) override;
  bool visit(SpirvImageQuery *) override;
  bool visit(SpirvImageOp *) override;
  bool visit(SpirvImageSparseTexelsResident *) override;
  bool visit(SpirvExtInstImport *) override;
  bool visit(SpirvAtomic *) override;
  bool visit(SpirvDemoteToHelperInvocation *) override;
  bool visit(SpirvIsHelperInvocationEXT *) override;
  bool visit(SpirvReadClock *) override;

  using Visitor::visit;

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override;

private:
  /// Adds necessary capabilities for using the given type.
  /// The called may also provide the storage class for variable types, because
  /// in the case of variable types, the storage class may affect the capability
  /// that is used.
  void addCapabilityForType(const SpirvType *, SourceLocation loc,
                            spv::StorageClass sc);

  /// Checks that the given extension is a valid extension for the target
  /// environment (e.g. Vulkan 1.0). And if so, utilizes the SpirvBuilder to add
  /// the given extension to the SPIR-V module in memory.
  void addExtension(Extension ext, llvm::StringRef target, SourceLocation loc);

  /// Checks that the given extension is enabled based on command line arguments
  /// before calling addExtension and addCapability.
  /// Returns `true` if the extension was enabled, `false` otherwise.
  bool addExtensionAndCapabilitiesIfEnabled(
      Extension ext, llvm::ArrayRef<spv::Capability> capabilities);

  /// Checks that the given capability is a valid capability. And if so,
  /// utilizes the SpirvBuilder to add the given capability to the SPIR-V module
  /// in memory.
  void addCapability(spv::Capability, SourceLocation loc = {});

  /// Returns the capability required to non-uniformly index into the given
  /// type.
  spv::Capability getNonUniformCapability(const SpirvType *);

  /// Returns whether the shader model is one of the ray tracing execution
  /// models.
  bool IsShaderModelForRayTracing();

  /// Adds VulkanMemoryModel capability if decoration needs Volatile semantics
  /// for OpLoad instructions. For Vulkan 1.3 or above, we can simply add
  /// Volatile decoration for the variable. Therefore, in that case, we do not
  /// need VulkanMemoryModel capability.
  void AddVulkanMemoryModelForVolatile(SpirvDecoration *decor,
                                       SourceLocation loc);

private:
  SpirvBuilder &spvBuilder;        ///< SPIR-V builder
  spv::ExecutionModel shaderModel; ///< Execution model
  FeatureManager featureManager;   ///< SPIR-V version/extension manager.
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_CAPABILITYVISITOR_H
