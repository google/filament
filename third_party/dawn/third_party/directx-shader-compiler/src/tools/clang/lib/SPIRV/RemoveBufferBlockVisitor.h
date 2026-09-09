//===--- RemoveBufferBlockVisitor.h - RemoveBufferBlock Visitor --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_REMOVEBUFFERBLOCKVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_REMOVEBUFFERBLOCKVISITOR_H

#include "clang/AST/ASTContext.h"
#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class SpirvContext;

class RemoveBufferBlockVisitor : public Visitor {
public:
  RemoveBufferBlockVisitor(ASTContext &astCtx, SpirvContext &spvCtx,
                           const SpirvCodeGenOptions &opts,
                           FeatureManager &featureMgr)
      : Visitor(opts, spvCtx), featureManager(featureMgr) {}

  bool visit(SpirvModule *, Phase) override;
  bool visit(SpirvFunction *, Phase) override;

  using Visitor::visit;

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override;

private:
  /// Returns true if |type| is a SPIR-V type whose interface type is
  /// StorageBuffer.
  bool hasStorageBufferInterfaceType(const SpirvType *type);

  /// Returns true if the BufferBlock decoration is available (SPIR-V 1.3
  /// or below).
  bool isBufferBlockDecorationAvailable();

  /// Transforms the given |type| if it is one of the following cases:
  ///
  /// 1- a pointer to a structure with StorageBuffer interface
  /// 2- a pointer to a pointer to a structure with StorageBuffer interface
  /// 3- a pointer to a struct containing a structure with StorageBuffer
  /// interface
  ///
  /// by updating the storage class of the pointer whose pointee is the struct.
  ///
  /// Example of case (1):
  /// type:              _ptr_Uniform_StructuredBuffer_float
  /// new type:          _ptr_StorageBuffer_StructuredBuffer_float
  /// new storage class: StorageBuffer
  ///
  /// Example of case (2):
  /// type:              _ptr_Function__ptr_Uniform_StructuredBuffer_float
  /// new type:          _ptr_Function__ptr_StorageBuffer_StructuredBuffer_float
  /// new storage class: Function
  ///
  /// Example of case (3):
  /// type:              _ptr_Function_Struct
  ///                    where %Struct = OpTypeStruct
  ///                        %_ptr_Uniform_type_StructuredBuffer_float
  /// new type:          _ptr_Function_Struct
  ///                    where %Struct = OpTypeStruct
  ///                        %_ptr_StorageBuffer_type_StructuredBuffer_float
  /// new storage class: Function
  ///
  /// If |type| is transformed, the |newType| and |newStorageClass| are
  /// returned by reference and the function returns true.
  ///
  /// If |type| is not transformed, |newType| and |newStorageClass| are
  /// untouched, and the function returns false.
  bool updateStorageClass(const SpirvType *type, const SpirvType **newType,
                          spv::StorageClass *newStorageClass);

  FeatureManager featureManager;
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_REMOVEBUFFERBLOCKVISITOR_H
