//===--- CapabilityVisitor.cpp - Capability Visitor --------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CapabilityVisitor.h"
#include "clang/SPIRV/SpirvBuilder.h"

namespace clang {
namespace spirv {

void CapabilityVisitor::addExtension(Extension ext, llvm::StringRef target,
                                     SourceLocation loc) {
  // Do not emit OpExtension if the given extension is natively supported in
  // the target environment.
  if (!featureManager.isExtensionRequiredForTargetEnv(ext))
    return;
  if (featureManager.requestExtension(ext, target, loc))
    spvBuilder.requireExtension(featureManager.getExtensionName(ext), loc);
}

bool CapabilityVisitor::addExtensionAndCapabilitiesIfEnabled(
    Extension ext, llvm::ArrayRef<spv::Capability> capabilities) {
  if (!featureManager.isExtensionEnabled(ext)) {
    return false;
  }

  addExtension(ext, "", {});

  for (auto cap : capabilities) {
    addCapability(cap);
  }
  return true;
}

void CapabilityVisitor::addCapability(spv::Capability cap, SourceLocation loc) {
  if (cap != spv::Capability::Max) {
    spvBuilder.requireCapability(cap, loc);
  }
}

void CapabilityVisitor::addCapabilityForType(const SpirvType *type,
                                             SourceLocation loc,
                                             spv::StorageClass sc) {
  // Defend against instructions that do not have a return type.
  if (!type)
    return;

  // Integer-related capabilities
  if (const auto *intType = dyn_cast<IntegerType>(type)) {
    switch (intType->getBitwidth()) {
    case 8: {
      addCapability(spv::Capability::Int8);
      break;
    }
    case 16: {
      // Usage of a 16-bit integer type.
      addCapability(spv::Capability::Int16);

      // Usage of a 16-bit integer type as stage I/O.
      if (sc == spv::StorageClass::Input || sc == spv::StorageClass::Output) {
        addExtension(Extension::KHR_16bit_storage, "16-bit stage IO variables",
                     loc);
        addCapability(spv::Capability::StorageInputOutput16);
      }
      break;
    }
    case 64: {
      addCapability(spv::Capability::Int64);
      break;
    }
    default:
      break;
    }
  }
  // Float-related capabilities
  else if (const auto *floatType = dyn_cast<FloatType>(type)) {
    switch (floatType->getBitwidth()) {
    case 16: {
      // Usage of a 16-bit float type.
      addCapability(spv::Capability::Float16);

      // Usage of a 16-bit float type as stage I/O.
      if (sc == spv::StorageClass::Input || sc == spv::StorageClass::Output) {
        addExtension(Extension::KHR_16bit_storage, "16-bit stage IO variables",
                     loc);
        addCapability(spv::Capability::StorageInputOutput16);
      }
      break;
    }
    case 64: {
      addCapability(spv::Capability::Float64);
      break;
    }
    default:
      break;
    }
  }
  // Vectors
  else if (const auto *vecType = dyn_cast<VectorType>(type)) {
    addCapabilityForType(vecType->getElementType(), loc, sc);
  }
  // Matrices
  else if (const auto *matType = dyn_cast<MatrixType>(type)) {
    addCapabilityForType(matType->getElementType(), loc, sc);
  }
  // Arrays
  else if (const auto *arrType = dyn_cast<ArrayType>(type)) {
    addCapabilityForType(arrType->getElementType(), loc, sc);
  }
  // Runtime array of resources requires additional capability.
  else if (const auto *raType = dyn_cast<RuntimeArrayType>(type)) {
    if (SpirvType::isResourceType(raType->getElementType())) {
      // the elements inside the runtime array are resources
      addExtension(Extension::EXT_descriptor_indexing,
                   "runtime array of resources", loc);
      addCapability(spv::Capability::RuntimeDescriptorArrayEXT);
    }
    addCapabilityForType(raType->getElementType(), loc, sc);
  }
  // Node payload array also requires additional capability.
  else if (const auto *npaType = dyn_cast<NodePayloadArrayType>(type)) {
    addExtension(Extension::AMD_shader_enqueue, "Vulkan 1.3", loc);
    addCapability(spv::Capability::ShaderEnqueueAMDX, loc);
    addCapabilityForType(npaType->getElementType(), loc, sc);
  }
  // Image types
  else if (const auto *imageType = dyn_cast<ImageType>(type)) {
    switch (imageType->getDimension()) {
    case spv::Dim::Buffer: {
      addCapability(spv::Capability::SampledBuffer);
      if (imageType->withSampler() == ImageType::WithSampler::No) {
        addCapability(spv::Capability::ImageBuffer);
      }
      break;
    }
    case spv::Dim::Dim1D: {
      if (imageType->withSampler() == ImageType::WithSampler::No) {
        addCapability(spv::Capability::Image1D);
      } else {
        addCapability(spv::Capability::Sampled1D);
      }
      break;
    }
    case spv::Dim::SubpassData: {
      addCapability(spv::Capability::InputAttachment);
      break;
    }
    default:
      break;
    }

    switch (imageType->getImageFormat()) {
    case spv::ImageFormat::Rg32f:
    case spv::ImageFormat::Rg16f:
    case spv::ImageFormat::R11fG11fB10f:
    case spv::ImageFormat::R16f:
    case spv::ImageFormat::Rgba16:
    case spv::ImageFormat::Rgb10A2:
    case spv::ImageFormat::Rg16:
    case spv::ImageFormat::Rg8:
    case spv::ImageFormat::R16:
    case spv::ImageFormat::R8:
    case spv::ImageFormat::Rgba16Snorm:
    case spv::ImageFormat::Rg16Snorm:
    case spv::ImageFormat::Rg8Snorm:
    case spv::ImageFormat::R16Snorm:
    case spv::ImageFormat::R8Snorm:
    case spv::ImageFormat::Rg32i:
    case spv::ImageFormat::Rg16i:
    case spv::ImageFormat::Rg8i:
    case spv::ImageFormat::R16i:
    case spv::ImageFormat::R8i:
    case spv::ImageFormat::Rgb10a2ui:
    case spv::ImageFormat::Rg32ui:
    case spv::ImageFormat::Rg16ui:
    case spv::ImageFormat::Rg8ui:
    case spv::ImageFormat::R16ui:
    case spv::ImageFormat::R8ui:
      addCapability(spv::Capability::StorageImageExtendedFormats);
      break;
    default:
      // Only image formats requiring extended formats are relevant. The rest
      // just pass through.
      break;
    }

    if (const auto *sampledType = imageType->getSampledType()) {
      addCapabilityForType(sampledType, loc, sc);
      if (const auto *sampledIntType = dyn_cast<IntegerType>(sampledType)) {
        if (sampledIntType->getBitwidth() == 64) {
          addCapability(spv::Capability::Int64ImageEXT);
          addExtension(Extension::EXT_shader_image_int64,
                       "64-bit image types in resource", loc);
        }
      }
    }
  }
  // Sampled image type
  else if (const auto *sampledImageType = dyn_cast<SampledImageType>(type)) {
    addCapabilityForType(sampledImageType->getImageType(), loc, sc);
  }
  // Pointer type
  else if (const auto *ptrType = dyn_cast<SpirvPointerType>(type)) {
    addCapabilityForType(ptrType->getPointeeType(), loc,
                         ptrType->getStorageClass());
    if (ptrType->getStorageClass() ==
        spv::StorageClass::PhysicalStorageBuffer) {
      addExtension(Extension::KHR_physical_storage_buffer,
                   "SPV_KHR_physical_storage_buffer", loc);
      addCapability(spv::Capability::PhysicalStorageBufferAddresses);
    }
  }
  // Struct type
  else if (const auto *structType = dyn_cast<StructType>(type)) {
    if (SpirvType::isOrContainsType<NumericalType, 16>(structType)) {
      addExtension(Extension::KHR_16bit_storage, "16-bit types in resource",
                   loc);
      if (sc == spv::StorageClass::PushConstant) {
        addCapability(spv::Capability::StoragePushConstant16);
      } else if (structType->getInterfaceType() ==
                 StructInterfaceType::UniformBuffer) {
        addCapability(spv::Capability::StorageUniform16);
      } else if (structType->getInterfaceType() ==
                 StructInterfaceType::StorageBuffer) {
        addCapability(spv::Capability::StorageUniformBufferBlock16);
      }
    }
    for (auto field : structType->getFields())
      addCapabilityForType(field.type, loc, sc);
  }
}

bool CapabilityVisitor::visit(SpirvDecoration *decor) {
  const auto loc = decor->getSourceLocation();
  switch (decor->getDecoration()) {
  case spv::Decoration::Sample: {
    addCapability(spv::Capability::SampleRateShading, loc);
    break;
  }
  case spv::Decoration::NonUniformEXT: {
    addExtension(Extension::EXT_descriptor_indexing, "NonUniformEXT", loc);
    addCapability(spv::Capability::ShaderNonUniformEXT);

    break;
  }
  case spv::Decoration::HlslSemanticGOOGLE:
  case spv::Decoration::HlslCounterBufferGOOGLE: {
    addExtension(Extension::GOOGLE_hlsl_functionality1, "SPIR-V reflection",
                 loc);
    break;
  }
  case spv::Decoration::PerVertexKHR: {
    addExtension(Extension::KHR_fragment_shader_barycentric, "PerVertexKHR",
                 loc);
    addCapability(spv::Capability::FragmentBarycentricKHR);
    break;
  }
  case spv::Decoration::NodeSharesPayloadLimitsWithAMDX:
  case spv::Decoration::NodeMaxPayloadsAMDX:
  case spv::Decoration::TrackFinishWritingAMDX:
  case spv::Decoration::PayloadNodeNameAMDX:
  case spv::Decoration::PayloadNodeBaseIndexAMDX:
  case spv::Decoration::PayloadNodeSparseArrayAMDX:
  case spv::Decoration::PayloadNodeArraySizeAMDX:
  case spv::Decoration::PayloadDispatchIndirectAMDX: {
    featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_3, "WorkGraphs", loc);
    addCapability(spv::Capability::ShaderEnqueueAMDX, loc);
    addExtension(Extension::AMD_shader_enqueue, "Vulkan 1.3", loc);
    break;
  }
  // Capabilities needed for built-ins
  case spv::Decoration::BuiltIn: {
    AddVulkanMemoryModelForVolatile(decor, loc);
    assert(decor->getParams().size() == 1);
    const auto builtin = static_cast<spv::BuiltIn>(decor->getParams()[0]);
    switch (builtin) {
    case spv::BuiltIn::SampleId:
    case spv::BuiltIn::SamplePosition: {
      addCapability(spv::Capability::SampleRateShading, loc);
      break;
    }
    case spv::BuiltIn::SubgroupSize:
    case spv::BuiltIn::NumSubgroups:
    case spv::BuiltIn::SubgroupId:
    case spv::BuiltIn::SubgroupLocalInvocationId: {
      addCapability(spv::Capability::GroupNonUniform, loc);
      break;
    }
    case spv::BuiltIn::BaseVertex: {
      addExtension(Extension::KHR_shader_draw_parameters, "BaseVertex Builtin",
                   loc);
      addCapability(spv::Capability::DrawParameters);
      break;
    }
    case spv::BuiltIn::BaseInstance: {
      addExtension(Extension::KHR_shader_draw_parameters,
                   "BaseInstance Builtin", loc);
      addCapability(spv::Capability::DrawParameters);
      break;
    }
    case spv::BuiltIn::DrawIndex: {
      addExtension(Extension::KHR_shader_draw_parameters, "DrawIndex Builtin",
                   loc);
      addCapability(spv::Capability::DrawParameters);
      break;
    }
    case spv::BuiltIn::DeviceIndex: {
      addExtension(Extension::KHR_device_group, "DeviceIndex Builtin", loc);
      addCapability(spv::Capability::DeviceGroup);
      break;
    }
    case spv::BuiltIn::FragStencilRefEXT: {
      addExtension(Extension::EXT_shader_stencil_export, "SV_StencilRef", loc);
      addCapability(spv::Capability::StencilExportEXT);
      break;
    }
    case spv::BuiltIn::ViewIndex: {
      addExtension(Extension::KHR_multiview, "SV_ViewID", loc);
      addCapability(spv::Capability::MultiView);
      break;
    }
    case spv::BuiltIn::FullyCoveredEXT: {
      addExtension(Extension::EXT_fragment_fully_covered, "SV_InnerCoverage",
                   loc);
      addCapability(spv::Capability::FragmentFullyCoveredEXT);
      break;
    }
    case spv::BuiltIn::PrimitiveId: {
      // PrimitiveID can be used as PSIn or MSPOut.
      if (shaderModel == spv::ExecutionModel::Fragment ||
          shaderModel == spv::ExecutionModel::MeshNV ||
          shaderModel == spv::ExecutionModel::MeshEXT)
        addCapability(spv::Capability::Geometry);
      break;
    }
    case spv::BuiltIn::Layer: {
      if (shaderModel == spv::ExecutionModel::Vertex ||
          shaderModel == spv::ExecutionModel::TessellationControl ||
          shaderModel == spv::ExecutionModel::TessellationEvaluation) {

        if (featureManager.isTargetEnvVulkan1p2OrAbove()) {
          addCapability(spv::Capability::ShaderLayer);
        } else {
          addExtension(Extension::EXT_shader_viewport_index_layer,
                       "SV_RenderTargetArrayIndex", loc);
          addCapability(spv::Capability::ShaderViewportIndexLayerEXT);
        }
      } else if (shaderModel == spv::ExecutionModel::Fragment ||
                 shaderModel == spv::ExecutionModel::MeshNV ||
                 shaderModel == spv::ExecutionModel::MeshEXT) {
        // SV_RenderTargetArrayIndex can be used as PSIn or MSPOut.
        addCapability(spv::Capability::Geometry);
      }
      break;
    }
    case spv::BuiltIn::ViewportIndex: {
      if (shaderModel == spv::ExecutionModel::Vertex ||
          shaderModel == spv::ExecutionModel::TessellationControl ||
          shaderModel == spv::ExecutionModel::TessellationEvaluation) {
        if (featureManager.isTargetEnvVulkan1p2OrAbove()) {
          addCapability(spv::Capability::ShaderViewportIndex);
        } else {
          addExtension(Extension::EXT_shader_viewport_index_layer,
                       "SV_ViewPortArrayIndex", loc);
          addCapability(spv::Capability::ShaderViewportIndexLayerEXT);
        }
      } else if (shaderModel == spv::ExecutionModel::Fragment ||
                 shaderModel == spv::ExecutionModel::Geometry ||
                 shaderModel == spv::ExecutionModel::MeshNV ||
                 shaderModel == spv::ExecutionModel::MeshEXT) {
        // SV_ViewportArrayIndex can be used as PSIn or GSOut or MSPOut.
        addCapability(spv::Capability::MultiViewport);
      }
      break;
    }
    case spv::BuiltIn::ClipDistance: {
      addCapability(spv::Capability::ClipDistance);
      break;
    }
    case spv::BuiltIn::CullDistance: {
      addCapability(spv::Capability::CullDistance);
      break;
    }
    case spv::BuiltIn::BaryCoordKHR:
    case spv::BuiltIn::BaryCoordNoPerspKHR: {
      // SV_Barycentrics will have only two builtins
      // But it is still allowed to decorate those two builtins with
      // interpolation qualifier like centroid or sample.
      addExtension(Extension::KHR_fragment_shader_barycentric,
                   "SV_Barycentrics", loc);
      addCapability(spv::Capability::FragmentBarycentricKHR);
      break;
    }
    case spv::BuiltIn::ShadingRateKHR:
    case spv::BuiltIn::PrimitiveShadingRateKHR: {
      addExtension(Extension::KHR_fragment_shading_rate, "SV_ShadingRate", loc);
      addCapability(spv::Capability::FragmentShadingRateKHR);
      break;
    }
    default:
      break;
    }

    break;
  }
  case spv::Decoration::LinkageAttributes:
    addCapability(spv::Capability::Linkage);
    break;
  default:
    break;
  }

