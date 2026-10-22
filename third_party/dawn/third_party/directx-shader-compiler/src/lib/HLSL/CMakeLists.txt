# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.

add_hlsl_hctgen(DxcOptimizer OUTPUT DxcOptimizer.inc BUILD_DIR)

add_llvm_library(LLVMHLSL
  ComputeViewIdState.cpp
  ComputeViewIdStateBuilder.cpp
  ControlDependence.cpp
  DxilCondenseResources.cpp
  DxilContainerReflection.cpp
  DxilConvergent.cpp
  DxilEliminateOutputDynamicIndexing.cpp
  DxilExpandTrigIntrinsics.cpp
  DxilGenerationPass.cpp
  DxilLegalizeEvalOperations.cpp
  DxilLegalizeSampleOffsetPass.cpp
  DxilLinker.cpp
  DxilLoopDeletion.cpp
  DxilPrecisePropagatePass.cpp
  DxilPreparePasses.cpp
  DxilPromoteResourcePasses.cpp
  DxilPackSignatureElement.cpp
  DxilPatchShaderRecordBindings.cpp
  DxilNoops.cpp
  DxilPreserveAllOutputs.cpp
  DxilRenameResourcesPass.cpp
  DxilScalarizeVectorIntrinsics.cpp
  DxilSimpleGVNHoist.cpp
  DxilSignatureValidation.cpp
  DxilTargetLowering.cpp
  DxilTargetTransformInfo.cpp
  DxilTranslateRawBuffer.cpp
  DxilExportMap.cpp
  DxcOptimizer.cpp
  HLDeadFunctionElimination.cpp
  HLExpandStoreIntrinsics.cpp
  HLLegalizeParameter.cpp
  HLLowerUDT.cpp
  HLMatrixBitcastLowerPass.cpp
  HLMatrixLowerPass.cpp
  HLMatrixSubscriptUseReplacer.cpp
  HLMatrixType.cpp
  HLMetadataPasses.cpp
  HLModule.cpp
  HLOperations.cpp
  HLOperationLower.cpp
  HLOperationLowerExtension.cpp
  HLPreprocess.cpp
  HLResource.cpp
  HLSignatureLower.cpp
  HLUtil.cpp
  PauseResumePasses.cpp
  WaveSensitivityAnalysis.cpp
  DxilNoOptLegalize.cpp
  DxilPoisonValues.cpp
  DxilDeleteRedundantDebugValues.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR
)

add_dependencies(LLVMHLSL intrinsics_gen)
