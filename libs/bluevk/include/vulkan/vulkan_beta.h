#ifndef VULKAN_BETA_H_
#define VULKAN_BETA_H_ 1

/*
** Copyright 2015-2022 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0
*/

/*
** This header is generated from the Khronos Vulkan XML API Registry.
**
*/


#ifdef __cplusplus
extern "C" {
#endif



#define VK_KHR_video_queue 1
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkVideoSessionKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkVideoSessionParametersKHR)
#define VK_KHR_VIDEO_QUEUE_SPEC_VERSION   3
#define VK_KHR_VIDEO_QUEUE_EXTENSION_NAME "VK_KHR_video_queue"

typedef enum VkQueryResultStatusKHR {
    VK_QUERY_RESULT_STATUS_ERROR_KHR = -1,
    VK_QUERY_RESULT_STATUS_NOT_READY_KHR = 0,
    VK_QUERY_RESULT_STATUS_COMPLETE_KHR = 1,
    VK_QUERY_RESULT_STATUS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkQueryResultStatusKHR;

typedef enum VkVideoCodecOperationFlagBitsKHR {
    VK_VIDEO_CODEC_OPERATION_INVALID_BIT_KHR = 0,
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT = 0x00010000,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_EXT = 0x00020000,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_EXT = 0x00000001,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_EXT = 0x00000002,
#endif
    VK_VIDEO_CODEC_OPERATION_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoCodecOperationFlagBitsKHR;
typedef VkFlags VkVideoCodecOperationFlagsKHR;

typedef enum VkVideoChromaSubsamplingFlagBitsKHR {
    VK_VIDEO_CHROMA_SUBSAMPLING_INVALID_BIT_KHR = 0,
    VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR = 0x00000001,
    VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR = 0x00000002,
    VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR = 0x00000004,
    VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR = 0x00000008,
    VK_VIDEO_CHROMA_SUBSAMPLING_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoChromaSubsamplingFlagBitsKHR;
typedef VkFlags VkVideoChromaSubsamplingFlagsKHR;

typedef enum VkVideoComponentBitDepthFlagBitsKHR {
    VK_VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR = 0,
    VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR = 0x00000001,
    VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR = 0x00000004,
    VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR = 0x00000010,
    VK_VIDEO_COMPONENT_BIT_DEPTH_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoComponentBitDepthFlagBitsKHR;
typedef VkFlags VkVideoComponentBitDepthFlagsKHR;

typedef enum VkVideoCapabilityFlagBitsKHR {
    VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR = 0x00000001,
    VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR = 0x00000002,
    VK_VIDEO_CAPABILITY_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoCapabilityFlagBitsKHR;
typedef VkFlags VkVideoCapabilityFlagsKHR;

typedef enum VkVideoSessionCreateFlagBitsKHR {
    VK_VIDEO_SESSION_CREATE_DEFAULT_KHR = 0,
    VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR = 0x00000001,
    VK_VIDEO_SESSION_CREATE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoSessionCreateFlagBitsKHR;
typedef VkFlags VkVideoSessionCreateFlagsKHR;
typedef VkFlags VkVideoBeginCodingFlagsKHR;
typedef VkFlags VkVideoEndCodingFlagsKHR;

typedef enum VkVideoCodingControlFlagBitsKHR {
    VK_VIDEO_CODING_CONTROL_DEFAULT_KHR = 0,
    VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR = 0x00000001,
    VK_VIDEO_CODING_CONTROL_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoCodingControlFlagBitsKHR;
typedef VkFlags VkVideoCodingControlFlagsKHR;

typedef enum VkVideoCodingQualityPresetFlagBitsKHR {
    VK_VIDEO_CODING_QUALITY_PRESET_NORMAL_BIT_KHR = 0x00000001,
    VK_VIDEO_CODING_QUALITY_PRESET_POWER_BIT_KHR = 0x00000002,
    VK_VIDEO_CODING_QUALITY_PRESET_QUALITY_BIT_KHR = 0x00000004,
    VK_VIDEO_CODING_QUALITY_PRESET_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoCodingQualityPresetFlagBitsKHR;
typedef VkFlags VkVideoCodingQualityPresetFlagsKHR;
typedef struct VkQueueFamilyQueryResultStatusProperties2KHR {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           supported;
} VkQueueFamilyQueryResultStatusProperties2KHR;

typedef struct VkVideoQueueFamilyProperties2KHR {
    VkStructureType                  sType;
    void*                            pNext;
    VkVideoCodecOperationFlagsKHR    videoCodecOperations;
} VkVideoQueueFamilyProperties2KHR;

typedef struct VkVideoProfileKHR {
    VkStructureType                     sType;
    void*                               pNext;
    VkVideoCodecOperationFlagBitsKHR    videoCodecOperation;
    VkVideoChromaSubsamplingFlagsKHR    chromaSubsampling;
    VkVideoComponentBitDepthFlagsKHR    lumaBitDepth;
    VkVideoComponentBitDepthFlagsKHR    chromaBitDepth;
} VkVideoProfileKHR;

typedef struct VkVideoProfilesKHR {
    VkStructureType             sType;
    void*                       pNext;
    uint32_t                    profileCount;
    const VkVideoProfileKHR*    pProfiles;
} VkVideoProfilesKHR;

typedef struct VkVideoCapabilitiesKHR {
    VkStructureType              sType;
    void*                        pNext;
    VkVideoCapabilityFlagsKHR    capabilityFlags;
    VkDeviceSize                 minBitstreamBufferOffsetAlignment;
    VkDeviceSize                 minBitstreamBufferSizeAlignment;
    VkExtent2D                   videoPictureExtentGranularity;
    VkExtent2D                   minExtent;
    VkExtent2D                   maxExtent;
    uint32_t                     maxReferencePicturesSlotsCount;
    uint32_t                     maxReferencePicturesActiveCount;
    VkExtensionProperties        stdHeaderVersion;
} VkVideoCapabilitiesKHR;

typedef struct VkPhysicalDeviceVideoFormatInfoKHR {
    VkStructureType              sType;
    void*                        pNext;
    VkImageUsageFlags            imageUsage;
    const VkVideoProfilesKHR*    pVideoProfiles;
} VkPhysicalDeviceVideoFormatInfoKHR;

typedef struct VkVideoFormatPropertiesKHR {
    VkStructureType    sType;
    void*              pNext;
    VkFormat           format;
} VkVideoFormatPropertiesKHR;

typedef struct VkVideoPictureResourceKHR {
    VkStructureType    sType;
    const void*        pNext;
    VkOffset2D         codedOffset;
    VkExtent2D         codedExtent;
    uint32_t           baseArrayLayer;
    VkImageView        imageViewBinding;
} VkVideoPictureResourceKHR;

typedef struct VkVideoReferenceSlotKHR {
    VkStructureType                     sType;
    const void*                         pNext;
    int8_t                              slotIndex;
    const VkVideoPictureResourceKHR*    pPictureResource;
} VkVideoReferenceSlotKHR;

typedef struct VkVideoGetMemoryPropertiesKHR {
    VkStructureType           sType;
    const void*               pNext;
    uint32_t                  memoryBindIndex;
    VkMemoryRequirements2*    pMemoryRequirements;
} VkVideoGetMemoryPropertiesKHR;

typedef struct VkVideoBindMemoryKHR {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           memoryBindIndex;
    VkDeviceMemory     memory;
    VkDeviceSize       memoryOffset;
    VkDeviceSize       memorySize;
} VkVideoBindMemoryKHR;

typedef struct VkVideoSessionCreateInfoKHR {
    VkStructureType                 sType;
    const void*                     pNext;
    uint32_t                        queueFamilyIndex;
    VkVideoSessionCreateFlagsKHR    flags;
    const VkVideoProfileKHR*        pVideoProfile;
    VkFormat                        pictureFormat;
    VkExtent2D                      maxCodedExtent;
    VkFormat                        referencePicturesFormat;
    uint32_t                        maxReferencePicturesSlotsCount;
    uint32_t                        maxReferencePicturesActiveCount;
    const VkExtensionProperties*    pStdHeaderVersion;
} VkVideoSessionCreateInfoKHR;

typedef struct VkVideoSessionParametersCreateInfoKHR {
    VkStructureType                sType;
    const void*                    pNext;
    VkVideoSessionParametersKHR    videoSessionParametersTemplate;
    VkVideoSessionKHR              videoSession;
} VkVideoSessionParametersCreateInfoKHR;

typedef struct VkVideoSessionParametersUpdateInfoKHR {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           updateSequenceCount;
} VkVideoSessionParametersUpdateInfoKHR;

typedef struct VkVideoBeginCodingInfoKHR {
    VkStructureType                       sType;
    const void*                           pNext;
    VkVideoBeginCodingFlagsKHR            flags;
    VkVideoCodingQualityPresetFlagsKHR    codecQualityPreset;
    VkVideoSessionKHR                     videoSession;
    VkVideoSessionParametersKHR           videoSessionParameters;
    uint32_t                              referenceSlotCount;
    const VkVideoReferenceSlotKHR*        pReferenceSlots;
} VkVideoBeginCodingInfoKHR;

typedef struct VkVideoEndCodingInfoKHR {
    VkStructureType             sType;
    const void*                 pNext;
    VkVideoEndCodingFlagsKHR    flags;
} VkVideoEndCodingInfoKHR;

typedef struct VkVideoCodingControlInfoKHR {
    VkStructureType                 sType;
    const void*                     pNext;
    VkVideoCodingControlFlagsKHR    flags;
} VkVideoCodingControlInfoKHR;

typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)(VkPhysicalDevice physicalDevice, const VkVideoProfileKHR* pVideoProfile, VkVideoCapabilitiesKHR* pCapabilities);
typedef VkResult (VKAPI_PTR *PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo, uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties);
typedef VkResult (VKAPI_PTR *PFN_vkCreateVideoSessionKHR)(VkDevice device, const VkVideoSessionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession);
typedef void (VKAPI_PTR *PFN_vkDestroyVideoSessionKHR)(VkDevice device, VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator);
typedef VkResult (VKAPI_PTR *PFN_vkGetVideoSessionMemoryRequirementsKHR)(VkDevice device, VkVideoSessionKHR videoSession, uint32_t* pVideoSessionMemoryRequirementsCount, VkVideoGetMemoryPropertiesKHR* pVideoSessionMemoryRequirements);
typedef VkResult (VKAPI_PTR *PFN_vkBindVideoSessionMemoryKHR)(VkDevice device, VkVideoSessionKHR videoSession, uint32_t videoSessionBindMemoryCount, const VkVideoBindMemoryKHR* pVideoSessionBindMemories);
typedef VkResult (VKAPI_PTR *PFN_vkCreateVideoSessionParametersKHR)(VkDevice device, const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionParametersKHR* pVideoSessionParameters);
typedef VkResult (VKAPI_PTR *PFN_vkUpdateVideoSessionParametersKHR)(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo);
typedef void (VKAPI_PTR *PFN_vkDestroyVideoSessionParametersKHR)(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks* pAllocator);
typedef void (VKAPI_PTR *PFN_vkCmdBeginVideoCodingKHR)(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo);
typedef void (VKAPI_PTR *PFN_vkCmdEndVideoCodingKHR)(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo);
typedef void (VKAPI_PTR *PFN_vkCmdControlVideoCodingKHR)(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileKHR*                    pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities);

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties);

VKAPI_ATTR VkResult VKAPI_CALL vkCreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession);

VKAPI_ATTR void VKAPI_CALL vkDestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR VkResult VKAPI_CALL vkGetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pVideoSessionMemoryRequirementsCount,
    VkVideoGetMemoryPropertiesKHR*              pVideoSessionMemoryRequirements);

VKAPI_ATTR VkResult VKAPI_CALL vkBindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    videoSessionBindMemoryCount,
    const VkVideoBindMemoryKHR*                 pVideoSessionBindMemories);

VKAPI_ATTR VkResult VKAPI_CALL vkCreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters);

VKAPI_ATTR VkResult VKAPI_CALL vkUpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo);

VKAPI_ATTR void VKAPI_CALL vkDestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator);