  return true;
}

spv::Capability
CapabilityVisitor::getNonUniformCapability(const SpirvType *type) {
  if (!type)
    return spv::Capability::Max;

  if (const auto *arrayType = dyn_cast<ArrayType>(type)) {
    return getNonUniformCapability(arrayType->getElementType());
  }
  if (SpirvType::isTexture(type) || SpirvType::isSampler(type)) {
    return spv::Capability::SampledImageArrayNonUniformIndexingEXT;
  }
  if (SpirvType::isRWTexture(type)) {
    return spv::Capability::StorageImageArrayNonUniformIndexingEXT;
  }
  if (SpirvType::isBuffer(type)) {
    return spv::Capability::UniformTexelBufferArrayNonUniformIndexingEXT;
  }
  if (SpirvType::isRWBuffer(type)) {
    return spv::Capability::StorageTexelBufferArrayNonUniformIndexingEXT;
  }
  if (SpirvType::isSubpassInput(type) || SpirvType::isSubpassInputMS(type)) {
    return spv::Capability::InputAttachmentArrayNonUniformIndexingEXT;
  }

  return spv::Capability::Max;
}

bool CapabilityVisitor::visit(SpirvImageQuery *instr) {
  addCapabilityForType(instr->getResultType(), instr->getSourceLocation(),
                       instr->getStorageClass());
  addCapability(spv::Capability::ImageQuery);
  return true;
}

bool CapabilityVisitor::visit(SpirvImageSparseTexelsResident *instr) {
  addCapabilityForType(instr->getResultType(), instr->getSourceLocation(),
                       instr->getStorageClass());
  addCapability(spv::Capability::ImageGatherExtended);
  addCapability(spv::Capability::SparseResidency);
  return true;
}

bool CapabilityVisitor::visit(SpirvImageOp *instr) {
  addCapabilityForType(instr->getResultType(), instr->getSourceLocation(),
                       instr->getStorageClass());
  if (instr->hasOffset() || instr->hasConstOffsets())
    addCapability(spv::Capability::ImageGatherExtended);
  if (instr->isSparse())
    addCapability(spv::Capability::SparseResidency);
  return true;
}

bool CapabilityVisitor::visitInstruction(SpirvInstruction *instr) {
  const SpirvType *resultType = instr->getResultType();
  const auto opcode = instr->getopcode();
  const auto loc = instr->getSourceLocation();

  // Add result-type-specific capabilities
  addCapabilityForType(resultType, loc, instr->getStorageClass());

  // Add NonUniform capabilities if necessary
  if (instr->isNonUniform()) {
    addExtension(Extension::EXT_descriptor_indexing, "NonUniformEXT", loc);
    addCapability(spv::Capability::ShaderNonUniformEXT);
    addCapability(getNonUniformCapability(resultType));
  }

  if (instr->getKind() == SpirvInstruction::IK_SpirvIntrinsicInstruction) {
    SpirvIntrinsicInstruction *pSpvInst =
        dyn_cast<SpirvIntrinsicInstruction>(instr);
    for (auto &cap : pSpvInst->getCapabilities()) {
      addCapability(static_cast<spv::Capability>(cap));
    }
    for (const auto &ext : pSpvInst->getExtensions()) {
      spvBuilder.requireExtension(ext, loc);
    }
  }

  // Add opcode-specific capabilities
  switch (opcode) {
  case spv::Op::OpDPdxCoarse:
  case spv::Op::OpDPdyCoarse:
  case spv::Op::OpFwidthCoarse:
  case spv::Op::OpDPdxFine:
  case spv::Op::OpDPdyFine:
  case spv::Op::OpFwidthFine:
    addCapability(spv::Capability::DerivativeControl);
    break;
  case spv::Op::OpGroupNonUniformElect:
    addCapability(spv::Capability::GroupNonUniform);
    break;
  case spv::Op::OpGroupNonUniformAny:
  case spv::Op::OpGroupNonUniformAll:
  case spv::Op::OpGroupNonUniformAllEqual:
    addCapability(spv::Capability::GroupNonUniformVote);
    break;
  case spv::Op::OpGroupNonUniformBallot:
  case spv::Op::OpGroupNonUniformInverseBallot:
  case spv::Op::OpGroupNonUniformBallotBitExtract:
  case spv::Op::OpGroupNonUniformBallotBitCount:
  case spv::Op::OpGroupNonUniformBallotFindLSB:
  case spv::Op::OpGroupNonUniformBallotFindMSB:
  case spv::Op::OpGroupNonUniformBroadcast:
  case spv::Op::OpGroupNonUniformBroadcastFirst:
    addCapability(spv::Capability::GroupNonUniformBallot);
    break;
  case spv::Op::OpGroupNonUniformShuffle:
  case spv::Op::OpGroupNonUniformShuffleXor:
    addCapability(spv::Capability::GroupNonUniformShuffle);
    break;
  case spv::Op::OpGroupNonUniformIAdd:
  case spv::Op::OpGroupNonUniformFAdd:
  case spv::Op::OpGroupNonUniformIMul:
  case spv::Op::OpGroupNonUniformFMul:
  case spv::Op::OpGroupNonUniformSMax:
  case spv::Op::OpGroupNonUniformUMax:
  case spv::Op::OpGroupNonUniformFMax:
  case spv::Op::OpGroupNonUniformSMin:
  case spv::Op::OpGroupNonUniformUMin:
  case spv::Op::OpGroupNonUniformFMin:
  case spv::Op::OpGroupNonUniformBitwiseAnd:
  case spv::Op::OpGroupNonUniformBitwiseOr:
  case spv::Op::OpGroupNonUniformBitwiseXor:
  case spv::Op::OpGroupNonUniformLogicalAnd:
  case spv::Op::OpGroupNonUniformLogicalOr:
  case spv::Op::OpGroupNonUniformLogicalXor:
    addCapability(spv::Capability::GroupNonUniformArithmetic);
    break;
  case spv::Op::OpGroupNonUniformQuadBroadcast:
  case spv::Op::OpGroupNonUniformQuadSwap:
    addCapability(spv::Capability::GroupNonUniformQuad);
    break;
  case spv::Op::OpVariable: {
    auto var = cast<SpirvVariable>(instr);
    auto storage = var->getStorageClass();
    if (storage == spv::StorageClass::NodePayloadAMDX) {
      featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_3, "WorkGraphs", loc);
      addCapability(spv::Capability::ShaderEnqueueAMDX, loc);
      addExtension(Extension::AMD_shader_enqueue, "Vulkan 1.3", loc);
    }
    if (spvOptions.enableReflect && !var->getHlslUserType().empty()) {
      addExtension(Extension::GOOGLE_user_type, "HLSL User Type", loc);
      addExtension(Extension::GOOGLE_hlsl_functionality1, "HLSL User Type",
                   loc);
    }
    break;
  }
  case spv::Op::OpRayQueryInitializeKHR: {
    auto rayQueryInst = dyn_cast<SpirvRayQueryOpKHR>(instr);
    if (rayQueryInst && rayQueryInst->hasCullFlags()) {
      addCapability(spv::Capability::RayTraversalPrimitiveCullingKHR);
    }

    break;
  }

  case spv::Op::OpReportIntersectionKHR:
  case spv::Op::OpIgnoreIntersectionKHR:
  case spv::Op::OpTerminateRayKHR:
  case spv::Op::OpTraceRayKHR:
  case spv::Op::OpExecuteCallableKHR: {
    if (featureManager.isExtensionEnabled(Extension::NV_ray_tracing)) {
      addCapability(spv::Capability::RayTracingNV);
      addExtension(Extension::NV_ray_tracing, "SPV_NV_ray_tracing", {});
    } else {
      // KHR_ray_tracing extension requires Vulkan 1.1 with VK_KHR_spirv_1_4
      // extention or Vulkan 1.2.
      featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_1_SPIRV_1_4,
                                      "Raytracing", {});
      addCapability(spv::Capability::RayTracingKHR);
      addExtension(Extension::KHR_ray_tracing, "SPV_KHR_ray_tracing", {});
    }
    break;
  }

