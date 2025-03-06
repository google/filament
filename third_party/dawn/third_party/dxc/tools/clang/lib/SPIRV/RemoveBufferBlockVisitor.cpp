//===-- RemoveBufferBlockVisitor.cpp - RemoveBufferBlock Visitor -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RemoveBufferBlockVisitor.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvFunction.h"

namespace clang {
namespace spirv {

bool RemoveBufferBlockVisitor::isBufferBlockDecorationAvailable() {
  return !featureManager.isTargetEnvSpirv1p4OrAbove();
}

bool RemoveBufferBlockVisitor::visit(SpirvModule *mod, Phase phase) {
  // The BufferBlock decoration requires SPIR-V version 1.3 or earlier. It
  // should be removed from the module on newer versions. Otherwise, no action
  // is needed by this IMR visitor.
  if (phase == Visitor::Phase::Init)
    if (isBufferBlockDecorationAvailable())
      return false;

  return true;
}

bool RemoveBufferBlockVisitor::hasStorageBufferInterfaceType(
    const SpirvType *type) {
  while (type != nullptr) {
    if (const auto *structType = dyn_cast<StructType>(type)) {
      return structType->getInterfaceType() ==
             StructInterfaceType::StorageBuffer;
    } else if (const auto *elemType = dyn_cast<ArrayType>(type)) {
      type = elemType->getElementType();
    } else if (const auto *elemType = dyn_cast<RuntimeArrayType>(type)) {
      type = elemType->getElementType();
    } else {
      return false;
    }
  }
  return false;
}

bool RemoveBufferBlockVisitor::visitInstruction(SpirvInstruction *inst) {
  if (!inst->getResultType())
    return true;

  // OpAccessChain can obtain pointers to any type. Its result type is
  // OpTypePointer, and it should get the same storage class as its base.
  if (auto *accessChain = dyn_cast<SpirvAccessChain>(inst)) {
    auto *accessChainType = accessChain->getResultType();
    auto *baseType = accessChain->getBase()->getResultType();
    // The result type of OpAccessChain and the result type of its base must be
    // OpTypePointer.
    assert(isa<SpirvPointerType>(accessChainType));
    assert(isa<SpirvPointerType>(baseType));
    auto *accessChainPtr = dyn_cast<SpirvPointerType>(accessChainType);
    auto *basePtr = dyn_cast<SpirvPointerType>(baseType);
    auto baseStorageClass = basePtr->getStorageClass();
    if (accessChainPtr->getStorageClass() != baseStorageClass) {
      auto *newAccessChainType = context.getPointerType(
          accessChainPtr->getPointeeType(), baseStorageClass);
      inst->setStorageClass(baseStorageClass);
      inst->setResultType(newAccessChainType);
    }
  }

  // For all instructions, if the result type is a pointer pointing to a struct
  // with StorageBuffer interface, the storage class must be updated.
  const auto *instType = inst->getResultType();
  const auto *newInstType = instType;
  spv::StorageClass newInstStorageClass = spv::StorageClass::Max;
  if (updateStorageClass(instType, &newInstType, &newInstStorageClass)) {
    inst->setResultType(newInstType);
    inst->setStorageClass(newInstStorageClass);
  }

  return true;
}

bool RemoveBufferBlockVisitor::updateStorageClass(
    const SpirvType *type, const SpirvType **newType,
    spv::StorageClass *newStorageClass) {

  // Update pointer types.
  if (const auto *ptrType = dyn_cast<SpirvPointerType>(type)) {
    const auto *innerType = ptrType->getPointeeType();

    // For pointees with storage buffer interface, update pointer storage class.
    if (hasStorageBufferInterfaceType(innerType) &&
        ptrType->getStorageClass() != spv::StorageClass::StorageBuffer) {
      *newType =
          context.getPointerType(innerType, spv::StorageClass::StorageBuffer);
      *newStorageClass = spv::StorageClass::StorageBuffer;
      return true;
    }

    // Update storage class of pointee, if applicable.
    const auto *newInnerType = innerType;
    spv::StorageClass newInnerSC = spv::StorageClass::Max;
    if (updateStorageClass(innerType, &newInnerType, &newInnerSC)) {
      *newType =
          context.getPointerType(newInnerType, ptrType->getStorageClass());
      *newStorageClass = ptrType->getStorageClass();
      return true;
    }
  }

  // Update struct types.
  if (const auto *structType = dyn_cast<StructType>(type)) {
    bool transformed = false;
    llvm::SmallVector<StructType::FieldInfo, 2> newFields;

    // Update storage class of each field, if applicable.
    for (auto field : structType->getFields()) {
      const auto *newFieldType = field.type;
      spv::StorageClass newFieldSC = spv::StorageClass::Max;
      transformed |= updateStorageClass(field.type, &newFieldType, &newFieldSC);
      field.type = newFieldType;
      newFields.push_back(field);
    }
    *newType =
        context.getStructType(llvm::ArrayRef<StructType::FieldInfo>(newFields),
                              structType->getStructName());
    *newStorageClass = spv::StorageClass::StorageBuffer;
    return transformed;
  }

  // TODO: Handle other composite types.

  return false;
}

bool RemoveBufferBlockVisitor::visit(SpirvFunction *fn, Phase phase) {
  if (phase == Visitor::Phase::Init) {
    llvm::SmallVector<const SpirvType *, 4> paramTypes;
    bool updatedParamTypes = false;
    for (auto *param : fn->getParameters()) {
      const auto *paramType = param->getResultType();
      // This pass is run after all types are lowered.
      assert(paramType != nullptr);

      // Update the parameter type if needed (update storage class of pointers).
      const auto *newParamType = paramType;
      spv::StorageClass newParamSC = spv::StorageClass::Max;
      if (updateStorageClass(paramType, &newParamType, &newParamSC)) {
        param->setStorageClass(newParamSC);
        param->setResultType(newParamType);
        updatedParamTypes = true;
      }
      paramTypes.push_back(newParamType);
    }

    // Update the return type if needed (update storage class of pointers).
    const auto *returnType = fn->getReturnType();
    const auto *newReturnType = returnType;
    spv::StorageClass newReturnSC = spv::StorageClass::Max;
    bool updatedReturnType =
        updateStorageClass(returnType, &newReturnType, &newReturnSC);
    if (updatedReturnType) {
      fn->setReturnType(newReturnType);
    }

    if (updatedParamTypes || updatedReturnType) {
      fn->setFunctionType(context.getFunctionType(newReturnType, paramTypes));
    }
    return true;
  }
  return true;
}

} // end namespace spirv
} // end namespace clang
