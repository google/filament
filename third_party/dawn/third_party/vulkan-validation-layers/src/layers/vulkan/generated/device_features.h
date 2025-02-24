// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See device_features_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 Google Inc.
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

#pragma once

#include <vulkan/vulkan.h>
class APIVersion;

// Union of all features defined in VkPhysicalDevice*Features* structs
struct DeviceFeatures {
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    bool storageBuffer16BitAccess;
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    bool storageInputOutput16;
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    bool storagePushConstant16;
    // VkPhysicalDevice16BitStorageFeatures, VkPhysicalDeviceVulkan11Features
    bool uniformAndStorageBuffer16BitAccess;
    // VkPhysicalDevice4444FormatsFeaturesEXT
    bool formatA4B4G4R4;
    // VkPhysicalDevice4444FormatsFeaturesEXT
    bool formatA4R4G4B4;
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    bool storageBuffer8BitAccess;
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    bool storagePushConstant8;
    // VkPhysicalDevice8BitStorageFeatures, VkPhysicalDeviceVulkan12Features
    bool uniformAndStorageBuffer8BitAccess;
    // VkPhysicalDeviceASTCDecodeFeaturesEXT
    bool decodeModeSharedExponent;
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    bool accelerationStructure;
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    bool accelerationStructureCaptureReplay;
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    bool accelerationStructureHostCommands;
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    bool accelerationStructureIndirectBuild;
    // VkPhysicalDeviceAccelerationStructureFeaturesKHR
    bool descriptorBindingAccelerationStructureUpdateAfterBind;
    // VkPhysicalDeviceAddressBindingReportFeaturesEXT
    bool reportAddressBinding;
    // VkPhysicalDeviceAmigoProfilingFeaturesSEC
    bool amigoProfiling;
    // VkPhysicalDeviceAntiLagFeaturesAMD
    bool antiLag;
    // VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT
    bool attachmentFeedbackLoopDynamicState;
    // VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT
    bool attachmentFeedbackLoopLayout;
    // VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT
    bool advancedBlendCoherentOperations;
    // VkPhysicalDeviceBorderColorSwizzleFeaturesEXT
    bool borderColorSwizzle;
    // VkPhysicalDeviceBorderColorSwizzleFeaturesEXT
    bool borderColorSwizzleFromImage;
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bool bufferDeviceAddress;
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bool bufferDeviceAddressCaptureReplay;
    // VkPhysicalDeviceBufferDeviceAddressFeatures, VkPhysicalDeviceVulkan12Features
    bool bufferDeviceAddressMultiDevice;
    // VkPhysicalDeviceBufferDeviceAddressFeaturesEXT
    bool bufferDeviceAddressCaptureReplayEXT;
    // VkPhysicalDeviceBufferDeviceAddressFeaturesEXT
    bool bufferDeviceAddressEXT;
    // VkPhysicalDeviceBufferDeviceAddressFeaturesEXT
    bool bufferDeviceAddressMultiDeviceEXT;
    // VkPhysicalDeviceClusterAccelerationStructureFeaturesNV
    bool clusterAccelerationStructure;
    // VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI
    bool clustercullingShader;
    // VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI
    bool multiviewClusterCullingShader;
    // VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI
    bool clusterShadingRate;
    // VkPhysicalDeviceCoherentMemoryFeaturesAMD
    bool deviceCoherentMemory;
    // VkPhysicalDeviceColorWriteEnableFeaturesEXT
    bool colorWriteEnable;
    // VkPhysicalDeviceCommandBufferInheritanceFeaturesNV
    bool commandBufferInheritance;
    // VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR
    bool computeDerivativeGroupLinear;
    // VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR
    bool computeDerivativeGroupQuads;
    // VkPhysicalDeviceConditionalRenderingFeaturesEXT
    bool conditionalRendering;
    // VkPhysicalDeviceConditionalRenderingFeaturesEXT
    bool inheritedConditionalRendering;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixBlockLoads;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixConversions;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixFlexibleDimensions;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixPerElementOperations;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixReductions;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixTensorAddressing;
    // VkPhysicalDeviceCooperativeMatrix2FeaturesNV
    bool cooperativeMatrixWorkgroupScope;
    // VkPhysicalDeviceCooperativeMatrixFeaturesKHR, VkPhysicalDeviceCooperativeMatrixFeaturesNV
    bool cooperativeMatrix;
    // VkPhysicalDeviceCooperativeMatrixFeaturesKHR, VkPhysicalDeviceCooperativeMatrixFeaturesNV
    bool cooperativeMatrixRobustBufferAccess;
    // VkPhysicalDeviceCooperativeVectorFeaturesNV
    bool cooperativeVector;
    // VkPhysicalDeviceCooperativeVectorFeaturesNV
    bool cooperativeVectorTraining;
    // VkPhysicalDeviceCopyMemoryIndirectFeaturesNV
    bool indirectCopy;
    // VkPhysicalDeviceCornerSampledImageFeaturesNV
    bool cornerSampledImage;
    // VkPhysicalDeviceCoverageReductionModeFeaturesNV
    bool coverageReductionMode;
    // VkPhysicalDeviceCubicClampFeaturesQCOM
    bool cubicRangeClamp;
    // VkPhysicalDeviceCubicWeightsFeaturesQCOM
    bool selectableCubicWeights;
    // VkPhysicalDeviceCudaKernelLaunchFeaturesNV
    bool cudaKernelLaunchFeatures;
    // VkPhysicalDeviceCustomBorderColorFeaturesEXT
    bool customBorderColorWithoutFormat;
    // VkPhysicalDeviceCustomBorderColorFeaturesEXT
    bool customBorderColors;
    // VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV
    bool dedicatedAllocationImageAliasing;
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    bool depthBiasControl;
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    bool depthBiasExact;
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    bool floatRepresentation;
    // VkPhysicalDeviceDepthBiasControlFeaturesEXT
    bool leastRepresentableValueForceUnormRepresentation;
    // VkPhysicalDeviceDepthClampControlFeaturesEXT
    bool depthClampControl;
    // VkPhysicalDeviceDepthClampZeroOneFeaturesKHR
    bool depthClampZeroOne;
    // VkPhysicalDeviceDepthClipControlFeaturesEXT
    bool depthClipControl;
    // VkPhysicalDeviceDepthClipEnableFeaturesEXT
    bool depthClipEnable;
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    bool descriptorBuffer;
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    bool descriptorBufferCaptureReplay;
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    bool descriptorBufferImageLayoutIgnored;
    // VkPhysicalDeviceDescriptorBufferFeaturesEXT
    bool descriptorBufferPushDescriptors;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingPartiallyBound;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingSampledImageUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingStorageBufferUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingStorageImageUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingStorageTexelBufferUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingUniformBufferUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingUniformTexelBufferUpdateAfterBind;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingUpdateUnusedWhilePending;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool descriptorBindingVariableDescriptorCount;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool runtimeDescriptorArray;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderInputAttachmentArrayDynamicIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderInputAttachmentArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderSampledImageArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderStorageBufferArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderStorageImageArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderStorageTexelBufferArrayDynamicIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderStorageTexelBufferArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderUniformBufferArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderUniformTexelBufferArrayDynamicIndexing;
    // VkPhysicalDeviceDescriptorIndexingFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderUniformTexelBufferArrayNonUniformIndexing;
    // VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV
    bool descriptorPoolOverallocation;
    // VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE
    bool descriptorSetHostMapping;
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    bool deviceGeneratedCompute;
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    bool deviceGeneratedComputeCaptureReplay;
    // VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV
    bool deviceGeneratedComputePipelines;
    // VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT
    bool deviceGeneratedCommands;
    // VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT
    bool dynamicGeneratedPipelineLayout;
    // VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV
    bool deviceGeneratedCommandsNV;
    // VkPhysicalDeviceDeviceMemoryReportFeaturesEXT
    bool deviceMemoryReport;
    // VkPhysicalDeviceDiagnosticsConfigFeaturesNV
    bool diagnosticsConfig;
    // VkPhysicalDeviceDisplacementMicromapFeaturesNV
    bool displacementMicromap;
    // VkPhysicalDeviceDynamicRenderingFeatures, VkPhysicalDeviceVulkan13Features
    bool dynamicRendering;
    // VkPhysicalDeviceDynamicRenderingLocalReadFeatures, VkPhysicalDeviceVulkan14Features
    bool dynamicRenderingLocalRead;
    // VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT
    bool dynamicRenderingUnusedAttachments;
    // VkPhysicalDeviceExclusiveScissorFeaturesNV
    bool exclusiveScissor;
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    bool extendedDynamicState2;
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    bool extendedDynamicState2LogicOp;
    // VkPhysicalDeviceExtendedDynamicState2FeaturesEXT
    bool extendedDynamicState2PatchControlPoints;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3AlphaToCoverageEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3AlphaToOneEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ColorBlendAdvanced;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ColorBlendEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ColorBlendEquation;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ColorWriteMask;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ConservativeRasterizationMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageModulationMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageModulationTable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageModulationTableEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageReductionMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageToColorEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3CoverageToColorLocation;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3DepthClampEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3DepthClipEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3DepthClipNegativeOneToOne;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ExtraPrimitiveOverestimationSize;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3LineRasterizationMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3LineStippleEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3LogicOpEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3PolygonMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ProvokingVertexMode;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3RasterizationSamples;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3RasterizationStream;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3RepresentativeFragmentTestEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3SampleLocationsEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3SampleMask;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ShadingRateImageEnable;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3TessellationDomainOrigin;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ViewportSwizzle;
    // VkPhysicalDeviceExtendedDynamicState3FeaturesEXT
    bool extendedDynamicState3ViewportWScalingEnable;
    // VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
    bool extendedDynamicState;
    // VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV
    bool extendedSparseAddressSpace;
    // VkPhysicalDeviceExternalFormatResolveFeaturesANDROID
    bool externalFormatResolve;
    // VkPhysicalDeviceExternalMemoryRDMAFeaturesNV
    bool externalMemoryRDMA;
    // VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX
    bool screenBufferImport;
    // VkPhysicalDeviceFaultFeaturesEXT
    bool deviceFault;
    // VkPhysicalDeviceFaultFeaturesEXT
    bool deviceFaultVendorBinary;
    // VkPhysicalDeviceFeatures
    bool alphaToOne;
    // VkPhysicalDeviceFeatures
    bool depthBiasClamp;
    // VkPhysicalDeviceFeatures
    bool depthBounds;
    // VkPhysicalDeviceFeatures
    bool depthClamp;
    // VkPhysicalDeviceFeatures
    bool drawIndirectFirstInstance;
    // VkPhysicalDeviceFeatures
    bool dualSrcBlend;
    // VkPhysicalDeviceFeatures
    bool fillModeNonSolid;
    // VkPhysicalDeviceFeatures
    bool fragmentStoresAndAtomics;
    // VkPhysicalDeviceFeatures
    bool fullDrawIndexUint32;
    // VkPhysicalDeviceFeatures
    bool geometryShader;
    // VkPhysicalDeviceFeatures
    bool imageCubeArray;
    // VkPhysicalDeviceFeatures
    bool independentBlend;
    // VkPhysicalDeviceFeatures
    bool inheritedQueries;
    // VkPhysicalDeviceFeatures
    bool largePoints;
    // VkPhysicalDeviceFeatures
    bool logicOp;
    // VkPhysicalDeviceFeatures
    bool multiDrawIndirect;
    // VkPhysicalDeviceFeatures
    bool multiViewport;
    // VkPhysicalDeviceFeatures
    bool occlusionQueryPrecise;
    // VkPhysicalDeviceFeatures
    bool pipelineStatisticsQuery;
    // VkPhysicalDeviceFeatures
    bool robustBufferAccess;
    // VkPhysicalDeviceFeatures
    bool sampleRateShading;
    // VkPhysicalDeviceFeatures
    bool samplerAnisotropy;
    // VkPhysicalDeviceFeatures
    bool shaderClipDistance;
    // VkPhysicalDeviceFeatures
    bool shaderCullDistance;
    // VkPhysicalDeviceFeatures
    bool shaderFloat64;
    // VkPhysicalDeviceFeatures
    bool shaderImageGatherExtended;
    // VkPhysicalDeviceFeatures
    bool shaderInt16;
    // VkPhysicalDeviceFeatures
    bool shaderInt64;
    // VkPhysicalDeviceFeatures
    bool shaderResourceMinLod;
    // VkPhysicalDeviceFeatures
    bool shaderResourceResidency;
    // VkPhysicalDeviceFeatures
    bool shaderSampledImageArrayDynamicIndexing;
    // VkPhysicalDeviceFeatures
    bool shaderStorageBufferArrayDynamicIndexing;
    // VkPhysicalDeviceFeatures
    bool shaderStorageImageArrayDynamicIndexing;
    // VkPhysicalDeviceFeatures
    bool shaderStorageImageExtendedFormats;
    // VkPhysicalDeviceFeatures
    bool shaderStorageImageMultisample;
    // VkPhysicalDeviceFeatures
    bool shaderStorageImageReadWithoutFormat;
    // VkPhysicalDeviceFeatures
    bool shaderStorageImageWriteWithoutFormat;
    // VkPhysicalDeviceFeatures
    bool shaderTessellationAndGeometryPointSize;
    // VkPhysicalDeviceFeatures
    bool shaderUniformBufferArrayDynamicIndexing;
    // VkPhysicalDeviceFeatures
    bool sparseBinding;
    // VkPhysicalDeviceFeatures
    bool sparseResidency16Samples;
    // VkPhysicalDeviceFeatures
    bool sparseResidency2Samples;
    // VkPhysicalDeviceFeatures
    bool sparseResidency4Samples;
    // VkPhysicalDeviceFeatures
    bool sparseResidency8Samples;
    // VkPhysicalDeviceFeatures
    bool sparseResidencyAliased;
    // VkPhysicalDeviceFeatures
    bool sparseResidencyBuffer;
    // VkPhysicalDeviceFeatures
    bool sparseResidencyImage2D;
    // VkPhysicalDeviceFeatures
    bool sparseResidencyImage3D;
    // VkPhysicalDeviceFeatures
    bool tessellationShader;
    // VkPhysicalDeviceFeatures
    bool textureCompressionASTC_LDR;
    // VkPhysicalDeviceFeatures
    bool textureCompressionBC;
    // VkPhysicalDeviceFeatures
    bool textureCompressionETC2;
    // VkPhysicalDeviceFeatures
    bool variableMultisampleRate;
    // VkPhysicalDeviceFeatures
    bool vertexPipelineStoresAndAtomics;
    // VkPhysicalDeviceFeatures
    bool wideLines;
    // VkPhysicalDeviceFragmentDensityMap2FeaturesEXT
    bool fragmentDensityMapDeferred;
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    bool fragmentDensityMap;
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    bool fragmentDensityMapDynamic;
    // VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    bool fragmentDensityMapNonSubsampledImages;
    // VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM
    bool fragmentDensityMapOffset;
    // VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR
    bool fragmentShaderBarycentric;
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    bool fragmentShaderPixelInterlock;
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    bool fragmentShaderSampleInterlock;
    // VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    bool fragmentShaderShadingRateInterlock;
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    bool fragmentShadingRateEnums;
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    bool noInvocationFragmentShadingRates;
    // VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV
    bool supersampleFragmentShadingRates;
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    bool attachmentFragmentShadingRate;
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    bool pipelineFragmentShadingRate;
    // VkPhysicalDeviceFragmentShadingRateFeaturesKHR
    bool primitiveFragmentShadingRate;
    // VkPhysicalDeviceFrameBoundaryFeaturesEXT
    bool frameBoundary;
    // VkPhysicalDeviceGlobalPriorityQueryFeatures, VkPhysicalDeviceVulkan14Features
    bool globalPriorityQuery;
    // VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT
    bool graphicsPipelineLibrary;
    // VkPhysicalDeviceHdrVividFeaturesHUAWEI
    bool hdrVivid;
    // VkPhysicalDeviceHostImageCopyFeatures, VkPhysicalDeviceVulkan14Features
    bool hostImageCopy;
    // VkPhysicalDeviceHostQueryResetFeatures, VkPhysicalDeviceVulkan12Features
    bool hostQueryReset;
    // VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    bool image2DViewOf3D;
    // VkPhysicalDeviceImage2DViewOf3DFeaturesEXT
    bool sampler2DViewOf3D;
    // VkPhysicalDeviceImageAlignmentControlFeaturesMESA
    bool imageAlignmentControl;
    // VkPhysicalDeviceImageCompressionControlFeaturesEXT
    bool imageCompressionControl;
    // VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT
    bool imageCompressionControlSwapchain;
    // VkPhysicalDeviceImageProcessing2FeaturesQCOM
    bool textureBlockMatch2;
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    bool textureBlockMatch;
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    bool textureBoxFilter;
    // VkPhysicalDeviceImageProcessingFeaturesQCOM
    bool textureSampleWeighted;
    // VkPhysicalDeviceImageRobustnessFeatures, VkPhysicalDeviceVulkan13Features
    bool robustImageAccess;
    // VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT
    bool imageSlicedViewOf3D;
    // VkPhysicalDeviceImageViewMinLodFeaturesEXT
    bool minLod;
    // VkPhysicalDeviceImagelessFramebufferFeatures, VkPhysicalDeviceVulkan12Features
    bool imagelessFramebuffer;
    // VkPhysicalDeviceIndexTypeUint8Features, VkPhysicalDeviceVulkan14Features
    bool indexTypeUint8;
    // VkPhysicalDeviceInheritedViewportScissorFeaturesNV
    bool inheritedViewportScissor2D;
    // VkPhysicalDeviceInlineUniformBlockFeatures, VkPhysicalDeviceVulkan13Features
    bool descriptorBindingInlineUniformBlockUpdateAfterBind;
    // VkPhysicalDeviceInlineUniformBlockFeatures, VkPhysicalDeviceVulkan13Features
    bool inlineUniformBlock;
    // VkPhysicalDeviceInvocationMaskFeaturesHUAWEI
    bool invocationMask;
    // VkPhysicalDeviceLegacyDitheringFeaturesEXT
    bool legacyDithering;
    // VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT
    bool legacyVertexAttributes;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool bresenhamLines;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool rectangularLines;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool smoothLines;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool stippledBresenhamLines;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool stippledRectangularLines;
    // VkPhysicalDeviceLineRasterizationFeatures, VkPhysicalDeviceVulkan14Features
    bool stippledSmoothLines;
    // VkPhysicalDeviceLinearColorAttachmentFeaturesNV
    bool linearColorAttachment;
    // VkPhysicalDeviceMaintenance4Features, VkPhysicalDeviceVulkan13Features
    bool maintenance4;
    // VkPhysicalDeviceMaintenance5Features, VkPhysicalDeviceVulkan14Features
    bool maintenance5;
    // VkPhysicalDeviceMaintenance6Features, VkPhysicalDeviceVulkan14Features
    bool maintenance6;
    // VkPhysicalDeviceMaintenance7FeaturesKHR
    bool maintenance7;
    // VkPhysicalDeviceMaintenance8FeaturesKHR
    bool maintenance8;
    // VkPhysicalDeviceMapMemoryPlacedFeaturesEXT
    bool memoryMapPlaced;
    // VkPhysicalDeviceMapMemoryPlacedFeaturesEXT
    bool memoryMapRangePlaced;
    // VkPhysicalDeviceMapMemoryPlacedFeaturesEXT
    bool memoryUnmapReserve;
    // VkPhysicalDeviceMemoryDecompressionFeaturesNV
    bool memoryDecompression;
    // VkPhysicalDeviceMemoryPriorityFeaturesEXT
    bool memoryPriority;
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    bool meshShaderQueries;
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    bool multiviewMeshShader;
    // VkPhysicalDeviceMeshShaderFeaturesEXT
    bool primitiveFragmentShadingRateMeshShader;
    // VkPhysicalDeviceMeshShaderFeaturesEXT, VkPhysicalDeviceMeshShaderFeaturesNV
    bool meshShader;
    // VkPhysicalDeviceMeshShaderFeaturesEXT, VkPhysicalDeviceMeshShaderFeaturesNV
    bool taskShader;
    // VkPhysicalDeviceMultiDrawFeaturesEXT
    bool multiDraw;
    // VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT
    bool multisampledRenderToSingleSampled;
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    bool multiview;
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    bool multiviewGeometryShader;
    // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVulkan11Features
    bool multiviewTessellationShader;
    // VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM
    bool multiviewPerViewRenderAreas;
    // VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM
    bool multiviewPerViewViewports;
    // VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT
    bool mutableDescriptorType;
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    bool nestedCommandBuffer;
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    bool nestedCommandBufferRendering;
    // VkPhysicalDeviceNestedCommandBufferFeaturesEXT
    bool nestedCommandBufferSimultaneousUse;
    // VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT
    bool nonSeamlessCubeMap;
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    bool micromap;
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    bool micromapCaptureReplay;
    // VkPhysicalDeviceOpacityMicromapFeaturesEXT
    bool micromapHostCommands;
    // VkPhysicalDeviceOpticalFlowFeaturesNV
    bool opticalFlow;
    // VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT
    bool pageableDeviceLocalMemory;
    // VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV
    bool partitionedAccelerationStructure;
    // VkPhysicalDevicePerStageDescriptorSetFeaturesNV
    bool dynamicPipelineLayout;
    // VkPhysicalDevicePerStageDescriptorSetFeaturesNV
    bool perStageDescriptorSet;
    // VkPhysicalDevicePerformanceQueryFeaturesKHR
    bool performanceCounterMultipleQueryPools;
    // VkPhysicalDevicePerformanceQueryFeaturesKHR
    bool performanceCounterQueryPools;
    // VkPhysicalDevicePipelineBinaryFeaturesKHR
    bool pipelineBinaries;
    // VkPhysicalDevicePipelineCreationCacheControlFeatures, VkPhysicalDeviceVulkan13Features
    bool pipelineCreationCacheControl;
    // VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR
    bool pipelineExecutableInfo;
    // VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT
    bool pipelineLibraryGroupHandles;
    // VkPhysicalDevicePipelineOpacityMicromapFeaturesARM
    bool pipelineOpacityMicromap;
    // VkPhysicalDevicePipelinePropertiesFeaturesEXT
    bool pipelinePropertiesIdentifier;
    // VkPhysicalDevicePipelineProtectedAccessFeatures, VkPhysicalDeviceVulkan14Features
    bool pipelineProtectedAccess;
    // VkPhysicalDevicePipelineRobustnessFeatures, VkPhysicalDeviceVulkan14Features
    bool pipelineRobustness;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool constantAlphaColorBlendFactors;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool events;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool imageView2DOn3DImage;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool imageViewFormatReinterpretation;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool imageViewFormatSwizzle;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool multisampleArrayImage;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool mutableComparisonSamplers;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool pointPolygons;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool samplerMipLodBias;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool separateStencilMaskRef;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool shaderSampleRateInterpolationFunctions;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool tessellationIsolines;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool tessellationPointMode;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool triangleFans;
    // VkPhysicalDevicePortabilitySubsetFeaturesKHR
    bool vertexAttributeAccessBeyondStride;
    // VkPhysicalDevicePresentBarrierFeaturesNV
    bool presentBarrier;
    // VkPhysicalDevicePresentIdFeaturesKHR
    bool presentId;
    // VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT
    bool presentModeFifoLatestReady;
    // VkPhysicalDevicePresentWaitFeaturesKHR
    bool presentWait;
    // VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT
    bool primitiveTopologyListRestart;
    // VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT
    bool primitiveTopologyPatchListRestart;
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    bool primitivesGeneratedQuery;
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    bool primitivesGeneratedQueryWithNonZeroStreams;
    // VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT
    bool primitivesGeneratedQueryWithRasterizerDiscard;
    // VkPhysicalDevicePrivateDataFeatures, VkPhysicalDeviceVulkan13Features
    bool privateData;
    // VkPhysicalDeviceProtectedMemoryFeatures, VkPhysicalDeviceVulkan11Features
    bool protectedMemory;
    // VkPhysicalDeviceProvokingVertexFeaturesEXT
    bool provokingVertexLast;
    // VkPhysicalDeviceProvokingVertexFeaturesEXT
    bool transformFeedbackPreservesProvokingVertex;
    // VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT
    bool formatRgba10x6WithoutYCbCrSampler;
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    bool rasterizationOrderColorAttachmentAccess;
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    bool rasterizationOrderDepthAttachmentAccess;
    // VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
    bool rasterizationOrderStencilAttachmentAccess;
    // VkPhysicalDeviceRawAccessChainsFeaturesNV
    bool shaderRawAccessChains;
    // VkPhysicalDeviceRayQueryFeaturesKHR
    bool rayQuery;
    // VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV
    bool rayTracingInvocationReorder;
    // VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV
    bool linearSweptSpheres;
    // VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV
    bool spheres;
    // VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR
    bool rayTracingMaintenance1;
    // VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR
    bool rayTracingPipelineTraceRaysIndirect2;
    // VkPhysicalDeviceRayTracingMotionBlurFeaturesNV
    bool rayTracingMotionBlur;
    // VkPhysicalDeviceRayTracingMotionBlurFeaturesNV
    bool rayTracingMotionBlurPipelineTraceRaysIndirect;
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    bool rayTracingPipeline;
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    bool rayTracingPipelineShaderGroupHandleCaptureReplay;
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    bool rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    bool rayTracingPipelineTraceRaysIndirect;
    // VkPhysicalDeviceRayTracingPipelineFeaturesKHR
    bool rayTraversalPrimitiveCulling;
    // VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR
    bool rayTracingPositionFetch;
    // VkPhysicalDeviceRayTracingValidationFeaturesNV
    bool rayTracingValidation;
    // VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG
    bool relaxedLineRasterization;
    // VkPhysicalDeviceRenderPassStripedFeaturesARM
    bool renderPassStriped;
    // VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV
    bool representativeFragmentTest;
    // VkPhysicalDeviceRobustness2FeaturesEXT
    bool nullDescriptor;
    // VkPhysicalDeviceRobustness2FeaturesEXT
    bool robustBufferAccess2;
    // VkPhysicalDeviceRobustness2FeaturesEXT
    bool robustImageAccess2;
    // VkPhysicalDeviceSamplerYcbcrConversionFeatures, VkPhysicalDeviceVulkan11Features
    bool samplerYcbcrConversion;
    // VkPhysicalDeviceScalarBlockLayoutFeatures, VkPhysicalDeviceVulkan12Features
    bool scalarBlockLayout;
    // VkPhysicalDeviceSchedulingControlsFeaturesARM
    bool schedulingControls;
    // VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures, VkPhysicalDeviceVulkan12Features
    bool separateDepthStencilLayouts;
    // VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV
    bool shaderFloat16VectorAtomics;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderBufferFloat16AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderBufferFloat16AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderBufferFloat16Atomics;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderBufferFloat32AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderBufferFloat64AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderImageFloat32AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderSharedFloat16AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderSharedFloat16AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderSharedFloat16Atomics;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderSharedFloat32AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool shaderSharedFloat64AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT
    bool sparseImageFloat32AtomicMinMax;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderBufferFloat32AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderBufferFloat32Atomics;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderBufferFloat64AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderBufferFloat64Atomics;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderImageFloat32AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderImageFloat32Atomics;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderSharedFloat32AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderSharedFloat32Atomics;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderSharedFloat64AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool shaderSharedFloat64Atomics;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool sparseImageFloat32AtomicAdd;
    // VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
    bool sparseImageFloat32Atomics;
    // VkPhysicalDeviceShaderAtomicInt64Features, VkPhysicalDeviceVulkan12Features
    bool shaderBufferInt64Atomics;
    // VkPhysicalDeviceShaderAtomicInt64Features, VkPhysicalDeviceVulkan12Features
    bool shaderSharedInt64Atomics;
    // VkPhysicalDeviceShaderClockFeaturesKHR
    bool shaderDeviceClock;
    // VkPhysicalDeviceShaderClockFeaturesKHR
    bool shaderSubgroupClock;
    // VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM
    bool shaderCoreBuiltins;
    // VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures, VkPhysicalDeviceVulkan13Features
    bool shaderDemoteToHelperInvocation;
    // VkPhysicalDeviceShaderDrawParametersFeatures, VkPhysicalDeviceVulkan11Features
    bool shaderDrawParameters;
    // VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD
    bool shaderEarlyAndLateFragmentTests;
    // VkPhysicalDeviceShaderEnqueueFeaturesAMDX
    bool shaderEnqueue;
    // VkPhysicalDeviceShaderEnqueueFeaturesAMDX
    bool shaderMeshEnqueue;
    // VkPhysicalDeviceShaderExpectAssumeFeatures, VkPhysicalDeviceVulkan14Features
    bool shaderExpectAssume;
    // VkPhysicalDeviceShaderFloat16Int8Features, VkPhysicalDeviceVulkan12Features
    bool shaderFloat16;
    // VkPhysicalDeviceShaderFloat16Int8Features, VkPhysicalDeviceVulkan12Features
    bool shaderInt8;
    // VkPhysicalDeviceShaderFloatControls2Features, VkPhysicalDeviceVulkan14Features
    bool shaderFloatControls2;
    // VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
    bool shaderImageInt64Atomics;
    // VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT
    bool sparseImageInt64Atomics;
    // VkPhysicalDeviceShaderImageFootprintFeaturesNV
    bool imageFootprint;
    // VkPhysicalDeviceShaderIntegerDotProductFeatures, VkPhysicalDeviceVulkan13Features
    bool shaderIntegerDotProduct;
    // VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL
    bool shaderIntegerFunctions2;
    // VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR
    bool shaderMaximalReconvergence;
    // VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT
    bool shaderModuleIdentifier;
    // VkPhysicalDeviceShaderObjectFeaturesEXT
    bool shaderObject;
    // VkPhysicalDeviceShaderQuadControlFeaturesKHR
    bool shaderQuadControl;
    // VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR
    bool shaderRelaxedExtendedInstruction;
    // VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT
    bool shaderReplicatedComposites;
    // VkPhysicalDeviceShaderSMBuiltinsFeaturesNV
    bool shaderSMBuiltins;
    // VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures, VkPhysicalDeviceVulkan12Features
    bool shaderSubgroupExtendedTypes;
    // VkPhysicalDeviceShaderSubgroupRotateFeatures, VkPhysicalDeviceVulkan14Features
    bool shaderSubgroupRotate;
    // VkPhysicalDeviceShaderSubgroupRotateFeatures, VkPhysicalDeviceVulkan14Features
    bool shaderSubgroupRotateClustered;
    // VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR
    bool shaderSubgroupUniformControlFlow;
    // VkPhysicalDeviceShaderTerminateInvocationFeatures, VkPhysicalDeviceVulkan13Features
    bool shaderTerminateInvocation;
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    bool shaderTileImageColorReadAccess;
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    bool shaderTileImageDepthReadAccess;
    // VkPhysicalDeviceShaderTileImageFeaturesEXT
    bool shaderTileImageStencilReadAccess;
    // VkPhysicalDeviceShadingRateImageFeaturesNV
    bool shadingRateCoarseSampleOrder;
    // VkPhysicalDeviceShadingRateImageFeaturesNV
    bool shadingRateImage;
    // VkPhysicalDeviceSubgroupSizeControlFeatures, VkPhysicalDeviceVulkan13Features
    bool computeFullSubgroups;
    // VkPhysicalDeviceSubgroupSizeControlFeatures, VkPhysicalDeviceVulkan13Features
    bool subgroupSizeControl;
    // VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT
    bool subpassMergeFeedback;
    // VkPhysicalDeviceSubpassShadingFeaturesHUAWEI
    bool subpassShading;
    // VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT
    bool swapchainMaintenance1;
    // VkPhysicalDeviceSynchronization2Features, VkPhysicalDeviceVulkan13Features
    bool synchronization2;
    // VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT
    bool texelBufferAlignment;
    // VkPhysicalDeviceTextureCompressionASTCHDRFeatures, VkPhysicalDeviceVulkan13Features
    bool textureCompressionASTC_HDR;
    // VkPhysicalDeviceTilePropertiesFeaturesQCOM
    bool tileProperties;
    // VkPhysicalDeviceTimelineSemaphoreFeatures, VkPhysicalDeviceVulkan12Features
    bool timelineSemaphore;
    // VkPhysicalDeviceTransformFeedbackFeaturesEXT
    bool geometryStreams;
    // VkPhysicalDeviceTransformFeedbackFeaturesEXT
    bool transformFeedback;
    // VkPhysicalDeviceUniformBufferStandardLayoutFeatures, VkPhysicalDeviceVulkan12Features
    bool uniformBufferStandardLayout;
    // VkPhysicalDeviceVariablePointersFeatures, VkPhysicalDeviceVulkan11Features
    bool variablePointers;
    // VkPhysicalDeviceVariablePointersFeatures, VkPhysicalDeviceVulkan11Features
    bool variablePointersStorageBuffer;
    // VkPhysicalDeviceVertexAttributeDivisorFeatures, VkPhysicalDeviceVulkan14Features
    bool vertexAttributeInstanceRateDivisor;
    // VkPhysicalDeviceVertexAttributeDivisorFeatures, VkPhysicalDeviceVulkan14Features
    bool vertexAttributeInstanceRateZeroDivisor;
    // VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT
    bool vertexAttributeRobustness;
    // VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT
    bool vertexInputDynamicState;
    // VkPhysicalDeviceVideoEncodeAV1FeaturesKHR
    bool videoEncodeAV1;
    // VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR
    bool videoEncodeQuantizationMap;
    // VkPhysicalDeviceVideoMaintenance1FeaturesKHR
    bool videoMaintenance1;
    // VkPhysicalDeviceVideoMaintenance2FeaturesKHR
    bool videoMaintenance2;
    // VkPhysicalDeviceVulkan12Features
    bool descriptorIndexing;
    // VkPhysicalDeviceVulkan12Features
    bool drawIndirectCount;
    // VkPhysicalDeviceVulkan12Features
    bool samplerFilterMinmax;
    // VkPhysicalDeviceVulkan12Features
    bool samplerMirrorClampToEdge;
    // VkPhysicalDeviceVulkan12Features
    bool shaderOutputLayer;
    // VkPhysicalDeviceVulkan12Features
    bool shaderOutputViewportIndex;
    // VkPhysicalDeviceVulkan12Features
    bool subgroupBroadcastDynamicId;
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    bool vulkanMemoryModel;
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    bool vulkanMemoryModelAvailabilityVisibilityChains;
    // VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkanMemoryModelFeatures
    bool vulkanMemoryModelDeviceScope;
    // VkPhysicalDeviceVulkan13Features, VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures
    bool shaderZeroInitializeWorkgroupMemory;
    // VkPhysicalDeviceVulkan14Features
    bool pushDescriptor;
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    bool workgroupMemoryExplicitLayout;
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    bool workgroupMemoryExplicitLayout16BitAccess;
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    bool workgroupMemoryExplicitLayout8BitAccess;
    // VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR
    bool workgroupMemoryExplicitLayoutScalarBlockLayout;
    // VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT
    bool ycbcr2plane444Formats;
    // VkPhysicalDeviceYcbcrDegammaFeaturesQCOM
    bool ycbcrDegamma;
    // VkPhysicalDeviceYcbcrImageArraysFeaturesEXT
    bool ycbcrImageArrays;
};

void GetEnabledDeviceFeatures(const VkDeviceCreateInfo *pCreateInfo, DeviceFeatures *features, const APIVersion &api_version);

// NOLINTEND