  case spv::Op::OpSetMeshOutputsEXT:
  case spv::Op::OpEmitMeshTasksEXT: {
    if (featureManager.isExtensionEnabled(Extension::EXT_mesh_shader)) {
      featureManager.requestTargetEnv(SPV_ENV_UNIVERSAL_1_4, "MeshShader", {});
      addCapability(spv::Capability::MeshShadingEXT);
      addExtension(Extension::EXT_mesh_shader, "SPV_EXT_mesh_shader", {});
    }
    break;
  }
  case spv::Op::OpConstantStringAMDX:
  case spv::Op::OpSpecConstantStringAMDX:
  case spv::Op::OpAllocateNodePayloadsAMDX:
  case spv::Op::OpEnqueueNodePayloadsAMDX:
  case spv::Op::OpIsNodePayloadValidAMDX:
  case spv::Op::OpFinishWritingNodePayloadAMDX: {
    featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_3, "WorkGraphs", loc);
    addCapability(spv::Capability::ShaderEnqueueAMDX, loc);
    addExtension(Extension::AMD_shader_enqueue, "Vulkan 1.3", loc);
    break;
  }
  case spv::Op::OpControlBarrier:
  case spv::Op::OpMemoryBarrier: {
    auto barrier = cast<SpirvBarrier>(instr);
    if ((bool)(barrier->getMemorySemantics() &
               spv::MemorySemanticsMask::OutputMemoryKHR)) {
      featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_3, "NODE_OUTPUT_MEMORY",
                                      loc);
      addCapability(spv::Capability::VulkanMemoryModel, loc);
    }
    break;
  }

  default:
    break;
  }

  return true;
}