VKAPI_ATTR void VKAPI_CALL vkCmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo);

VKAPI_ATTR void VKAPI_CALL vkCmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo);

VKAPI_ATTR void VKAPI_CALL vkCmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo);
#endif


#define VK_KHR_video_decode_queue 1
#define VK_KHR_VIDEO_DECODE_QUEUE_SPEC_VERSION 4
#define VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME "VK_KHR_video_decode_queue"

typedef enum VkVideoDecodeCapabilityFlagBitsKHR {
    VK_VIDEO_DECODE_CAPABILITY_DEFAULT_KHR = 0,
    VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR = 0x00000001,
    VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR = 0x00000002,
    VK_VIDEO_DECODE_CAPABILITY_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoDecodeCapabilityFlagBitsKHR;
typedef VkFlags VkVideoDecodeCapabilityFlagsKHR;

typedef enum VkVideoDecodeFlagBitsKHR {
    VK_VIDEO_DECODE_DEFAULT_KHR = 0,
    VK_VIDEO_DECODE_RESERVED_0_BIT_KHR = 0x00000001,
    VK_VIDEO_DECODE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoDecodeFlagBitsKHR;
typedef VkFlags VkVideoDecodeFlagsKHR;
typedef struct VkVideoDecodeCapabilitiesKHR {
    VkStructureType                    sType;
    void*                              pNext;
    VkVideoDecodeCapabilityFlagsKHR    flags;
} VkVideoDecodeCapabilitiesKHR;

typedef struct VkVideoDecodeInfoKHR {
    VkStructureType                   sType;
    const void*                       pNext;
    VkVideoDecodeFlagsKHR             flags;
    VkBuffer                          srcBuffer;
    VkDeviceSize                      srcBufferOffset;
    VkDeviceSize                      srcBufferRange;
    VkVideoPictureResourceKHR         dstPictureResource;
    const VkVideoReferenceSlotKHR*    pSetupReferenceSlot;
    uint32_t                          referenceSlotCount;
    const VkVideoReferenceSlotKHR*    pReferenceSlots;
} VkVideoDecodeInfoKHR;

typedef void (VKAPI_PTR *PFN_vkCmdDecodeVideoKHR)(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pFrameInfo);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR void VKAPI_CALL vkCmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pFrameInfo);
#endif


#define VK_KHR_portability_subset 1
#define VK_KHR_PORTABILITY_SUBSET_SPEC_VERSION 1
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
typedef struct VkPhysicalDevicePortabilitySubsetFeaturesKHR {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           constantAlphaColorBlendFactors;
    VkBool32           events;
    VkBool32           imageViewFormatReinterpretation;
    VkBool32           imageViewFormatSwizzle;
    VkBool32           imageView2DOn3DImage;
    VkBool32           multisampleArrayImage;
    VkBool32           mutableComparisonSamplers;
    VkBool32           pointPolygons;
    VkBool32           samplerMipLodBias;
    VkBool32           separateStencilMaskRef;
    VkBool32           shaderSampleRateInterpolationFunctions;
    VkBool32           tessellationIsolines;
    VkBool32           tessellationPointMode;
    VkBool32           triangleFans;
    VkBool32           vertexAttributeAccessBeyondStride;
} VkPhysicalDevicePortabilitySubsetFeaturesKHR;

typedef struct VkPhysicalDevicePortabilitySubsetPropertiesKHR {
    VkStructureType    sType;
    void*              pNext;
    uint32_t           minVertexInputBindingStrideAlignment;
} VkPhysicalDevicePortabilitySubsetPropertiesKHR;



#define VK_KHR_video_encode_queue 1
#define VK_KHR_VIDEO_ENCODE_QUEUE_SPEC_VERSION 5
#define VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME "VK_KHR_video_encode_queue"

typedef enum VkVideoEncodeFlagBitsKHR {
    VK_VIDEO_ENCODE_DEFAULT_KHR = 0,
    VK_VIDEO_ENCODE_RESERVED_0_BIT_KHR = 0x00000001,
    VK_VIDEO_ENCODE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoEncodeFlagBitsKHR;
typedef VkFlags VkVideoEncodeFlagsKHR;

typedef enum VkVideoEncodeCapabilityFlagBitsKHR {
    VK_VIDEO_ENCODE_CAPABILITY_DEFAULT_KHR = 0,
    VK_VIDEO_ENCODE_CAPABILITY_PRECEDING_EXTERNALLY_ENCODED_BYTES_BIT_KHR = 0x00000001,
    VK_VIDEO_ENCODE_CAPABILITY_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoEncodeCapabilityFlagBitsKHR;
typedef VkFlags VkVideoEncodeCapabilityFlagsKHR;

typedef enum VkVideoEncodeRateControlModeFlagBitsKHR {
    VK_VIDEO_ENCODE_RATE_CONTROL_MODE_NONE_BIT_KHR = 0,
    VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR = 1,
    VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR = 2,
    VK_VIDEO_ENCODE_RATE_CONTROL_MODE_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoEncodeRateControlModeFlagBitsKHR;
typedef VkFlags VkVideoEncodeRateControlModeFlagsKHR;

typedef enum VkVideoEncodeRateControlFlagBitsKHR {
    VK_VIDEO_ENCODE_RATE_CONTROL_DEFAULT_KHR = 0,
    VK_VIDEO_ENCODE_RATE_CONTROL_RESERVED_0_BIT_KHR = 0x00000001,
    VK_VIDEO_ENCODE_RATE_CONTROL_FLAG_BITS_MAX_ENUM_KHR = 0x7FFFFFFF
} VkVideoEncodeRateControlFlagBitsKHR;
typedef VkFlags VkVideoEncodeRateControlFlagsKHR;
typedef struct VkVideoEncodeInfoKHR {
    VkStructureType                   sType;
    const void*                       pNext;
    VkVideoEncodeFlagsKHR             flags;
    uint32_t                          qualityLevel;
    VkBuffer                          dstBitstreamBuffer;
    VkDeviceSize                      dstBitstreamBufferOffset;
    VkDeviceSize                      dstBitstreamBufferMaxRange;
    VkVideoPictureResourceKHR         srcPictureResource;
    const VkVideoReferenceSlotKHR*    pSetupReferenceSlot;
    uint32_t                          referenceSlotCount;
    const VkVideoReferenceSlotKHR*    pReferenceSlots;
    uint32_t                          precedingExternallyEncodedBytes;
} VkVideoEncodeInfoKHR;

typedef struct VkVideoEncodeCapabilitiesKHR {
    VkStructureType                         sType;
    void*                                   pNext;
    VkVideoEncodeCapabilityFlagsKHR         flags;
    VkVideoEncodeRateControlModeFlagsKHR    rateControlModes;
    uint8_t                                 rateControlLayerCount;
    uint8_t                                 qualityLevelCount;
    VkExtent2D                              inputImageDataFillAlignment;
} VkVideoEncodeCapabilitiesKHR;

typedef struct VkVideoEncodeRateControlLayerInfoKHR {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           averageBitrate;
    uint32_t           maxBitrate;
    uint32_t           frameRateNumerator;
    uint32_t           frameRateDenominator;
    uint32_t           virtualBufferSizeInMs;
    uint32_t           initialVirtualBufferSizeInMs;
} VkVideoEncodeRateControlLayerInfoKHR;

typedef struct VkVideoEncodeRateControlInfoKHR {
    VkStructureType                                sType;
    const void*                                    pNext;
    VkVideoEncodeRateControlFlagsKHR               flags;
    VkVideoEncodeRateControlModeFlagBitsKHR        rateControlMode;
    uint8_t                                        layerCount;
    const VkVideoEncodeRateControlLayerInfoKHR*    pLayerConfigs;
} VkVideoEncodeRateControlInfoKHR;

typedef void (VKAPI_PTR *PFN_vkCmdEncodeVideoKHR)(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR void VKAPI_CALL vkCmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo);
#endif


#define VK_EXT_video_encode_h264 1
#include "vk_video/vulkan_video_codec_h264std.h"
#include "vk_video/vulkan_video_codec_h264std_encode.h"
#define VK_EXT_VIDEO_ENCODE_H264_SPEC_VERSION 7
#define VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME "VK_EXT_video_encode_h264"

typedef enum VkVideoEncodeH264CapabilityFlagBitsEXT {
    VK_VIDEO_ENCODE_H264_CAPABILITY_DIRECT_8X8_INFERENCE_ENABLED_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DIRECT_8X8_INFERENCE_DISABLED_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H264_CAPABILITY_SEPARATE_COLOUR_PLANE_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H264_CAPABILITY_QPPRIME_Y_ZERO_TRANSFORM_BYPASS_BIT_EXT = 0x00000008,
    VK_VIDEO_ENCODE_H264_CAPABILITY_SCALING_LISTS_BIT_EXT = 0x00000010,
    VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_EXT = 0x00000020,
    VK_VIDEO_ENCODE_H264_CAPABILITY_CHROMA_QP_OFFSET_BIT_EXT = 0x00000040,
    VK_VIDEO_ENCODE_H264_CAPABILITY_SECOND_CHROMA_QP_OFFSET_BIT_EXT = 0x00000080,
    VK_VIDEO_ENCODE_H264_CAPABILITY_PIC_INIT_QP_MINUS26_BIT_EXT = 0x00000100,
    VK_VIDEO_ENCODE_H264_CAPABILITY_WEIGHTED_PRED_BIT_EXT = 0x00000200,
    VK_VIDEO_ENCODE_H264_CAPABILITY_WEIGHTED_BIPRED_EXPLICIT_BIT_EXT = 0x00000400,
    VK_VIDEO_ENCODE_H264_CAPABILITY_WEIGHTED_BIPRED_IMPLICIT_BIT_EXT = 0x00000800,
    VK_VIDEO_ENCODE_H264_CAPABILITY_WEIGHTED_PRED_NO_TABLE_BIT_EXT = 0x00001000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_TRANSFORM_8X8_BIT_EXT = 0x00002000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_CABAC_BIT_EXT = 0x00004000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_CAVLC_BIT_EXT = 0x00008000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DEBLOCKING_FILTER_DISABLED_BIT_EXT = 0x00010000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DEBLOCKING_FILTER_ENABLED_BIT_EXT = 0x00020000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DEBLOCKING_FILTER_PARTIAL_BIT_EXT = 0x00040000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DISABLE_DIRECT_SPATIAL_MV_PRED_BIT_EXT = 0x00080000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_MULTIPLE_SLICE_PER_FRAME_BIT_EXT = 0x00100000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_SLICE_MB_COUNT_BIT_EXT = 0x00200000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_EXT = 0x00400000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_EXT = 0x00800000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_EXT = 0x01000000,
    VK_VIDEO_ENCODE_H264_CAPABILITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH264CapabilityFlagBitsEXT;
typedef VkFlags VkVideoEncodeH264CapabilityFlagsEXT;

typedef enum VkVideoEncodeH264InputModeFlagBitsEXT {
    VK_VIDEO_ENCODE_H264_INPUT_MODE_FRAME_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H264_INPUT_MODE_SLICE_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H264_INPUT_MODE_NON_VCL_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H264_INPUT_MODE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH264InputModeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH264InputModeFlagsEXT;

typedef enum VkVideoEncodeH264OutputModeFlagBitsEXT {
    VK_VIDEO_ENCODE_H264_OUTPUT_MODE_FRAME_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H264_OUTPUT_MODE_SLICE_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H264_OUTPUT_MODE_NON_VCL_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H264_OUTPUT_MODE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH264OutputModeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH264OutputModeFlagsEXT;

typedef enum VkVideoEncodeH264RateControlStructureFlagBitsEXT {
    VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_UNKNOWN_EXT = 0,
    VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_FLAT_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_DYADIC_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H264_RATE_CONTROL_STRUCTURE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH264RateControlStructureFlagBitsEXT;
typedef VkFlags VkVideoEncodeH264RateControlStructureFlagsEXT;
typedef struct VkVideoEncodeH264CapabilitiesEXT {
    VkStructureType                        sType;
    void*                                  pNext;
    VkVideoEncodeH264CapabilityFlagsEXT    flags;
    VkVideoEncodeH264InputModeFlagsEXT     inputModeFlags;
    VkVideoEncodeH264OutputModeFlagsEXT    outputModeFlags;
    uint8_t                                maxPPictureL0ReferenceCount;
    uint8_t                                maxBPictureL0ReferenceCount;
    uint8_t                                maxL1ReferenceCount;
    VkBool32                               motionVectorsOverPicBoundariesFlag;
    uint32_t                               maxBytesPerPicDenom;
    uint32_t                               maxBitsPerMbDenom;
    uint32_t                               log2MaxMvLengthHorizontal;
    uint32_t                               log2MaxMvLengthVertical;
} VkVideoEncodeH264CapabilitiesEXT;

typedef struct VkVideoEncodeH264SessionParametersAddInfoEXT {
    VkStructureType                            sType;
    const void*                                pNext;
    uint32_t                                   spsStdCount;
    const StdVideoH264SequenceParameterSet*    pSpsStd;
    uint32_t                                   ppsStdCount;
    const StdVideoH264PictureParameterSet*     pPpsStd;
} VkVideoEncodeH264SessionParametersAddInfoEXT;

typedef struct VkVideoEncodeH264SessionParametersCreateInfoEXT {
    VkStructureType                                        sType;
    const void*                                            pNext;
    uint32_t                                               maxSpsStdCount;
    uint32_t                                               maxPpsStdCount;
    const VkVideoEncodeH264SessionParametersAddInfoEXT*    pParametersAddInfo;
} VkVideoEncodeH264SessionParametersCreateInfoEXT;

typedef struct VkVideoEncodeH264DpbSlotInfoEXT {
    VkStructureType                           sType;
    const void*                               pNext;
    int8_t                                    slotIndex;
    const StdVideoEncodeH264ReferenceInfo*    pStdReferenceInfo;
} VkVideoEncodeH264DpbSlotInfoEXT;

typedef struct VkVideoEncodeH264ReferenceListsEXT {
    VkStructureType                                      sType;
    const void*                                          pNext;
    uint8_t                                              referenceList0EntryCount;
    const VkVideoEncodeH264DpbSlotInfoEXT*               pReferenceList0Entries;
    uint8_t                                              referenceList1EntryCount;
    const VkVideoEncodeH264DpbSlotInfoEXT*               pReferenceList1Entries;
    const StdVideoEncodeH264RefMemMgmtCtrlOperations*    pMemMgmtCtrlOperations;
} VkVideoEncodeH264ReferenceListsEXT;

typedef struct VkVideoEncodeH264NaluSliceEXT {
    VkStructureType                              sType;
    const void*                                  pNext;
    uint32_t                                     mbCount;
    const VkVideoEncodeH264ReferenceListsEXT*    pReferenceFinalLists;
    const StdVideoEncodeH264SliceHeader*         pSliceHeaderStd;
} VkVideoEncodeH264NaluSliceEXT;

typedef struct VkVideoEncodeH264VclFrameInfoEXT {
    VkStructureType                              sType;
    const void*                                  pNext;
    const VkVideoEncodeH264ReferenceListsEXT*    pReferenceFinalLists;
    uint32_t                                     naluSliceEntryCount;
    const VkVideoEncodeH264NaluSliceEXT*         pNaluSliceEntries;
    const StdVideoEncodeH264PictureInfo*         pCurrentPictureInfo;
} VkVideoEncodeH264VclFrameInfoEXT;

typedef struct VkVideoEncodeH264EmitPictureParametersEXT {
    VkStructureType    sType;
    const void*        pNext;
    uint8_t            spsId;
    VkBool32           emitSpsEnable;
    uint32_t           ppsIdEntryCount;
    const uint8_t*     ppsIdEntries;
} VkVideoEncodeH264EmitPictureParametersEXT;

typedef struct VkVideoEncodeH264ProfileEXT {
    VkStructureType           sType;
    const void*               pNext;
    StdVideoH264ProfileIdc    stdProfileIdc;
} VkVideoEncodeH264ProfileEXT;

typedef struct VkVideoEncodeH264RateControlInfoEXT {
    VkStructureType                                     sType;
    const void*                                         pNext;
    uint32_t                                            gopFrameCount;
    uint32_t                                            idrPeriod;
    uint32_t                                            consecutiveBFrameCount;
    VkVideoEncodeH264RateControlStructureFlagBitsEXT    rateControlStructure;
    uint8_t                                             temporalLayerCount;
} VkVideoEncodeH264RateControlInfoEXT;

typedef struct VkVideoEncodeH264QpEXT {
    int32_t    qpI;
    int32_t    qpP;
    int32_t    qpB;
} VkVideoEncodeH264QpEXT;

typedef struct VkVideoEncodeH264FrameSizeEXT {
    uint32_t    frameISize;
    uint32_t    framePSize;
    uint32_t    frameBSize;
} VkVideoEncodeH264FrameSizeEXT;

typedef struct VkVideoEncodeH264RateControlLayerInfoEXT {
    VkStructureType                  sType;
    const void*                      pNext;
    uint8_t                          temporalLayerId;
    VkBool32                         useInitialRcQp;
    VkVideoEncodeH264QpEXT           initialRcQp;
    VkBool32                         useMinQp;
    VkVideoEncodeH264QpEXT           minQp;
    VkBool32                         useMaxQp;
    VkVideoEncodeH264QpEXT           maxQp;
    VkBool32                         useMaxFrameSize;
    VkVideoEncodeH264FrameSizeEXT    maxFrameSize;
} VkVideoEncodeH264RateControlLayerInfoEXT;



#define VK_EXT_video_encode_h265 1
#include "vk_video/vulkan_video_codec_h265std.h"
#include "vk_video/vulkan_video_codec_h265std_encode.h"
#define VK_EXT_VIDEO_ENCODE_H265_SPEC_VERSION 7
#define VK_EXT_VIDEO_ENCODE_H265_EXTENSION_NAME "VK_EXT_video_encode_h265"

typedef enum VkVideoEncodeH265CapabilityFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_CAPABILITY_SEPARATE_COLOUR_PLANE_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_CAPABILITY_SCALING_LISTS_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_CAPABILITY_SAMPLE_ADAPTIVE_OFFSET_ENABLED_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H265_CAPABILITY_PCM_ENABLE_BIT_EXT = 0x00000008,
    VK_VIDEO_ENCODE_H265_CAPABILITY_SPS_TEMPORAL_MVP_ENABLED_BIT_EXT = 0x00000010,
    VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_EXT = 0x00000020,
    VK_VIDEO_ENCODE_H265_CAPABILITY_INIT_QP_MINUS26_BIT_EXT = 0x00000040,
    VK_VIDEO_ENCODE_H265_CAPABILITY_LOG2_PARALLEL_MERGE_LEVEL_MINUS2_BIT_EXT = 0x00000080,
    VK_VIDEO_ENCODE_H265_CAPABILITY_SIGN_DATA_HIDING_ENABLED_BIT_EXT = 0x00000100,
    VK_VIDEO_ENCODE_H265_CAPABILITY_TRANSFORM_SKIP_ENABLED_BIT_EXT = 0x00000200,
    VK_VIDEO_ENCODE_H265_CAPABILITY_TRANSFORM_SKIP_DISABLED_BIT_EXT = 0x00000400,
    VK_VIDEO_ENCODE_H265_CAPABILITY_PPS_SLICE_CHROMA_QP_OFFSETS_PRESENT_BIT_EXT = 0x00000800,
    VK_VIDEO_ENCODE_H265_CAPABILITY_WEIGHTED_PRED_BIT_EXT = 0x00001000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_WEIGHTED_BIPRED_BIT_EXT = 0x00002000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_WEIGHTED_PRED_NO_TABLE_BIT_EXT = 0x00004000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_TRANSQUANT_BYPASS_ENABLED_BIT_EXT = 0x00008000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_ENTROPY_CODING_SYNC_ENABLED_BIT_EXT = 0x00010000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_DEBLOCKING_FILTER_OVERRIDE_ENABLED_BIT_EXT = 0x00020000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILE_PER_FRAME_BIT_EXT = 0x00040000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_PER_TILE_BIT_EXT = 0x00080000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILE_PER_SLICE_BIT_EXT = 0x00100000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_SLICE_SEGMENT_CTB_COUNT_BIT_EXT = 0x00200000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_EXT = 0x00400000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_DEPENDENT_SLICE_SEGMENT_BIT_EXT = 0x00800000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_EXT = 0x01000000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_EXT = 0x02000000,
    VK_VIDEO_ENCODE_H265_CAPABILITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265CapabilityFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265CapabilityFlagsEXT;

typedef enum VkVideoEncodeH265InputModeFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_INPUT_MODE_FRAME_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_INPUT_MODE_SLICE_SEGMENT_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_INPUT_MODE_NON_VCL_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H265_INPUT_MODE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265InputModeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265InputModeFlagsEXT;

typedef enum VkVideoEncodeH265OutputModeFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_OUTPUT_MODE_FRAME_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_OUTPUT_MODE_SLICE_SEGMENT_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_OUTPUT_MODE_NON_VCL_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H265_OUTPUT_MODE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265OutputModeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265OutputModeFlagsEXT;

typedef enum VkVideoEncodeH265CtbSizeFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_CTB_SIZE_16_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_CTB_SIZE_32_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_CTB_SIZE_64_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H265_CTB_SIZE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265CtbSizeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265CtbSizeFlagsEXT;

typedef enum VkVideoEncodeH265TransformBlockSizeFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_4_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_8_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_16_BIT_EXT = 0x00000004,
    VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_32_BIT_EXT = 0x00000008,
    VK_VIDEO_ENCODE_H265_TRANSFORM_BLOCK_SIZE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265TransformBlockSizeFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265TransformBlockSizeFlagsEXT;

typedef enum VkVideoEncodeH265RateControlStructureFlagBitsEXT {
    VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_UNKNOWN_EXT = 0,
    VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_FLAT_BIT_EXT = 0x00000001,
    VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_DYADIC_BIT_EXT = 0x00000002,
    VK_VIDEO_ENCODE_H265_RATE_CONTROL_STRUCTURE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoEncodeH265RateControlStructureFlagBitsEXT;
typedef VkFlags VkVideoEncodeH265RateControlStructureFlagsEXT;
typedef struct VkVideoEncodeH265CapabilitiesEXT {
    VkStructureType                                sType;
    void*                                          pNext;
    VkVideoEncodeH265CapabilityFlagsEXT            flags;
    VkVideoEncodeH265InputModeFlagsEXT             inputModeFlags;
    VkVideoEncodeH265OutputModeFlagsEXT            outputModeFlags;
    VkVideoEncodeH265CtbSizeFlagsEXT               ctbSizes;
    VkVideoEncodeH265TransformBlockSizeFlagsEXT    transformBlockSizes;
    uint8_t                                        maxPPictureL0ReferenceCount;
    uint8_t                                        maxBPictureL0ReferenceCount;
    uint8_t                                        maxL1ReferenceCount;
    uint8_t                                        maxSubLayersCount;
    uint8_t                                        minLog2MinLumaCodingBlockSizeMinus3;
    uint8_t                                        maxLog2MinLumaCodingBlockSizeMinus3;
    uint8_t                                        minLog2MinLumaTransformBlockSizeMinus2;
    uint8_t                                        maxLog2MinLumaTransformBlockSizeMinus2;
    uint8_t                                        minMaxTransformHierarchyDepthInter;
    uint8_t                                        maxMaxTransformHierarchyDepthInter;
    uint8_t                                        minMaxTransformHierarchyDepthIntra;
    uint8_t                                        maxMaxTransformHierarchyDepthIntra;
    uint8_t                                        maxDiffCuQpDeltaDepth;
    uint8_t                                        minMaxNumMergeCand;
    uint8_t                                        maxMaxNumMergeCand;
} VkVideoEncodeH265CapabilitiesEXT;

typedef struct VkVideoEncodeH265SessionParametersAddInfoEXT {
    VkStructureType                            sType;
    const void*                                pNext;
    uint32_t                                   vpsStdCount;
    const StdVideoH265VideoParameterSet*       pVpsStd;
    uint32_t                                   spsStdCount;
    const StdVideoH265SequenceParameterSet*    pSpsStd;
    uint32_t                                   ppsStdCount;
    const StdVideoH265PictureParameterSet*     pPpsStd;
} VkVideoEncodeH265SessionParametersAddInfoEXT;

typedef struct VkVideoEncodeH265SessionParametersCreateInfoEXT {
    VkStructureType                                        sType;
    const void*                                            pNext;
    uint32_t                                               maxVpsStdCount;
    uint32_t                                               maxSpsStdCount;
    uint32_t                                               maxPpsStdCount;
    const VkVideoEncodeH265SessionParametersAddInfoEXT*    pParametersAddInfo;
} VkVideoEncodeH265SessionParametersCreateInfoEXT;

typedef struct VkVideoEncodeH265DpbSlotInfoEXT {
    VkStructureType                           sType;
    const void*                               pNext;
    int8_t                                    slotIndex;
    const StdVideoEncodeH265ReferenceInfo*    pStdReferenceInfo;
} VkVideoEncodeH265DpbSlotInfoEXT;

typedef struct VkVideoEncodeH265ReferenceListsEXT {
    VkStructureType                                    sType;
    const void*                                        pNext;
    uint8_t                                            referenceList0EntryCount;
    const VkVideoEncodeH265DpbSlotInfoEXT*             pReferenceList0Entries;
    uint8_t                                            referenceList1EntryCount;
    const VkVideoEncodeH265DpbSlotInfoEXT*             pReferenceList1Entries;
    const StdVideoEncodeH265ReferenceModifications*    pReferenceModifications;
} VkVideoEncodeH265ReferenceListsEXT;

typedef struct VkVideoEncodeH265NaluSliceSegmentEXT {
    VkStructureType                                sType;
    const void*                                    pNext;
    uint32_t                                       ctbCount;
    const VkVideoEncodeH265ReferenceListsEXT*      pReferenceFinalLists;
    const StdVideoEncodeH265SliceSegmentHeader*    pSliceSegmentHeaderStd;
} VkVideoEncodeH265NaluSliceSegmentEXT;

typedef struct VkVideoEncodeH265VclFrameInfoEXT {
    VkStructureType                                sType;
    const void*                                    pNext;
    const VkVideoEncodeH265ReferenceListsEXT*      pReferenceFinalLists;
    uint32_t                                       naluSliceSegmentEntryCount;
    const VkVideoEncodeH265NaluSliceSegmentEXT*    pNaluSliceSegmentEntries;
    const StdVideoEncodeH265PictureInfo*           pCurrentPictureInfo;
} VkVideoEncodeH265VclFrameInfoEXT;

typedef struct VkVideoEncodeH265EmitPictureParametersEXT {
    VkStructureType    sType;
    const void*        pNext;
    uint8_t            vpsId;
    uint8_t            spsId;
    VkBool32           emitVpsEnable;
    VkBool32           emitSpsEnable;
    uint32_t           ppsIdEntryCount;
    const uint8_t*     ppsIdEntries;
} VkVideoEncodeH265EmitPictureParametersEXT;

typedef struct VkVideoEncodeH265ProfileEXT {
    VkStructureType           sType;
    const void*               pNext;
    StdVideoH265ProfileIdc    stdProfileIdc;
} VkVideoEncodeH265ProfileEXT;

typedef struct VkVideoEncodeH265RateControlInfoEXT {
    VkStructureType                                     sType;
    const void*                                         pNext;
    uint32_t                                            gopFrameCount;
    uint32_t                                            idrPeriod;
    uint32_t                                            consecutiveBFrameCount;
    VkVideoEncodeH265RateControlStructureFlagBitsEXT    rateControlStructure;
    uint8_t                                             subLayerCount;
} VkVideoEncodeH265RateControlInfoEXT;

typedef struct VkVideoEncodeH265QpEXT {
    int32_t    qpI;
    int32_t    qpP;
    int32_t    qpB;
} VkVideoEncodeH265QpEXT;

typedef struct VkVideoEncodeH265FrameSizeEXT {
    uint32_t    frameISize;
    uint32_t    framePSize;
    uint32_t    frameBSize;
} VkVideoEncodeH265FrameSizeEXT;

typedef struct VkVideoEncodeH265RateControlLayerInfoEXT {
    VkStructureType                  sType;
    const void*                      pNext;
    uint8_t                          temporalId;
    VkBool32                         useInitialRcQp;
    VkVideoEncodeH265QpEXT           initialRcQp;
    VkBool32                         useMinQp;
    VkVideoEncodeH265QpEXT           minQp;
    VkBool32                         useMaxQp;
    VkVideoEncodeH265QpEXT           maxQp;
    VkBool32                         useMaxFrameSize;
    VkVideoEncodeH265FrameSizeEXT    maxFrameSize;
} VkVideoEncodeH265RateControlLayerInfoEXT;



#define VK_EXT_video_decode_h264 1
#include "vk_video/vulkan_video_codec_h264std_decode.h"
#define VK_EXT_VIDEO_DECODE_H264_SPEC_VERSION 5
#define VK_EXT_VIDEO_DECODE_H264_EXTENSION_NAME "VK_EXT_video_decode_h264"

typedef enum VkVideoDecodeH264PictureLayoutFlagBitsEXT {
    VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_EXT = 0,
    VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_EXT = 0x00000001,
    VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_EXT = 0x00000002,
    VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkVideoDecodeH264PictureLayoutFlagBitsEXT;
typedef VkFlags VkVideoDecodeH264PictureLayoutFlagsEXT;
typedef struct VkVideoDecodeH264ProfileEXT {
    VkStructureType                           sType;
    const void*                               pNext;
    StdVideoH264ProfileIdc                    stdProfileIdc;
    VkVideoDecodeH264PictureLayoutFlagsEXT    pictureLayout;
} VkVideoDecodeH264ProfileEXT;

typedef struct VkVideoDecodeH264CapabilitiesEXT {
    VkStructureType      sType;
    void*                pNext;
    StdVideoH264Level    maxLevel;
    VkOffset2D           fieldOffsetGranularity;
} VkVideoDecodeH264CapabilitiesEXT;

typedef struct VkVideoDecodeH264SessionParametersAddInfoEXT {
    VkStructureType                            sType;
    const void*                                pNext;
    uint32_t                                   spsStdCount;
    const StdVideoH264SequenceParameterSet*    pSpsStd;
    uint32_t                                   ppsStdCount;
    const StdVideoH264PictureParameterSet*     pPpsStd;
} VkVideoDecodeH264SessionParametersAddInfoEXT;

typedef struct VkVideoDecodeH264SessionParametersCreateInfoEXT {
    VkStructureType                                        sType;
    const void*                                            pNext;
    uint32_t                                               maxSpsStdCount;
    uint32_t                                               maxPpsStdCount;
    const VkVideoDecodeH264SessionParametersAddInfoEXT*    pParametersAddInfo;
} VkVideoDecodeH264SessionParametersCreateInfoEXT;

typedef struct VkVideoDecodeH264PictureInfoEXT {
    VkStructureType                         sType;
    const void*                             pNext;
    const StdVideoDecodeH264PictureInfo*    pStdPictureInfo;
    uint32_t                                slicesCount;
    const uint32_t*                         pSlicesDataOffsets;
} VkVideoDecodeH264PictureInfoEXT;

typedef struct VkVideoDecodeH264MvcEXT {
    VkStructureType                 sType;
    const void*                     pNext;
    const StdVideoDecodeH264Mvc*    pStdMvc;
} VkVideoDecodeH264MvcEXT;

typedef struct VkVideoDecodeH264DpbSlotInfoEXT {
    VkStructureType                           sType;
    const void*                               pNext;
    const StdVideoDecodeH264ReferenceInfo*    pStdReferenceInfo;
} VkVideoDecodeH264DpbSlotInfoEXT;



#define VK_EXT_video_decode_h265 1
#include "vk_video/vulkan_video_codec_h265std_decode.h"
#define VK_EXT_VIDEO_DECODE_H265_SPEC_VERSION 3
#define VK_EXT_VIDEO_DECODE_H265_EXTENSION_NAME "VK_EXT_video_decode_h265"
typedef struct VkVideoDecodeH265ProfileEXT {
    VkStructureType           sType;
    const void*               pNext;
    StdVideoH265ProfileIdc    stdProfileIdc;
} VkVideoDecodeH265ProfileEXT;

typedef struct VkVideoDecodeH265CapabilitiesEXT {
    VkStructureType      sType;
    void*                pNext;
    StdVideoH265Level    maxLevel;
} VkVideoDecodeH265CapabilitiesEXT;

typedef struct VkVideoDecodeH265SessionParametersAddInfoEXT {
    VkStructureType                            sType;
    const void*                                pNext;
    uint32_t                                   vpsStdCount;
    const StdVideoH265VideoParameterSet*       pVpsStd;
    uint32_t                                   spsStdCount;
    const StdVideoH265SequenceParameterSet*    pSpsStd;
    uint32_t                                   ppsStdCount;
    const StdVideoH265PictureParameterSet*     pPpsStd;
} VkVideoDecodeH265SessionParametersAddInfoEXT;

typedef struct VkVideoDecodeH265SessionParametersCreateInfoEXT {
    VkStructureType                                        sType;
    const void*                                            pNext;
    uint32_t                                               maxVpsStdCount;
    uint32_t                                               maxSpsStdCount;
    uint32_t                                               maxPpsStdCount;
    const VkVideoDecodeH265SessionParametersAddInfoEXT*    pParametersAddInfo;
} VkVideoDecodeH265SessionParametersCreateInfoEXT;

typedef struct VkVideoDecodeH265PictureInfoEXT {
    VkStructureType                   sType;
    const void*                       pNext;
    StdVideoDecodeH265PictureInfo*    pStdPictureInfo;
    uint32_t                          slicesCount;
    const uint32_t*                   pSlicesDataOffsets;
} VkVideoDecodeH265PictureInfoEXT;

typedef struct VkVideoDecodeH265DpbSlotInfoEXT {
    VkStructureType                           sType;
    const void*                               pNext;
    const StdVideoDecodeH265ReferenceInfo*    pStdReferenceInfo;
} VkVideoDecodeH265DpbSlotInfoEXT;


#ifdef __cplusplus
}
#endif

#endif