bool CapabilityVisitor::visit(SpirvEntryPoint *entryPoint) {
  shaderModel = entryPoint->getExecModel();
  switch (shaderModel) {
  case spv::ExecutionModel::Fragment:
  case spv::ExecutionModel::Vertex:
  case spv::ExecutionModel::GLCompute:
    addCapability(spv::Capability::Shader);
    break;
  case spv::ExecutionModel::Geometry:
    addCapability(spv::Capability::Geometry);
    break;
  case spv::ExecutionModel::TessellationControl:
  case spv::ExecutionModel::TessellationEvaluation:
    addCapability(spv::Capability::Tessellation);
    break;
  case spv::ExecutionModel::RayGenerationNV:
  case spv::ExecutionModel::IntersectionNV:
  case spv::ExecutionModel::ClosestHitNV:
  case spv::ExecutionModel::AnyHitNV:
  case spv::ExecutionModel::MissNV:
  case spv::ExecutionModel::CallableNV:
    if (featureManager.isExtensionEnabled(Extension::NV_ray_tracing)) {
      addCapability(spv::Capability::RayTracingNV);
      addExtension(Extension::NV_ray_tracing, "SPV_NV_ray_tracing", {});
    } else {
      // KHR_ray_tracing extension requires Vulkan 1.1 with VK_KHR_spirv_1_4
      // extention or Vulkan 1.2.
      featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_1_SPIRV_1_4,
                                      "Raytracing", {});
      addCapability(spv::Capability::RayTracingKHR);
      addExtension(Extension::KHR_ray_tracing, "SPV_KHR_ray_tracing", {});
    }
    break;
  case spv::ExecutionModel::MeshNV:
  case spv::ExecutionModel::TaskNV:
    addCapability(spv::Capability::MeshShadingNV);
    addExtension(Extension::NV_mesh_shader, "SPV_NV_mesh_shader", {});
    break;
  case spv::ExecutionModel::MeshEXT:
  case spv::ExecutionModel::TaskEXT:
    addCapability(spv::Capability::MeshShadingEXT);
    addExtension(Extension::EXT_mesh_shader, "SPV_EXT_mesh_shader", {});
    break;
  default:
    llvm_unreachable("found unknown shader model");
    break;
  }

  return true;
}

bool CapabilityVisitor::visit(SpirvExecutionModeBase *execMode) {
  spv::ExecutionMode executionMode = execMode->getExecutionMode();
  SourceLocation execModeSourceLocation = execMode->getSourceLocation();
  SourceLocation entryPointSourceLocation =
      execMode->getEntryPoint()->getSourceLocation();
  switch (executionMode) {
  case spv::ExecutionMode::CoalescingAMDX:
  case spv::ExecutionMode::MaxNodeRecursionAMDX:
  case spv::ExecutionMode::StaticNumWorkgroupsAMDX:
  case spv::ExecutionMode::MaxNumWorkgroupsAMDX:
    featureManager.requestTargetEnv(SPV_ENV_VULKAN_1_3, "WorkGraphs",
                                    execModeSourceLocation);
    addCapability(spv::Capability::ShaderEnqueueAMDX, execModeSourceLocation);
    addExtension(Extension::AMD_shader_enqueue, "Vulkan 1.3",
                 execModeSourceLocation);
    break;
  case spv::ExecutionMode::SubgroupSize:
    addCapability(spv::Capability::SubgroupDispatch, execModeSourceLocation);
    break;
  case spv::ExecutionMode::PostDepthCoverage:
    addCapability(spv::Capability::SampleMaskPostDepthCoverage,
                  entryPointSourceLocation);
    addExtension(Extension::KHR_post_depth_coverage,
                 "[[vk::post_depth_coverage]]", execModeSourceLocation);
    break;
  case spv::ExecutionMode::EarlyAndLateFragmentTestsAMD:
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::early_and_late_tests]]", execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefUnchangedFrontAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_unchanged_front]]", execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_unchanged_front]]", execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefGreaterFrontAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_greater_equal_front]]",
                 execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_greater_equal_front]]",
                 execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefLessFrontAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_less_equal_front]]",
                 execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_less_equal_front]]",
                 execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefUnchangedBackAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_unchanged_back]]", execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_unchanged_back]]", execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefGreaterBackAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_greater_equal_back]]",
                 execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_greater_equal_back]]",
                 execModeSourceLocation);
    break;
  case spv::ExecutionMode::StencilRefLessBackAMD:
    addCapability(spv::Capability::StencilExportEXT, entryPointSourceLocation);
    addExtension(Extension::AMD_shader_early_and_late_fragment_tests,
                 "[[vk::stencil_ref_less_equal_back]]", execModeSourceLocation);
    addExtension(Extension::EXT_shader_stencil_export,
                 "[[vk::stencil_ref_less_equal_back]]", execModeSourceLocation);
    break;
  case spv::ExecutionMode::MaximallyReconvergesKHR:
    addExtension(Extension::KHR_maximal_reconvergence, "",
                 execModeSourceLocation);
    break;
  case spv::ExecutionMode::DenormPreserve:
  case spv::ExecutionMode::DenormFlushToZero:
    // KHR_float_controls was promoted to core in Vulkan 1.2.
    if (!featureManager.isTargetEnvVulkan1p2OrAbove()) {
      addExtension(Extension::KHR_float_controls, "SPV_KHR_float_controls",
                   execModeSourceLocation);
    }
    addCapability(executionMode == spv::ExecutionMode::DenormPreserve
                      ? spv::Capability::DenormPreserve
                      : spv::Capability::DenormFlushToZero,
                  execModeSourceLocation);
    break;
  default:
    break;
  }
  return true;
}

bool CapabilityVisitor::visit(SpirvExtInstImport *instr) {
  if (instr->getExtendedInstSetName() == "NonSemantic.DebugPrintf") {
    addExtension(Extension::KHR_non_semantic_info, "DebugPrintf",
                 /*SourceLocation*/ {});
  } else if (instr->getExtendedInstSetName() ==
             "NonSemantic.Shader.DebugInfo.100") {
    addExtension(Extension::KHR_non_semantic_info, "Shader.DebugInfo.100",
                 /*SourceLocation*/ {});
  }
  return true;
}

bool CapabilityVisitor::visit(SpirvAtomic *instr) {
  if (instr->hasValue() && SpirvType::isOrContainsType<IntegerType, 64>(
                               instr->getValue()->getResultType())) {
    addCapability(spv::Capability::Int64Atomics, instr->getSourceLocation());
  }
  return true;
}

bool CapabilityVisitor::visit(SpirvDemoteToHelperInvocation *inst) {
  addCapability(spv::Capability::DemoteToHelperInvocation,
                inst->getSourceLocation());
  if (!featureManager.isTargetEnvVulkan1p3OrAbove()) {
    addExtension(Extension::EXT_demote_to_helper_invocation, "discard",
                 inst->getSourceLocation());
  }
  return true;
}

bool CapabilityVisitor::IsShaderModelForRayTracing() {
  switch (shaderModel) {
  case spv::ExecutionModel::RayGenerationKHR:
  case spv::ExecutionModel::ClosestHitKHR:
  case spv::ExecutionModel::MissKHR:
  case spv::ExecutionModel::CallableKHR:
  case spv::ExecutionModel::IntersectionKHR:
    return true;
  default:
    return false;
  }
}

void CapabilityVisitor::AddVulkanMemoryModelForVolatile(SpirvDecoration *decor,
                                                        SourceLocation loc) {
  // For Vulkan 1.3 or above, we can simply add Volatile decoration. We do not
  // need VulkanMemoryModel capability.
  if (featureManager.isTargetEnvVulkan1p3OrAbove()) {
    return;
  }

  const auto builtin = static_cast<spv::BuiltIn>(decor->getParams()[0]);
  bool enableVkMemoryModel = false;
  switch (builtin) {
  case spv::BuiltIn::SubgroupSize:
  case spv::BuiltIn::SubgroupLocalInvocationId:
  case spv::BuiltIn::SMIDNV:
  case spv::BuiltIn::WarpIDNV:
  case spv::BuiltIn::SubgroupEqMask:
  case spv::BuiltIn::SubgroupGeMask:
  case spv::BuiltIn::SubgroupGtMask:
  case spv::BuiltIn::SubgroupLeMask:
  case spv::BuiltIn::SubgroupLtMask: {
    if (IsShaderModelForRayTracing()) {
      enableVkMemoryModel = true;
    }
    break;
  }
  case spv::BuiltIn::RayTmaxKHR: {
    if (shaderModel == spv::ExecutionModel::IntersectionKHR) {
      enableVkMemoryModel = true;
    }
    break;
  }
  default:
    break;
  }

  if (enableVkMemoryModel) {
    // VulkanMemoryModel was promoted to the core for Vulkan 1.2 or above. For
    // Vulkan 1.1 or earlier, we have to use SPV_KHR_vulkan_memory_model
    // extension.
    if (!featureManager.isTargetEnvVulkan1p2OrAbove()) {
      addExtension(Extension::KHR_vulkan_memory_model,
                   "Volatile builtin variable in raytracing", loc);
    }
    addCapability(spv::Capability::VulkanMemoryModel, loc);
  }
}

bool CapabilityVisitor::visit(SpirvIsHelperInvocationEXT *inst) {
  addCapability(spv::Capability::DemoteToHelperInvocation,
                inst->getSourceLocation());
  addExtension(Extension::EXT_demote_to_helper_invocation,
               "[[vk::HelperInvocation]]", inst->getSourceLocation());
  return true;
}

bool CapabilityVisitor::visit(SpirvReadClock *inst) {
  auto loc = inst->getSourceLocation();
  addCapabilityForType(inst->getResultType(), loc, inst->getStorageClass());
  addCapability(spv::Capability::ShaderClockKHR, loc);
  addExtension(Extension::KHR_shader_clock, "ReadClock", loc);
  return true;
}

bool CapabilityVisitor::visit(SpirvModule *, Visitor::Phase phase) {
  // If there are no entry-points in the module add the Shader capability.
  // This allows library shader models with no entry pointer and just exported
  // function. ExecutionModel::Max means that no entrypoints exist.
  if (phase == Visitor::Phase::Done &&
      shaderModel == spv::ExecutionModel::Max) {
    addCapability(spv::Capability::Shader);
  }

  // SPIRV-Tools now has a pass to trim superfluous capabilities. This means we
  // can remove most capability-selection logic from here, and just add
  // capabilities by default. SPIRV-Tools will clean those up. Note: this pass
  // supports only some capabilities. This list should be expanded to match the
  // supported capabilities.
  addCapability(spv::Capability::MinLod);
  addCapability(spv::Capability::StorageImageWriteWithoutFormat);
  addCapability(spv::Capability::StorageImageReadWithoutFormat);

  addExtensionAndCapabilitiesIfEnabled(
      Extension::EXT_fragment_shader_interlock,
      {
          spv::Capability::FragmentShaderSampleInterlockEXT,
          spv::Capability::FragmentShaderPixelInterlockEXT,
          spv::Capability::FragmentShaderShadingRateInterlockEXT,
      });

  addExtensionAndCapabilitiesIfEnabled(
      Extension::KHR_compute_shader_derivatives,
      {
          spv::Capability::ComputeDerivativeGroupQuadsKHR,
          spv::Capability::ComputeDerivativeGroupLinearKHR,
      });
  addExtensionAndCapabilitiesIfEnabled(
      Extension::NV_compute_shader_derivatives,
      {
          spv::Capability::ComputeDerivativeGroupQuadsNV,
          spv::Capability::ComputeDerivativeGroupLinearNV,
      });

  // AccelerationStructureType or RayQueryType can be provided by both
  // ray_tracing and ray_query extension. By default, we select ray_query to
  // provide it. This is an arbitrary decision. If the user wants avoid one
  // extension (lack of support by ex), if can be done by providing the list
  // of enabled extensions.
  if (!addExtensionAndCapabilitiesIfEnabled(Extension::KHR_ray_query,
                                            {spv::Capability::RayQueryKHR})) {
    addExtensionAndCapabilitiesIfEnabled(Extension::KHR_ray_tracing,
                                         {spv::Capability::RayTracingKHR});
  }

  addExtensionAndCapabilitiesIfEnabled(
      Extension::NV_shader_subgroup_partitioned,
      {spv::Capability::GroupNonUniformPartitionedNV});

  addCapability(spv::Capability::InterpolationFunction);

  addExtensionAndCapabilitiesIfEnabled(Extension::KHR_quad_control,
                                       {spv::Capability::QuadControlKHR});

  return true;
}

} // end namespace spirv
} // end namespace clang
