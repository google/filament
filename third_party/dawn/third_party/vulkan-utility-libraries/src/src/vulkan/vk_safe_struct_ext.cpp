// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See safe_struct_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

// NOLINTBEGIN

#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>

#include <cstddef>
#include <cstring>

namespace vku {

safe_VkAttachmentFeedbackLoopInfoEXT::safe_VkAttachmentFeedbackLoopInfoEXT(const VkAttachmentFeedbackLoopInfoEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), feedbackLoopEnable(in_struct->feedbackLoopEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAttachmentFeedbackLoopInfoEXT::safe_VkAttachmentFeedbackLoopInfoEXT()
    : sType(VK_STRUCTURE_TYPE_ATTACHMENT_FEEDBACK_LOOP_INFO_EXT), pNext(nullptr), feedbackLoopEnable() {}

safe_VkAttachmentFeedbackLoopInfoEXT::safe_VkAttachmentFeedbackLoopInfoEXT(const safe_VkAttachmentFeedbackLoopInfoEXT& copy_src) {
    sType = copy_src.sType;
    feedbackLoopEnable = copy_src.feedbackLoopEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAttachmentFeedbackLoopInfoEXT& safe_VkAttachmentFeedbackLoopInfoEXT::operator=(
    const safe_VkAttachmentFeedbackLoopInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    feedbackLoopEnable = copy_src.feedbackLoopEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAttachmentFeedbackLoopInfoEXT::~safe_VkAttachmentFeedbackLoopInfoEXT() { FreePnextChain(pNext); }

void safe_VkAttachmentFeedbackLoopInfoEXT::initialize(const VkAttachmentFeedbackLoopInfoEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    feedbackLoopEnable = in_struct->feedbackLoopEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAttachmentFeedbackLoopInfoEXT::initialize(const safe_VkAttachmentFeedbackLoopInfoEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    feedbackLoopEnable = copy_src->feedbackLoopEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSetDescriptorBufferOffsetsInfoEXT::safe_VkSetDescriptorBufferOffsetsInfoEXT(
    const VkSetDescriptorBufferOffsetsInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stageFlags(in_struct->stageFlags),
      layout(in_struct->layout),
      firstSet(in_struct->firstSet),
      setCount(in_struct->setCount),
      pBufferIndices(nullptr),
      pOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pBufferIndices) {
        pBufferIndices = new uint32_t[in_struct->setCount];
        memcpy((void*)pBufferIndices, (void*)in_struct->pBufferIndices, sizeof(uint32_t) * in_struct->setCount);
    }

    if (in_struct->pOffsets) {
        pOffsets = new VkDeviceSize[in_struct->setCount];
        memcpy((void*)pOffsets, (void*)in_struct->pOffsets, sizeof(VkDeviceSize) * in_struct->setCount);
    }
}

safe_VkSetDescriptorBufferOffsetsInfoEXT::safe_VkSetDescriptorBufferOffsetsInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SET_DESCRIPTOR_BUFFER_OFFSETS_INFO_EXT),
      pNext(nullptr),
      stageFlags(),
      layout(),
      firstSet(),
      setCount(),
      pBufferIndices(nullptr),
      pOffsets(nullptr) {}

safe_VkSetDescriptorBufferOffsetsInfoEXT::safe_VkSetDescriptorBufferOffsetsInfoEXT(
    const safe_VkSetDescriptorBufferOffsetsInfoEXT& copy_src) {
    sType = copy_src.sType;
    stageFlags = copy_src.stageFlags;
    layout = copy_src.layout;
    firstSet = copy_src.firstSet;
    setCount = copy_src.setCount;
    pBufferIndices = nullptr;
    pOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pBufferIndices) {
        pBufferIndices = new uint32_t[copy_src.setCount];
        memcpy((void*)pBufferIndices, (void*)copy_src.pBufferIndices, sizeof(uint32_t) * copy_src.setCount);
    }

    if (copy_src.pOffsets) {
        pOffsets = new VkDeviceSize[copy_src.setCount];
        memcpy((void*)pOffsets, (void*)copy_src.pOffsets, sizeof(VkDeviceSize) * copy_src.setCount);
    }
}

safe_VkSetDescriptorBufferOffsetsInfoEXT& safe_VkSetDescriptorBufferOffsetsInfoEXT::operator=(
    const safe_VkSetDescriptorBufferOffsetsInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pBufferIndices) delete[] pBufferIndices;
    if (pOffsets) delete[] pOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stageFlags = copy_src.stageFlags;
    layout = copy_src.layout;
    firstSet = copy_src.firstSet;
    setCount = copy_src.setCount;
    pBufferIndices = nullptr;
    pOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pBufferIndices) {
        pBufferIndices = new uint32_t[copy_src.setCount];
        memcpy((void*)pBufferIndices, (void*)copy_src.pBufferIndices, sizeof(uint32_t) * copy_src.setCount);
    }

    if (copy_src.pOffsets) {
        pOffsets = new VkDeviceSize[copy_src.setCount];
        memcpy((void*)pOffsets, (void*)copy_src.pOffsets, sizeof(VkDeviceSize) * copy_src.setCount);
    }

    return *this;
}

safe_VkSetDescriptorBufferOffsetsInfoEXT::~safe_VkSetDescriptorBufferOffsetsInfoEXT() {
    if (pBufferIndices) delete[] pBufferIndices;
    if (pOffsets) delete[] pOffsets;
    FreePnextChain(pNext);
}

void safe_VkSetDescriptorBufferOffsetsInfoEXT::initialize(const VkSetDescriptorBufferOffsetsInfoEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pBufferIndices) delete[] pBufferIndices;
    if (pOffsets) delete[] pOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stageFlags = in_struct->stageFlags;
    layout = in_struct->layout;
    firstSet = in_struct->firstSet;
    setCount = in_struct->setCount;
    pBufferIndices = nullptr;
    pOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pBufferIndices) {
        pBufferIndices = new uint32_t[in_struct->setCount];
        memcpy((void*)pBufferIndices, (void*)in_struct->pBufferIndices, sizeof(uint32_t) * in_struct->setCount);
    }

    if (in_struct->pOffsets) {
        pOffsets = new VkDeviceSize[in_struct->setCount];
        memcpy((void*)pOffsets, (void*)in_struct->pOffsets, sizeof(VkDeviceSize) * in_struct->setCount);
    }
}

void safe_VkSetDescriptorBufferOffsetsInfoEXT::initialize(const safe_VkSetDescriptorBufferOffsetsInfoEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stageFlags = copy_src->stageFlags;
    layout = copy_src->layout;
    firstSet = copy_src->firstSet;
    setCount = copy_src->setCount;
    pBufferIndices = nullptr;
    pOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pBufferIndices) {
        pBufferIndices = new uint32_t[copy_src->setCount];
        memcpy((void*)pBufferIndices, (void*)copy_src->pBufferIndices, sizeof(uint32_t) * copy_src->setCount);
    }

    if (copy_src->pOffsets) {
        pOffsets = new VkDeviceSize[copy_src->setCount];
        memcpy((void*)pOffsets, (void*)copy_src->pOffsets, sizeof(VkDeviceSize) * copy_src->setCount);
    }
}

safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT(
    const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), stageFlags(in_struct->stageFlags), layout(in_struct->layout), set(in_struct->set) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT()
    : sType(VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_BUFFER_EMBEDDED_SAMPLERS_INFO_EXT), pNext(nullptr), stageFlags(), layout(), set() {}

safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT(
    const safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT& copy_src) {
    sType = copy_src.sType;
    stageFlags = copy_src.stageFlags;
    layout = copy_src.layout;
    set = copy_src.set;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT& safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::operator=(
    const safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stageFlags = copy_src.stageFlags;
    layout = copy_src.layout;
    set = copy_src.set;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::~safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT() { FreePnextChain(pNext); }

void safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::initialize(const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stageFlags = in_struct->stageFlags;
    layout = in_struct->layout;
    set = in_struct->set;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT::initialize(
    const safe_VkBindDescriptorBufferEmbeddedSamplersInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stageFlags = copy_src->stageFlags;
    layout = copy_src->layout;
    set = copy_src->set;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT(
    const VkDebugReportCallbackCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), pfnCallback(in_struct->pfnCallback), pUserData(in_struct->pUserData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT), pNext(nullptr), flags(), pfnCallback(), pUserData(nullptr) {}

safe_VkDebugReportCallbackCreateInfoEXT::safe_VkDebugReportCallbackCreateInfoEXT(
    const safe_VkDebugReportCallbackCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnCallback = copy_src.pfnCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDebugReportCallbackCreateInfoEXT& safe_VkDebugReportCallbackCreateInfoEXT::operator=(
    const safe_VkDebugReportCallbackCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnCallback = copy_src.pfnCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDebugReportCallbackCreateInfoEXT::~safe_VkDebugReportCallbackCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkDebugReportCallbackCreateInfoEXT::initialize(const VkDebugReportCallbackCreateInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pfnCallback = in_struct->pfnCallback;
    pUserData = in_struct->pUserData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDebugReportCallbackCreateInfoEXT::initialize(const safe_VkDebugReportCallbackCreateInfoEXT* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pfnCallback = copy_src->pfnCallback;
    pUserData = copy_src->pUserData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT(const VkDebugMarkerObjectNameInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), objectType(in_struct->objectType), object(in_struct->object) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pObjectName = SafeStringCopy(in_struct->pObjectName);
}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT), pNext(nullptr), objectType(), object(), pObjectName(nullptr) {}

safe_VkDebugMarkerObjectNameInfoEXT::safe_VkDebugMarkerObjectNameInfoEXT(const safe_VkDebugMarkerObjectNameInfoEXT& copy_src) {
    sType = copy_src.sType;
    objectType = copy_src.objectType;
    object = copy_src.object;
    pNext = SafePnextCopy(copy_src.pNext);
    pObjectName = SafeStringCopy(copy_src.pObjectName);
}

safe_VkDebugMarkerObjectNameInfoEXT& safe_VkDebugMarkerObjectNameInfoEXT::operator=(
    const safe_VkDebugMarkerObjectNameInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    objectType = copy_src.objectType;
    object = copy_src.object;
    pNext = SafePnextCopy(copy_src.pNext);
    pObjectName = SafeStringCopy(copy_src.pObjectName);

    return *this;
}

safe_VkDebugMarkerObjectNameInfoEXT::~safe_VkDebugMarkerObjectNameInfoEXT() {
    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);
}

void safe_VkDebugMarkerObjectNameInfoEXT::initialize(const VkDebugMarkerObjectNameInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    objectType = in_struct->objectType;
    object = in_struct->object;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pObjectName = SafeStringCopy(in_struct->pObjectName);
}

void safe_VkDebugMarkerObjectNameInfoEXT::initialize(const safe_VkDebugMarkerObjectNameInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    objectType = copy_src->objectType;
    object = copy_src->object;
    pNext = SafePnextCopy(copy_src->pNext);
    pObjectName = SafeStringCopy(copy_src->pObjectName);
}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT(const VkDebugMarkerObjectTagInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      objectType(in_struct->objectType),
      object(in_struct->object),
      tagName(in_struct->tagName),
      tagSize(in_struct->tagSize),
      pTag(in_struct->pTag) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT),
      pNext(nullptr),
      objectType(),
      object(),
      tagName(),
      tagSize(),
      pTag(nullptr) {}

safe_VkDebugMarkerObjectTagInfoEXT::safe_VkDebugMarkerObjectTagInfoEXT(const safe_VkDebugMarkerObjectTagInfoEXT& copy_src) {
    sType = copy_src.sType;
    objectType = copy_src.objectType;
    object = copy_src.object;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDebugMarkerObjectTagInfoEXT& safe_VkDebugMarkerObjectTagInfoEXT::operator=(
    const safe_VkDebugMarkerObjectTagInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    objectType = copy_src.objectType;
    object = copy_src.object;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDebugMarkerObjectTagInfoEXT::~safe_VkDebugMarkerObjectTagInfoEXT() { FreePnextChain(pNext); }

void safe_VkDebugMarkerObjectTagInfoEXT::initialize(const VkDebugMarkerObjectTagInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    objectType = in_struct->objectType;
    object = in_struct->object;
    tagName = in_struct->tagName;
    tagSize = in_struct->tagSize;
    pTag = in_struct->pTag;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDebugMarkerObjectTagInfoEXT::initialize(const safe_VkDebugMarkerObjectTagInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    objectType = copy_src->objectType;
    object = copy_src->object;
    tagName = copy_src->tagName;
    tagSize = copy_src->tagSize;
    pTag = copy_src->pTag;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT(const VkDebugMarkerMarkerInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pMarkerName = SafeStringCopy(in_struct->pMarkerName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = in_struct->color[i];
    }
}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT), pNext(nullptr), pMarkerName(nullptr) {}

safe_VkDebugMarkerMarkerInfoEXT::safe_VkDebugMarkerMarkerInfoEXT(const safe_VkDebugMarkerMarkerInfoEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    pMarkerName = SafeStringCopy(copy_src.pMarkerName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src.color[i];
    }
}

safe_VkDebugMarkerMarkerInfoEXT& safe_VkDebugMarkerMarkerInfoEXT::operator=(const safe_VkDebugMarkerMarkerInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pMarkerName) delete[] pMarkerName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    pMarkerName = SafeStringCopy(copy_src.pMarkerName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src.color[i];
    }

    return *this;
}

safe_VkDebugMarkerMarkerInfoEXT::~safe_VkDebugMarkerMarkerInfoEXT() {
    if (pMarkerName) delete[] pMarkerName;
    FreePnextChain(pNext);
}

void safe_VkDebugMarkerMarkerInfoEXT::initialize(const VkDebugMarkerMarkerInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    if (pMarkerName) delete[] pMarkerName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pMarkerName = SafeStringCopy(in_struct->pMarkerName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = in_struct->color[i];
    }
}

void safe_VkDebugMarkerMarkerInfoEXT::initialize(const safe_VkDebugMarkerMarkerInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
    pMarkerName = SafeStringCopy(copy_src->pMarkerName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src->color[i];
    }
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(
    const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), transformFeedback(in_struct->transformFeedback), geometryStreams(in_struct->geometryStreams) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT),
      pNext(nullptr),
      transformFeedback(),
      geometryStreams() {}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(
    const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    transformFeedback = copy_src.transformFeedback;
    geometryStreams = copy_src.geometryStreams;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    transformFeedback = copy_src.transformFeedback;
    geometryStreams = copy_src.geometryStreams;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::~safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::initialize(const VkPhysicalDeviceTransformFeedbackFeaturesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    transformFeedback = in_struct->transformFeedback;
    geometryStreams = in_struct->geometryStreams;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    transformFeedback = copy_src->transformFeedback;
    geometryStreams = copy_src->geometryStreams;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(
    const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxTransformFeedbackStreams(in_struct->maxTransformFeedbackStreams),
      maxTransformFeedbackBuffers(in_struct->maxTransformFeedbackBuffers),
      maxTransformFeedbackBufferSize(in_struct->maxTransformFeedbackBufferSize),
      maxTransformFeedbackStreamDataSize(in_struct->maxTransformFeedbackStreamDataSize),
      maxTransformFeedbackBufferDataSize(in_struct->maxTransformFeedbackBufferDataSize),
      maxTransformFeedbackBufferDataStride(in_struct->maxTransformFeedbackBufferDataStride),
      transformFeedbackQueries(in_struct->transformFeedbackQueries),
      transformFeedbackStreamsLinesTriangles(in_struct->transformFeedbackStreamsLinesTriangles),
      transformFeedbackRasterizationStreamSelect(in_struct->transformFeedbackRasterizationStreamSelect),
      transformFeedbackDraw(in_struct->transformFeedbackDraw) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT),
      pNext(nullptr),
      maxTransformFeedbackStreams(),
      maxTransformFeedbackBuffers(),
      maxTransformFeedbackBufferSize(),
      maxTransformFeedbackStreamDataSize(),
      maxTransformFeedbackBufferDataSize(),
      maxTransformFeedbackBufferDataStride(),
      transformFeedbackQueries(),
      transformFeedbackStreamsLinesTriangles(),
      transformFeedbackRasterizationStreamSelect(),
      transformFeedbackDraw() {}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(
    const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxTransformFeedbackStreams = copy_src.maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = copy_src.maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = copy_src.maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = copy_src.maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = copy_src.maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = copy_src.maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = copy_src.transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = copy_src.transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = copy_src.transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = copy_src.transformFeedbackDraw;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxTransformFeedbackStreams = copy_src.maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = copy_src.maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = copy_src.maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = copy_src.maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = copy_src.maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = copy_src.maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = copy_src.transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = copy_src.transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = copy_src.transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = copy_src.transformFeedbackDraw;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::~safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::initialize(
    const VkPhysicalDeviceTransformFeedbackPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxTransformFeedbackStreams = in_struct->maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = in_struct->maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = in_struct->maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = in_struct->maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = in_struct->maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = in_struct->maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = in_struct->transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = in_struct->transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = in_struct->transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = in_struct->transformFeedbackDraw;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxTransformFeedbackStreams = copy_src->maxTransformFeedbackStreams;
    maxTransformFeedbackBuffers = copy_src->maxTransformFeedbackBuffers;
    maxTransformFeedbackBufferSize = copy_src->maxTransformFeedbackBufferSize;
    maxTransformFeedbackStreamDataSize = copy_src->maxTransformFeedbackStreamDataSize;
    maxTransformFeedbackBufferDataSize = copy_src->maxTransformFeedbackBufferDataSize;
    maxTransformFeedbackBufferDataStride = copy_src->maxTransformFeedbackBufferDataStride;
    transformFeedbackQueries = copy_src->transformFeedbackQueries;
    transformFeedbackStreamsLinesTriangles = copy_src->transformFeedbackStreamsLinesTriangles;
    transformFeedbackRasterizationStreamSelect = copy_src->transformFeedbackRasterizationStreamSelect;
    transformFeedbackDraw = copy_src->transformFeedbackDraw;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT(
    const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), rasterizationStream(in_struct->rasterizationStream) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      rasterizationStream() {}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::safe_VkPipelineRasterizationStateStreamCreateInfoEXT(
    const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    rasterizationStream = copy_src.rasterizationStream;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT& safe_VkPipelineRasterizationStateStreamCreateInfoEXT::operator=(
    const safe_VkPipelineRasterizationStateStreamCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    rasterizationStream = copy_src.rasterizationStream;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRasterizationStateStreamCreateInfoEXT::~safe_VkPipelineRasterizationStateStreamCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRasterizationStateStreamCreateInfoEXT::initialize(
    const VkPipelineRasterizationStateStreamCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    rasterizationStream = in_struct->rasterizationStream;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRasterizationStateStreamCreateInfoEXT::initialize(
    const safe_VkPipelineRasterizationStateStreamCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    rasterizationStream = copy_src->rasterizationStream;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT(const VkValidationFlagsEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      disabledValidationCheckCount(in_struct->disabledValidationCheckCount),
      pDisabledValidationChecks(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[in_struct->disabledValidationCheckCount];
        memcpy((void*)pDisabledValidationChecks, (void*)in_struct->pDisabledValidationChecks,
               sizeof(VkValidationCheckEXT) * in_struct->disabledValidationCheckCount);
    }
}

safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT()
    : sType(VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT),
      pNext(nullptr),
      disabledValidationCheckCount(),
      pDisabledValidationChecks(nullptr) {}

safe_VkValidationFlagsEXT::safe_VkValidationFlagsEXT(const safe_VkValidationFlagsEXT& copy_src) {
    sType = copy_src.sType;
    disabledValidationCheckCount = copy_src.disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[copy_src.disabledValidationCheckCount];
        memcpy((void*)pDisabledValidationChecks, (void*)copy_src.pDisabledValidationChecks,
               sizeof(VkValidationCheckEXT) * copy_src.disabledValidationCheckCount);
    }
}

safe_VkValidationFlagsEXT& safe_VkValidationFlagsEXT::operator=(const safe_VkValidationFlagsEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDisabledValidationChecks) delete[] pDisabledValidationChecks;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    disabledValidationCheckCount = copy_src.disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[copy_src.disabledValidationCheckCount];
        memcpy((void*)pDisabledValidationChecks, (void*)copy_src.pDisabledValidationChecks,
               sizeof(VkValidationCheckEXT) * copy_src.disabledValidationCheckCount);
    }

    return *this;
}

safe_VkValidationFlagsEXT::~safe_VkValidationFlagsEXT() {
    if (pDisabledValidationChecks) delete[] pDisabledValidationChecks;
    FreePnextChain(pNext);
}

void safe_VkValidationFlagsEXT::initialize(const VkValidationFlagsEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDisabledValidationChecks) delete[] pDisabledValidationChecks;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    disabledValidationCheckCount = in_struct->disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[in_struct->disabledValidationCheckCount];
        memcpy((void*)pDisabledValidationChecks, (void*)in_struct->pDisabledValidationChecks,
               sizeof(VkValidationCheckEXT) * in_struct->disabledValidationCheckCount);
    }
}

void safe_VkValidationFlagsEXT::initialize(const safe_VkValidationFlagsEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    disabledValidationCheckCount = copy_src->disabledValidationCheckCount;
    pDisabledValidationChecks = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDisabledValidationChecks) {
        pDisabledValidationChecks = new VkValidationCheckEXT[copy_src->disabledValidationCheckCount];
        memcpy((void*)pDisabledValidationChecks, (void*)copy_src->pDisabledValidationChecks,
               sizeof(VkValidationCheckEXT) * copy_src->disabledValidationCheckCount);
    }
}

safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT(const VkImageViewASTCDecodeModeEXT* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), decodeMode(in_struct->decodeMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT), pNext(nullptr), decodeMode() {}

safe_VkImageViewASTCDecodeModeEXT::safe_VkImageViewASTCDecodeModeEXT(const safe_VkImageViewASTCDecodeModeEXT& copy_src) {
    sType = copy_src.sType;
    decodeMode = copy_src.decodeMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewASTCDecodeModeEXT& safe_VkImageViewASTCDecodeModeEXT::operator=(const safe_VkImageViewASTCDecodeModeEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    decodeMode = copy_src.decodeMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewASTCDecodeModeEXT::~safe_VkImageViewASTCDecodeModeEXT() { FreePnextChain(pNext); }

void safe_VkImageViewASTCDecodeModeEXT::initialize(const VkImageViewASTCDecodeModeEXT* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    decodeMode = in_struct->decodeMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewASTCDecodeModeEXT::initialize(const safe_VkImageViewASTCDecodeModeEXT* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    decodeMode = copy_src->decodeMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(
    const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), decodeModeSharedExponent(in_struct->decodeModeSharedExponent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT), pNext(nullptr), decodeModeSharedExponent() {}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(
    const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    decodeModeSharedExponent = copy_src.decodeModeSharedExponent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    decodeModeSharedExponent = copy_src.decodeModeSharedExponent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::~safe_VkPhysicalDeviceASTCDecodeFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::initialize(const VkPhysicalDeviceASTCDecodeFeaturesEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    decodeModeSharedExponent = in_struct->decodeModeSharedExponent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceASTCDecodeFeaturesEXT::initialize(const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    decodeModeSharedExponent = copy_src->decodeModeSharedExponent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT(
    const VkConditionalRenderingBeginInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), buffer(in_struct->buffer), offset(in_struct->offset), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT()
    : sType(VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT), pNext(nullptr), buffer(), offset(), flags() {}

safe_VkConditionalRenderingBeginInfoEXT::safe_VkConditionalRenderingBeginInfoEXT(
    const safe_VkConditionalRenderingBeginInfoEXT& copy_src) {
    sType = copy_src.sType;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkConditionalRenderingBeginInfoEXT& safe_VkConditionalRenderingBeginInfoEXT::operator=(
    const safe_VkConditionalRenderingBeginInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkConditionalRenderingBeginInfoEXT::~safe_VkConditionalRenderingBeginInfoEXT() { FreePnextChain(pNext); }

void safe_VkConditionalRenderingBeginInfoEXT::initialize(const VkConditionalRenderingBeginInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    buffer = in_struct->buffer;
    offset = in_struct->offset;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkConditionalRenderingBeginInfoEXT::initialize(const safe_VkConditionalRenderingBeginInfoEXT* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    buffer = copy_src->buffer;
    offset = copy_src->offset;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(
    const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      conditionalRendering(in_struct->conditionalRendering),
      inheritedConditionalRendering(in_struct->inheritedConditionalRendering) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT),
      pNext(nullptr),
      conditionalRendering(),
      inheritedConditionalRendering() {}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(
    const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    conditionalRendering = copy_src.conditionalRendering;
    inheritedConditionalRendering = copy_src.inheritedConditionalRendering;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    conditionalRendering = copy_src.conditionalRendering;
    inheritedConditionalRendering = copy_src.inheritedConditionalRendering;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::~safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::initialize(
    const VkPhysicalDeviceConditionalRenderingFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    conditionalRendering = in_struct->conditionalRendering;
    inheritedConditionalRendering = in_struct->inheritedConditionalRendering;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    conditionalRendering = copy_src->conditionalRendering;
    inheritedConditionalRendering = copy_src->inheritedConditionalRendering;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(
    const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), conditionalRenderingEnable(in_struct->conditionalRenderingEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT()
    : sType(VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT),
      pNext(nullptr),
      conditionalRenderingEnable() {}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(
    const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& copy_src) {
    sType = copy_src.sType;
    conditionalRenderingEnable = copy_src.conditionalRenderingEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::operator=(
    const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    conditionalRenderingEnable = copy_src.conditionalRenderingEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::~safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::initialize(
    const VkCommandBufferInheritanceConditionalRenderingInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    conditionalRenderingEnable = in_struct->conditionalRenderingEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT::initialize(
    const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    conditionalRenderingEnable = copy_src->conditionalRenderingEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT(const VkSurfaceCapabilities2EXT* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minImageCount(in_struct->minImageCount),
      maxImageCount(in_struct->maxImageCount),
      currentExtent(in_struct->currentExtent),
      minImageExtent(in_struct->minImageExtent),
      maxImageExtent(in_struct->maxImageExtent),
      maxImageArrayLayers(in_struct->maxImageArrayLayers),
      supportedTransforms(in_struct->supportedTransforms),
      currentTransform(in_struct->currentTransform),
      supportedCompositeAlpha(in_struct->supportedCompositeAlpha),
      supportedUsageFlags(in_struct->supportedUsageFlags),
      supportedSurfaceCounters(in_struct->supportedSurfaceCounters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT),
      pNext(nullptr),
      minImageCount(),
      maxImageCount(),
      currentExtent(),
      minImageExtent(),
      maxImageExtent(),
      maxImageArrayLayers(),
      supportedTransforms(),
      currentTransform(),
      supportedCompositeAlpha(),
      supportedUsageFlags(),
      supportedSurfaceCounters() {}

safe_VkSurfaceCapabilities2EXT::safe_VkSurfaceCapabilities2EXT(const safe_VkSurfaceCapabilities2EXT& copy_src) {
    sType = copy_src.sType;
    minImageCount = copy_src.minImageCount;
    maxImageCount = copy_src.maxImageCount;
    currentExtent = copy_src.currentExtent;
    minImageExtent = copy_src.minImageExtent;
    maxImageExtent = copy_src.maxImageExtent;
    maxImageArrayLayers = copy_src.maxImageArrayLayers;
    supportedTransforms = copy_src.supportedTransforms;
    currentTransform = copy_src.currentTransform;
    supportedCompositeAlpha = copy_src.supportedCompositeAlpha;
    supportedUsageFlags = copy_src.supportedUsageFlags;
    supportedSurfaceCounters = copy_src.supportedSurfaceCounters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilities2EXT& safe_VkSurfaceCapabilities2EXT::operator=(const safe_VkSurfaceCapabilities2EXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minImageCount = copy_src.minImageCount;
    maxImageCount = copy_src.maxImageCount;
    currentExtent = copy_src.currentExtent;
    minImageExtent = copy_src.minImageExtent;
    maxImageExtent = copy_src.maxImageExtent;
    maxImageArrayLayers = copy_src.maxImageArrayLayers;
    supportedTransforms = copy_src.supportedTransforms;
    currentTransform = copy_src.currentTransform;
    supportedCompositeAlpha = copy_src.supportedCompositeAlpha;
    supportedUsageFlags = copy_src.supportedUsageFlags;
    supportedSurfaceCounters = copy_src.supportedSurfaceCounters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilities2EXT::~safe_VkSurfaceCapabilities2EXT() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilities2EXT::initialize(const VkSurfaceCapabilities2EXT* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minImageCount = in_struct->minImageCount;
    maxImageCount = in_struct->maxImageCount;
    currentExtent = in_struct->currentExtent;
    minImageExtent = in_struct->minImageExtent;
    maxImageExtent = in_struct->maxImageExtent;
    maxImageArrayLayers = in_struct->maxImageArrayLayers;
    supportedTransforms = in_struct->supportedTransforms;
    currentTransform = in_struct->currentTransform;
    supportedCompositeAlpha = in_struct->supportedCompositeAlpha;
    supportedUsageFlags = in_struct->supportedUsageFlags;
    supportedSurfaceCounters = in_struct->supportedSurfaceCounters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilities2EXT::initialize(const safe_VkSurfaceCapabilities2EXT* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minImageCount = copy_src->minImageCount;
    maxImageCount = copy_src->maxImageCount;
    currentExtent = copy_src->currentExtent;
    minImageExtent = copy_src->minImageExtent;
    maxImageExtent = copy_src->maxImageExtent;
    maxImageArrayLayers = copy_src->maxImageArrayLayers;
    supportedTransforms = copy_src->supportedTransforms;
    currentTransform = copy_src->currentTransform;
    supportedCompositeAlpha = copy_src->supportedCompositeAlpha;
    supportedUsageFlags = copy_src->supportedUsageFlags;
    supportedSurfaceCounters = copy_src->supportedSurfaceCounters;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT(const VkDisplayPowerInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), powerState(in_struct->powerState) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT), pNext(nullptr), powerState() {}

safe_VkDisplayPowerInfoEXT::safe_VkDisplayPowerInfoEXT(const safe_VkDisplayPowerInfoEXT& copy_src) {
    sType = copy_src.sType;
    powerState = copy_src.powerState;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPowerInfoEXT& safe_VkDisplayPowerInfoEXT::operator=(const safe_VkDisplayPowerInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    powerState = copy_src.powerState;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPowerInfoEXT::~safe_VkDisplayPowerInfoEXT() { FreePnextChain(pNext); }

void safe_VkDisplayPowerInfoEXT::initialize(const VkDisplayPowerInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    powerState = in_struct->powerState;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPowerInfoEXT::initialize(const safe_VkDisplayPowerInfoEXT* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    powerState = copy_src->powerState;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT(const VkDeviceEventInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), deviceEvent(in_struct->deviceEvent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT), pNext(nullptr), deviceEvent() {}

safe_VkDeviceEventInfoEXT::safe_VkDeviceEventInfoEXT(const safe_VkDeviceEventInfoEXT& copy_src) {
    sType = copy_src.sType;
    deviceEvent = copy_src.deviceEvent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceEventInfoEXT& safe_VkDeviceEventInfoEXT::operator=(const safe_VkDeviceEventInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceEvent = copy_src.deviceEvent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceEventInfoEXT::~safe_VkDeviceEventInfoEXT() { FreePnextChain(pNext); }

void safe_VkDeviceEventInfoEXT::initialize(const VkDeviceEventInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceEvent = in_struct->deviceEvent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceEventInfoEXT::initialize(const safe_VkDeviceEventInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceEvent = copy_src->deviceEvent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT(const VkDisplayEventInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayEvent(in_struct->displayEvent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT), pNext(nullptr), displayEvent() {}

safe_VkDisplayEventInfoEXT::safe_VkDisplayEventInfoEXT(const safe_VkDisplayEventInfoEXT& copy_src) {
    sType = copy_src.sType;
    displayEvent = copy_src.displayEvent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayEventInfoEXT& safe_VkDisplayEventInfoEXT::operator=(const safe_VkDisplayEventInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayEvent = copy_src.displayEvent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayEventInfoEXT::~safe_VkDisplayEventInfoEXT() { FreePnextChain(pNext); }

void safe_VkDisplayEventInfoEXT::initialize(const VkDisplayEventInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayEvent = in_struct->displayEvent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayEventInfoEXT::initialize(const safe_VkDisplayEventInfoEXT* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayEvent = copy_src->displayEvent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT(const VkSwapchainCounterCreateInfoEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), surfaceCounters(in_struct->surfaceCounters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT), pNext(nullptr), surfaceCounters() {}

safe_VkSwapchainCounterCreateInfoEXT::safe_VkSwapchainCounterCreateInfoEXT(const safe_VkSwapchainCounterCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    surfaceCounters = copy_src.surfaceCounters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSwapchainCounterCreateInfoEXT& safe_VkSwapchainCounterCreateInfoEXT::operator=(
    const safe_VkSwapchainCounterCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surfaceCounters = copy_src.surfaceCounters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSwapchainCounterCreateInfoEXT::~safe_VkSwapchainCounterCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkSwapchainCounterCreateInfoEXT::initialize(const VkSwapchainCounterCreateInfoEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surfaceCounters = in_struct->surfaceCounters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSwapchainCounterCreateInfoEXT::initialize(const safe_VkSwapchainCounterCreateInfoEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surfaceCounters = copy_src->surfaceCounters;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(
    const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxDiscardRectangles(in_struct->maxDiscardRectangles) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT), pNext(nullptr), maxDiscardRectangles() {}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(
    const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxDiscardRectangles = copy_src.maxDiscardRectangles;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::operator=(
    const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxDiscardRectangles = copy_src.maxDiscardRectangles;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::~safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::initialize(const VkPhysicalDeviceDiscardRectanglePropertiesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxDiscardRectangles = in_struct->maxDiscardRectangles;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT::initialize(
    const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxDiscardRectangles = copy_src->maxDiscardRectangles;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT(
    const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      discardRectangleMode(in_struct->discardRectangleMode),
      discardRectangleCount(in_struct->discardRectangleCount),
      pDiscardRectangles(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[in_struct->discardRectangleCount];
        memcpy((void*)pDiscardRectangles, (void*)in_struct->pDiscardRectangles,
               sizeof(VkRect2D) * in_struct->discardRectangleCount);
    }
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      discardRectangleMode(),
      discardRectangleCount(),
      pDiscardRectangles(nullptr) {}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::safe_VkPipelineDiscardRectangleStateCreateInfoEXT(
    const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    discardRectangleMode = copy_src.discardRectangleMode;
    discardRectangleCount = copy_src.discardRectangleCount;
    pDiscardRectangles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[copy_src.discardRectangleCount];
        memcpy((void*)pDiscardRectangles, (void*)copy_src.pDiscardRectangles, sizeof(VkRect2D) * copy_src.discardRectangleCount);
    }
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT& safe_VkPipelineDiscardRectangleStateCreateInfoEXT::operator=(
    const safe_VkPipelineDiscardRectangleStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDiscardRectangles) delete[] pDiscardRectangles;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    discardRectangleMode = copy_src.discardRectangleMode;
    discardRectangleCount = copy_src.discardRectangleCount;
    pDiscardRectangles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[copy_src.discardRectangleCount];
        memcpy((void*)pDiscardRectangles, (void*)copy_src.pDiscardRectangles, sizeof(VkRect2D) * copy_src.discardRectangleCount);
    }

    return *this;
}

safe_VkPipelineDiscardRectangleStateCreateInfoEXT::~safe_VkPipelineDiscardRectangleStateCreateInfoEXT() {
    if (pDiscardRectangles) delete[] pDiscardRectangles;
    FreePnextChain(pNext);
}

void safe_VkPipelineDiscardRectangleStateCreateInfoEXT::initialize(const VkPipelineDiscardRectangleStateCreateInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDiscardRectangles) delete[] pDiscardRectangles;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    discardRectangleMode = in_struct->discardRectangleMode;
    discardRectangleCount = in_struct->discardRectangleCount;
    pDiscardRectangles = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[in_struct->discardRectangleCount];
        memcpy((void*)pDiscardRectangles, (void*)in_struct->pDiscardRectangles,
               sizeof(VkRect2D) * in_struct->discardRectangleCount);
    }
}

void safe_VkPipelineDiscardRectangleStateCreateInfoEXT::initialize(
    const safe_VkPipelineDiscardRectangleStateCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    discardRectangleMode = copy_src->discardRectangleMode;
    discardRectangleCount = copy_src->discardRectangleCount;
    pDiscardRectangles = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDiscardRectangles) {
        pDiscardRectangles = new VkRect2D[copy_src->discardRectangleCount];
        memcpy((void*)pDiscardRectangles, (void*)copy_src->pDiscardRectangles, sizeof(VkRect2D) * copy_src->discardRectangleCount);
    }
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(
    const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      primitiveOverestimationSize(in_struct->primitiveOverestimationSize),
      maxExtraPrimitiveOverestimationSize(in_struct->maxExtraPrimitiveOverestimationSize),
      extraPrimitiveOverestimationSizeGranularity(in_struct->extraPrimitiveOverestimationSizeGranularity),
      primitiveUnderestimation(in_struct->primitiveUnderestimation),
      conservativePointAndLineRasterization(in_struct->conservativePointAndLineRasterization),
      degenerateTrianglesRasterized(in_struct->degenerateTrianglesRasterized),
      degenerateLinesRasterized(in_struct->degenerateLinesRasterized),
      fullyCoveredFragmentShaderInputVariable(in_struct->fullyCoveredFragmentShaderInputVariable),
      conservativeRasterizationPostDepthCoverage(in_struct->conservativeRasterizationPostDepthCoverage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT),
      pNext(nullptr),
      primitiveOverestimationSize(),
      maxExtraPrimitiveOverestimationSize(),
      extraPrimitiveOverestimationSizeGranularity(),
      primitiveUnderestimation(),
      conservativePointAndLineRasterization(),
      degenerateTrianglesRasterized(),
      degenerateLinesRasterized(),
      fullyCoveredFragmentShaderInputVariable(),
      conservativeRasterizationPostDepthCoverage() {}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(
    const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    primitiveOverestimationSize = copy_src.primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = copy_src.maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = copy_src.extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = copy_src.primitiveUnderestimation;
    conservativePointAndLineRasterization = copy_src.conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = copy_src.degenerateTrianglesRasterized;
    degenerateLinesRasterized = copy_src.degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = copy_src.fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = copy_src.conservativeRasterizationPostDepthCoverage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    primitiveOverestimationSize = copy_src.primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = copy_src.maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = copy_src.extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = copy_src.primitiveUnderestimation;
    conservativePointAndLineRasterization = copy_src.conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = copy_src.degenerateTrianglesRasterized;
    degenerateLinesRasterized = copy_src.degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = copy_src.fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = copy_src.conservativeRasterizationPostDepthCoverage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::~safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::initialize(
    const VkPhysicalDeviceConservativeRasterizationPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    primitiveOverestimationSize = in_struct->primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = in_struct->maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = in_struct->extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = in_struct->primitiveUnderestimation;
    conservativePointAndLineRasterization = in_struct->conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = in_struct->degenerateTrianglesRasterized;
    degenerateLinesRasterized = in_struct->degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = in_struct->fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = in_struct->conservativeRasterizationPostDepthCoverage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    primitiveOverestimationSize = copy_src->primitiveOverestimationSize;
    maxExtraPrimitiveOverestimationSize = copy_src->maxExtraPrimitiveOverestimationSize;
    extraPrimitiveOverestimationSizeGranularity = copy_src->extraPrimitiveOverestimationSizeGranularity;
    primitiveUnderestimation = copy_src->primitiveUnderestimation;
    conservativePointAndLineRasterization = copy_src->conservativePointAndLineRasterization;
    degenerateTrianglesRasterized = copy_src->degenerateTrianglesRasterized;
    degenerateLinesRasterized = copy_src->degenerateLinesRasterized;
    fullyCoveredFragmentShaderInputVariable = copy_src->fullyCoveredFragmentShaderInputVariable;
    conservativeRasterizationPostDepthCoverage = copy_src->conservativeRasterizationPostDepthCoverage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(
    const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      conservativeRasterizationMode(in_struct->conservativeRasterizationMode),
      extraPrimitiveOverestimationSize(in_struct->extraPrimitiveOverestimationSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      conservativeRasterizationMode(),
      extraPrimitiveOverestimationSize() {}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(
    const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    conservativeRasterizationMode = copy_src.conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = copy_src.extraPrimitiveOverestimationSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::operator=(
    const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    conservativeRasterizationMode = copy_src.conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = copy_src.extraPrimitiveOverestimationSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::~safe_VkPipelineRasterizationConservativeStateCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::initialize(
    const VkPipelineRasterizationConservativeStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    conservativeRasterizationMode = in_struct->conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = in_struct->extraPrimitiveOverestimationSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRasterizationConservativeStateCreateInfoEXT::initialize(
    const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    conservativeRasterizationMode = copy_src->conservativeRasterizationMode;
    extraPrimitiveOverestimationSize = copy_src->extraPrimitiveOverestimationSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(
    const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), depthClipEnable(in_struct->depthClipEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT), pNext(nullptr), depthClipEnable() {}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(
    const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    depthClipEnable = copy_src.depthClipEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthClipEnable = copy_src.depthClipEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::~safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::initialize(const VkPhysicalDeviceDepthClipEnableFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthClipEnable = in_struct->depthClipEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT::initialize(const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthClipEnable = copy_src->depthClipEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(
    const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), depthClipEnable(in_struct->depthClipEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      depthClipEnable() {}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(
    const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    depthClipEnable = copy_src.depthClipEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::operator=(
    const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    depthClipEnable = copy_src.depthClipEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::~safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::initialize(
    const VkPipelineRasterizationDepthClipStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    depthClipEnable = in_struct->depthClipEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT::initialize(
    const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    depthClipEnable = copy_src->depthClipEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT(const VkHdrMetadataEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                             bool copy_pnext)
    : sType(in_struct->sType),
      displayPrimaryRed(in_struct->displayPrimaryRed),
      displayPrimaryGreen(in_struct->displayPrimaryGreen),
      displayPrimaryBlue(in_struct->displayPrimaryBlue),
      whitePoint(in_struct->whitePoint),
      maxLuminance(in_struct->maxLuminance),
      minLuminance(in_struct->minLuminance),
      maxContentLightLevel(in_struct->maxContentLightLevel),
      maxFrameAverageLightLevel(in_struct->maxFrameAverageLightLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT()
    : sType(VK_STRUCTURE_TYPE_HDR_METADATA_EXT),
      pNext(nullptr),
      displayPrimaryRed(),
      displayPrimaryGreen(),
      displayPrimaryBlue(),
      whitePoint(),
      maxLuminance(),
      minLuminance(),
      maxContentLightLevel(),
      maxFrameAverageLightLevel() {}

safe_VkHdrMetadataEXT::safe_VkHdrMetadataEXT(const safe_VkHdrMetadataEXT& copy_src) {
    sType = copy_src.sType;
    displayPrimaryRed = copy_src.displayPrimaryRed;
    displayPrimaryGreen = copy_src.displayPrimaryGreen;
    displayPrimaryBlue = copy_src.displayPrimaryBlue;
    whitePoint = copy_src.whitePoint;
    maxLuminance = copy_src.maxLuminance;
    minLuminance = copy_src.minLuminance;
    maxContentLightLevel = copy_src.maxContentLightLevel;
    maxFrameAverageLightLevel = copy_src.maxFrameAverageLightLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkHdrMetadataEXT& safe_VkHdrMetadataEXT::operator=(const safe_VkHdrMetadataEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayPrimaryRed = copy_src.displayPrimaryRed;
    displayPrimaryGreen = copy_src.displayPrimaryGreen;
    displayPrimaryBlue = copy_src.displayPrimaryBlue;
    whitePoint = copy_src.whitePoint;
    maxLuminance = copy_src.maxLuminance;
    minLuminance = copy_src.minLuminance;
    maxContentLightLevel = copy_src.maxContentLightLevel;
    maxFrameAverageLightLevel = copy_src.maxFrameAverageLightLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkHdrMetadataEXT::~safe_VkHdrMetadataEXT() { FreePnextChain(pNext); }

void safe_VkHdrMetadataEXT::initialize(const VkHdrMetadataEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayPrimaryRed = in_struct->displayPrimaryRed;
    displayPrimaryGreen = in_struct->displayPrimaryGreen;
    displayPrimaryBlue = in_struct->displayPrimaryBlue;
    whitePoint = in_struct->whitePoint;
    maxLuminance = in_struct->maxLuminance;
    minLuminance = in_struct->minLuminance;
    maxContentLightLevel = in_struct->maxContentLightLevel;
    maxFrameAverageLightLevel = in_struct->maxFrameAverageLightLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkHdrMetadataEXT::initialize(const safe_VkHdrMetadataEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayPrimaryRed = copy_src->displayPrimaryRed;
    displayPrimaryGreen = copy_src->displayPrimaryGreen;
    displayPrimaryBlue = copy_src->displayPrimaryBlue;
    whitePoint = copy_src->whitePoint;
    maxLuminance = copy_src->maxLuminance;
    minLuminance = copy_src->minLuminance;
    maxContentLightLevel = copy_src->maxContentLightLevel;
    maxFrameAverageLightLevel = copy_src->maxFrameAverageLightLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT(const VkDebugUtilsLabelEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pLabelName = SafeStringCopy(in_struct->pLabelName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = in_struct->color[i];
    }
}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT), pNext(nullptr), pLabelName(nullptr) {}

safe_VkDebugUtilsLabelEXT::safe_VkDebugUtilsLabelEXT(const safe_VkDebugUtilsLabelEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    pLabelName = SafeStringCopy(copy_src.pLabelName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src.color[i];
    }
}

safe_VkDebugUtilsLabelEXT& safe_VkDebugUtilsLabelEXT::operator=(const safe_VkDebugUtilsLabelEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pLabelName) delete[] pLabelName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    pLabelName = SafeStringCopy(copy_src.pLabelName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src.color[i];
    }

    return *this;
}

safe_VkDebugUtilsLabelEXT::~safe_VkDebugUtilsLabelEXT() {
    if (pLabelName) delete[] pLabelName;
    FreePnextChain(pNext);
}

void safe_VkDebugUtilsLabelEXT::initialize(const VkDebugUtilsLabelEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLabelName) delete[] pLabelName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pLabelName = SafeStringCopy(in_struct->pLabelName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = in_struct->color[i];
    }
}

void safe_VkDebugUtilsLabelEXT::initialize(const safe_VkDebugUtilsLabelEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
    pLabelName = SafeStringCopy(copy_src->pLabelName);

    for (uint32_t i = 0; i < 4; ++i) {
        color[i] = copy_src->color[i];
    }
}

safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT(const VkDebugUtilsObjectNameInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), objectType(in_struct->objectType), objectHandle(in_struct->objectHandle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pObjectName = SafeStringCopy(in_struct->pObjectName);
}

safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT),
      pNext(nullptr),
      objectType(),
      objectHandle(),
      pObjectName(nullptr) {}

safe_VkDebugUtilsObjectNameInfoEXT::safe_VkDebugUtilsObjectNameInfoEXT(const safe_VkDebugUtilsObjectNameInfoEXT& copy_src) {
    sType = copy_src.sType;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    pNext = SafePnextCopy(copy_src.pNext);
    pObjectName = SafeStringCopy(copy_src.pObjectName);
}

safe_VkDebugUtilsObjectNameInfoEXT& safe_VkDebugUtilsObjectNameInfoEXT::operator=(
    const safe_VkDebugUtilsObjectNameInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    pNext = SafePnextCopy(copy_src.pNext);
    pObjectName = SafeStringCopy(copy_src.pObjectName);

    return *this;
}

safe_VkDebugUtilsObjectNameInfoEXT::~safe_VkDebugUtilsObjectNameInfoEXT() {
    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);
}

void safe_VkDebugUtilsObjectNameInfoEXT::initialize(const VkDebugUtilsObjectNameInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pObjectName) delete[] pObjectName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    objectType = in_struct->objectType;
    objectHandle = in_struct->objectHandle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pObjectName = SafeStringCopy(in_struct->pObjectName);
}

void safe_VkDebugUtilsObjectNameInfoEXT::initialize(const safe_VkDebugUtilsObjectNameInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    objectType = copy_src->objectType;
    objectHandle = copy_src->objectHandle;
    pNext = SafePnextCopy(copy_src->pNext);
    pObjectName = SafeStringCopy(copy_src->pObjectName);
}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT(
    const VkDebugUtilsMessengerCallbackDataEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      messageIdNumber(in_struct->messageIdNumber),
      queueLabelCount(in_struct->queueLabelCount),
      pQueueLabels(nullptr),
      cmdBufLabelCount(in_struct->cmdBufLabelCount),
      pCmdBufLabels(nullptr),
      objectCount(in_struct->objectCount),
      pObjects(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pMessageIdName = SafeStringCopy(in_struct->pMessageIdName);
    pMessage = SafeStringCopy(in_struct->pMessage);
    if (queueLabelCount && in_struct->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i = 0; i < queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&in_struct->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && in_struct->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i = 0; i < cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&in_struct->pCmdBufLabels[i]);
        }
    }
    if (objectCount && in_struct->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i = 0; i < objectCount; ++i) {
            pObjects[i].initialize(&in_struct->pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT),
      pNext(nullptr),
      flags(),
      pMessageIdName(nullptr),
      messageIdNumber(),
      pMessage(nullptr),
      queueLabelCount(),
      pQueueLabels(nullptr),
      cmdBufLabelCount(),
      pCmdBufLabels(nullptr),
      objectCount(),
      pObjects(nullptr) {}

safe_VkDebugUtilsMessengerCallbackDataEXT::safe_VkDebugUtilsMessengerCallbackDataEXT(
    const safe_VkDebugUtilsMessengerCallbackDataEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    messageIdNumber = copy_src.messageIdNumber;
    queueLabelCount = copy_src.queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = copy_src.cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = copy_src.objectCount;
    pObjects = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    pMessageIdName = SafeStringCopy(copy_src.pMessageIdName);
    pMessage = SafeStringCopy(copy_src.pMessage);
    if (queueLabelCount && copy_src.pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i = 0; i < queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&copy_src.pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && copy_src.pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i = 0; i < cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&copy_src.pCmdBufLabels[i]);
        }
    }
    if (objectCount && copy_src.pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i = 0; i < objectCount; ++i) {
            pObjects[i].initialize(&copy_src.pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCallbackDataEXT& safe_VkDebugUtilsMessengerCallbackDataEXT::operator=(
    const safe_VkDebugUtilsMessengerCallbackDataEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pMessageIdName) delete[] pMessageIdName;
    if (pMessage) delete[] pMessage;
    if (pQueueLabels) delete[] pQueueLabels;
    if (pCmdBufLabels) delete[] pCmdBufLabels;
    if (pObjects) delete[] pObjects;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    messageIdNumber = copy_src.messageIdNumber;
    queueLabelCount = copy_src.queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = copy_src.cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = copy_src.objectCount;
    pObjects = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    pMessageIdName = SafeStringCopy(copy_src.pMessageIdName);
    pMessage = SafeStringCopy(copy_src.pMessage);
    if (queueLabelCount && copy_src.pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i = 0; i < queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&copy_src.pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && copy_src.pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i = 0; i < cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&copy_src.pCmdBufLabels[i]);
        }
    }
    if (objectCount && copy_src.pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i = 0; i < objectCount; ++i) {
            pObjects[i].initialize(&copy_src.pObjects[i]);
        }
    }

    return *this;
}

safe_VkDebugUtilsMessengerCallbackDataEXT::~safe_VkDebugUtilsMessengerCallbackDataEXT() {
    if (pMessageIdName) delete[] pMessageIdName;
    if (pMessage) delete[] pMessage;
    if (pQueueLabels) delete[] pQueueLabels;
    if (pCmdBufLabels) delete[] pCmdBufLabels;
    if (pObjects) delete[] pObjects;
    FreePnextChain(pNext);
}

void safe_VkDebugUtilsMessengerCallbackDataEXT::initialize(const VkDebugUtilsMessengerCallbackDataEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pMessageIdName) delete[] pMessageIdName;
    if (pMessage) delete[] pMessage;
    if (pQueueLabels) delete[] pQueueLabels;
    if (pCmdBufLabels) delete[] pCmdBufLabels;
    if (pObjects) delete[] pObjects;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    messageIdNumber = in_struct->messageIdNumber;
    queueLabelCount = in_struct->queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = in_struct->cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = in_struct->objectCount;
    pObjects = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pMessageIdName = SafeStringCopy(in_struct->pMessageIdName);
    pMessage = SafeStringCopy(in_struct->pMessage);
    if (queueLabelCount && in_struct->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i = 0; i < queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&in_struct->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && in_struct->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i = 0; i < cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&in_struct->pCmdBufLabels[i]);
        }
    }
    if (objectCount && in_struct->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i = 0; i < objectCount; ++i) {
            pObjects[i].initialize(&in_struct->pObjects[i]);
        }
    }
}

void safe_VkDebugUtilsMessengerCallbackDataEXT::initialize(const safe_VkDebugUtilsMessengerCallbackDataEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    messageIdNumber = copy_src->messageIdNumber;
    queueLabelCount = copy_src->queueLabelCount;
    pQueueLabels = nullptr;
    cmdBufLabelCount = copy_src->cmdBufLabelCount;
    pCmdBufLabels = nullptr;
    objectCount = copy_src->objectCount;
    pObjects = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    pMessageIdName = SafeStringCopy(copy_src->pMessageIdName);
    pMessage = SafeStringCopy(copy_src->pMessage);
    if (queueLabelCount && copy_src->pQueueLabels) {
        pQueueLabels = new safe_VkDebugUtilsLabelEXT[queueLabelCount];
        for (uint32_t i = 0; i < queueLabelCount; ++i) {
            pQueueLabels[i].initialize(&copy_src->pQueueLabels[i]);
        }
    }
    if (cmdBufLabelCount && copy_src->pCmdBufLabels) {
        pCmdBufLabels = new safe_VkDebugUtilsLabelEXT[cmdBufLabelCount];
        for (uint32_t i = 0; i < cmdBufLabelCount; ++i) {
            pCmdBufLabels[i].initialize(&copy_src->pCmdBufLabels[i]);
        }
    }
    if (objectCount && copy_src->pObjects) {
        pObjects = new safe_VkDebugUtilsObjectNameInfoEXT[objectCount];
        for (uint32_t i = 0; i < objectCount; ++i) {
            pObjects[i].initialize(&copy_src->pObjects[i]);
        }
    }
}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT(
    const VkDebugUtilsMessengerCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      messageSeverity(in_struct->messageSeverity),
      messageType(in_struct->messageType),
      pfnUserCallback(in_struct->pfnUserCallback),
      pUserData(in_struct->pUserData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      messageSeverity(),
      messageType(),
      pfnUserCallback(),
      pUserData(nullptr) {}

safe_VkDebugUtilsMessengerCreateInfoEXT::safe_VkDebugUtilsMessengerCreateInfoEXT(
    const safe_VkDebugUtilsMessengerCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    messageSeverity = copy_src.messageSeverity;
    messageType = copy_src.messageType;
    pfnUserCallback = copy_src.pfnUserCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDebugUtilsMessengerCreateInfoEXT& safe_VkDebugUtilsMessengerCreateInfoEXT::operator=(
    const safe_VkDebugUtilsMessengerCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    messageSeverity = copy_src.messageSeverity;
    messageType = copy_src.messageType;
    pfnUserCallback = copy_src.pfnUserCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDebugUtilsMessengerCreateInfoEXT::~safe_VkDebugUtilsMessengerCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkDebugUtilsMessengerCreateInfoEXT::initialize(const VkDebugUtilsMessengerCreateInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    messageSeverity = in_struct->messageSeverity;
    messageType = in_struct->messageType;
    pfnUserCallback = in_struct->pfnUserCallback;
    pUserData = in_struct->pUserData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDebugUtilsMessengerCreateInfoEXT::initialize(const safe_VkDebugUtilsMessengerCreateInfoEXT* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    messageSeverity = copy_src->messageSeverity;
    messageType = copy_src->messageType;
    pfnUserCallback = copy_src->pfnUserCallback;
    pUserData = copy_src->pUserData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT(const VkDebugUtilsObjectTagInfoEXT* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      objectType(in_struct->objectType),
      objectHandle(in_struct->objectHandle),
      tagName(in_struct->tagName),
      tagSize(in_struct->tagSize),
      pTag(in_struct->pTag) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT),
      pNext(nullptr),
      objectType(),
      objectHandle(),
      tagName(),
      tagSize(),
      pTag(nullptr) {}

safe_VkDebugUtilsObjectTagInfoEXT::safe_VkDebugUtilsObjectTagInfoEXT(const safe_VkDebugUtilsObjectTagInfoEXT& copy_src) {
    sType = copy_src.sType;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDebugUtilsObjectTagInfoEXT& safe_VkDebugUtilsObjectTagInfoEXT::operator=(const safe_VkDebugUtilsObjectTagInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDebugUtilsObjectTagInfoEXT::~safe_VkDebugUtilsObjectTagInfoEXT() { FreePnextChain(pNext); }

void safe_VkDebugUtilsObjectTagInfoEXT::initialize(const VkDebugUtilsObjectTagInfoEXT* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    objectType = in_struct->objectType;
    objectHandle = in_struct->objectHandle;
    tagName = in_struct->tagName;
    tagSize = in_struct->tagSize;
    pTag = in_struct->pTag;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDebugUtilsObjectTagInfoEXT::initialize(const safe_VkDebugUtilsObjectTagInfoEXT* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    objectType = copy_src->objectType;
    objectHandle = copy_src->objectHandle;
    tagName = copy_src->tagName;
    tagSize = copy_src->tagSize;
    pTag = copy_src->pTag;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT(const VkSampleLocationsInfoEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      sampleLocationsPerPixel(in_struct->sampleLocationsPerPixel),
      sampleLocationGridSize(in_struct->sampleLocationGridSize),
      sampleLocationsCount(in_struct->sampleLocationsCount),
      pSampleLocations(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[in_struct->sampleLocationsCount];
        memcpy((void*)pSampleLocations, (void*)in_struct->pSampleLocations,
               sizeof(VkSampleLocationEXT) * in_struct->sampleLocationsCount);
    }
}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT),
      pNext(nullptr),
      sampleLocationsPerPixel(),
      sampleLocationGridSize(),
      sampleLocationsCount(),
      pSampleLocations(nullptr) {}

safe_VkSampleLocationsInfoEXT::safe_VkSampleLocationsInfoEXT(const safe_VkSampleLocationsInfoEXT& copy_src) {
    sType = copy_src.sType;
    sampleLocationsPerPixel = copy_src.sampleLocationsPerPixel;
    sampleLocationGridSize = copy_src.sampleLocationGridSize;
    sampleLocationsCount = copy_src.sampleLocationsCount;
    pSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[copy_src.sampleLocationsCount];
        memcpy((void*)pSampleLocations, (void*)copy_src.pSampleLocations,
               sizeof(VkSampleLocationEXT) * copy_src.sampleLocationsCount);
    }
}

safe_VkSampleLocationsInfoEXT& safe_VkSampleLocationsInfoEXT::operator=(const safe_VkSampleLocationsInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pSampleLocations) delete[] pSampleLocations;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleLocationsPerPixel = copy_src.sampleLocationsPerPixel;
    sampleLocationGridSize = copy_src.sampleLocationGridSize;
    sampleLocationsCount = copy_src.sampleLocationsCount;
    pSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[copy_src.sampleLocationsCount];
        memcpy((void*)pSampleLocations, (void*)copy_src.pSampleLocations,
               sizeof(VkSampleLocationEXT) * copy_src.sampleLocationsCount);
    }

    return *this;
}

safe_VkSampleLocationsInfoEXT::~safe_VkSampleLocationsInfoEXT() {
    if (pSampleLocations) delete[] pSampleLocations;
    FreePnextChain(pNext);
}

void safe_VkSampleLocationsInfoEXT::initialize(const VkSampleLocationsInfoEXT* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSampleLocations) delete[] pSampleLocations;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleLocationsPerPixel = in_struct->sampleLocationsPerPixel;
    sampleLocationGridSize = in_struct->sampleLocationGridSize;
    sampleLocationsCount = in_struct->sampleLocationsCount;
    pSampleLocations = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[in_struct->sampleLocationsCount];
        memcpy((void*)pSampleLocations, (void*)in_struct->pSampleLocations,
               sizeof(VkSampleLocationEXT) * in_struct->sampleLocationsCount);
    }
}

void safe_VkSampleLocationsInfoEXT::initialize(const safe_VkSampleLocationsInfoEXT* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleLocationsPerPixel = copy_src->sampleLocationsPerPixel;
    sampleLocationGridSize = copy_src->sampleLocationGridSize;
    sampleLocationsCount = copy_src->sampleLocationsCount;
    pSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pSampleLocations) {
        pSampleLocations = new VkSampleLocationEXT[copy_src->sampleLocationsCount];
        memcpy((void*)pSampleLocations, (void*)copy_src->pSampleLocations,
               sizeof(VkSampleLocationEXT) * copy_src->sampleLocationsCount);
    }
}

safe_VkAttachmentSampleLocationsEXT::safe_VkAttachmentSampleLocationsEXT(const VkAttachmentSampleLocationsEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : attachmentIndex(in_struct->attachmentIndex), sampleLocationsInfo(&in_struct->sampleLocationsInfo) {}

safe_VkAttachmentSampleLocationsEXT::safe_VkAttachmentSampleLocationsEXT() : attachmentIndex() {}

safe_VkAttachmentSampleLocationsEXT::safe_VkAttachmentSampleLocationsEXT(const safe_VkAttachmentSampleLocationsEXT& copy_src) {
    attachmentIndex = copy_src.attachmentIndex;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);
}

safe_VkAttachmentSampleLocationsEXT& safe_VkAttachmentSampleLocationsEXT::operator=(
    const safe_VkAttachmentSampleLocationsEXT& copy_src) {
    if (&copy_src == this) return *this;

    attachmentIndex = copy_src.attachmentIndex;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);

    return *this;
}

safe_VkAttachmentSampleLocationsEXT::~safe_VkAttachmentSampleLocationsEXT() {}

void safe_VkAttachmentSampleLocationsEXT::initialize(const VkAttachmentSampleLocationsEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    attachmentIndex = in_struct->attachmentIndex;
    sampleLocationsInfo.initialize(&in_struct->sampleLocationsInfo);
}

void safe_VkAttachmentSampleLocationsEXT::initialize(const safe_VkAttachmentSampleLocationsEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    attachmentIndex = copy_src->attachmentIndex;
    sampleLocationsInfo.initialize(&copy_src->sampleLocationsInfo);
}

safe_VkSubpassSampleLocationsEXT::safe_VkSubpassSampleLocationsEXT(const VkSubpassSampleLocationsEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state)
    : subpassIndex(in_struct->subpassIndex), sampleLocationsInfo(&in_struct->sampleLocationsInfo) {}

safe_VkSubpassSampleLocationsEXT::safe_VkSubpassSampleLocationsEXT() : subpassIndex() {}

safe_VkSubpassSampleLocationsEXT::safe_VkSubpassSampleLocationsEXT(const safe_VkSubpassSampleLocationsEXT& copy_src) {
    subpassIndex = copy_src.subpassIndex;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);
}

safe_VkSubpassSampleLocationsEXT& safe_VkSubpassSampleLocationsEXT::operator=(const safe_VkSubpassSampleLocationsEXT& copy_src) {
    if (&copy_src == this) return *this;

    subpassIndex = copy_src.subpassIndex;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);

    return *this;
}

safe_VkSubpassSampleLocationsEXT::~safe_VkSubpassSampleLocationsEXT() {}

void safe_VkSubpassSampleLocationsEXT::initialize(const VkSubpassSampleLocationsEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    subpassIndex = in_struct->subpassIndex;
    sampleLocationsInfo.initialize(&in_struct->sampleLocationsInfo);
}

void safe_VkSubpassSampleLocationsEXT::initialize(const safe_VkSubpassSampleLocationsEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    subpassIndex = copy_src->subpassIndex;
    sampleLocationsInfo.initialize(&copy_src->sampleLocationsInfo);
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT(
    const VkRenderPassSampleLocationsBeginInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      attachmentInitialSampleLocationsCount(in_struct->attachmentInitialSampleLocationsCount),
      pAttachmentInitialSampleLocations(nullptr),
      postSubpassSampleLocationsCount(in_struct->postSubpassSampleLocationsCount),
      pPostSubpassSampleLocations(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (attachmentInitialSampleLocationsCount && in_struct->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new safe_VkAttachmentSampleLocationsEXT[attachmentInitialSampleLocationsCount];
        for (uint32_t i = 0; i < attachmentInitialSampleLocationsCount; ++i) {
            pAttachmentInitialSampleLocations[i].initialize(&in_struct->pAttachmentInitialSampleLocations[i]);
        }
    }
    if (postSubpassSampleLocationsCount && in_struct->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new safe_VkSubpassSampleLocationsEXT[postSubpassSampleLocationsCount];
        for (uint32_t i = 0; i < postSubpassSampleLocationsCount; ++i) {
            pPostSubpassSampleLocations[i].initialize(&in_struct->pPostSubpassSampleLocations[i]);
        }
    }
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT),
      pNext(nullptr),
      attachmentInitialSampleLocationsCount(),
      pAttachmentInitialSampleLocations(nullptr),
      postSubpassSampleLocationsCount(),
      pPostSubpassSampleLocations(nullptr) {}

safe_VkRenderPassSampleLocationsBeginInfoEXT::safe_VkRenderPassSampleLocationsBeginInfoEXT(
    const safe_VkRenderPassSampleLocationsBeginInfoEXT& copy_src) {
    sType = copy_src.sType;
    attachmentInitialSampleLocationsCount = copy_src.attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = copy_src.postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (attachmentInitialSampleLocationsCount && copy_src.pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new safe_VkAttachmentSampleLocationsEXT[attachmentInitialSampleLocationsCount];
        for (uint32_t i = 0; i < attachmentInitialSampleLocationsCount; ++i) {
            pAttachmentInitialSampleLocations[i].initialize(&copy_src.pAttachmentInitialSampleLocations[i]);
        }
    }
    if (postSubpassSampleLocationsCount && copy_src.pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new safe_VkSubpassSampleLocationsEXT[postSubpassSampleLocationsCount];
        for (uint32_t i = 0; i < postSubpassSampleLocationsCount; ++i) {
            pPostSubpassSampleLocations[i].initialize(&copy_src.pPostSubpassSampleLocations[i]);
        }
    }
}

safe_VkRenderPassSampleLocationsBeginInfoEXT& safe_VkRenderPassSampleLocationsBeginInfoEXT::operator=(
    const safe_VkRenderPassSampleLocationsBeginInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttachmentInitialSampleLocations) delete[] pAttachmentInitialSampleLocations;
    if (pPostSubpassSampleLocations) delete[] pPostSubpassSampleLocations;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    attachmentInitialSampleLocationsCount = copy_src.attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = copy_src.postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (attachmentInitialSampleLocationsCount && copy_src.pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new safe_VkAttachmentSampleLocationsEXT[attachmentInitialSampleLocationsCount];
        for (uint32_t i = 0; i < attachmentInitialSampleLocationsCount; ++i) {
            pAttachmentInitialSampleLocations[i].initialize(&copy_src.pAttachmentInitialSampleLocations[i]);
        }
    }
    if (postSubpassSampleLocationsCount && copy_src.pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new safe_VkSubpassSampleLocationsEXT[postSubpassSampleLocationsCount];
        for (uint32_t i = 0; i < postSubpassSampleLocationsCount; ++i) {
            pPostSubpassSampleLocations[i].initialize(&copy_src.pPostSubpassSampleLocations[i]);
        }
    }

    return *this;
}

safe_VkRenderPassSampleLocationsBeginInfoEXT::~safe_VkRenderPassSampleLocationsBeginInfoEXT() {
    if (pAttachmentInitialSampleLocations) delete[] pAttachmentInitialSampleLocations;
    if (pPostSubpassSampleLocations) delete[] pPostSubpassSampleLocations;
    FreePnextChain(pNext);
}

void safe_VkRenderPassSampleLocationsBeginInfoEXT::initialize(const VkRenderPassSampleLocationsBeginInfoEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttachmentInitialSampleLocations) delete[] pAttachmentInitialSampleLocations;
    if (pPostSubpassSampleLocations) delete[] pPostSubpassSampleLocations;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    attachmentInitialSampleLocationsCount = in_struct->attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = in_struct->postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (attachmentInitialSampleLocationsCount && in_struct->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new safe_VkAttachmentSampleLocationsEXT[attachmentInitialSampleLocationsCount];
        for (uint32_t i = 0; i < attachmentInitialSampleLocationsCount; ++i) {
            pAttachmentInitialSampleLocations[i].initialize(&in_struct->pAttachmentInitialSampleLocations[i]);
        }
    }
    if (postSubpassSampleLocationsCount && in_struct->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new safe_VkSubpassSampleLocationsEXT[postSubpassSampleLocationsCount];
        for (uint32_t i = 0; i < postSubpassSampleLocationsCount; ++i) {
            pPostSubpassSampleLocations[i].initialize(&in_struct->pPostSubpassSampleLocations[i]);
        }
    }
}

void safe_VkRenderPassSampleLocationsBeginInfoEXT::initialize(const safe_VkRenderPassSampleLocationsBeginInfoEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    attachmentInitialSampleLocationsCount = copy_src->attachmentInitialSampleLocationsCount;
    pAttachmentInitialSampleLocations = nullptr;
    postSubpassSampleLocationsCount = copy_src->postSubpassSampleLocationsCount;
    pPostSubpassSampleLocations = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (attachmentInitialSampleLocationsCount && copy_src->pAttachmentInitialSampleLocations) {
        pAttachmentInitialSampleLocations = new safe_VkAttachmentSampleLocationsEXT[attachmentInitialSampleLocationsCount];
        for (uint32_t i = 0; i < attachmentInitialSampleLocationsCount; ++i) {
            pAttachmentInitialSampleLocations[i].initialize(&copy_src->pAttachmentInitialSampleLocations[i]);
        }
    }
    if (postSubpassSampleLocationsCount && copy_src->pPostSubpassSampleLocations) {
        pPostSubpassSampleLocations = new safe_VkSubpassSampleLocationsEXT[postSubpassSampleLocationsCount];
        for (uint32_t i = 0; i < postSubpassSampleLocationsCount; ++i) {
            pPostSubpassSampleLocations[i].initialize(&copy_src->pPostSubpassSampleLocations[i]);
        }
    }
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT(
    const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      sampleLocationsEnable(in_struct->sampleLocationsEnable),
      sampleLocationsInfo(&in_struct->sampleLocationsInfo) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT), pNext(nullptr), sampleLocationsEnable() {}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::safe_VkPipelineSampleLocationsStateCreateInfoEXT(
    const safe_VkPipelineSampleLocationsStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    sampleLocationsEnable = copy_src.sampleLocationsEnable;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT& safe_VkPipelineSampleLocationsStateCreateInfoEXT::operator=(
    const safe_VkPipelineSampleLocationsStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleLocationsEnable = copy_src.sampleLocationsEnable;
    sampleLocationsInfo.initialize(&copy_src.sampleLocationsInfo);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineSampleLocationsStateCreateInfoEXT::~safe_VkPipelineSampleLocationsStateCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkPipelineSampleLocationsStateCreateInfoEXT::initialize(const VkPipelineSampleLocationsStateCreateInfoEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleLocationsEnable = in_struct->sampleLocationsEnable;
    sampleLocationsInfo.initialize(&in_struct->sampleLocationsInfo);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineSampleLocationsStateCreateInfoEXT::initialize(const safe_VkPipelineSampleLocationsStateCreateInfoEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleLocationsEnable = copy_src->sampleLocationsEnable;
    sampleLocationsInfo.initialize(&copy_src->sampleLocationsInfo);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(
    const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      sampleLocationSampleCounts(in_struct->sampleLocationSampleCounts),
      maxSampleLocationGridSize(in_struct->maxSampleLocationGridSize),
      sampleLocationSubPixelBits(in_struct->sampleLocationSubPixelBits),
      variableSampleLocations(in_struct->variableSampleLocations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 2; ++i) {
        sampleLocationCoordinateRange[i] = in_struct->sampleLocationCoordinateRange[i];
    }
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT),
      pNext(nullptr),
      sampleLocationSampleCounts(),
      maxSampleLocationGridSize(),
      sampleLocationSubPixelBits(),
      variableSampleLocations() {}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(
    const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    sampleLocationSampleCounts = copy_src.sampleLocationSampleCounts;
    maxSampleLocationGridSize = copy_src.maxSampleLocationGridSize;
    sampleLocationSubPixelBits = copy_src.sampleLocationSubPixelBits;
    variableSampleLocations = copy_src.variableSampleLocations;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        sampleLocationCoordinateRange[i] = copy_src.sampleLocationCoordinateRange[i];
    }
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleLocationSampleCounts = copy_src.sampleLocationSampleCounts;
    maxSampleLocationGridSize = copy_src.maxSampleLocationGridSize;
    sampleLocationSubPixelBits = copy_src.sampleLocationSubPixelBits;
    variableSampleLocations = copy_src.variableSampleLocations;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        sampleLocationCoordinateRange[i] = copy_src.sampleLocationCoordinateRange[i];
    }

    return *this;
}

safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::~safe_VkPhysicalDeviceSampleLocationsPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::initialize(const VkPhysicalDeviceSampleLocationsPropertiesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleLocationSampleCounts = in_struct->sampleLocationSampleCounts;
    maxSampleLocationGridSize = in_struct->maxSampleLocationGridSize;
    sampleLocationSubPixelBits = in_struct->sampleLocationSubPixelBits;
    variableSampleLocations = in_struct->variableSampleLocations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 2; ++i) {
        sampleLocationCoordinateRange[i] = in_struct->sampleLocationCoordinateRange[i];
    }
}

void safe_VkPhysicalDeviceSampleLocationsPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleLocationSampleCounts = copy_src->sampleLocationSampleCounts;
    maxSampleLocationGridSize = copy_src->maxSampleLocationGridSize;
    sampleLocationSubPixelBits = copy_src->sampleLocationSubPixelBits;
    variableSampleLocations = copy_src->variableSampleLocations;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        sampleLocationCoordinateRange[i] = copy_src->sampleLocationCoordinateRange[i];
    }
}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT(const VkMultisamplePropertiesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxSampleLocationGridSize(in_struct->maxSampleLocationGridSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT), pNext(nullptr), maxSampleLocationGridSize() {}

safe_VkMultisamplePropertiesEXT::safe_VkMultisamplePropertiesEXT(const safe_VkMultisamplePropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxSampleLocationGridSize = copy_src.maxSampleLocationGridSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMultisamplePropertiesEXT& safe_VkMultisamplePropertiesEXT::operator=(const safe_VkMultisamplePropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxSampleLocationGridSize = copy_src.maxSampleLocationGridSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMultisamplePropertiesEXT::~safe_VkMultisamplePropertiesEXT() { FreePnextChain(pNext); }

void safe_VkMultisamplePropertiesEXT::initialize(const VkMultisamplePropertiesEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxSampleLocationGridSize = in_struct->maxSampleLocationGridSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMultisamplePropertiesEXT::initialize(const safe_VkMultisamplePropertiesEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxSampleLocationGridSize = copy_src->maxSampleLocationGridSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(
    const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), advancedBlendCoherentOperations(in_struct->advancedBlendCoherentOperations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT),
      pNext(nullptr),
      advancedBlendCoherentOperations() {}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(
    const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    advancedBlendCoherentOperations = copy_src.advancedBlendCoherentOperations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    advancedBlendCoherentOperations = copy_src.advancedBlendCoherentOperations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::~safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::initialize(
    const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    advancedBlendCoherentOperations = in_struct->advancedBlendCoherentOperations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    advancedBlendCoherentOperations = copy_src->advancedBlendCoherentOperations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(
    const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      advancedBlendMaxColorAttachments(in_struct->advancedBlendMaxColorAttachments),
      advancedBlendIndependentBlend(in_struct->advancedBlendIndependentBlend),
      advancedBlendNonPremultipliedSrcColor(in_struct->advancedBlendNonPremultipliedSrcColor),
      advancedBlendNonPremultipliedDstColor(in_struct->advancedBlendNonPremultipliedDstColor),
      advancedBlendCorrelatedOverlap(in_struct->advancedBlendCorrelatedOverlap),
      advancedBlendAllOperations(in_struct->advancedBlendAllOperations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT),
      pNext(nullptr),
      advancedBlendMaxColorAttachments(),
      advancedBlendIndependentBlend(),
      advancedBlendNonPremultipliedSrcColor(),
      advancedBlendNonPremultipliedDstColor(),
      advancedBlendCorrelatedOverlap(),
      advancedBlendAllOperations() {}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(
    const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    advancedBlendMaxColorAttachments = copy_src.advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = copy_src.advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = copy_src.advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = copy_src.advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = copy_src.advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = copy_src.advancedBlendAllOperations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    advancedBlendMaxColorAttachments = copy_src.advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = copy_src.advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = copy_src.advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = copy_src.advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = copy_src.advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = copy_src.advancedBlendAllOperations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::~safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::initialize(
    const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    advancedBlendMaxColorAttachments = in_struct->advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = in_struct->advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = in_struct->advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = in_struct->advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = in_struct->advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = in_struct->advancedBlendAllOperations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    advancedBlendMaxColorAttachments = copy_src->advancedBlendMaxColorAttachments;
    advancedBlendIndependentBlend = copy_src->advancedBlendIndependentBlend;
    advancedBlendNonPremultipliedSrcColor = copy_src->advancedBlendNonPremultipliedSrcColor;
    advancedBlendNonPremultipliedDstColor = copy_src->advancedBlendNonPremultipliedDstColor;
    advancedBlendCorrelatedOverlap = copy_src->advancedBlendCorrelatedOverlap;
    advancedBlendAllOperations = copy_src->advancedBlendAllOperations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(
    const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      srcPremultiplied(in_struct->srcPremultiplied),
      dstPremultiplied(in_struct->dstPremultiplied),
      blendOverlap(in_struct->blendOverlap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT),
      pNext(nullptr),
      srcPremultiplied(),
      dstPremultiplied(),
      blendOverlap() {}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(
    const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    srcPremultiplied = copy_src.srcPremultiplied;
    dstPremultiplied = copy_src.dstPremultiplied;
    blendOverlap = copy_src.blendOverlap;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::operator=(
    const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    srcPremultiplied = copy_src.srcPremultiplied;
    dstPremultiplied = copy_src.dstPremultiplied;
    blendOverlap = copy_src.blendOverlap;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::~safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::initialize(
    const VkPipelineColorBlendAdvancedStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    srcPremultiplied = in_struct->srcPremultiplied;
    dstPremultiplied = in_struct->dstPremultiplied;
    blendOverlap = in_struct->blendOverlap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT::initialize(
    const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    srcPremultiplied = copy_src->srcPremultiplied;
    dstPremultiplied = copy_src->dstPremultiplied;
    blendOverlap = copy_src->blendOverlap;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT(
    const VkDrmFormatModifierPropertiesListEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), drmFormatModifierCount(in_struct->drmFormatModifierCount), pDrmFormatModifierProperties(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)in_struct->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierPropertiesEXT) * in_struct->drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT()
    : sType(VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT),
      pNext(nullptr),
      drmFormatModifierCount(),
      pDrmFormatModifierProperties(nullptr) {}

safe_VkDrmFormatModifierPropertiesListEXT::safe_VkDrmFormatModifierPropertiesListEXT(
    const safe_VkDrmFormatModifierPropertiesListEXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src.pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierPropertiesEXT) * copy_src.drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesListEXT& safe_VkDrmFormatModifierPropertiesListEXT::operator=(
    const safe_VkDrmFormatModifierPropertiesListEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src.pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierPropertiesEXT) * copy_src.drmFormatModifierCount);
    }

    return *this;
}

safe_VkDrmFormatModifierPropertiesListEXT::~safe_VkDrmFormatModifierPropertiesListEXT() {
    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);
}

void safe_VkDrmFormatModifierPropertiesListEXT::initialize(const VkDrmFormatModifierPropertiesListEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifierCount = in_struct->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)in_struct->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierPropertiesEXT) * in_struct->drmFormatModifierCount);
    }
}

void safe_VkDrmFormatModifierPropertiesListEXT::initialize(const safe_VkDrmFormatModifierPropertiesListEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifierCount = copy_src->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierPropertiesEXT[copy_src->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierPropertiesEXT) * copy_src->drmFormatModifierCount);
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(
    const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      drmFormatModifier(in_struct->drmFormatModifier),
      sharingMode(in_struct->sharingMode),
      queueFamilyIndexCount(0),
      pQueueFamilyIndices(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if ((in_struct->sharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT),
      pNext(nullptr),
      drmFormatModifier(),
      sharingMode(),
      queueFamilyIndexCount(),
      pQueueFamilyIndices(nullptr) {}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(
    const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    sharingMode = copy_src.sharingMode;
    pQueueFamilyIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.sharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::operator=(
    const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    sharingMode = copy_src.sharingMode;
    pQueueFamilyIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.sharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }

    return *this;
}

safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::~safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT() {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::initialize(const VkPhysicalDeviceImageDrmFormatModifierInfoEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifier = in_struct->drmFormatModifier;
    sharingMode = in_struct->sharingMode;
    pQueueFamilyIndices = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if ((in_struct->sharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

void safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT::initialize(
    const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifier = copy_src->drmFormatModifier;
    sharingMode = copy_src->sharingMode;
    pQueueFamilyIndices = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if ((copy_src->sharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src->pQueueFamilyIndices,
               sizeof(uint32_t) * copy_src->queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT(
    const VkImageDrmFormatModifierListCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), drmFormatModifierCount(in_struct->drmFormatModifierCount), pDrmFormatModifiers(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifiers, (void*)in_struct->pDrmFormatModifiers,
               sizeof(uint64_t) * in_struct->drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT),
      pNext(nullptr),
      drmFormatModifierCount(),
      pDrmFormatModifiers(nullptr) {}

safe_VkImageDrmFormatModifierListCreateInfoEXT::safe_VkImageDrmFormatModifierListCreateInfoEXT(
    const safe_VkImageDrmFormatModifierListCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifiers, (void*)copy_src.pDrmFormatModifiers, sizeof(uint64_t) * copy_src.drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierListCreateInfoEXT& safe_VkImageDrmFormatModifierListCreateInfoEXT::operator=(
    const safe_VkImageDrmFormatModifierListCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDrmFormatModifiers) delete[] pDrmFormatModifiers;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifiers, (void*)copy_src.pDrmFormatModifiers, sizeof(uint64_t) * copy_src.drmFormatModifierCount);
    }

    return *this;
}

safe_VkImageDrmFormatModifierListCreateInfoEXT::~safe_VkImageDrmFormatModifierListCreateInfoEXT() {
    if (pDrmFormatModifiers) delete[] pDrmFormatModifiers;
    FreePnextChain(pNext);
}

void safe_VkImageDrmFormatModifierListCreateInfoEXT::initialize(const VkImageDrmFormatModifierListCreateInfoEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDrmFormatModifiers) delete[] pDrmFormatModifiers;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifierCount = in_struct->drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifiers, (void*)in_struct->pDrmFormatModifiers,
               sizeof(uint64_t) * in_struct->drmFormatModifierCount);
    }
}

void safe_VkImageDrmFormatModifierListCreateInfoEXT::initialize(const safe_VkImageDrmFormatModifierListCreateInfoEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifierCount = copy_src->drmFormatModifierCount;
    pDrmFormatModifiers = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDrmFormatModifiers) {
        pDrmFormatModifiers = new uint64_t[copy_src->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifiers, (void*)copy_src->pDrmFormatModifiers,
               sizeof(uint64_t) * copy_src->drmFormatModifierCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(
    const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      drmFormatModifier(in_struct->drmFormatModifier),
      drmFormatModifierPlaneCount(in_struct->drmFormatModifierPlaneCount),
      pPlaneLayouts(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[in_struct->drmFormatModifierPlaneCount];
        memcpy((void*)pPlaneLayouts, (void*)in_struct->pPlaneLayouts,
               sizeof(VkSubresourceLayout) * in_struct->drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT),
      pNext(nullptr),
      drmFormatModifier(),
      drmFormatModifierPlaneCount(),
      pPlaneLayouts(nullptr) {}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(
    const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    drmFormatModifierPlaneCount = copy_src.drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[copy_src.drmFormatModifierPlaneCount];
        memcpy((void*)pPlaneLayouts, (void*)copy_src.pPlaneLayouts,
               sizeof(VkSubresourceLayout) * copy_src.drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::operator=(
    const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pPlaneLayouts) delete[] pPlaneLayouts;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    drmFormatModifierPlaneCount = copy_src.drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[copy_src.drmFormatModifierPlaneCount];
        memcpy((void*)pPlaneLayouts, (void*)copy_src.pPlaneLayouts,
               sizeof(VkSubresourceLayout) * copy_src.drmFormatModifierPlaneCount);
    }

    return *this;
}

safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::~safe_VkImageDrmFormatModifierExplicitCreateInfoEXT() {
    if (pPlaneLayouts) delete[] pPlaneLayouts;
    FreePnextChain(pNext);
}

void safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::initialize(const VkImageDrmFormatModifierExplicitCreateInfoEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPlaneLayouts) delete[] pPlaneLayouts;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifier = in_struct->drmFormatModifier;
    drmFormatModifierPlaneCount = in_struct->drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[in_struct->drmFormatModifierPlaneCount];
        memcpy((void*)pPlaneLayouts, (void*)in_struct->pPlaneLayouts,
               sizeof(VkSubresourceLayout) * in_struct->drmFormatModifierPlaneCount);
    }
}

void safe_VkImageDrmFormatModifierExplicitCreateInfoEXT::initialize(
    const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifier = copy_src->drmFormatModifier;
    drmFormatModifierPlaneCount = copy_src->drmFormatModifierPlaneCount;
    pPlaneLayouts = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPlaneLayouts) {
        pPlaneLayouts = new VkSubresourceLayout[copy_src->drmFormatModifierPlaneCount];
        memcpy((void*)pPlaneLayouts, (void*)copy_src->pPlaneLayouts,
               sizeof(VkSubresourceLayout) * copy_src->drmFormatModifierPlaneCount);
    }
}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT(
    const VkImageDrmFormatModifierPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), drmFormatModifier(in_struct->drmFormatModifier) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT), pNext(nullptr), drmFormatModifier() {}

safe_VkImageDrmFormatModifierPropertiesEXT::safe_VkImageDrmFormatModifierPropertiesEXT(
    const safe_VkImageDrmFormatModifierPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageDrmFormatModifierPropertiesEXT& safe_VkImageDrmFormatModifierPropertiesEXT::operator=(
    const safe_VkImageDrmFormatModifierPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifier = copy_src.drmFormatModifier;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageDrmFormatModifierPropertiesEXT::~safe_VkImageDrmFormatModifierPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkImageDrmFormatModifierPropertiesEXT::initialize(const VkImageDrmFormatModifierPropertiesEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifier = in_struct->drmFormatModifier;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageDrmFormatModifierPropertiesEXT::initialize(const safe_VkImageDrmFormatModifierPropertiesEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifier = copy_src->drmFormatModifier;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDrmFormatModifierPropertiesList2EXT::safe_VkDrmFormatModifierPropertiesList2EXT(
    const VkDrmFormatModifierPropertiesList2EXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), drmFormatModifierCount(in_struct->drmFormatModifierCount), pDrmFormatModifierProperties(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierProperties2EXT[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)in_struct->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierProperties2EXT) * in_struct->drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesList2EXT::safe_VkDrmFormatModifierPropertiesList2EXT()
    : sType(VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT),
      pNext(nullptr),
      drmFormatModifierCount(),
      pDrmFormatModifierProperties(nullptr) {}

safe_VkDrmFormatModifierPropertiesList2EXT::safe_VkDrmFormatModifierPropertiesList2EXT(
    const safe_VkDrmFormatModifierPropertiesList2EXT& copy_src) {
    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierProperties2EXT[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src.pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierProperties2EXT) * copy_src.drmFormatModifierCount);
    }
}

safe_VkDrmFormatModifierPropertiesList2EXT& safe_VkDrmFormatModifierPropertiesList2EXT::operator=(
    const safe_VkDrmFormatModifierPropertiesList2EXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    drmFormatModifierCount = copy_src.drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierProperties2EXT[copy_src.drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src.pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierProperties2EXT) * copy_src.drmFormatModifierCount);
    }

    return *this;
}

safe_VkDrmFormatModifierPropertiesList2EXT::~safe_VkDrmFormatModifierPropertiesList2EXT() {
    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);
}

void safe_VkDrmFormatModifierPropertiesList2EXT::initialize(const VkDrmFormatModifierPropertiesList2EXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDrmFormatModifierProperties) delete[] pDrmFormatModifierProperties;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    drmFormatModifierCount = in_struct->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierProperties2EXT[in_struct->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)in_struct->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierProperties2EXT) * in_struct->drmFormatModifierCount);
    }
}

void safe_VkDrmFormatModifierPropertiesList2EXT::initialize(const safe_VkDrmFormatModifierPropertiesList2EXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    drmFormatModifierCount = copy_src->drmFormatModifierCount;
    pDrmFormatModifierProperties = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDrmFormatModifierProperties) {
        pDrmFormatModifierProperties = new VkDrmFormatModifierProperties2EXT[copy_src->drmFormatModifierCount];
        memcpy((void*)pDrmFormatModifierProperties, (void*)copy_src->pDrmFormatModifierProperties,
               sizeof(VkDrmFormatModifierProperties2EXT) * copy_src->drmFormatModifierCount);
    }
}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT(const VkValidationCacheCreateInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      initialDataSize(in_struct->initialDataSize),
      pInitialData(in_struct->pInitialData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      initialDataSize(),
      pInitialData(nullptr) {}

safe_VkValidationCacheCreateInfoEXT::safe_VkValidationCacheCreateInfoEXT(const safe_VkValidationCacheCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    initialDataSize = copy_src.initialDataSize;
    pInitialData = copy_src.pInitialData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkValidationCacheCreateInfoEXT& safe_VkValidationCacheCreateInfoEXT::operator=(
    const safe_VkValidationCacheCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    initialDataSize = copy_src.initialDataSize;
    pInitialData = copy_src.pInitialData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkValidationCacheCreateInfoEXT::~safe_VkValidationCacheCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkValidationCacheCreateInfoEXT::initialize(const VkValidationCacheCreateInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    initialDataSize = in_struct->initialDataSize;
    pInitialData = in_struct->pInitialData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkValidationCacheCreateInfoEXT::initialize(const safe_VkValidationCacheCreateInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    initialDataSize = copy_src->initialDataSize;
    pInitialData = copy_src->pInitialData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT(
    const VkShaderModuleValidationCacheCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), validationCache(in_struct->validationCache) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT), pNext(nullptr), validationCache() {}

safe_VkShaderModuleValidationCacheCreateInfoEXT::safe_VkShaderModuleValidationCacheCreateInfoEXT(
    const safe_VkShaderModuleValidationCacheCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    validationCache = copy_src.validationCache;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkShaderModuleValidationCacheCreateInfoEXT& safe_VkShaderModuleValidationCacheCreateInfoEXT::operator=(
    const safe_VkShaderModuleValidationCacheCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    validationCache = copy_src.validationCache;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkShaderModuleValidationCacheCreateInfoEXT::~safe_VkShaderModuleValidationCacheCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkShaderModuleValidationCacheCreateInfoEXT::initialize(const VkShaderModuleValidationCacheCreateInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    validationCache = in_struct->validationCache;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkShaderModuleValidationCacheCreateInfoEXT::initialize(const safe_VkShaderModuleValidationCacheCreateInfoEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    validationCache = copy_src->validationCache;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(
    const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageViewType(in_struct->imageViewType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT), pNext(nullptr), imageViewType() {}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(
    const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& copy_src) {
    sType = copy_src.sType;
    imageViewType = copy_src.imageViewType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::operator=(
    const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageViewType = copy_src.imageViewType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::~safe_VkPhysicalDeviceImageViewImageFormatInfoEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::initialize(const VkPhysicalDeviceImageViewImageFormatInfoEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageViewType = in_struct->imageViewType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageViewImageFormatInfoEXT::initialize(const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageViewType = copy_src->imageViewType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT(
    const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), filterCubic(in_struct->filterCubic), filterCubicMinmax(in_struct->filterCubicMinmax) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT),
      pNext(nullptr),
      filterCubic(),
      filterCubicMinmax() {}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::safe_VkFilterCubicImageViewImageFormatPropertiesEXT(
    const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    filterCubic = copy_src.filterCubic;
    filterCubicMinmax = copy_src.filterCubicMinmax;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT& safe_VkFilterCubicImageViewImageFormatPropertiesEXT::operator=(
    const safe_VkFilterCubicImageViewImageFormatPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    filterCubic = copy_src.filterCubic;
    filterCubicMinmax = copy_src.filterCubicMinmax;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFilterCubicImageViewImageFormatPropertiesEXT::~safe_VkFilterCubicImageViewImageFormatPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkFilterCubicImageViewImageFormatPropertiesEXT::initialize(
    const VkFilterCubicImageViewImageFormatPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    filterCubic = in_struct->filterCubic;
    filterCubicMinmax = in_struct->filterCubicMinmax;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFilterCubicImageViewImageFormatPropertiesEXT::initialize(
    const safe_VkFilterCubicImageViewImageFormatPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    filterCubic = copy_src->filterCubic;
    filterCubicMinmax = copy_src->filterCubicMinmax;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT(const VkImportMemoryHostPointerInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), pHostPointer(in_struct->pHostPointer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT), pNext(nullptr), handleType(), pHostPointer(nullptr) {}

safe_VkImportMemoryHostPointerInfoEXT::safe_VkImportMemoryHostPointerInfoEXT(
    const safe_VkImportMemoryHostPointerInfoEXT& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    pHostPointer = copy_src.pHostPointer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryHostPointerInfoEXT& safe_VkImportMemoryHostPointerInfoEXT::operator=(
    const safe_VkImportMemoryHostPointerInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    pHostPointer = copy_src.pHostPointer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryHostPointerInfoEXT::~safe_VkImportMemoryHostPointerInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMemoryHostPointerInfoEXT::initialize(const VkImportMemoryHostPointerInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    pHostPointer = in_struct->pHostPointer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryHostPointerInfoEXT::initialize(const safe_VkImportMemoryHostPointerInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    pHostPointer = copy_src->pHostPointer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT(const VkMemoryHostPointerPropertiesEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryHostPointerPropertiesEXT::safe_VkMemoryHostPointerPropertiesEXT(
    const safe_VkMemoryHostPointerPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryHostPointerPropertiesEXT& safe_VkMemoryHostPointerPropertiesEXT::operator=(
    const safe_VkMemoryHostPointerPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryHostPointerPropertiesEXT::~safe_VkMemoryHostPointerPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkMemoryHostPointerPropertiesEXT::initialize(const VkMemoryHostPointerPropertiesEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryHostPointerPropertiesEXT::initialize(const safe_VkMemoryHostPointerPropertiesEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(
    const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minImportedHostPointerAlignment(in_struct->minImportedHostPointerAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT),
      pNext(nullptr),
      minImportedHostPointerAlignment() {}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(
    const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    minImportedHostPointerAlignment = copy_src.minImportedHostPointerAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minImportedHostPointerAlignment = copy_src.minImportedHostPointerAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::~safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::initialize(
    const VkPhysicalDeviceExternalMemoryHostPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minImportedHostPointerAlignment = in_struct->minImportedHostPointerAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minImportedHostPointerAlignment = copy_src->minImportedHostPointerAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(
    const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), maxVertexAttribDivisor(in_struct->maxVertexAttribDivisor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT), pNext(nullptr), maxVertexAttribDivisor() {}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(
    const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxVertexAttribDivisor = copy_src.maxVertexAttribDivisor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxVertexAttribDivisor = copy_src.maxVertexAttribDivisor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::~safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::initialize(
    const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxVertexAttribDivisor = in_struct->maxVertexAttribDivisor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxVertexAttribDivisor = copy_src->maxVertexAttribDivisor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(
    const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pciDomain(in_struct->pciDomain),
      pciBus(in_struct->pciBus),
      pciDevice(in_struct->pciDevice),
      pciFunction(in_struct->pciFunction) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT),
      pNext(nullptr),
      pciDomain(),
      pciBus(),
      pciDevice(),
      pciFunction() {}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(
    const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    pciDomain = copy_src.pciDomain;
    pciBus = copy_src.pciBus;
    pciDevice = copy_src.pciDevice;
    pciFunction = copy_src.pciFunction;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::operator=(
    const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pciDomain = copy_src.pciDomain;
    pciBus = copy_src.pciBus;
    pciDevice = copy_src.pciDevice;
    pciFunction = copy_src.pciFunction;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::~safe_VkPhysicalDevicePCIBusInfoPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::initialize(const VkPhysicalDevicePCIBusInfoPropertiesEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pciDomain = in_struct->pciDomain;
    pciBus = in_struct->pciBus;
    pciDevice = in_struct->pciDevice;
    pciFunction = in_struct->pciFunction;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePCIBusInfoPropertiesEXT::initialize(const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pciDomain = copy_src->pciDomain;
    pciBus = copy_src->pciBus;
    pciDevice = copy_src->pciDevice;
    pciFunction = copy_src->pciFunction;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(
    const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      fragmentDensityMap(in_struct->fragmentDensityMap),
      fragmentDensityMapDynamic(in_struct->fragmentDensityMapDynamic),
      fragmentDensityMapNonSubsampledImages(in_struct->fragmentDensityMapNonSubsampledImages) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT),
      pNext(nullptr),
      fragmentDensityMap(),
      fragmentDensityMapDynamic(),
      fragmentDensityMapNonSubsampledImages() {}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityMap = copy_src.fragmentDensityMap;
    fragmentDensityMapDynamic = copy_src.fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = copy_src.fragmentDensityMapNonSubsampledImages;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityMap = copy_src.fragmentDensityMap;
    fragmentDensityMapDynamic = copy_src.fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = copy_src.fragmentDensityMapNonSubsampledImages;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::~safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::initialize(const VkPhysicalDeviceFragmentDensityMapFeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityMap = in_struct->fragmentDensityMap;
    fragmentDensityMapDynamic = in_struct->fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = in_struct->fragmentDensityMapNonSubsampledImages;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityMap = copy_src->fragmentDensityMap;
    fragmentDensityMapDynamic = copy_src->fragmentDensityMapDynamic;
    fragmentDensityMapNonSubsampledImages = copy_src->fragmentDensityMapNonSubsampledImages;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(
    const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minFragmentDensityTexelSize(in_struct->minFragmentDensityTexelSize),
      maxFragmentDensityTexelSize(in_struct->maxFragmentDensityTexelSize),
      fragmentDensityInvocations(in_struct->fragmentDensityInvocations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT),
      pNext(nullptr),
      minFragmentDensityTexelSize(),
      maxFragmentDensityTexelSize(),
      fragmentDensityInvocations() {}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    minFragmentDensityTexelSize = copy_src.minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = copy_src.maxFragmentDensityTexelSize;
    fragmentDensityInvocations = copy_src.fragmentDensityInvocations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minFragmentDensityTexelSize = copy_src.minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = copy_src.maxFragmentDensityTexelSize;
    fragmentDensityInvocations = copy_src.fragmentDensityInvocations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::~safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::initialize(
    const VkPhysicalDeviceFragmentDensityMapPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minFragmentDensityTexelSize = in_struct->minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = in_struct->maxFragmentDensityTexelSize;
    fragmentDensityInvocations = in_struct->fragmentDensityInvocations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minFragmentDensityTexelSize = copy_src->minFragmentDensityTexelSize;
    maxFragmentDensityTexelSize = copy_src->maxFragmentDensityTexelSize;
    fragmentDensityInvocations = copy_src->fragmentDensityInvocations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT(
    const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityMapAttachment(in_struct->fragmentDensityMapAttachment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT), pNext(nullptr), fragmentDensityMapAttachment() {}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::safe_VkRenderPassFragmentDensityMapCreateInfoEXT(
    const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityMapAttachment = copy_src.fragmentDensityMapAttachment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT& safe_VkRenderPassFragmentDensityMapCreateInfoEXT::operator=(
    const safe_VkRenderPassFragmentDensityMapCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityMapAttachment = copy_src.fragmentDensityMapAttachment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderPassFragmentDensityMapCreateInfoEXT::~safe_VkRenderPassFragmentDensityMapCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkRenderPassFragmentDensityMapCreateInfoEXT::initialize(const VkRenderPassFragmentDensityMapCreateInfoEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityMapAttachment = in_struct->fragmentDensityMapAttachment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderPassFragmentDensityMapCreateInfoEXT::initialize(const safe_VkRenderPassFragmentDensityMapCreateInfoEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityMapAttachment = copy_src->fragmentDensityMapAttachment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::safe_VkRenderingFragmentDensityMapAttachmentInfoEXT(
    const VkRenderingFragmentDensityMapAttachmentInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageView(in_struct->imageView), imageLayout(in_struct->imageLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::safe_VkRenderingFragmentDensityMapAttachmentInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT), pNext(nullptr), imageView(), imageLayout() {}

safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::safe_VkRenderingFragmentDensityMapAttachmentInfoEXT(
    const safe_VkRenderingFragmentDensityMapAttachmentInfoEXT& copy_src) {
    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderingFragmentDensityMapAttachmentInfoEXT& safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::operator=(
    const safe_VkRenderingFragmentDensityMapAttachmentInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::~safe_VkRenderingFragmentDensityMapAttachmentInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::initialize(
    const VkRenderingFragmentDensityMapAttachmentInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageView = in_struct->imageView;
    imageLayout = in_struct->imageLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderingFragmentDensityMapAttachmentInfoEXT::initialize(
    const safe_VkRenderingFragmentDensityMapAttachmentInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageView = copy_src->imageView;
    imageLayout = copy_src->imageLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(
    const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      shaderImageInt64Atomics(in_struct->shaderImageInt64Atomics),
      sparseImageInt64Atomics(in_struct->sparseImageInt64Atomics) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT),
      pNext(nullptr),
      shaderImageInt64Atomics(),
      sparseImageInt64Atomics() {}

safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(
    const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderImageInt64Atomics = copy_src.shaderImageInt64Atomics;
    sparseImageInt64Atomics = copy_src.sparseImageInt64Atomics;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderImageInt64Atomics = copy_src.shaderImageInt64Atomics;
    sparseImageInt64Atomics = copy_src.sparseImageInt64Atomics;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::~safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::initialize(
    const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderImageInt64Atomics = in_struct->shaderImageInt64Atomics;
    sparseImageInt64Atomics = in_struct->sparseImageInt64Atomics;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::initialize(
    const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderImageInt64Atomics = copy_src->shaderImageInt64Atomics;
    sparseImageInt64Atomics = copy_src->sparseImageInt64Atomics;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(
    const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = in_struct->heapBudget[i];
    }

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = in_struct->heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT), pNext(nullptr) {}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(
    const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = copy_src.heapBudget[i];
    }

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = copy_src.heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = copy_src.heapBudget[i];
    }

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = copy_src.heapUsage[i];
    }

    return *this;
}

safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::~safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::initialize(const VkPhysicalDeviceMemoryBudgetPropertiesEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = in_struct->heapBudget[i];
    }

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = in_struct->heapUsage[i];
    }
}

void safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT::initialize(const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapBudget[i] = copy_src->heapBudget[i];
    }

    for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
        heapUsage[i] = copy_src->heapUsage[i];
    }
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(
    const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryPriority(in_struct->memoryPriority) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT), pNext(nullptr), memoryPriority() {}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(
    const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    memoryPriority = copy_src.memoryPriority;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryPriority = copy_src.memoryPriority;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::~safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::initialize(const VkPhysicalDeviceMemoryPriorityFeaturesEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryPriority = in_struct->memoryPriority;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT::initialize(const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryPriority = copy_src->memoryPriority;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT(const VkMemoryPriorityAllocateInfoEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), priority(in_struct->priority) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT), pNext(nullptr), priority() {}

safe_VkMemoryPriorityAllocateInfoEXT::safe_VkMemoryPriorityAllocateInfoEXT(const safe_VkMemoryPriorityAllocateInfoEXT& copy_src) {
    sType = copy_src.sType;
    priority = copy_src.priority;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryPriorityAllocateInfoEXT& safe_VkMemoryPriorityAllocateInfoEXT::operator=(
    const safe_VkMemoryPriorityAllocateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    priority = copy_src.priority;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryPriorityAllocateInfoEXT::~safe_VkMemoryPriorityAllocateInfoEXT() { FreePnextChain(pNext); }

void safe_VkMemoryPriorityAllocateInfoEXT::initialize(const VkMemoryPriorityAllocateInfoEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    priority = in_struct->priority;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryPriorityAllocateInfoEXT::initialize(const safe_VkMemoryPriorityAllocateInfoEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    priority = copy_src->priority;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(
    const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      bufferDeviceAddress(in_struct->bufferDeviceAddress),
      bufferDeviceAddressCaptureReplay(in_struct->bufferDeviceAddressCaptureReplay),
      bufferDeviceAddressMultiDevice(in_struct->bufferDeviceAddressMultiDevice) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT),
      pNext(nullptr),
      bufferDeviceAddress(),
      bufferDeviceAddressCaptureReplay(),
      bufferDeviceAddressMultiDevice() {}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(
    const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    bufferDeviceAddress = copy_src.bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = copy_src.bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = copy_src.bufferDeviceAddressMultiDevice;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    bufferDeviceAddress = copy_src.bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = copy_src.bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = copy_src.bufferDeviceAddressMultiDevice;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::~safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::initialize(
    const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    bufferDeviceAddress = in_struct->bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = in_struct->bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = in_struct->bufferDeviceAddressMultiDevice;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    bufferDeviceAddress = copy_src->bufferDeviceAddress;
    bufferDeviceAddressCaptureReplay = copy_src->bufferDeviceAddressCaptureReplay;
    bufferDeviceAddressMultiDevice = copy_src->bufferDeviceAddressMultiDevice;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT(
    const VkBufferDeviceAddressCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), deviceAddress(in_struct->deviceAddress) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT), pNext(nullptr), deviceAddress() {}

safe_VkBufferDeviceAddressCreateInfoEXT::safe_VkBufferDeviceAddressCreateInfoEXT(
    const safe_VkBufferDeviceAddressCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferDeviceAddressCreateInfoEXT& safe_VkBufferDeviceAddressCreateInfoEXT::operator=(
    const safe_VkBufferDeviceAddressCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferDeviceAddressCreateInfoEXT::~safe_VkBufferDeviceAddressCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkBufferDeviceAddressCreateInfoEXT::initialize(const VkBufferDeviceAddressCreateInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceAddress = in_struct->deviceAddress;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferDeviceAddressCreateInfoEXT::initialize(const safe_VkBufferDeviceAddressCreateInfoEXT* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceAddress = copy_src->deviceAddress;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT(const VkValidationFeaturesEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      enabledValidationFeatureCount(in_struct->enabledValidationFeatureCount),
      pEnabledValidationFeatures(nullptr),
      disabledValidationFeatureCount(in_struct->disabledValidationFeatureCount),
      pDisabledValidationFeatures(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[in_struct->enabledValidationFeatureCount];
        memcpy((void*)pEnabledValidationFeatures, (void*)in_struct->pEnabledValidationFeatures,
               sizeof(VkValidationFeatureEnableEXT) * in_struct->enabledValidationFeatureCount);
    }

    if (in_struct->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[in_struct->disabledValidationFeatureCount];
        memcpy((void*)pDisabledValidationFeatures, (void*)in_struct->pDisabledValidationFeatures,
               sizeof(VkValidationFeatureDisableEXT) * in_struct->disabledValidationFeatureCount);
    }
}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT),
      pNext(nullptr),
      enabledValidationFeatureCount(),
      pEnabledValidationFeatures(nullptr),
      disabledValidationFeatureCount(),
      pDisabledValidationFeatures(nullptr) {}

safe_VkValidationFeaturesEXT::safe_VkValidationFeaturesEXT(const safe_VkValidationFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    enabledValidationFeatureCount = copy_src.enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = copy_src.disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[copy_src.enabledValidationFeatureCount];
        memcpy((void*)pEnabledValidationFeatures, (void*)copy_src.pEnabledValidationFeatures,
               sizeof(VkValidationFeatureEnableEXT) * copy_src.enabledValidationFeatureCount);
    }

    if (copy_src.pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[copy_src.disabledValidationFeatureCount];
        memcpy((void*)pDisabledValidationFeatures, (void*)copy_src.pDisabledValidationFeatures,
               sizeof(VkValidationFeatureDisableEXT) * copy_src.disabledValidationFeatureCount);
    }
}

safe_VkValidationFeaturesEXT& safe_VkValidationFeaturesEXT::operator=(const safe_VkValidationFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pEnabledValidationFeatures) delete[] pEnabledValidationFeatures;
    if (pDisabledValidationFeatures) delete[] pDisabledValidationFeatures;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    enabledValidationFeatureCount = copy_src.enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = copy_src.disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[copy_src.enabledValidationFeatureCount];
        memcpy((void*)pEnabledValidationFeatures, (void*)copy_src.pEnabledValidationFeatures,
               sizeof(VkValidationFeatureEnableEXT) * copy_src.enabledValidationFeatureCount);
    }

    if (copy_src.pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[copy_src.disabledValidationFeatureCount];
        memcpy((void*)pDisabledValidationFeatures, (void*)copy_src.pDisabledValidationFeatures,
               sizeof(VkValidationFeatureDisableEXT) * copy_src.disabledValidationFeatureCount);
    }

    return *this;
}

safe_VkValidationFeaturesEXT::~safe_VkValidationFeaturesEXT() {
    if (pEnabledValidationFeatures) delete[] pEnabledValidationFeatures;
    if (pDisabledValidationFeatures) delete[] pDisabledValidationFeatures;
    FreePnextChain(pNext);
}

void safe_VkValidationFeaturesEXT::initialize(const VkValidationFeaturesEXT* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pEnabledValidationFeatures) delete[] pEnabledValidationFeatures;
    if (pDisabledValidationFeatures) delete[] pDisabledValidationFeatures;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    enabledValidationFeatureCount = in_struct->enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = in_struct->disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[in_struct->enabledValidationFeatureCount];
        memcpy((void*)pEnabledValidationFeatures, (void*)in_struct->pEnabledValidationFeatures,
               sizeof(VkValidationFeatureEnableEXT) * in_struct->enabledValidationFeatureCount);
    }

    if (in_struct->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[in_struct->disabledValidationFeatureCount];
        memcpy((void*)pDisabledValidationFeatures, (void*)in_struct->pDisabledValidationFeatures,
               sizeof(VkValidationFeatureDisableEXT) * in_struct->disabledValidationFeatureCount);
    }
}

void safe_VkValidationFeaturesEXT::initialize(const safe_VkValidationFeaturesEXT* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    enabledValidationFeatureCount = copy_src->enabledValidationFeatureCount;
    pEnabledValidationFeatures = nullptr;
    disabledValidationFeatureCount = copy_src->disabledValidationFeatureCount;
    pDisabledValidationFeatures = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pEnabledValidationFeatures) {
        pEnabledValidationFeatures = new VkValidationFeatureEnableEXT[copy_src->enabledValidationFeatureCount];
        memcpy((void*)pEnabledValidationFeatures, (void*)copy_src->pEnabledValidationFeatures,
               sizeof(VkValidationFeatureEnableEXT) * copy_src->enabledValidationFeatureCount);
    }

    if (copy_src->pDisabledValidationFeatures) {
        pDisabledValidationFeatures = new VkValidationFeatureDisableEXT[copy_src->disabledValidationFeatureCount];
        memcpy((void*)pDisabledValidationFeatures, (void*)copy_src->pDisabledValidationFeatures,
               sizeof(VkValidationFeatureDisableEXT) * copy_src->disabledValidationFeatureCount);
    }
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(
    const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      fragmentShaderSampleInterlock(in_struct->fragmentShaderSampleInterlock),
      fragmentShaderPixelInterlock(in_struct->fragmentShaderPixelInterlock),
      fragmentShaderShadingRateInterlock(in_struct->fragmentShaderShadingRateInterlock) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT),
      pNext(nullptr),
      fragmentShaderSampleInterlock(),
      fragmentShaderPixelInterlock(),
      fragmentShaderShadingRateInterlock() {}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(
    const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    fragmentShaderSampleInterlock = copy_src.fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = copy_src.fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = copy_src.fragmentShaderShadingRateInterlock;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentShaderSampleInterlock = copy_src.fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = copy_src.fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = copy_src.fragmentShaderShadingRateInterlock;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::~safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::initialize(
    const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentShaderSampleInterlock = in_struct->fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = in_struct->fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = in_struct->fragmentShaderShadingRateInterlock;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentShaderSampleInterlock = copy_src->fragmentShaderSampleInterlock;
    fragmentShaderPixelInterlock = copy_src->fragmentShaderPixelInterlock;
    fragmentShaderShadingRateInterlock = copy_src->fragmentShaderShadingRateInterlock;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(
    const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), ycbcrImageArrays(in_struct->ycbcrImageArrays) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT), pNext(nullptr), ycbcrImageArrays() {}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(
    const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    ycbcrImageArrays = copy_src.ycbcrImageArrays;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    ycbcrImageArrays = copy_src.ycbcrImageArrays;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::~safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::initialize(const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    ycbcrImageArrays = in_struct->ycbcrImageArrays;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::initialize(const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    ycbcrImageArrays = copy_src->ycbcrImageArrays;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::safe_VkPhysicalDeviceProvokingVertexFeaturesEXT(
    const VkPhysicalDeviceProvokingVertexFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      provokingVertexLast(in_struct->provokingVertexLast),
      transformFeedbackPreservesProvokingVertex(in_struct->transformFeedbackPreservesProvokingVertex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::safe_VkPhysicalDeviceProvokingVertexFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT),
      pNext(nullptr),
      provokingVertexLast(),
      transformFeedbackPreservesProvokingVertex() {}

safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::safe_VkPhysicalDeviceProvokingVertexFeaturesEXT(
    const safe_VkPhysicalDeviceProvokingVertexFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    provokingVertexLast = copy_src.provokingVertexLast;
    transformFeedbackPreservesProvokingVertex = copy_src.transformFeedbackPreservesProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceProvokingVertexFeaturesEXT& safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceProvokingVertexFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    provokingVertexLast = copy_src.provokingVertexLast;
    transformFeedbackPreservesProvokingVertex = copy_src.transformFeedbackPreservesProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::~safe_VkPhysicalDeviceProvokingVertexFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::initialize(const VkPhysicalDeviceProvokingVertexFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    provokingVertexLast = in_struct->provokingVertexLast;
    transformFeedbackPreservesProvokingVertex = in_struct->transformFeedbackPreservesProvokingVertex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceProvokingVertexFeaturesEXT::initialize(const safe_VkPhysicalDeviceProvokingVertexFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    provokingVertexLast = copy_src->provokingVertexLast;
    transformFeedbackPreservesProvokingVertex = copy_src->transformFeedbackPreservesProvokingVertex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::safe_VkPhysicalDeviceProvokingVertexPropertiesEXT(
    const VkPhysicalDeviceProvokingVertexPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      provokingVertexModePerPipeline(in_struct->provokingVertexModePerPipeline),
      transformFeedbackPreservesTriangleFanProvokingVertex(in_struct->transformFeedbackPreservesTriangleFanProvokingVertex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::safe_VkPhysicalDeviceProvokingVertexPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT),
      pNext(nullptr),
      provokingVertexModePerPipeline(),
      transformFeedbackPreservesTriangleFanProvokingVertex() {}

safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::safe_VkPhysicalDeviceProvokingVertexPropertiesEXT(
    const safe_VkPhysicalDeviceProvokingVertexPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    provokingVertexModePerPipeline = copy_src.provokingVertexModePerPipeline;
    transformFeedbackPreservesTriangleFanProvokingVertex = copy_src.transformFeedbackPreservesTriangleFanProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceProvokingVertexPropertiesEXT& safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceProvokingVertexPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    provokingVertexModePerPipeline = copy_src.provokingVertexModePerPipeline;
    transformFeedbackPreservesTriangleFanProvokingVertex = copy_src.transformFeedbackPreservesTriangleFanProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::~safe_VkPhysicalDeviceProvokingVertexPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::initialize(const VkPhysicalDeviceProvokingVertexPropertiesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    provokingVertexModePerPipeline = in_struct->provokingVertexModePerPipeline;
    transformFeedbackPreservesTriangleFanProvokingVertex = in_struct->transformFeedbackPreservesTriangleFanProvokingVertex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceProvokingVertexPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceProvokingVertexPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    provokingVertexModePerPipeline = copy_src->provokingVertexModePerPipeline;
    transformFeedbackPreservesTriangleFanProvokingVertex = copy_src->transformFeedbackPreservesTriangleFanProvokingVertex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT(
    const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), provokingVertexMode(in_struct->provokingVertexMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT),
      pNext(nullptr),
      provokingVertexMode() {}

safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT(
    const safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    provokingVertexMode = copy_src.provokingVertexMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT&
safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::operator=(
    const safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    provokingVertexMode = copy_src.provokingVertexMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::~safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::initialize(
    const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    provokingVertexMode = in_struct->provokingVertexMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT::initialize(
    const safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    provokingVertexMode = copy_src->provokingVertexMode;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT(
    const VkSurfaceFullScreenExclusiveInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fullScreenExclusive(in_struct->fullScreenExclusive) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT), pNext(nullptr), fullScreenExclusive() {}

safe_VkSurfaceFullScreenExclusiveInfoEXT::safe_VkSurfaceFullScreenExclusiveInfoEXT(
    const safe_VkSurfaceFullScreenExclusiveInfoEXT& copy_src) {
    sType = copy_src.sType;
    fullScreenExclusive = copy_src.fullScreenExclusive;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceFullScreenExclusiveInfoEXT& safe_VkSurfaceFullScreenExclusiveInfoEXT::operator=(
    const safe_VkSurfaceFullScreenExclusiveInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fullScreenExclusive = copy_src.fullScreenExclusive;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceFullScreenExclusiveInfoEXT::~safe_VkSurfaceFullScreenExclusiveInfoEXT() { FreePnextChain(pNext); }

void safe_VkSurfaceFullScreenExclusiveInfoEXT::initialize(const VkSurfaceFullScreenExclusiveInfoEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fullScreenExclusive = in_struct->fullScreenExclusive;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceFullScreenExclusiveInfoEXT::initialize(const safe_VkSurfaceFullScreenExclusiveInfoEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fullScreenExclusive = copy_src->fullScreenExclusive;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(
    const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fullScreenExclusiveSupported(in_struct->fullScreenExclusiveSupported) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT), pNext(nullptr), fullScreenExclusiveSupported() {}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(
    const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& copy_src) {
    sType = copy_src.sType;
    fullScreenExclusiveSupported = copy_src.fullScreenExclusiveSupported;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::operator=(
    const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fullScreenExclusiveSupported = copy_src.fullScreenExclusiveSupported;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::~safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::initialize(const VkSurfaceCapabilitiesFullScreenExclusiveEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fullScreenExclusiveSupported = in_struct->fullScreenExclusiveSupported;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT::initialize(const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fullScreenExclusiveSupported = copy_src->fullScreenExclusiveSupported;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(
    const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), hmonitor(in_struct->hmonitor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT()
    : sType(VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT), pNext(nullptr), hmonitor() {}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(
    const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& copy_src) {
    sType = copy_src.sType;
    hmonitor = copy_src.hmonitor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::operator=(
    const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hmonitor = copy_src.hmonitor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::~safe_VkSurfaceFullScreenExclusiveWin32InfoEXT() { FreePnextChain(pNext); }

void safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::initialize(const VkSurfaceFullScreenExclusiveWin32InfoEXT* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hmonitor = in_struct->hmonitor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceFullScreenExclusiveWin32InfoEXT::initialize(const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hmonitor = copy_src->hmonitor;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT(const VkHeadlessSurfaceCreateInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT), pNext(nullptr), flags() {}

safe_VkHeadlessSurfaceCreateInfoEXT::safe_VkHeadlessSurfaceCreateInfoEXT(const safe_VkHeadlessSurfaceCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkHeadlessSurfaceCreateInfoEXT& safe_VkHeadlessSurfaceCreateInfoEXT::operator=(
    const safe_VkHeadlessSurfaceCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkHeadlessSurfaceCreateInfoEXT::~safe_VkHeadlessSurfaceCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkHeadlessSurfaceCreateInfoEXT::initialize(const VkHeadlessSurfaceCreateInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkHeadlessSurfaceCreateInfoEXT::initialize(const safe_VkHeadlessSurfaceCreateInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(
    const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderBufferFloat32Atomics(in_struct->shaderBufferFloat32Atomics),
      shaderBufferFloat32AtomicAdd(in_struct->shaderBufferFloat32AtomicAdd),
      shaderBufferFloat64Atomics(in_struct->shaderBufferFloat64Atomics),
      shaderBufferFloat64AtomicAdd(in_struct->shaderBufferFloat64AtomicAdd),
      shaderSharedFloat32Atomics(in_struct->shaderSharedFloat32Atomics),
      shaderSharedFloat32AtomicAdd(in_struct->shaderSharedFloat32AtomicAdd),
      shaderSharedFloat64Atomics(in_struct->shaderSharedFloat64Atomics),
      shaderSharedFloat64AtomicAdd(in_struct->shaderSharedFloat64AtomicAdd),
      shaderImageFloat32Atomics(in_struct->shaderImageFloat32Atomics),
      shaderImageFloat32AtomicAdd(in_struct->shaderImageFloat32AtomicAdd),
      sparseImageFloat32Atomics(in_struct->sparseImageFloat32Atomics),
      sparseImageFloat32AtomicAdd(in_struct->sparseImageFloat32AtomicAdd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT),
      pNext(nullptr),
      shaderBufferFloat32Atomics(),
      shaderBufferFloat32AtomicAdd(),
      shaderBufferFloat64Atomics(),
      shaderBufferFloat64AtomicAdd(),
      shaderSharedFloat32Atomics(),
      shaderSharedFloat32AtomicAdd(),
      shaderSharedFloat64Atomics(),
      shaderSharedFloat64AtomicAdd(),
      shaderImageFloat32Atomics(),
      shaderImageFloat32AtomicAdd(),
      sparseImageFloat32Atomics(),
      sparseImageFloat32AtomicAdd() {}

safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(
    const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderBufferFloat32Atomics = copy_src.shaderBufferFloat32Atomics;
    shaderBufferFloat32AtomicAdd = copy_src.shaderBufferFloat32AtomicAdd;
    shaderBufferFloat64Atomics = copy_src.shaderBufferFloat64Atomics;
    shaderBufferFloat64AtomicAdd = copy_src.shaderBufferFloat64AtomicAdd;
    shaderSharedFloat32Atomics = copy_src.shaderSharedFloat32Atomics;
    shaderSharedFloat32AtomicAdd = copy_src.shaderSharedFloat32AtomicAdd;
    shaderSharedFloat64Atomics = copy_src.shaderSharedFloat64Atomics;
    shaderSharedFloat64AtomicAdd = copy_src.shaderSharedFloat64AtomicAdd;
    shaderImageFloat32Atomics = copy_src.shaderImageFloat32Atomics;
    shaderImageFloat32AtomicAdd = copy_src.shaderImageFloat32AtomicAdd;
    sparseImageFloat32Atomics = copy_src.sparseImageFloat32Atomics;
    sparseImageFloat32AtomicAdd = copy_src.sparseImageFloat32AtomicAdd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderBufferFloat32Atomics = copy_src.shaderBufferFloat32Atomics;
    shaderBufferFloat32AtomicAdd = copy_src.shaderBufferFloat32AtomicAdd;
    shaderBufferFloat64Atomics = copy_src.shaderBufferFloat64Atomics;
    shaderBufferFloat64AtomicAdd = copy_src.shaderBufferFloat64AtomicAdd;
    shaderSharedFloat32Atomics = copy_src.shaderSharedFloat32Atomics;
    shaderSharedFloat32AtomicAdd = copy_src.shaderSharedFloat32AtomicAdd;
    shaderSharedFloat64Atomics = copy_src.shaderSharedFloat64Atomics;
    shaderSharedFloat64AtomicAdd = copy_src.shaderSharedFloat64AtomicAdd;
    shaderImageFloat32Atomics = copy_src.shaderImageFloat32Atomics;
    shaderImageFloat32AtomicAdd = copy_src.shaderImageFloat32AtomicAdd;
    sparseImageFloat32Atomics = copy_src.sparseImageFloat32Atomics;
    sparseImageFloat32AtomicAdd = copy_src.sparseImageFloat32AtomicAdd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::~safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::initialize(const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderBufferFloat32Atomics = in_struct->shaderBufferFloat32Atomics;
    shaderBufferFloat32AtomicAdd = in_struct->shaderBufferFloat32AtomicAdd;
    shaderBufferFloat64Atomics = in_struct->shaderBufferFloat64Atomics;
    shaderBufferFloat64AtomicAdd = in_struct->shaderBufferFloat64AtomicAdd;
    shaderSharedFloat32Atomics = in_struct->shaderSharedFloat32Atomics;
    shaderSharedFloat32AtomicAdd = in_struct->shaderSharedFloat32AtomicAdd;
    shaderSharedFloat64Atomics = in_struct->shaderSharedFloat64Atomics;
    shaderSharedFloat64AtomicAdd = in_struct->shaderSharedFloat64AtomicAdd;
    shaderImageFloat32Atomics = in_struct->shaderImageFloat32Atomics;
    shaderImageFloat32AtomicAdd = in_struct->shaderImageFloat32AtomicAdd;
    sparseImageFloat32Atomics = in_struct->sparseImageFloat32Atomics;
    sparseImageFloat32AtomicAdd = in_struct->sparseImageFloat32AtomicAdd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderBufferFloat32Atomics = copy_src->shaderBufferFloat32Atomics;
    shaderBufferFloat32AtomicAdd = copy_src->shaderBufferFloat32AtomicAdd;
    shaderBufferFloat64Atomics = copy_src->shaderBufferFloat64Atomics;
    shaderBufferFloat64AtomicAdd = copy_src->shaderBufferFloat64AtomicAdd;
    shaderSharedFloat32Atomics = copy_src->shaderSharedFloat32Atomics;
    shaderSharedFloat32AtomicAdd = copy_src->shaderSharedFloat32AtomicAdd;
    shaderSharedFloat64Atomics = copy_src->shaderSharedFloat64Atomics;
    shaderSharedFloat64AtomicAdd = copy_src->shaderSharedFloat64AtomicAdd;
    shaderImageFloat32Atomics = copy_src->shaderImageFloat32Atomics;
    shaderImageFloat32AtomicAdd = copy_src->shaderImageFloat32AtomicAdd;
    sparseImageFloat32Atomics = copy_src->sparseImageFloat32Atomics;
    sparseImageFloat32AtomicAdd = copy_src->sparseImageFloat32AtomicAdd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(
    const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), extendedDynamicState(in_struct->extendedDynamicState) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT), pNext(nullptr), extendedDynamicState() {}

safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(
    const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    extendedDynamicState = copy_src.extendedDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    extendedDynamicState = copy_src.extendedDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::~safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::initialize(
    const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    extendedDynamicState = in_struct->extendedDynamicState;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    extendedDynamicState = copy_src->extendedDynamicState;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT(
    const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      memoryMapPlaced(in_struct->memoryMapPlaced),
      memoryMapRangePlaced(in_struct->memoryMapRangePlaced),
      memoryUnmapReserve(in_struct->memoryUnmapReserve) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_FEATURES_EXT),
      pNext(nullptr),
      memoryMapPlaced(),
      memoryMapRangePlaced(),
      memoryUnmapReserve() {}

safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT(
    const safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    memoryMapPlaced = copy_src.memoryMapPlaced;
    memoryMapRangePlaced = copy_src.memoryMapRangePlaced;
    memoryUnmapReserve = copy_src.memoryUnmapReserve;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT& safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryMapPlaced = copy_src.memoryMapPlaced;
    memoryMapRangePlaced = copy_src.memoryMapRangePlaced;
    memoryUnmapReserve = copy_src.memoryUnmapReserve;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::~safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::initialize(const VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryMapPlaced = in_struct->memoryMapPlaced;
    memoryMapRangePlaced = in_struct->memoryMapRangePlaced;
    memoryUnmapReserve = in_struct->memoryUnmapReserve;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::initialize(const safe_VkPhysicalDeviceMapMemoryPlacedFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryMapPlaced = copy_src->memoryMapPlaced;
    memoryMapRangePlaced = copy_src->memoryMapRangePlaced;
    memoryUnmapReserve = copy_src->memoryUnmapReserve;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT(
    const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minPlacedMemoryMapAlignment(in_struct->minPlacedMemoryMapAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAP_MEMORY_PLACED_PROPERTIES_EXT), pNext(nullptr), minPlacedMemoryMapAlignment() {}

safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT(
    const safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    minPlacedMemoryMapAlignment = copy_src.minPlacedMemoryMapAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT& safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minPlacedMemoryMapAlignment = copy_src.minPlacedMemoryMapAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::~safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::initialize(const VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minPlacedMemoryMapAlignment = in_struct->minPlacedMemoryMapAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceMapMemoryPlacedPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minPlacedMemoryMapAlignment = copy_src->minPlacedMemoryMapAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryMapPlacedInfoEXT::safe_VkMemoryMapPlacedInfoEXT(const VkMemoryMapPlacedInfoEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pPlacedAddress(in_struct->pPlacedAddress) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryMapPlacedInfoEXT::safe_VkMemoryMapPlacedInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MEMORY_MAP_PLACED_INFO_EXT), pNext(nullptr), pPlacedAddress(nullptr) {}

safe_VkMemoryMapPlacedInfoEXT::safe_VkMemoryMapPlacedInfoEXT(const safe_VkMemoryMapPlacedInfoEXT& copy_src) {
    sType = copy_src.sType;
    pPlacedAddress = copy_src.pPlacedAddress;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryMapPlacedInfoEXT& safe_VkMemoryMapPlacedInfoEXT::operator=(const safe_VkMemoryMapPlacedInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pPlacedAddress = copy_src.pPlacedAddress;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryMapPlacedInfoEXT::~safe_VkMemoryMapPlacedInfoEXT() { FreePnextChain(pNext); }

void safe_VkMemoryMapPlacedInfoEXT::initialize(const VkMemoryMapPlacedInfoEXT* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pPlacedAddress = in_struct->pPlacedAddress;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryMapPlacedInfoEXT::initialize(const safe_VkMemoryMapPlacedInfoEXT* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pPlacedAddress = copy_src->pPlacedAddress;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(
    const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderBufferFloat16Atomics(in_struct->shaderBufferFloat16Atomics),
      shaderBufferFloat16AtomicAdd(in_struct->shaderBufferFloat16AtomicAdd),
      shaderBufferFloat16AtomicMinMax(in_struct->shaderBufferFloat16AtomicMinMax),
      shaderBufferFloat32AtomicMinMax(in_struct->shaderBufferFloat32AtomicMinMax),
      shaderBufferFloat64AtomicMinMax(in_struct->shaderBufferFloat64AtomicMinMax),
      shaderSharedFloat16Atomics(in_struct->shaderSharedFloat16Atomics),
      shaderSharedFloat16AtomicAdd(in_struct->shaderSharedFloat16AtomicAdd),
      shaderSharedFloat16AtomicMinMax(in_struct->shaderSharedFloat16AtomicMinMax),
      shaderSharedFloat32AtomicMinMax(in_struct->shaderSharedFloat32AtomicMinMax),
      shaderSharedFloat64AtomicMinMax(in_struct->shaderSharedFloat64AtomicMinMax),
      shaderImageFloat32AtomicMinMax(in_struct->shaderImageFloat32AtomicMinMax),
      sparseImageFloat32AtomicMinMax(in_struct->sparseImageFloat32AtomicMinMax) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT),
      pNext(nullptr),
      shaderBufferFloat16Atomics(),
      shaderBufferFloat16AtomicAdd(),
      shaderBufferFloat16AtomicMinMax(),
      shaderBufferFloat32AtomicMinMax(),
      shaderBufferFloat64AtomicMinMax(),
      shaderSharedFloat16Atomics(),
      shaderSharedFloat16AtomicAdd(),
      shaderSharedFloat16AtomicMinMax(),
      shaderSharedFloat32AtomicMinMax(),
      shaderSharedFloat64AtomicMinMax(),
      shaderImageFloat32AtomicMinMax(),
      sparseImageFloat32AtomicMinMax() {}

safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(
    const safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderBufferFloat16Atomics = copy_src.shaderBufferFloat16Atomics;
    shaderBufferFloat16AtomicAdd = copy_src.shaderBufferFloat16AtomicAdd;
    shaderBufferFloat16AtomicMinMax = copy_src.shaderBufferFloat16AtomicMinMax;
    shaderBufferFloat32AtomicMinMax = copy_src.shaderBufferFloat32AtomicMinMax;
    shaderBufferFloat64AtomicMinMax = copy_src.shaderBufferFloat64AtomicMinMax;
    shaderSharedFloat16Atomics = copy_src.shaderSharedFloat16Atomics;
    shaderSharedFloat16AtomicAdd = copy_src.shaderSharedFloat16AtomicAdd;
    shaderSharedFloat16AtomicMinMax = copy_src.shaderSharedFloat16AtomicMinMax;
    shaderSharedFloat32AtomicMinMax = copy_src.shaderSharedFloat32AtomicMinMax;
    shaderSharedFloat64AtomicMinMax = copy_src.shaderSharedFloat64AtomicMinMax;
    shaderImageFloat32AtomicMinMax = copy_src.shaderImageFloat32AtomicMinMax;
    sparseImageFloat32AtomicMinMax = copy_src.sparseImageFloat32AtomicMinMax;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT& safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderBufferFloat16Atomics = copy_src.shaderBufferFloat16Atomics;
    shaderBufferFloat16AtomicAdd = copy_src.shaderBufferFloat16AtomicAdd;
    shaderBufferFloat16AtomicMinMax = copy_src.shaderBufferFloat16AtomicMinMax;
    shaderBufferFloat32AtomicMinMax = copy_src.shaderBufferFloat32AtomicMinMax;
    shaderBufferFloat64AtomicMinMax = copy_src.shaderBufferFloat64AtomicMinMax;
    shaderSharedFloat16Atomics = copy_src.shaderSharedFloat16Atomics;
    shaderSharedFloat16AtomicAdd = copy_src.shaderSharedFloat16AtomicAdd;
    shaderSharedFloat16AtomicMinMax = copy_src.shaderSharedFloat16AtomicMinMax;
    shaderSharedFloat32AtomicMinMax = copy_src.shaderSharedFloat32AtomicMinMax;
    shaderSharedFloat64AtomicMinMax = copy_src.shaderSharedFloat64AtomicMinMax;
    shaderImageFloat32AtomicMinMax = copy_src.shaderImageFloat32AtomicMinMax;
    sparseImageFloat32AtomicMinMax = copy_src.sparseImageFloat32AtomicMinMax;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::~safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::initialize(const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderBufferFloat16Atomics = in_struct->shaderBufferFloat16Atomics;
    shaderBufferFloat16AtomicAdd = in_struct->shaderBufferFloat16AtomicAdd;
    shaderBufferFloat16AtomicMinMax = in_struct->shaderBufferFloat16AtomicMinMax;
    shaderBufferFloat32AtomicMinMax = in_struct->shaderBufferFloat32AtomicMinMax;
    shaderBufferFloat64AtomicMinMax = in_struct->shaderBufferFloat64AtomicMinMax;
    shaderSharedFloat16Atomics = in_struct->shaderSharedFloat16Atomics;
    shaderSharedFloat16AtomicAdd = in_struct->shaderSharedFloat16AtomicAdd;
    shaderSharedFloat16AtomicMinMax = in_struct->shaderSharedFloat16AtomicMinMax;
    shaderSharedFloat32AtomicMinMax = in_struct->shaderSharedFloat32AtomicMinMax;
    shaderSharedFloat64AtomicMinMax = in_struct->shaderSharedFloat64AtomicMinMax;
    shaderImageFloat32AtomicMinMax = in_struct->shaderImageFloat32AtomicMinMax;
    sparseImageFloat32AtomicMinMax = in_struct->sparseImageFloat32AtomicMinMax;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::initialize(
    const safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderBufferFloat16Atomics = copy_src->shaderBufferFloat16Atomics;
    shaderBufferFloat16AtomicAdd = copy_src->shaderBufferFloat16AtomicAdd;
    shaderBufferFloat16AtomicMinMax = copy_src->shaderBufferFloat16AtomicMinMax;
    shaderBufferFloat32AtomicMinMax = copy_src->shaderBufferFloat32AtomicMinMax;
    shaderBufferFloat64AtomicMinMax = copy_src->shaderBufferFloat64AtomicMinMax;
    shaderSharedFloat16Atomics = copy_src->shaderSharedFloat16Atomics;
    shaderSharedFloat16AtomicAdd = copy_src->shaderSharedFloat16AtomicAdd;
    shaderSharedFloat16AtomicMinMax = copy_src->shaderSharedFloat16AtomicMinMax;
    shaderSharedFloat32AtomicMinMax = copy_src->shaderSharedFloat32AtomicMinMax;
    shaderSharedFloat64AtomicMinMax = copy_src->shaderSharedFloat64AtomicMinMax;
    shaderImageFloat32AtomicMinMax = copy_src->shaderImageFloat32AtomicMinMax;
    sparseImageFloat32AtomicMinMax = copy_src->sparseImageFloat32AtomicMinMax;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(
    const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), texelBufferAlignment(in_struct->texelBufferAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT), pNext(nullptr), texelBufferAlignment() {}

safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(
    const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    texelBufferAlignment = copy_src.texelBufferAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    texelBufferAlignment = copy_src.texelBufferAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::~safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::initialize(
    const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    texelBufferAlignment = in_struct->texelBufferAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    texelBufferAlignment = copy_src->texelBufferAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT(
    const VkPhysicalDeviceDepthBiasControlFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      depthBiasControl(in_struct->depthBiasControl),
      leastRepresentableValueForceUnormRepresentation(in_struct->leastRepresentableValueForceUnormRepresentation),
      floatRepresentation(in_struct->floatRepresentation),
      depthBiasExact(in_struct->depthBiasExact) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT),
      pNext(nullptr),
      depthBiasControl(),
      leastRepresentableValueForceUnormRepresentation(),
      floatRepresentation(),
      depthBiasExact() {}

safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT(
    const safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    depthBiasControl = copy_src.depthBiasControl;
    leastRepresentableValueForceUnormRepresentation = copy_src.leastRepresentableValueForceUnormRepresentation;
    floatRepresentation = copy_src.floatRepresentation;
    depthBiasExact = copy_src.depthBiasExact;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT& safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthBiasControl = copy_src.depthBiasControl;
    leastRepresentableValueForceUnormRepresentation = copy_src.leastRepresentableValueForceUnormRepresentation;
    floatRepresentation = copy_src.floatRepresentation;
    depthBiasExact = copy_src.depthBiasExact;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::~safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::initialize(const VkPhysicalDeviceDepthBiasControlFeaturesEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthBiasControl = in_struct->depthBiasControl;
    leastRepresentableValueForceUnormRepresentation = in_struct->leastRepresentableValueForceUnormRepresentation;
    floatRepresentation = in_struct->floatRepresentation;
    depthBiasExact = in_struct->depthBiasExact;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT::initialize(const safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthBiasControl = copy_src->depthBiasControl;
    leastRepresentableValueForceUnormRepresentation = copy_src->leastRepresentableValueForceUnormRepresentation;
    floatRepresentation = copy_src->floatRepresentation;
    depthBiasExact = copy_src->depthBiasExact;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDepthBiasInfoEXT::safe_VkDepthBiasInfoEXT(const VkDepthBiasInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                                 bool copy_pnext)
    : sType(in_struct->sType),
      depthBiasConstantFactor(in_struct->depthBiasConstantFactor),
      depthBiasClamp(in_struct->depthBiasClamp),
      depthBiasSlopeFactor(in_struct->depthBiasSlopeFactor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDepthBiasInfoEXT::safe_VkDepthBiasInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT),
      pNext(nullptr),
      depthBiasConstantFactor(),
      depthBiasClamp(),
      depthBiasSlopeFactor() {}

safe_VkDepthBiasInfoEXT::safe_VkDepthBiasInfoEXT(const safe_VkDepthBiasInfoEXT& copy_src) {
    sType = copy_src.sType;
    depthBiasConstantFactor = copy_src.depthBiasConstantFactor;
    depthBiasClamp = copy_src.depthBiasClamp;
    depthBiasSlopeFactor = copy_src.depthBiasSlopeFactor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDepthBiasInfoEXT& safe_VkDepthBiasInfoEXT::operator=(const safe_VkDepthBiasInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthBiasConstantFactor = copy_src.depthBiasConstantFactor;
    depthBiasClamp = copy_src.depthBiasClamp;
    depthBiasSlopeFactor = copy_src.depthBiasSlopeFactor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDepthBiasInfoEXT::~safe_VkDepthBiasInfoEXT() { FreePnextChain(pNext); }

void safe_VkDepthBiasInfoEXT::initialize(const VkDepthBiasInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthBiasConstantFactor = in_struct->depthBiasConstantFactor;
    depthBiasClamp = in_struct->depthBiasClamp;
    depthBiasSlopeFactor = in_struct->depthBiasSlopeFactor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDepthBiasInfoEXT::initialize(const safe_VkDepthBiasInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthBiasConstantFactor = copy_src->depthBiasConstantFactor;
    depthBiasClamp = copy_src->depthBiasClamp;
    depthBiasSlopeFactor = copy_src->depthBiasSlopeFactor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDepthBiasRepresentationInfoEXT::safe_VkDepthBiasRepresentationInfoEXT(const VkDepthBiasRepresentationInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      depthBiasRepresentation(in_struct->depthBiasRepresentation),
      depthBiasExact(in_struct->depthBiasExact) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDepthBiasRepresentationInfoEXT::safe_VkDepthBiasRepresentationInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT), pNext(nullptr), depthBiasRepresentation(), depthBiasExact() {}

safe_VkDepthBiasRepresentationInfoEXT::safe_VkDepthBiasRepresentationInfoEXT(
    const safe_VkDepthBiasRepresentationInfoEXT& copy_src) {
    sType = copy_src.sType;
    depthBiasRepresentation = copy_src.depthBiasRepresentation;
    depthBiasExact = copy_src.depthBiasExact;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDepthBiasRepresentationInfoEXT& safe_VkDepthBiasRepresentationInfoEXT::operator=(
    const safe_VkDepthBiasRepresentationInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthBiasRepresentation = copy_src.depthBiasRepresentation;
    depthBiasExact = copy_src.depthBiasExact;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDepthBiasRepresentationInfoEXT::~safe_VkDepthBiasRepresentationInfoEXT() { FreePnextChain(pNext); }

void safe_VkDepthBiasRepresentationInfoEXT::initialize(const VkDepthBiasRepresentationInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthBiasRepresentation = in_struct->depthBiasRepresentation;
    depthBiasExact = in_struct->depthBiasExact;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDepthBiasRepresentationInfoEXT::initialize(const safe_VkDepthBiasRepresentationInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthBiasRepresentation = copy_src->depthBiasRepresentation;
    depthBiasExact = copy_src->depthBiasExact;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT(
    const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), deviceMemoryReport(in_struct->deviceMemoryReport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT), pNext(nullptr), deviceMemoryReport() {}

safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT(
    const safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    deviceMemoryReport = copy_src.deviceMemoryReport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT& safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceMemoryReport = copy_src.deviceMemoryReport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::~safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::initialize(const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceMemoryReport = in_struct->deviceMemoryReport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceMemoryReport = copy_src->deviceMemoryReport;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceMemoryReportCallbackDataEXT::safe_VkDeviceMemoryReportCallbackDataEXT(
    const VkDeviceMemoryReportCallbackDataEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      type(in_struct->type),
      memoryObjectId(in_struct->memoryObjectId),
      size(in_struct->size),
      objectType(in_struct->objectType),
      objectHandle(in_struct->objectHandle),
      heapIndex(in_struct->heapIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceMemoryReportCallbackDataEXT::safe_VkDeviceMemoryReportCallbackDataEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_MEMORY_REPORT_CALLBACK_DATA_EXT),
      pNext(nullptr),
      flags(),
      type(),
      memoryObjectId(),
      size(),
      objectType(),
      objectHandle(),
      heapIndex() {}

safe_VkDeviceMemoryReportCallbackDataEXT::safe_VkDeviceMemoryReportCallbackDataEXT(
    const safe_VkDeviceMemoryReportCallbackDataEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    type = copy_src.type;
    memoryObjectId = copy_src.memoryObjectId;
    size = copy_src.size;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    heapIndex = copy_src.heapIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceMemoryReportCallbackDataEXT& safe_VkDeviceMemoryReportCallbackDataEXT::operator=(
    const safe_VkDeviceMemoryReportCallbackDataEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    type = copy_src.type;
    memoryObjectId = copy_src.memoryObjectId;
    size = copy_src.size;
    objectType = copy_src.objectType;
    objectHandle = copy_src.objectHandle;
    heapIndex = copy_src.heapIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceMemoryReportCallbackDataEXT::~safe_VkDeviceMemoryReportCallbackDataEXT() { FreePnextChain(pNext); }

void safe_VkDeviceMemoryReportCallbackDataEXT::initialize(const VkDeviceMemoryReportCallbackDataEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    type = in_struct->type;
    memoryObjectId = in_struct->memoryObjectId;
    size = in_struct->size;
    objectType = in_struct->objectType;
    objectHandle = in_struct->objectHandle;
    heapIndex = in_struct->heapIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceMemoryReportCallbackDataEXT::initialize(const safe_VkDeviceMemoryReportCallbackDataEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    type = copy_src->type;
    memoryObjectId = copy_src->memoryObjectId;
    size = copy_src->size;
    objectType = copy_src->objectType;
    objectHandle = copy_src->objectHandle;
    heapIndex = copy_src->heapIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceDeviceMemoryReportCreateInfoEXT::safe_VkDeviceDeviceMemoryReportCreateInfoEXT(
    const VkDeviceDeviceMemoryReportCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      pfnUserCallback(in_struct->pfnUserCallback),
      pUserData(in_struct->pUserData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceDeviceMemoryReportCreateInfoEXT::safe_VkDeviceDeviceMemoryReportCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      pfnUserCallback(),
      pUserData(nullptr) {}

safe_VkDeviceDeviceMemoryReportCreateInfoEXT::safe_VkDeviceDeviceMemoryReportCreateInfoEXT(
    const safe_VkDeviceDeviceMemoryReportCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnUserCallback = copy_src.pfnUserCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceDeviceMemoryReportCreateInfoEXT& safe_VkDeviceDeviceMemoryReportCreateInfoEXT::operator=(
    const safe_VkDeviceDeviceMemoryReportCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnUserCallback = copy_src.pfnUserCallback;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceDeviceMemoryReportCreateInfoEXT::~safe_VkDeviceDeviceMemoryReportCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkDeviceDeviceMemoryReportCreateInfoEXT::initialize(const VkDeviceDeviceMemoryReportCreateInfoEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pfnUserCallback = in_struct->pfnUserCallback;
    pUserData = in_struct->pUserData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceDeviceMemoryReportCreateInfoEXT::initialize(const safe_VkDeviceDeviceMemoryReportCreateInfoEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pfnUserCallback = copy_src->pfnUserCallback;
    pUserData = copy_src->pUserData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerCustomBorderColorCreateInfoEXT::safe_VkSamplerCustomBorderColorCreateInfoEXT(
    const VkSamplerCustomBorderColorCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), customBorderColor(in_struct->customBorderColor), format(in_struct->format) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerCustomBorderColorCreateInfoEXT::safe_VkSamplerCustomBorderColorCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT), pNext(nullptr), customBorderColor(), format() {}

safe_VkSamplerCustomBorderColorCreateInfoEXT::safe_VkSamplerCustomBorderColorCreateInfoEXT(
    const safe_VkSamplerCustomBorderColorCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    customBorderColor = copy_src.customBorderColor;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerCustomBorderColorCreateInfoEXT& safe_VkSamplerCustomBorderColorCreateInfoEXT::operator=(
    const safe_VkSamplerCustomBorderColorCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    customBorderColor = copy_src.customBorderColor;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerCustomBorderColorCreateInfoEXT::~safe_VkSamplerCustomBorderColorCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkSamplerCustomBorderColorCreateInfoEXT::initialize(const VkSamplerCustomBorderColorCreateInfoEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    customBorderColor = in_struct->customBorderColor;
    format = in_struct->format;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerCustomBorderColorCreateInfoEXT::initialize(const safe_VkSamplerCustomBorderColorCreateInfoEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    customBorderColor = copy_src->customBorderColor;
    format = copy_src->format;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(
    const VkPhysicalDeviceCustomBorderColorPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxCustomBorderColorSamplers(in_struct->maxCustomBorderColorSamplers) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT), pNext(nullptr), maxCustomBorderColorSamplers() {}

safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(
    const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxCustomBorderColorSamplers = copy_src.maxCustomBorderColorSamplers;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxCustomBorderColorSamplers = copy_src.maxCustomBorderColorSamplers;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::~safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::initialize(
    const VkPhysicalDeviceCustomBorderColorPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxCustomBorderColorSamplers = in_struct->maxCustomBorderColorSamplers;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxCustomBorderColorSamplers = copy_src->maxCustomBorderColorSamplers;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(
    const VkPhysicalDeviceCustomBorderColorFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      customBorderColors(in_struct->customBorderColors),
      customBorderColorWithoutFormat(in_struct->customBorderColorWithoutFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT),
      pNext(nullptr),
      customBorderColors(),
      customBorderColorWithoutFormat() {}

safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(
    const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    customBorderColors = copy_src.customBorderColors;
    customBorderColorWithoutFormat = copy_src.customBorderColorWithoutFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    customBorderColors = copy_src.customBorderColors;
    customBorderColorWithoutFormat = copy_src.customBorderColorWithoutFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::~safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::initialize(const VkPhysicalDeviceCustomBorderColorFeaturesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    customBorderColors = in_struct->customBorderColors;
    customBorderColorWithoutFormat = in_struct->customBorderColorWithoutFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    customBorderColors = copy_src->customBorderColors;
    customBorderColorWithoutFormat = copy_src->customBorderColorWithoutFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_METAL_EXT

safe_VkExportMetalObjectCreateInfoEXT::safe_VkExportMetalObjectCreateInfoEXT(const VkExportMetalObjectCreateInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), exportObjectType(in_struct->exportObjectType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalObjectCreateInfoEXT::safe_VkExportMetalObjectCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT), pNext(nullptr), exportObjectType() {}

safe_VkExportMetalObjectCreateInfoEXT::safe_VkExportMetalObjectCreateInfoEXT(
    const safe_VkExportMetalObjectCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    exportObjectType = copy_src.exportObjectType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalObjectCreateInfoEXT& safe_VkExportMetalObjectCreateInfoEXT::operator=(
    const safe_VkExportMetalObjectCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    exportObjectType = copy_src.exportObjectType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalObjectCreateInfoEXT::~safe_VkExportMetalObjectCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalObjectCreateInfoEXT::initialize(const VkExportMetalObjectCreateInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    exportObjectType = in_struct->exportObjectType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalObjectCreateInfoEXT::initialize(const safe_VkExportMetalObjectCreateInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    exportObjectType = copy_src->exportObjectType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalObjectsInfoEXT::safe_VkExportMetalObjectsInfoEXT(const VkExportMetalObjectsInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalObjectsInfoEXT::safe_VkExportMetalObjectsInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECTS_INFO_EXT), pNext(nullptr) {}

safe_VkExportMetalObjectsInfoEXT::safe_VkExportMetalObjectsInfoEXT(const safe_VkExportMetalObjectsInfoEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalObjectsInfoEXT& safe_VkExportMetalObjectsInfoEXT::operator=(const safe_VkExportMetalObjectsInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalObjectsInfoEXT::~safe_VkExportMetalObjectsInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalObjectsInfoEXT::initialize(const VkExportMetalObjectsInfoEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalObjectsInfoEXT::initialize(const safe_VkExportMetalObjectsInfoEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalDeviceInfoEXT::safe_VkExportMetalDeviceInfoEXT(const VkExportMetalDeviceInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), mtlDevice(in_struct->mtlDevice) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalDeviceInfoEXT::safe_VkExportMetalDeviceInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT), pNext(nullptr), mtlDevice() {}

safe_VkExportMetalDeviceInfoEXT::safe_VkExportMetalDeviceInfoEXT(const safe_VkExportMetalDeviceInfoEXT& copy_src) {
    sType = copy_src.sType;
    mtlDevice = copy_src.mtlDevice;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalDeviceInfoEXT& safe_VkExportMetalDeviceInfoEXT::operator=(const safe_VkExportMetalDeviceInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mtlDevice = copy_src.mtlDevice;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalDeviceInfoEXT::~safe_VkExportMetalDeviceInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalDeviceInfoEXT::initialize(const VkExportMetalDeviceInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mtlDevice = in_struct->mtlDevice;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalDeviceInfoEXT::initialize(const safe_VkExportMetalDeviceInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mtlDevice = copy_src->mtlDevice;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalCommandQueueInfoEXT::safe_VkExportMetalCommandQueueInfoEXT(const VkExportMetalCommandQueueInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), queue(in_struct->queue), mtlCommandQueue(in_struct->mtlCommandQueue) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalCommandQueueInfoEXT::safe_VkExportMetalCommandQueueInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT), pNext(nullptr), queue(), mtlCommandQueue() {}

safe_VkExportMetalCommandQueueInfoEXT::safe_VkExportMetalCommandQueueInfoEXT(
    const safe_VkExportMetalCommandQueueInfoEXT& copy_src) {
    sType = copy_src.sType;
    queue = copy_src.queue;
    mtlCommandQueue = copy_src.mtlCommandQueue;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalCommandQueueInfoEXT& safe_VkExportMetalCommandQueueInfoEXT::operator=(
    const safe_VkExportMetalCommandQueueInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    queue = copy_src.queue;
    mtlCommandQueue = copy_src.mtlCommandQueue;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalCommandQueueInfoEXT::~safe_VkExportMetalCommandQueueInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalCommandQueueInfoEXT::initialize(const VkExportMetalCommandQueueInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queue = in_struct->queue;
    mtlCommandQueue = in_struct->mtlCommandQueue;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalCommandQueueInfoEXT::initialize(const safe_VkExportMetalCommandQueueInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queue = copy_src->queue;
    mtlCommandQueue = copy_src->mtlCommandQueue;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalBufferInfoEXT::safe_VkExportMetalBufferInfoEXT(const VkExportMetalBufferInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), mtlBuffer(in_struct->mtlBuffer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalBufferInfoEXT::safe_VkExportMetalBufferInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT), pNext(nullptr), memory(), mtlBuffer() {}

safe_VkExportMetalBufferInfoEXT::safe_VkExportMetalBufferInfoEXT(const safe_VkExportMetalBufferInfoEXT& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    mtlBuffer = copy_src.mtlBuffer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalBufferInfoEXT& safe_VkExportMetalBufferInfoEXT::operator=(const safe_VkExportMetalBufferInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    mtlBuffer = copy_src.mtlBuffer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalBufferInfoEXT::~safe_VkExportMetalBufferInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalBufferInfoEXT::initialize(const VkExportMetalBufferInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    mtlBuffer = in_struct->mtlBuffer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalBufferInfoEXT::initialize(const safe_VkExportMetalBufferInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    mtlBuffer = copy_src->mtlBuffer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMetalBufferInfoEXT::safe_VkImportMetalBufferInfoEXT(const VkImportMetalBufferInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), mtlBuffer(in_struct->mtlBuffer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMetalBufferInfoEXT::safe_VkImportMetalBufferInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT), pNext(nullptr), mtlBuffer() {}

safe_VkImportMetalBufferInfoEXT::safe_VkImportMetalBufferInfoEXT(const safe_VkImportMetalBufferInfoEXT& copy_src) {
    sType = copy_src.sType;
    mtlBuffer = copy_src.mtlBuffer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMetalBufferInfoEXT& safe_VkImportMetalBufferInfoEXT::operator=(const safe_VkImportMetalBufferInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mtlBuffer = copy_src.mtlBuffer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMetalBufferInfoEXT::~safe_VkImportMetalBufferInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMetalBufferInfoEXT::initialize(const VkImportMetalBufferInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mtlBuffer = in_struct->mtlBuffer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMetalBufferInfoEXT::initialize(const safe_VkImportMetalBufferInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mtlBuffer = copy_src->mtlBuffer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalTextureInfoEXT::safe_VkExportMetalTextureInfoEXT(const VkExportMetalTextureInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      image(in_struct->image),
      imageView(in_struct->imageView),
      bufferView(in_struct->bufferView),
      plane(in_struct->plane),
      mtlTexture(in_struct->mtlTexture) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalTextureInfoEXT::safe_VkExportMetalTextureInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT),
      pNext(nullptr),
      image(),
      imageView(),
      bufferView(),
      plane(),
      mtlTexture() {}

safe_VkExportMetalTextureInfoEXT::safe_VkExportMetalTextureInfoEXT(const safe_VkExportMetalTextureInfoEXT& copy_src) {
    sType = copy_src.sType;
    image = copy_src.image;
    imageView = copy_src.imageView;
    bufferView = copy_src.bufferView;
    plane = copy_src.plane;
    mtlTexture = copy_src.mtlTexture;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalTextureInfoEXT& safe_VkExportMetalTextureInfoEXT::operator=(const safe_VkExportMetalTextureInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image = copy_src.image;
    imageView = copy_src.imageView;
    bufferView = copy_src.bufferView;
    plane = copy_src.plane;
    mtlTexture = copy_src.mtlTexture;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalTextureInfoEXT::~safe_VkExportMetalTextureInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalTextureInfoEXT::initialize(const VkExportMetalTextureInfoEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image = in_struct->image;
    imageView = in_struct->imageView;
    bufferView = in_struct->bufferView;
    plane = in_struct->plane;
    mtlTexture = in_struct->mtlTexture;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalTextureInfoEXT::initialize(const safe_VkExportMetalTextureInfoEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image = copy_src->image;
    imageView = copy_src->imageView;
    bufferView = copy_src->bufferView;
    plane = copy_src->plane;
    mtlTexture = copy_src->mtlTexture;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMetalTextureInfoEXT::safe_VkImportMetalTextureInfoEXT(const VkImportMetalTextureInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), plane(in_struct->plane), mtlTexture(in_struct->mtlTexture) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMetalTextureInfoEXT::safe_VkImportMetalTextureInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT), pNext(nullptr), plane(), mtlTexture() {}

safe_VkImportMetalTextureInfoEXT::safe_VkImportMetalTextureInfoEXT(const safe_VkImportMetalTextureInfoEXT& copy_src) {
    sType = copy_src.sType;
    plane = copy_src.plane;
    mtlTexture = copy_src.mtlTexture;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMetalTextureInfoEXT& safe_VkImportMetalTextureInfoEXT::operator=(const safe_VkImportMetalTextureInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    plane = copy_src.plane;
    mtlTexture = copy_src.mtlTexture;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMetalTextureInfoEXT::~safe_VkImportMetalTextureInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMetalTextureInfoEXT::initialize(const VkImportMetalTextureInfoEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    plane = in_struct->plane;
    mtlTexture = in_struct->mtlTexture;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMetalTextureInfoEXT::initialize(const safe_VkImportMetalTextureInfoEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    plane = copy_src->plane;
    mtlTexture = copy_src->mtlTexture;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalIOSurfaceInfoEXT::safe_VkExportMetalIOSurfaceInfoEXT(const VkExportMetalIOSurfaceInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), image(in_struct->image), ioSurface(in_struct->ioSurface) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalIOSurfaceInfoEXT::safe_VkExportMetalIOSurfaceInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT), pNext(nullptr), image(), ioSurface() {}

safe_VkExportMetalIOSurfaceInfoEXT::safe_VkExportMetalIOSurfaceInfoEXT(const safe_VkExportMetalIOSurfaceInfoEXT& copy_src) {
    sType = copy_src.sType;
    image = copy_src.image;
    ioSurface = copy_src.ioSurface;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalIOSurfaceInfoEXT& safe_VkExportMetalIOSurfaceInfoEXT::operator=(
    const safe_VkExportMetalIOSurfaceInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image = copy_src.image;
    ioSurface = copy_src.ioSurface;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalIOSurfaceInfoEXT::~safe_VkExportMetalIOSurfaceInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalIOSurfaceInfoEXT::initialize(const VkExportMetalIOSurfaceInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image = in_struct->image;
    ioSurface = in_struct->ioSurface;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalIOSurfaceInfoEXT::initialize(const safe_VkExportMetalIOSurfaceInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image = copy_src->image;
    ioSurface = copy_src->ioSurface;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMetalIOSurfaceInfoEXT::safe_VkImportMetalIOSurfaceInfoEXT(const VkImportMetalIOSurfaceInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), ioSurface(in_struct->ioSurface) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMetalIOSurfaceInfoEXT::safe_VkImportMetalIOSurfaceInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT), pNext(nullptr), ioSurface() {}

safe_VkImportMetalIOSurfaceInfoEXT::safe_VkImportMetalIOSurfaceInfoEXT(const safe_VkImportMetalIOSurfaceInfoEXT& copy_src) {
    sType = copy_src.sType;
    ioSurface = copy_src.ioSurface;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMetalIOSurfaceInfoEXT& safe_VkImportMetalIOSurfaceInfoEXT::operator=(
    const safe_VkImportMetalIOSurfaceInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    ioSurface = copy_src.ioSurface;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMetalIOSurfaceInfoEXT::~safe_VkImportMetalIOSurfaceInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMetalIOSurfaceInfoEXT::initialize(const VkImportMetalIOSurfaceInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    ioSurface = in_struct->ioSurface;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMetalIOSurfaceInfoEXT::initialize(const safe_VkImportMetalIOSurfaceInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    ioSurface = copy_src->ioSurface;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMetalSharedEventInfoEXT::safe_VkExportMetalSharedEventInfoEXT(const VkExportMetalSharedEventInfoEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), event(in_struct->event), mtlSharedEvent(in_struct->mtlSharedEvent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMetalSharedEventInfoEXT::safe_VkExportMetalSharedEventInfoEXT()
    : sType(VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT), pNext(nullptr), semaphore(), event(), mtlSharedEvent() {}

safe_VkExportMetalSharedEventInfoEXT::safe_VkExportMetalSharedEventInfoEXT(const safe_VkExportMetalSharedEventInfoEXT& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    event = copy_src.event;
    mtlSharedEvent = copy_src.mtlSharedEvent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMetalSharedEventInfoEXT& safe_VkExportMetalSharedEventInfoEXT::operator=(
    const safe_VkExportMetalSharedEventInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    event = copy_src.event;
    mtlSharedEvent = copy_src.mtlSharedEvent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMetalSharedEventInfoEXT::~safe_VkExportMetalSharedEventInfoEXT() { FreePnextChain(pNext); }

void safe_VkExportMetalSharedEventInfoEXT::initialize(const VkExportMetalSharedEventInfoEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    event = in_struct->event;
    mtlSharedEvent = in_struct->mtlSharedEvent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMetalSharedEventInfoEXT::initialize(const safe_VkExportMetalSharedEventInfoEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    event = copy_src->event;
    mtlSharedEvent = copy_src->mtlSharedEvent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMetalSharedEventInfoEXT::safe_VkImportMetalSharedEventInfoEXT(const VkImportMetalSharedEventInfoEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), mtlSharedEvent(in_struct->mtlSharedEvent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMetalSharedEventInfoEXT::safe_VkImportMetalSharedEventInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT), pNext(nullptr), mtlSharedEvent() {}

safe_VkImportMetalSharedEventInfoEXT::safe_VkImportMetalSharedEventInfoEXT(const safe_VkImportMetalSharedEventInfoEXT& copy_src) {
    sType = copy_src.sType;
    mtlSharedEvent = copy_src.mtlSharedEvent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMetalSharedEventInfoEXT& safe_VkImportMetalSharedEventInfoEXT::operator=(
    const safe_VkImportMetalSharedEventInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mtlSharedEvent = copy_src.mtlSharedEvent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMetalSharedEventInfoEXT::~safe_VkImportMetalSharedEventInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMetalSharedEventInfoEXT::initialize(const VkImportMetalSharedEventInfoEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mtlSharedEvent = in_struct->mtlSharedEvent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMetalSharedEventInfoEXT::initialize(const safe_VkImportMetalSharedEventInfoEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mtlSharedEvent = copy_src->mtlSharedEvent;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT(
    const VkPhysicalDeviceDescriptorBufferPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      combinedImageSamplerDescriptorSingleArray(in_struct->combinedImageSamplerDescriptorSingleArray),
      bufferlessPushDescriptors(in_struct->bufferlessPushDescriptors),
      allowSamplerImageViewPostSubmitCreation(in_struct->allowSamplerImageViewPostSubmitCreation),
      descriptorBufferOffsetAlignment(in_struct->descriptorBufferOffsetAlignment),
      maxDescriptorBufferBindings(in_struct->maxDescriptorBufferBindings),
      maxResourceDescriptorBufferBindings(in_struct->maxResourceDescriptorBufferBindings),
      maxSamplerDescriptorBufferBindings(in_struct->maxSamplerDescriptorBufferBindings),
      maxEmbeddedImmutableSamplerBindings(in_struct->maxEmbeddedImmutableSamplerBindings),
      maxEmbeddedImmutableSamplers(in_struct->maxEmbeddedImmutableSamplers),
      bufferCaptureReplayDescriptorDataSize(in_struct->bufferCaptureReplayDescriptorDataSize),
      imageCaptureReplayDescriptorDataSize(in_struct->imageCaptureReplayDescriptorDataSize),
      imageViewCaptureReplayDescriptorDataSize(in_struct->imageViewCaptureReplayDescriptorDataSize),
      samplerCaptureReplayDescriptorDataSize(in_struct->samplerCaptureReplayDescriptorDataSize),
      accelerationStructureCaptureReplayDescriptorDataSize(in_struct->accelerationStructureCaptureReplayDescriptorDataSize),
      samplerDescriptorSize(in_struct->samplerDescriptorSize),
      combinedImageSamplerDescriptorSize(in_struct->combinedImageSamplerDescriptorSize),
      sampledImageDescriptorSize(in_struct->sampledImageDescriptorSize),
      storageImageDescriptorSize(in_struct->storageImageDescriptorSize),
      uniformTexelBufferDescriptorSize(in_struct->uniformTexelBufferDescriptorSize),
      robustUniformTexelBufferDescriptorSize(in_struct->robustUniformTexelBufferDescriptorSize),
      storageTexelBufferDescriptorSize(in_struct->storageTexelBufferDescriptorSize),
      robustStorageTexelBufferDescriptorSize(in_struct->robustStorageTexelBufferDescriptorSize),
      uniformBufferDescriptorSize(in_struct->uniformBufferDescriptorSize),
      robustUniformBufferDescriptorSize(in_struct->robustUniformBufferDescriptorSize),
      storageBufferDescriptorSize(in_struct->storageBufferDescriptorSize),
      robustStorageBufferDescriptorSize(in_struct->robustStorageBufferDescriptorSize),
      inputAttachmentDescriptorSize(in_struct->inputAttachmentDescriptorSize),
      accelerationStructureDescriptorSize(in_struct->accelerationStructureDescriptorSize),
      maxSamplerDescriptorBufferRange(in_struct->maxSamplerDescriptorBufferRange),
      maxResourceDescriptorBufferRange(in_struct->maxResourceDescriptorBufferRange),
      samplerDescriptorBufferAddressSpaceSize(in_struct->samplerDescriptorBufferAddressSpaceSize),
      resourceDescriptorBufferAddressSpaceSize(in_struct->resourceDescriptorBufferAddressSpaceSize),
      descriptorBufferAddressSpaceSize(in_struct->descriptorBufferAddressSpaceSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT),
      pNext(nullptr),
      combinedImageSamplerDescriptorSingleArray(),
      bufferlessPushDescriptors(),
      allowSamplerImageViewPostSubmitCreation(),
      descriptorBufferOffsetAlignment(),
      maxDescriptorBufferBindings(),
      maxResourceDescriptorBufferBindings(),
      maxSamplerDescriptorBufferBindings(),
      maxEmbeddedImmutableSamplerBindings(),
      maxEmbeddedImmutableSamplers(),
      bufferCaptureReplayDescriptorDataSize(),
      imageCaptureReplayDescriptorDataSize(),
      imageViewCaptureReplayDescriptorDataSize(),
      samplerCaptureReplayDescriptorDataSize(),
      accelerationStructureCaptureReplayDescriptorDataSize(),
      samplerDescriptorSize(),
      combinedImageSamplerDescriptorSize(),
      sampledImageDescriptorSize(),
      storageImageDescriptorSize(),
      uniformTexelBufferDescriptorSize(),
      robustUniformTexelBufferDescriptorSize(),
      storageTexelBufferDescriptorSize(),
      robustStorageTexelBufferDescriptorSize(),
      uniformBufferDescriptorSize(),
      robustUniformBufferDescriptorSize(),
      storageBufferDescriptorSize(),
      robustStorageBufferDescriptorSize(),
      inputAttachmentDescriptorSize(),
      accelerationStructureDescriptorSize(),
      maxSamplerDescriptorBufferRange(),
      maxResourceDescriptorBufferRange(),
      samplerDescriptorBufferAddressSpaceSize(),
      resourceDescriptorBufferAddressSpaceSize(),
      descriptorBufferAddressSpaceSize() {}

safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT(
    const safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    combinedImageSamplerDescriptorSingleArray = copy_src.combinedImageSamplerDescriptorSingleArray;
    bufferlessPushDescriptors = copy_src.bufferlessPushDescriptors;
    allowSamplerImageViewPostSubmitCreation = copy_src.allowSamplerImageViewPostSubmitCreation;
    descriptorBufferOffsetAlignment = copy_src.descriptorBufferOffsetAlignment;
    maxDescriptorBufferBindings = copy_src.maxDescriptorBufferBindings;
    maxResourceDescriptorBufferBindings = copy_src.maxResourceDescriptorBufferBindings;
    maxSamplerDescriptorBufferBindings = copy_src.maxSamplerDescriptorBufferBindings;
    maxEmbeddedImmutableSamplerBindings = copy_src.maxEmbeddedImmutableSamplerBindings;
    maxEmbeddedImmutableSamplers = copy_src.maxEmbeddedImmutableSamplers;
    bufferCaptureReplayDescriptorDataSize = copy_src.bufferCaptureReplayDescriptorDataSize;
    imageCaptureReplayDescriptorDataSize = copy_src.imageCaptureReplayDescriptorDataSize;
    imageViewCaptureReplayDescriptorDataSize = copy_src.imageViewCaptureReplayDescriptorDataSize;
    samplerCaptureReplayDescriptorDataSize = copy_src.samplerCaptureReplayDescriptorDataSize;
    accelerationStructureCaptureReplayDescriptorDataSize = copy_src.accelerationStructureCaptureReplayDescriptorDataSize;
    samplerDescriptorSize = copy_src.samplerDescriptorSize;
    combinedImageSamplerDescriptorSize = copy_src.combinedImageSamplerDescriptorSize;
    sampledImageDescriptorSize = copy_src.sampledImageDescriptorSize;
    storageImageDescriptorSize = copy_src.storageImageDescriptorSize;
    uniformTexelBufferDescriptorSize = copy_src.uniformTexelBufferDescriptorSize;
    robustUniformTexelBufferDescriptorSize = copy_src.robustUniformTexelBufferDescriptorSize;
    storageTexelBufferDescriptorSize = copy_src.storageTexelBufferDescriptorSize;
    robustStorageTexelBufferDescriptorSize = copy_src.robustStorageTexelBufferDescriptorSize;
    uniformBufferDescriptorSize = copy_src.uniformBufferDescriptorSize;
    robustUniformBufferDescriptorSize = copy_src.robustUniformBufferDescriptorSize;
    storageBufferDescriptorSize = copy_src.storageBufferDescriptorSize;
    robustStorageBufferDescriptorSize = copy_src.robustStorageBufferDescriptorSize;
    inputAttachmentDescriptorSize = copy_src.inputAttachmentDescriptorSize;
    accelerationStructureDescriptorSize = copy_src.accelerationStructureDescriptorSize;
    maxSamplerDescriptorBufferRange = copy_src.maxSamplerDescriptorBufferRange;
    maxResourceDescriptorBufferRange = copy_src.maxResourceDescriptorBufferRange;
    samplerDescriptorBufferAddressSpaceSize = copy_src.samplerDescriptorBufferAddressSpaceSize;
    resourceDescriptorBufferAddressSpaceSize = copy_src.resourceDescriptorBufferAddressSpaceSize;
    descriptorBufferAddressSpaceSize = copy_src.descriptorBufferAddressSpaceSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT& safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    combinedImageSamplerDescriptorSingleArray = copy_src.combinedImageSamplerDescriptorSingleArray;
    bufferlessPushDescriptors = copy_src.bufferlessPushDescriptors;
    allowSamplerImageViewPostSubmitCreation = copy_src.allowSamplerImageViewPostSubmitCreation;
    descriptorBufferOffsetAlignment = copy_src.descriptorBufferOffsetAlignment;
    maxDescriptorBufferBindings = copy_src.maxDescriptorBufferBindings;
    maxResourceDescriptorBufferBindings = copy_src.maxResourceDescriptorBufferBindings;
    maxSamplerDescriptorBufferBindings = copy_src.maxSamplerDescriptorBufferBindings;
    maxEmbeddedImmutableSamplerBindings = copy_src.maxEmbeddedImmutableSamplerBindings;
    maxEmbeddedImmutableSamplers = copy_src.maxEmbeddedImmutableSamplers;
    bufferCaptureReplayDescriptorDataSize = copy_src.bufferCaptureReplayDescriptorDataSize;
    imageCaptureReplayDescriptorDataSize = copy_src.imageCaptureReplayDescriptorDataSize;
    imageViewCaptureReplayDescriptorDataSize = copy_src.imageViewCaptureReplayDescriptorDataSize;
    samplerCaptureReplayDescriptorDataSize = copy_src.samplerCaptureReplayDescriptorDataSize;
    accelerationStructureCaptureReplayDescriptorDataSize = copy_src.accelerationStructureCaptureReplayDescriptorDataSize;
    samplerDescriptorSize = copy_src.samplerDescriptorSize;
    combinedImageSamplerDescriptorSize = copy_src.combinedImageSamplerDescriptorSize;
    sampledImageDescriptorSize = copy_src.sampledImageDescriptorSize;
    storageImageDescriptorSize = copy_src.storageImageDescriptorSize;
    uniformTexelBufferDescriptorSize = copy_src.uniformTexelBufferDescriptorSize;
    robustUniformTexelBufferDescriptorSize = copy_src.robustUniformTexelBufferDescriptorSize;
    storageTexelBufferDescriptorSize = copy_src.storageTexelBufferDescriptorSize;
    robustStorageTexelBufferDescriptorSize = copy_src.robustStorageTexelBufferDescriptorSize;
    uniformBufferDescriptorSize = copy_src.uniformBufferDescriptorSize;
    robustUniformBufferDescriptorSize = copy_src.robustUniformBufferDescriptorSize;
    storageBufferDescriptorSize = copy_src.storageBufferDescriptorSize;
    robustStorageBufferDescriptorSize = copy_src.robustStorageBufferDescriptorSize;
    inputAttachmentDescriptorSize = copy_src.inputAttachmentDescriptorSize;
    accelerationStructureDescriptorSize = copy_src.accelerationStructureDescriptorSize;
    maxSamplerDescriptorBufferRange = copy_src.maxSamplerDescriptorBufferRange;
    maxResourceDescriptorBufferRange = copy_src.maxResourceDescriptorBufferRange;
    samplerDescriptorBufferAddressSpaceSize = copy_src.samplerDescriptorBufferAddressSpaceSize;
    resourceDescriptorBufferAddressSpaceSize = copy_src.resourceDescriptorBufferAddressSpaceSize;
    descriptorBufferAddressSpaceSize = copy_src.descriptorBufferAddressSpaceSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::~safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::initialize(const VkPhysicalDeviceDescriptorBufferPropertiesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    combinedImageSamplerDescriptorSingleArray = in_struct->combinedImageSamplerDescriptorSingleArray;
    bufferlessPushDescriptors = in_struct->bufferlessPushDescriptors;
    allowSamplerImageViewPostSubmitCreation = in_struct->allowSamplerImageViewPostSubmitCreation;
    descriptorBufferOffsetAlignment = in_struct->descriptorBufferOffsetAlignment;
    maxDescriptorBufferBindings = in_struct->maxDescriptorBufferBindings;
    maxResourceDescriptorBufferBindings = in_struct->maxResourceDescriptorBufferBindings;
    maxSamplerDescriptorBufferBindings = in_struct->maxSamplerDescriptorBufferBindings;
    maxEmbeddedImmutableSamplerBindings = in_struct->maxEmbeddedImmutableSamplerBindings;
    maxEmbeddedImmutableSamplers = in_struct->maxEmbeddedImmutableSamplers;
    bufferCaptureReplayDescriptorDataSize = in_struct->bufferCaptureReplayDescriptorDataSize;
    imageCaptureReplayDescriptorDataSize = in_struct->imageCaptureReplayDescriptorDataSize;
    imageViewCaptureReplayDescriptorDataSize = in_struct->imageViewCaptureReplayDescriptorDataSize;
    samplerCaptureReplayDescriptorDataSize = in_struct->samplerCaptureReplayDescriptorDataSize;
    accelerationStructureCaptureReplayDescriptorDataSize = in_struct->accelerationStructureCaptureReplayDescriptorDataSize;
    samplerDescriptorSize = in_struct->samplerDescriptorSize;
    combinedImageSamplerDescriptorSize = in_struct->combinedImageSamplerDescriptorSize;
    sampledImageDescriptorSize = in_struct->sampledImageDescriptorSize;
    storageImageDescriptorSize = in_struct->storageImageDescriptorSize;
    uniformTexelBufferDescriptorSize = in_struct->uniformTexelBufferDescriptorSize;
    robustUniformTexelBufferDescriptorSize = in_struct->robustUniformTexelBufferDescriptorSize;
    storageTexelBufferDescriptorSize = in_struct->storageTexelBufferDescriptorSize;
    robustStorageTexelBufferDescriptorSize = in_struct->robustStorageTexelBufferDescriptorSize;
    uniformBufferDescriptorSize = in_struct->uniformBufferDescriptorSize;
    robustUniformBufferDescriptorSize = in_struct->robustUniformBufferDescriptorSize;
    storageBufferDescriptorSize = in_struct->storageBufferDescriptorSize;
    robustStorageBufferDescriptorSize = in_struct->robustStorageBufferDescriptorSize;
    inputAttachmentDescriptorSize = in_struct->inputAttachmentDescriptorSize;
    accelerationStructureDescriptorSize = in_struct->accelerationStructureDescriptorSize;
    maxSamplerDescriptorBufferRange = in_struct->maxSamplerDescriptorBufferRange;
    maxResourceDescriptorBufferRange = in_struct->maxResourceDescriptorBufferRange;
    samplerDescriptorBufferAddressSpaceSize = in_struct->samplerDescriptorBufferAddressSpaceSize;
    resourceDescriptorBufferAddressSpaceSize = in_struct->resourceDescriptorBufferAddressSpaceSize;
    descriptorBufferAddressSpaceSize = in_struct->descriptorBufferAddressSpaceSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    combinedImageSamplerDescriptorSingleArray = copy_src->combinedImageSamplerDescriptorSingleArray;
    bufferlessPushDescriptors = copy_src->bufferlessPushDescriptors;
    allowSamplerImageViewPostSubmitCreation = copy_src->allowSamplerImageViewPostSubmitCreation;
    descriptorBufferOffsetAlignment = copy_src->descriptorBufferOffsetAlignment;
    maxDescriptorBufferBindings = copy_src->maxDescriptorBufferBindings;
    maxResourceDescriptorBufferBindings = copy_src->maxResourceDescriptorBufferBindings;
    maxSamplerDescriptorBufferBindings = copy_src->maxSamplerDescriptorBufferBindings;
    maxEmbeddedImmutableSamplerBindings = copy_src->maxEmbeddedImmutableSamplerBindings;
    maxEmbeddedImmutableSamplers = copy_src->maxEmbeddedImmutableSamplers;
    bufferCaptureReplayDescriptorDataSize = copy_src->bufferCaptureReplayDescriptorDataSize;
    imageCaptureReplayDescriptorDataSize = copy_src->imageCaptureReplayDescriptorDataSize;
    imageViewCaptureReplayDescriptorDataSize = copy_src->imageViewCaptureReplayDescriptorDataSize;
    samplerCaptureReplayDescriptorDataSize = copy_src->samplerCaptureReplayDescriptorDataSize;
    accelerationStructureCaptureReplayDescriptorDataSize = copy_src->accelerationStructureCaptureReplayDescriptorDataSize;
    samplerDescriptorSize = copy_src->samplerDescriptorSize;
    combinedImageSamplerDescriptorSize = copy_src->combinedImageSamplerDescriptorSize;
    sampledImageDescriptorSize = copy_src->sampledImageDescriptorSize;
    storageImageDescriptorSize = copy_src->storageImageDescriptorSize;
    uniformTexelBufferDescriptorSize = copy_src->uniformTexelBufferDescriptorSize;
    robustUniformTexelBufferDescriptorSize = copy_src->robustUniformTexelBufferDescriptorSize;
    storageTexelBufferDescriptorSize = copy_src->storageTexelBufferDescriptorSize;
    robustStorageTexelBufferDescriptorSize = copy_src->robustStorageTexelBufferDescriptorSize;
    uniformBufferDescriptorSize = copy_src->uniformBufferDescriptorSize;
    robustUniformBufferDescriptorSize = copy_src->robustUniformBufferDescriptorSize;
    storageBufferDescriptorSize = copy_src->storageBufferDescriptorSize;
    robustStorageBufferDescriptorSize = copy_src->robustStorageBufferDescriptorSize;
    inputAttachmentDescriptorSize = copy_src->inputAttachmentDescriptorSize;
    accelerationStructureDescriptorSize = copy_src->accelerationStructureDescriptorSize;
    maxSamplerDescriptorBufferRange = copy_src->maxSamplerDescriptorBufferRange;
    maxResourceDescriptorBufferRange = copy_src->maxResourceDescriptorBufferRange;
    samplerDescriptorBufferAddressSpaceSize = copy_src->samplerDescriptorBufferAddressSpaceSize;
    resourceDescriptorBufferAddressSpaceSize = copy_src->resourceDescriptorBufferAddressSpaceSize;
    descriptorBufferAddressSpaceSize = copy_src->descriptorBufferAddressSpaceSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(
    const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      combinedImageSamplerDensityMapDescriptorSize(in_struct->combinedImageSamplerDensityMapDescriptorSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT),
      pNext(nullptr),
      combinedImageSamplerDensityMapDescriptorSize() {}

safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(
    const safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    combinedImageSamplerDensityMapDescriptorSize = copy_src.combinedImageSamplerDensityMapDescriptorSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT&
safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    combinedImageSamplerDensityMapDescriptorSize = copy_src.combinedImageSamplerDensityMapDescriptorSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::~safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::initialize(
    const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    combinedImageSamplerDensityMapDescriptorSize = in_struct->combinedImageSamplerDensityMapDescriptorSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    combinedImageSamplerDensityMapDescriptorSize = copy_src->combinedImageSamplerDensityMapDescriptorSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT(
    const VkPhysicalDeviceDescriptorBufferFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      descriptorBuffer(in_struct->descriptorBuffer),
      descriptorBufferCaptureReplay(in_struct->descriptorBufferCaptureReplay),
      descriptorBufferImageLayoutIgnored(in_struct->descriptorBufferImageLayoutIgnored),
      descriptorBufferPushDescriptors(in_struct->descriptorBufferPushDescriptors) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT),
      pNext(nullptr),
      descriptorBuffer(),
      descriptorBufferCaptureReplay(),
      descriptorBufferImageLayoutIgnored(),
      descriptorBufferPushDescriptors() {}

safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT(
    const safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    descriptorBuffer = copy_src.descriptorBuffer;
    descriptorBufferCaptureReplay = copy_src.descriptorBufferCaptureReplay;
    descriptorBufferImageLayoutIgnored = copy_src.descriptorBufferImageLayoutIgnored;
    descriptorBufferPushDescriptors = copy_src.descriptorBufferPushDescriptors;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT& safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    descriptorBuffer = copy_src.descriptorBuffer;
    descriptorBufferCaptureReplay = copy_src.descriptorBufferCaptureReplay;
    descriptorBufferImageLayoutIgnored = copy_src.descriptorBufferImageLayoutIgnored;
    descriptorBufferPushDescriptors = copy_src.descriptorBufferPushDescriptors;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::~safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::initialize(const VkPhysicalDeviceDescriptorBufferFeaturesEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    descriptorBuffer = in_struct->descriptorBuffer;
    descriptorBufferCaptureReplay = in_struct->descriptorBufferCaptureReplay;
    descriptorBufferImageLayoutIgnored = in_struct->descriptorBufferImageLayoutIgnored;
    descriptorBufferPushDescriptors = in_struct->descriptorBufferPushDescriptors;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT::initialize(const safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    descriptorBuffer = copy_src->descriptorBuffer;
    descriptorBufferCaptureReplay = copy_src->descriptorBufferCaptureReplay;
    descriptorBufferImageLayoutIgnored = copy_src->descriptorBufferImageLayoutIgnored;
    descriptorBufferPushDescriptors = copy_src->descriptorBufferPushDescriptors;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorAddressInfoEXT::safe_VkDescriptorAddressInfoEXT(const VkDescriptorAddressInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), address(in_struct->address), range(in_struct->range), format(in_struct->format) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorAddressInfoEXT::safe_VkDescriptorAddressInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT), pNext(nullptr), address(), range(), format() {}

safe_VkDescriptorAddressInfoEXT::safe_VkDescriptorAddressInfoEXT(const safe_VkDescriptorAddressInfoEXT& copy_src) {
    sType = copy_src.sType;
    address = copy_src.address;
    range = copy_src.range;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorAddressInfoEXT& safe_VkDescriptorAddressInfoEXT::operator=(const safe_VkDescriptorAddressInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    address = copy_src.address;
    range = copy_src.range;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorAddressInfoEXT::~safe_VkDescriptorAddressInfoEXT() { FreePnextChain(pNext); }

void safe_VkDescriptorAddressInfoEXT::initialize(const VkDescriptorAddressInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    address = in_struct->address;
    range = in_struct->range;
    format = in_struct->format;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorAddressInfoEXT::initialize(const safe_VkDescriptorAddressInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    address = copy_src->address;
    range = copy_src->range;
    format = copy_src->format;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorBufferBindingInfoEXT::safe_VkDescriptorBufferBindingInfoEXT(const VkDescriptorBufferBindingInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), address(in_struct->address), usage(in_struct->usage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorBufferBindingInfoEXT::safe_VkDescriptorBufferBindingInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT), pNext(nullptr), address(), usage() {}

safe_VkDescriptorBufferBindingInfoEXT::safe_VkDescriptorBufferBindingInfoEXT(
    const safe_VkDescriptorBufferBindingInfoEXT& copy_src) {
    sType = copy_src.sType;
    address = copy_src.address;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorBufferBindingInfoEXT& safe_VkDescriptorBufferBindingInfoEXT::operator=(
    const safe_VkDescriptorBufferBindingInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    address = copy_src.address;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorBufferBindingInfoEXT::~safe_VkDescriptorBufferBindingInfoEXT() { FreePnextChain(pNext); }

void safe_VkDescriptorBufferBindingInfoEXT::initialize(const VkDescriptorBufferBindingInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    address = in_struct->address;
    usage = in_struct->usage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorBufferBindingInfoEXT::initialize(const safe_VkDescriptorBufferBindingInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    address = copy_src->address;
    usage = copy_src->usage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT(
    const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), buffer(in_struct->buffer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT), pNext(nullptr), buffer() {}

safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT(
    const safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT& copy_src) {
    sType = copy_src.sType;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT& safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::operator=(
    const safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::~safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT() {
    FreePnextChain(pNext);
}

void safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::initialize(
    const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    buffer = in_struct->buffer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT::initialize(
    const safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    buffer = copy_src->buffer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorGetInfoEXT::safe_VkDescriptorGetInfoEXT(const VkDescriptorGetInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type), data(in_struct->data) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorGetInfoEXT::safe_VkDescriptorGetInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT), pNext(nullptr), type(), data() {}

safe_VkDescriptorGetInfoEXT::safe_VkDescriptorGetInfoEXT(const safe_VkDescriptorGetInfoEXT& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    data = copy_src.data;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorGetInfoEXT& safe_VkDescriptorGetInfoEXT::operator=(const safe_VkDescriptorGetInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    data = copy_src.data;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorGetInfoEXT::~safe_VkDescriptorGetInfoEXT() { FreePnextChain(pNext); }

void safe_VkDescriptorGetInfoEXT::initialize(const VkDescriptorGetInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    data = in_struct->data;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorGetInfoEXT::initialize(const safe_VkDescriptorGetInfoEXT* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    data = copy_src->data;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCaptureDescriptorDataInfoEXT::safe_VkBufferCaptureDescriptorDataInfoEXT(
    const VkBufferCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), buffer(in_struct->buffer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCaptureDescriptorDataInfoEXT::safe_VkBufferCaptureDescriptorDataInfoEXT()
    : sType(VK_STRUCTURE_TYPE_BUFFER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT), pNext(nullptr), buffer() {}

safe_VkBufferCaptureDescriptorDataInfoEXT::safe_VkBufferCaptureDescriptorDataInfoEXT(
    const safe_VkBufferCaptureDescriptorDataInfoEXT& copy_src) {
    sType = copy_src.sType;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCaptureDescriptorDataInfoEXT& safe_VkBufferCaptureDescriptorDataInfoEXT::operator=(
    const safe_VkBufferCaptureDescriptorDataInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCaptureDescriptorDataInfoEXT::~safe_VkBufferCaptureDescriptorDataInfoEXT() { FreePnextChain(pNext); }

void safe_VkBufferCaptureDescriptorDataInfoEXT::initialize(const VkBufferCaptureDescriptorDataInfoEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    buffer = in_struct->buffer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCaptureDescriptorDataInfoEXT::initialize(const safe_VkBufferCaptureDescriptorDataInfoEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    buffer = copy_src->buffer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageCaptureDescriptorDataInfoEXT::safe_VkImageCaptureDescriptorDataInfoEXT(
    const VkImageCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), image(in_struct->image) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageCaptureDescriptorDataInfoEXT::safe_VkImageCaptureDescriptorDataInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT), pNext(nullptr), image() {}

safe_VkImageCaptureDescriptorDataInfoEXT::safe_VkImageCaptureDescriptorDataInfoEXT(
    const safe_VkImageCaptureDescriptorDataInfoEXT& copy_src) {
    sType = copy_src.sType;
    image = copy_src.image;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageCaptureDescriptorDataInfoEXT& safe_VkImageCaptureDescriptorDataInfoEXT::operator=(
    const safe_VkImageCaptureDescriptorDataInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image = copy_src.image;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageCaptureDescriptorDataInfoEXT::~safe_VkImageCaptureDescriptorDataInfoEXT() { FreePnextChain(pNext); }

void safe_VkImageCaptureDescriptorDataInfoEXT::initialize(const VkImageCaptureDescriptorDataInfoEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image = in_struct->image;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageCaptureDescriptorDataInfoEXT::initialize(const safe_VkImageCaptureDescriptorDataInfoEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image = copy_src->image;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewCaptureDescriptorDataInfoEXT::safe_VkImageViewCaptureDescriptorDataInfoEXT(
    const VkImageViewCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageView(in_struct->imageView) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewCaptureDescriptorDataInfoEXT::safe_VkImageViewCaptureDescriptorDataInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_CAPTURE_DESCRIPTOR_DATA_INFO_EXT), pNext(nullptr), imageView() {}

safe_VkImageViewCaptureDescriptorDataInfoEXT::safe_VkImageViewCaptureDescriptorDataInfoEXT(
    const safe_VkImageViewCaptureDescriptorDataInfoEXT& copy_src) {
    sType = copy_src.sType;
    imageView = copy_src.imageView;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewCaptureDescriptorDataInfoEXT& safe_VkImageViewCaptureDescriptorDataInfoEXT::operator=(
    const safe_VkImageViewCaptureDescriptorDataInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageView = copy_src.imageView;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewCaptureDescriptorDataInfoEXT::~safe_VkImageViewCaptureDescriptorDataInfoEXT() { FreePnextChain(pNext); }

void safe_VkImageViewCaptureDescriptorDataInfoEXT::initialize(const VkImageViewCaptureDescriptorDataInfoEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageView = in_struct->imageView;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewCaptureDescriptorDataInfoEXT::initialize(const safe_VkImageViewCaptureDescriptorDataInfoEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageView = copy_src->imageView;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerCaptureDescriptorDataInfoEXT::safe_VkSamplerCaptureDescriptorDataInfoEXT(
    const VkSamplerCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), sampler(in_struct->sampler) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerCaptureDescriptorDataInfoEXT::safe_VkSamplerCaptureDescriptorDataInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_CAPTURE_DESCRIPTOR_DATA_INFO_EXT), pNext(nullptr), sampler() {}

safe_VkSamplerCaptureDescriptorDataInfoEXT::safe_VkSamplerCaptureDescriptorDataInfoEXT(
    const safe_VkSamplerCaptureDescriptorDataInfoEXT& copy_src) {
    sType = copy_src.sType;
    sampler = copy_src.sampler;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerCaptureDescriptorDataInfoEXT& safe_VkSamplerCaptureDescriptorDataInfoEXT::operator=(
    const safe_VkSamplerCaptureDescriptorDataInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampler = copy_src.sampler;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerCaptureDescriptorDataInfoEXT::~safe_VkSamplerCaptureDescriptorDataInfoEXT() { FreePnextChain(pNext); }

void safe_VkSamplerCaptureDescriptorDataInfoEXT::initialize(const VkSamplerCaptureDescriptorDataInfoEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampler = in_struct->sampler;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerCaptureDescriptorDataInfoEXT::initialize(const safe_VkSamplerCaptureDescriptorDataInfoEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampler = copy_src->sampler;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT(
    const VkOpaqueCaptureDescriptorDataCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), opaqueCaptureDescriptorData(in_struct->opaqueCaptureDescriptorData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT),
      pNext(nullptr),
      opaqueCaptureDescriptorData(nullptr) {}

safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT(
    const safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    opaqueCaptureDescriptorData = copy_src.opaqueCaptureDescriptorData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT& safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::operator=(
    const safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    opaqueCaptureDescriptorData = copy_src.opaqueCaptureDescriptorData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::~safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::initialize(const VkOpaqueCaptureDescriptorDataCreateInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    opaqueCaptureDescriptorData = in_struct->opaqueCaptureDescriptorData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT::initialize(const safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    opaqueCaptureDescriptorData = copy_src->opaqueCaptureDescriptorData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT(
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructure(in_struct->accelerationStructure),
      accelerationStructureNV(in_struct->accelerationStructureNV) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CAPTURE_DESCRIPTOR_DATA_INFO_EXT),
      pNext(nullptr),
      accelerationStructure(),
      accelerationStructureNV() {}

safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT(
    const safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureNV = copy_src.accelerationStructureNV;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT& safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::operator=(
    const safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureNV = copy_src.accelerationStructureNV;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::~safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::initialize(
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    accelerationStructureNV = in_struct->accelerationStructureNV;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT::initialize(
    const safe_VkAccelerationStructureCaptureDescriptorDataInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    accelerationStructureNV = copy_src->accelerationStructureNV;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(
    const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), graphicsPipelineLibrary(in_struct->graphicsPipelineLibrary) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT), pNext(nullptr), graphicsPipelineLibrary() {}

safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    graphicsPipelineLibrary = copy_src.graphicsPipelineLibrary;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT& safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    graphicsPipelineLibrary = copy_src.graphicsPipelineLibrary;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::~safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::initialize(
    const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    graphicsPipelineLibrary = in_struct->graphicsPipelineLibrary;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    graphicsPipelineLibrary = copy_src->graphicsPipelineLibrary;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(
    const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      graphicsPipelineLibraryFastLinking(in_struct->graphicsPipelineLibraryFastLinking),
      graphicsPipelineLibraryIndependentInterpolationDecoration(
          in_struct->graphicsPipelineLibraryIndependentInterpolationDecoration) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT),
      pNext(nullptr),
      graphicsPipelineLibraryFastLinking(),
      graphicsPipelineLibraryIndependentInterpolationDecoration() {}

safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    graphicsPipelineLibraryFastLinking = copy_src.graphicsPipelineLibraryFastLinking;
    graphicsPipelineLibraryIndependentInterpolationDecoration = copy_src.graphicsPipelineLibraryIndependentInterpolationDecoration;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT& safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    graphicsPipelineLibraryFastLinking = copy_src.graphicsPipelineLibraryFastLinking;
    graphicsPipelineLibraryIndependentInterpolationDecoration = copy_src.graphicsPipelineLibraryIndependentInterpolationDecoration;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::~safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::initialize(
    const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    graphicsPipelineLibraryFastLinking = in_struct->graphicsPipelineLibraryFastLinking;
    graphicsPipelineLibraryIndependentInterpolationDecoration =
        in_struct->graphicsPipelineLibraryIndependentInterpolationDecoration;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    graphicsPipelineLibraryFastLinking = copy_src->graphicsPipelineLibraryFastLinking;
    graphicsPipelineLibraryIndependentInterpolationDecoration = copy_src->graphicsPipelineLibraryIndependentInterpolationDecoration;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGraphicsPipelineLibraryCreateInfoEXT::safe_VkGraphicsPipelineLibraryCreateInfoEXT(
    const VkGraphicsPipelineLibraryCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGraphicsPipelineLibraryCreateInfoEXT::safe_VkGraphicsPipelineLibraryCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT), pNext(nullptr), flags() {}

safe_VkGraphicsPipelineLibraryCreateInfoEXT::safe_VkGraphicsPipelineLibraryCreateInfoEXT(
    const safe_VkGraphicsPipelineLibraryCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGraphicsPipelineLibraryCreateInfoEXT& safe_VkGraphicsPipelineLibraryCreateInfoEXT::operator=(
    const safe_VkGraphicsPipelineLibraryCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGraphicsPipelineLibraryCreateInfoEXT::~safe_VkGraphicsPipelineLibraryCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkGraphicsPipelineLibraryCreateInfoEXT::initialize(const VkGraphicsPipelineLibraryCreateInfoEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGraphicsPipelineLibraryCreateInfoEXT::initialize(const safe_VkGraphicsPipelineLibraryCreateInfoEXT* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(
    const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), ycbcr2plane444Formats(in_struct->ycbcr2plane444Formats) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT), pNext(nullptr), ycbcr2plane444Formats() {}

safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(
    const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    ycbcr2plane444Formats = copy_src.ycbcr2plane444Formats;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    ycbcr2plane444Formats = copy_src.ycbcr2plane444Formats;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::~safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::initialize(
    const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    ycbcr2plane444Formats = in_struct->ycbcr2plane444Formats;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    ycbcr2plane444Formats = copy_src->ycbcr2plane444Formats;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT(
    const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityMapDeferred(in_struct->fragmentDensityMapDeferred) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT), pNext(nullptr), fragmentDensityMapDeferred() {}

safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityMapDeferred = copy_src.fragmentDensityMapDeferred;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT& safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityMapDeferred = copy_src.fragmentDensityMapDeferred;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::~safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::initialize(
    const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityMapDeferred = in_struct->fragmentDensityMapDeferred;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityMapDeferred = copy_src->fragmentDensityMapDeferred;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT(
    const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      subsampledLoads(in_struct->subsampledLoads),
      subsampledCoarseReconstructionEarlyAccess(in_struct->subsampledCoarseReconstructionEarlyAccess),
      maxSubsampledArrayLayers(in_struct->maxSubsampledArrayLayers),
      maxDescriptorSetSubsampledSamplers(in_struct->maxDescriptorSetSubsampledSamplers) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT),
      pNext(nullptr),
      subsampledLoads(),
      subsampledCoarseReconstructionEarlyAccess(),
      maxSubsampledArrayLayers(),
      maxDescriptorSetSubsampledSamplers() {}

safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT& copy_src) {
    sType = copy_src.sType;
    subsampledLoads = copy_src.subsampledLoads;
    subsampledCoarseReconstructionEarlyAccess = copy_src.subsampledCoarseReconstructionEarlyAccess;
    maxSubsampledArrayLayers = copy_src.maxSubsampledArrayLayers;
    maxDescriptorSetSubsampledSamplers = copy_src.maxDescriptorSetSubsampledSamplers;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT& safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    subsampledLoads = copy_src.subsampledLoads;
    subsampledCoarseReconstructionEarlyAccess = copy_src.subsampledCoarseReconstructionEarlyAccess;
    maxSubsampledArrayLayers = copy_src.maxSubsampledArrayLayers;
    maxDescriptorSetSubsampledSamplers = copy_src.maxDescriptorSetSubsampledSamplers;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::~safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::initialize(
    const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    subsampledLoads = in_struct->subsampledLoads;
    subsampledCoarseReconstructionEarlyAccess = in_struct->subsampledCoarseReconstructionEarlyAccess;
    maxSubsampledArrayLayers = in_struct->maxSubsampledArrayLayers;
    maxDescriptorSetSubsampledSamplers = in_struct->maxDescriptorSetSubsampledSamplers;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    subsampledLoads = copy_src->subsampledLoads;
    subsampledCoarseReconstructionEarlyAccess = copy_src->subsampledCoarseReconstructionEarlyAccess;
    maxSubsampledArrayLayers = copy_src->maxSubsampledArrayLayers;
    maxDescriptorSetSubsampledSamplers = copy_src->maxDescriptorSetSubsampledSamplers;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT(
    const VkPhysicalDeviceImageCompressionControlFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), imageCompressionControl(in_struct->imageCompressionControl) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT), pNext(nullptr), imageCompressionControl() {}

safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT(
    const safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    imageCompressionControl = copy_src.imageCompressionControl;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT& safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageCompressionControl = copy_src.imageCompressionControl;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::~safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::initialize(
    const VkPhysicalDeviceImageCompressionControlFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageCompressionControl = in_struct->imageCompressionControl;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageCompressionControl = copy_src->imageCompressionControl;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageCompressionControlEXT::safe_VkImageCompressionControlEXT(const VkImageCompressionControlEXT* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      compressionControlPlaneCount(in_struct->compressionControlPlaneCount),
      pFixedRateFlags(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pFixedRateFlags) {
        pFixedRateFlags = new VkImageCompressionFixedRateFlagsEXT[in_struct->compressionControlPlaneCount];
        memcpy((void*)pFixedRateFlags, (void*)in_struct->pFixedRateFlags,
               sizeof(VkImageCompressionFixedRateFlagsEXT) * in_struct->compressionControlPlaneCount);
    }
}

safe_VkImageCompressionControlEXT::safe_VkImageCompressionControlEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT),
      pNext(nullptr),
      flags(),
      compressionControlPlaneCount(),
      pFixedRateFlags(nullptr) {}

safe_VkImageCompressionControlEXT::safe_VkImageCompressionControlEXT(const safe_VkImageCompressionControlEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    compressionControlPlaneCount = copy_src.compressionControlPlaneCount;
    pFixedRateFlags = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFixedRateFlags) {
        pFixedRateFlags = new VkImageCompressionFixedRateFlagsEXT[copy_src.compressionControlPlaneCount];
        memcpy((void*)pFixedRateFlags, (void*)copy_src.pFixedRateFlags,
               sizeof(VkImageCompressionFixedRateFlagsEXT) * copy_src.compressionControlPlaneCount);
    }
}

safe_VkImageCompressionControlEXT& safe_VkImageCompressionControlEXT::operator=(const safe_VkImageCompressionControlEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pFixedRateFlags) delete[] pFixedRateFlags;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    compressionControlPlaneCount = copy_src.compressionControlPlaneCount;
    pFixedRateFlags = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFixedRateFlags) {
        pFixedRateFlags = new VkImageCompressionFixedRateFlagsEXT[copy_src.compressionControlPlaneCount];
        memcpy((void*)pFixedRateFlags, (void*)copy_src.pFixedRateFlags,
               sizeof(VkImageCompressionFixedRateFlagsEXT) * copy_src.compressionControlPlaneCount);
    }

    return *this;
}

safe_VkImageCompressionControlEXT::~safe_VkImageCompressionControlEXT() {
    if (pFixedRateFlags) delete[] pFixedRateFlags;
    FreePnextChain(pNext);
}

void safe_VkImageCompressionControlEXT::initialize(const VkImageCompressionControlEXT* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFixedRateFlags) delete[] pFixedRateFlags;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    compressionControlPlaneCount = in_struct->compressionControlPlaneCount;
    pFixedRateFlags = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pFixedRateFlags) {
        pFixedRateFlags = new VkImageCompressionFixedRateFlagsEXT[in_struct->compressionControlPlaneCount];
        memcpy((void*)pFixedRateFlags, (void*)in_struct->pFixedRateFlags,
               sizeof(VkImageCompressionFixedRateFlagsEXT) * in_struct->compressionControlPlaneCount);
    }
}

void safe_VkImageCompressionControlEXT::initialize(const safe_VkImageCompressionControlEXT* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    compressionControlPlaneCount = copy_src->compressionControlPlaneCount;
    pFixedRateFlags = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pFixedRateFlags) {
        pFixedRateFlags = new VkImageCompressionFixedRateFlagsEXT[copy_src->compressionControlPlaneCount];
        memcpy((void*)pFixedRateFlags, (void*)copy_src->pFixedRateFlags,
               sizeof(VkImageCompressionFixedRateFlagsEXT) * copy_src->compressionControlPlaneCount);
    }
}

safe_VkImageCompressionPropertiesEXT::safe_VkImageCompressionPropertiesEXT(const VkImageCompressionPropertiesEXT* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      imageCompressionFlags(in_struct->imageCompressionFlags),
      imageCompressionFixedRateFlags(in_struct->imageCompressionFixedRateFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageCompressionPropertiesEXT::safe_VkImageCompressionPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT),
      pNext(nullptr),
      imageCompressionFlags(),
      imageCompressionFixedRateFlags() {}

safe_VkImageCompressionPropertiesEXT::safe_VkImageCompressionPropertiesEXT(const safe_VkImageCompressionPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    imageCompressionFlags = copy_src.imageCompressionFlags;
    imageCompressionFixedRateFlags = copy_src.imageCompressionFixedRateFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageCompressionPropertiesEXT& safe_VkImageCompressionPropertiesEXT::operator=(
    const safe_VkImageCompressionPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageCompressionFlags = copy_src.imageCompressionFlags;
    imageCompressionFixedRateFlags = copy_src.imageCompressionFixedRateFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageCompressionPropertiesEXT::~safe_VkImageCompressionPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkImageCompressionPropertiesEXT::initialize(const VkImageCompressionPropertiesEXT* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageCompressionFlags = in_struct->imageCompressionFlags;
    imageCompressionFixedRateFlags = in_struct->imageCompressionFixedRateFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageCompressionPropertiesEXT::initialize(const safe_VkImageCompressionPropertiesEXT* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageCompressionFlags = copy_src->imageCompressionFlags;
    imageCompressionFixedRateFlags = copy_src->imageCompressionFixedRateFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(
    const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), attachmentFeedbackLoopLayout(in_struct->attachmentFeedbackLoopLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT),
      pNext(nullptr),
      attachmentFeedbackLoopLayout() {}

safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(
    const safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    attachmentFeedbackLoopLayout = copy_src.attachmentFeedbackLoopLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT&
safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    attachmentFeedbackLoopLayout = copy_src.attachmentFeedbackLoopLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::~safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::initialize(
    const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    attachmentFeedbackLoopLayout = in_struct->attachmentFeedbackLoopLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    attachmentFeedbackLoopLayout = copy_src->attachmentFeedbackLoopLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevice4444FormatsFeaturesEXT::safe_VkPhysicalDevice4444FormatsFeaturesEXT(
    const VkPhysicalDevice4444FormatsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), formatA4R4G4B4(in_struct->formatA4R4G4B4), formatA4B4G4R4(in_struct->formatA4B4G4R4) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevice4444FormatsFeaturesEXT::safe_VkPhysicalDevice4444FormatsFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT), pNext(nullptr), formatA4R4G4B4(), formatA4B4G4R4() {}

safe_VkPhysicalDevice4444FormatsFeaturesEXT::safe_VkPhysicalDevice4444FormatsFeaturesEXT(
    const safe_VkPhysicalDevice4444FormatsFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    formatA4R4G4B4 = copy_src.formatA4R4G4B4;
    formatA4B4G4R4 = copy_src.formatA4B4G4R4;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevice4444FormatsFeaturesEXT& safe_VkPhysicalDevice4444FormatsFeaturesEXT::operator=(
    const safe_VkPhysicalDevice4444FormatsFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    formatA4R4G4B4 = copy_src.formatA4R4G4B4;
    formatA4B4G4R4 = copy_src.formatA4B4G4R4;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevice4444FormatsFeaturesEXT::~safe_VkPhysicalDevice4444FormatsFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDevice4444FormatsFeaturesEXT::initialize(const VkPhysicalDevice4444FormatsFeaturesEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    formatA4R4G4B4 = in_struct->formatA4R4G4B4;
    formatA4B4G4R4 = in_struct->formatA4B4G4R4;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevice4444FormatsFeaturesEXT::initialize(const safe_VkPhysicalDevice4444FormatsFeaturesEXT* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    formatA4R4G4B4 = copy_src->formatA4R4G4B4;
    formatA4B4G4R4 = copy_src->formatA4B4G4R4;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFaultFeaturesEXT::safe_VkPhysicalDeviceFaultFeaturesEXT(const VkPhysicalDeviceFaultFeaturesEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), deviceFault(in_struct->deviceFault), deviceFaultVendorBinary(in_struct->deviceFaultVendorBinary) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFaultFeaturesEXT::safe_VkPhysicalDeviceFaultFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT), pNext(nullptr), deviceFault(), deviceFaultVendorBinary() {}

safe_VkPhysicalDeviceFaultFeaturesEXT::safe_VkPhysicalDeviceFaultFeaturesEXT(
    const safe_VkPhysicalDeviceFaultFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    deviceFault = copy_src.deviceFault;
    deviceFaultVendorBinary = copy_src.deviceFaultVendorBinary;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFaultFeaturesEXT& safe_VkPhysicalDeviceFaultFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFaultFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceFault = copy_src.deviceFault;
    deviceFaultVendorBinary = copy_src.deviceFaultVendorBinary;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFaultFeaturesEXT::~safe_VkPhysicalDeviceFaultFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceFaultFeaturesEXT::initialize(const VkPhysicalDeviceFaultFeaturesEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceFault = in_struct->deviceFault;
    deviceFaultVendorBinary = in_struct->deviceFaultVendorBinary;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFaultFeaturesEXT::initialize(const safe_VkPhysicalDeviceFaultFeaturesEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceFault = copy_src->deviceFault;
    deviceFaultVendorBinary = copy_src->deviceFaultVendorBinary;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceFaultCountsEXT::safe_VkDeviceFaultCountsEXT(const VkDeviceFaultCountsEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      addressInfoCount(in_struct->addressInfoCount),
      vendorInfoCount(in_struct->vendorInfoCount),
      vendorBinarySize(in_struct->vendorBinarySize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceFaultCountsEXT::safe_VkDeviceFaultCountsEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT), pNext(nullptr), addressInfoCount(), vendorInfoCount(), vendorBinarySize() {}

safe_VkDeviceFaultCountsEXT::safe_VkDeviceFaultCountsEXT(const safe_VkDeviceFaultCountsEXT& copy_src) {
    sType = copy_src.sType;
    addressInfoCount = copy_src.addressInfoCount;
    vendorInfoCount = copy_src.vendorInfoCount;
    vendorBinarySize = copy_src.vendorBinarySize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceFaultCountsEXT& safe_VkDeviceFaultCountsEXT::operator=(const safe_VkDeviceFaultCountsEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    addressInfoCount = copy_src.addressInfoCount;
    vendorInfoCount = copy_src.vendorInfoCount;
    vendorBinarySize = copy_src.vendorBinarySize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceFaultCountsEXT::~safe_VkDeviceFaultCountsEXT() { FreePnextChain(pNext); }

void safe_VkDeviceFaultCountsEXT::initialize(const VkDeviceFaultCountsEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    addressInfoCount = in_struct->addressInfoCount;
    vendorInfoCount = in_struct->vendorInfoCount;
    vendorBinarySize = in_struct->vendorBinarySize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceFaultCountsEXT::initialize(const safe_VkDeviceFaultCountsEXT* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    addressInfoCount = copy_src->addressInfoCount;
    vendorInfoCount = copy_src->vendorInfoCount;
    vendorBinarySize = copy_src->vendorBinarySize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceFaultInfoEXT::safe_VkDeviceFaultInfoEXT(const VkDeviceFaultInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pAddressInfos(nullptr), pVendorInfos(nullptr), pVendorBinaryData(in_struct->pVendorBinaryData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pAddressInfos) {
        pAddressInfos = new VkDeviceFaultAddressInfoEXT(*in_struct->pAddressInfos);
    }

    if (in_struct->pVendorInfos) {
        pVendorInfos = new VkDeviceFaultVendorInfoEXT(*in_struct->pVendorInfos);
    }
}

safe_VkDeviceFaultInfoEXT::safe_VkDeviceFaultInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT),
      pNext(nullptr),
      pAddressInfos(nullptr),
      pVendorInfos(nullptr),
      pVendorBinaryData(nullptr) {}

safe_VkDeviceFaultInfoEXT::safe_VkDeviceFaultInfoEXT(const safe_VkDeviceFaultInfoEXT& copy_src) {
    sType = copy_src.sType;
    pAddressInfos = nullptr;
    pVendorInfos = nullptr;
    pVendorBinaryData = copy_src.pVendorBinaryData;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pAddressInfos) {
        pAddressInfos = new VkDeviceFaultAddressInfoEXT(*copy_src.pAddressInfos);
    }

    if (copy_src.pVendorInfos) {
        pVendorInfos = new VkDeviceFaultVendorInfoEXT(*copy_src.pVendorInfos);
    }
}

safe_VkDeviceFaultInfoEXT& safe_VkDeviceFaultInfoEXT::operator=(const safe_VkDeviceFaultInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pAddressInfos) delete pAddressInfos;
    if (pVendorInfos) delete pVendorInfos;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAddressInfos = nullptr;
    pVendorInfos = nullptr;
    pVendorBinaryData = copy_src.pVendorBinaryData;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pAddressInfos) {
        pAddressInfos = new VkDeviceFaultAddressInfoEXT(*copy_src.pAddressInfos);
    }

    if (copy_src.pVendorInfos) {
        pVendorInfos = new VkDeviceFaultVendorInfoEXT(*copy_src.pVendorInfos);
    }

    return *this;
}

safe_VkDeviceFaultInfoEXT::~safe_VkDeviceFaultInfoEXT() {
    if (pAddressInfos) delete pAddressInfos;
    if (pVendorInfos) delete pVendorInfos;
    FreePnextChain(pNext);
}

void safe_VkDeviceFaultInfoEXT::initialize(const VkDeviceFaultInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAddressInfos) delete pAddressInfos;
    if (pVendorInfos) delete pVendorInfos;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAddressInfos = nullptr;
    pVendorInfos = nullptr;
    pVendorBinaryData = in_struct->pVendorBinaryData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pAddressInfos) {
        pAddressInfos = new VkDeviceFaultAddressInfoEXT(*in_struct->pAddressInfos);
    }

    if (in_struct->pVendorInfos) {
        pVendorInfos = new VkDeviceFaultVendorInfoEXT(*in_struct->pVendorInfos);
    }
}

void safe_VkDeviceFaultInfoEXT::initialize(const safe_VkDeviceFaultInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAddressInfos = nullptr;
    pVendorInfos = nullptr;
    pVendorBinaryData = copy_src->pVendorBinaryData;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }

    if (copy_src->pAddressInfos) {
        pAddressInfos = new VkDeviceFaultAddressInfoEXT(*copy_src->pAddressInfos);
    }

    if (copy_src->pVendorInfos) {
        pVendorInfos = new VkDeviceFaultVendorInfoEXT(*copy_src->pVendorInfos);
    }
}

safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::
    safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(
        const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType),
      rasterizationOrderColorAttachmentAccess(in_struct->rasterizationOrderColorAttachmentAccess),
      rasterizationOrderDepthAttachmentAccess(in_struct->rasterizationOrderDepthAttachmentAccess),
      rasterizationOrderStencilAttachmentAccess(in_struct->rasterizationOrderStencilAttachmentAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::
    safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT),
      pNext(nullptr),
      rasterizationOrderColorAttachmentAccess(),
      rasterizationOrderDepthAttachmentAccess(),
      rasterizationOrderStencilAttachmentAccess() {}

safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::
    safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(
        const safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    rasterizationOrderColorAttachmentAccess = copy_src.rasterizationOrderColorAttachmentAccess;
    rasterizationOrderDepthAttachmentAccess = copy_src.rasterizationOrderDepthAttachmentAccess;
    rasterizationOrderStencilAttachmentAccess = copy_src.rasterizationOrderStencilAttachmentAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT&
safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rasterizationOrderColorAttachmentAccess = copy_src.rasterizationOrderColorAttachmentAccess;
    rasterizationOrderDepthAttachmentAccess = copy_src.rasterizationOrderDepthAttachmentAccess;
    rasterizationOrderStencilAttachmentAccess = copy_src.rasterizationOrderStencilAttachmentAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::
    ~safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::initialize(
    const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rasterizationOrderColorAttachmentAccess = in_struct->rasterizationOrderColorAttachmentAccess;
    rasterizationOrderDepthAttachmentAccess = in_struct->rasterizationOrderDepthAttachmentAccess;
    rasterizationOrderStencilAttachmentAccess = in_struct->rasterizationOrderStencilAttachmentAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rasterizationOrderColorAttachmentAccess = copy_src->rasterizationOrderColorAttachmentAccess;
    rasterizationOrderDepthAttachmentAccess = copy_src->rasterizationOrderDepthAttachmentAccess;
    rasterizationOrderStencilAttachmentAccess = copy_src->rasterizationOrderStencilAttachmentAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(
    const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), formatRgba10x6WithoutYCbCrSampler(in_struct->formatRgba10x6WithoutYCbCrSampler) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT), pNext(nullptr), formatRgba10x6WithoutYCbCrSampler() {}

safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(
    const safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    formatRgba10x6WithoutYCbCrSampler = copy_src.formatRgba10x6WithoutYCbCrSampler;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT& safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    formatRgba10x6WithoutYCbCrSampler = copy_src.formatRgba10x6WithoutYCbCrSampler;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::~safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::initialize(const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    formatRgba10x6WithoutYCbCrSampler = in_struct->formatRgba10x6WithoutYCbCrSampler;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::initialize(const safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    formatRgba10x6WithoutYCbCrSampler = copy_src->formatRgba10x6WithoutYCbCrSampler;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_DIRECTFB_EXT

safe_VkDirectFBSurfaceCreateInfoEXT::safe_VkDirectFBSurfaceCreateInfoEXT(const VkDirectFBSurfaceCreateInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), dfb(nullptr), surface(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->dfb) {
        dfb = new IDirectFB(*in_struct->dfb);
    }

    if (in_struct->surface) {
        surface = new IDirectFBSurface(*in_struct->surface);
    }
}

safe_VkDirectFBSurfaceCreateInfoEXT::safe_VkDirectFBSurfaceCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT), pNext(nullptr), flags(), dfb(nullptr), surface(nullptr) {}

safe_VkDirectFBSurfaceCreateInfoEXT::safe_VkDirectFBSurfaceCreateInfoEXT(const safe_VkDirectFBSurfaceCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    dfb = nullptr;
    surface = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.dfb) {
        dfb = new IDirectFB(*copy_src.dfb);
    }

    if (copy_src.surface) {
        surface = new IDirectFBSurface(*copy_src.surface);
    }
}

safe_VkDirectFBSurfaceCreateInfoEXT& safe_VkDirectFBSurfaceCreateInfoEXT::operator=(
    const safe_VkDirectFBSurfaceCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (dfb) delete dfb;
    if (surface) delete surface;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    dfb = nullptr;
    surface = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.dfb) {
        dfb = new IDirectFB(*copy_src.dfb);
    }

    if (copy_src.surface) {
        surface = new IDirectFBSurface(*copy_src.surface);
    }

    return *this;
}

safe_VkDirectFBSurfaceCreateInfoEXT::~safe_VkDirectFBSurfaceCreateInfoEXT() {
    if (dfb) delete dfb;
    if (surface) delete surface;
    FreePnextChain(pNext);
}

void safe_VkDirectFBSurfaceCreateInfoEXT::initialize(const VkDirectFBSurfaceCreateInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (dfb) delete dfb;
    if (surface) delete surface;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    dfb = nullptr;
    surface = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->dfb) {
        dfb = new IDirectFB(*in_struct->dfb);
    }

    if (in_struct->surface) {
        surface = new IDirectFBSurface(*in_struct->surface);
    }
}

void safe_VkDirectFBSurfaceCreateInfoEXT::initialize(const safe_VkDirectFBSurfaceCreateInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    dfb = nullptr;
    surface = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->dfb) {
        dfb = new IDirectFB(*copy_src->dfb);
    }

    if (copy_src->surface) {
        surface = new IDirectFBSurface(*copy_src->surface);
    }
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(
    const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), mutableDescriptorType(in_struct->mutableDescriptorType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT), pNext(nullptr), mutableDescriptorType() {}

safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(
    const safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    mutableDescriptorType = copy_src.mutableDescriptorType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT& safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mutableDescriptorType = copy_src.mutableDescriptorType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::~safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::initialize(
    const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mutableDescriptorType = in_struct->mutableDescriptorType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mutableDescriptorType = copy_src->mutableDescriptorType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMutableDescriptorTypeListEXT::safe_VkMutableDescriptorTypeListEXT(const VkMutableDescriptorTypeListEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : descriptorTypeCount(in_struct->descriptorTypeCount), pDescriptorTypes(nullptr) {
    if (in_struct->pDescriptorTypes) {
        pDescriptorTypes = new VkDescriptorType[in_struct->descriptorTypeCount];
        memcpy((void*)pDescriptorTypes, (void*)in_struct->pDescriptorTypes,
               sizeof(VkDescriptorType) * in_struct->descriptorTypeCount);
    }
}

safe_VkMutableDescriptorTypeListEXT::safe_VkMutableDescriptorTypeListEXT() : descriptorTypeCount(), pDescriptorTypes(nullptr) {}

safe_VkMutableDescriptorTypeListEXT::safe_VkMutableDescriptorTypeListEXT(const safe_VkMutableDescriptorTypeListEXT& copy_src) {
    descriptorTypeCount = copy_src.descriptorTypeCount;
    pDescriptorTypes = nullptr;

    if (copy_src.pDescriptorTypes) {
        pDescriptorTypes = new VkDescriptorType[copy_src.descriptorTypeCount];
        memcpy((void*)pDescriptorTypes, (void*)copy_src.pDescriptorTypes, sizeof(VkDescriptorType) * copy_src.descriptorTypeCount);
    }
}

safe_VkMutableDescriptorTypeListEXT& safe_VkMutableDescriptorTypeListEXT::operator=(
    const safe_VkMutableDescriptorTypeListEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDescriptorTypes) delete[] pDescriptorTypes;

    descriptorTypeCount = copy_src.descriptorTypeCount;
    pDescriptorTypes = nullptr;

    if (copy_src.pDescriptorTypes) {
        pDescriptorTypes = new VkDescriptorType[copy_src.descriptorTypeCount];
        memcpy((void*)pDescriptorTypes, (void*)copy_src.pDescriptorTypes, sizeof(VkDescriptorType) * copy_src.descriptorTypeCount);
    }

    return *this;
}

safe_VkMutableDescriptorTypeListEXT::~safe_VkMutableDescriptorTypeListEXT() {
    if (pDescriptorTypes) delete[] pDescriptorTypes;
}

void safe_VkMutableDescriptorTypeListEXT::initialize(const VkMutableDescriptorTypeListEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDescriptorTypes) delete[] pDescriptorTypes;
    descriptorTypeCount = in_struct->descriptorTypeCount;
    pDescriptorTypes = nullptr;

    if (in_struct->pDescriptorTypes) {
        pDescriptorTypes = new VkDescriptorType[in_struct->descriptorTypeCount];
        memcpy((void*)pDescriptorTypes, (void*)in_struct->pDescriptorTypes,
               sizeof(VkDescriptorType) * in_struct->descriptorTypeCount);
    }
}

void safe_VkMutableDescriptorTypeListEXT::initialize(const safe_VkMutableDescriptorTypeListEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    descriptorTypeCount = copy_src->descriptorTypeCount;
    pDescriptorTypes = nullptr;

    if (copy_src->pDescriptorTypes) {
        pDescriptorTypes = new VkDescriptorType[copy_src->descriptorTypeCount];
        memcpy((void*)pDescriptorTypes, (void*)copy_src->pDescriptorTypes,
               sizeof(VkDescriptorType) * copy_src->descriptorTypeCount);
    }
}

safe_VkMutableDescriptorTypeCreateInfoEXT::safe_VkMutableDescriptorTypeCreateInfoEXT(
    const VkMutableDescriptorTypeCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      mutableDescriptorTypeListCount(in_struct->mutableDescriptorTypeListCount),
      pMutableDescriptorTypeLists(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (mutableDescriptorTypeListCount && in_struct->pMutableDescriptorTypeLists) {
        pMutableDescriptorTypeLists = new safe_VkMutableDescriptorTypeListEXT[mutableDescriptorTypeListCount];
        for (uint32_t i = 0; i < mutableDescriptorTypeListCount; ++i) {
            pMutableDescriptorTypeLists[i].initialize(&in_struct->pMutableDescriptorTypeLists[i]);
        }
    }
}

safe_VkMutableDescriptorTypeCreateInfoEXT::safe_VkMutableDescriptorTypeCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT),
      pNext(nullptr),
      mutableDescriptorTypeListCount(),
      pMutableDescriptorTypeLists(nullptr) {}

safe_VkMutableDescriptorTypeCreateInfoEXT::safe_VkMutableDescriptorTypeCreateInfoEXT(
    const safe_VkMutableDescriptorTypeCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    mutableDescriptorTypeListCount = copy_src.mutableDescriptorTypeListCount;
    pMutableDescriptorTypeLists = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (mutableDescriptorTypeListCount && copy_src.pMutableDescriptorTypeLists) {
        pMutableDescriptorTypeLists = new safe_VkMutableDescriptorTypeListEXT[mutableDescriptorTypeListCount];
        for (uint32_t i = 0; i < mutableDescriptorTypeListCount; ++i) {
            pMutableDescriptorTypeLists[i].initialize(&copy_src.pMutableDescriptorTypeLists[i]);
        }
    }
}

safe_VkMutableDescriptorTypeCreateInfoEXT& safe_VkMutableDescriptorTypeCreateInfoEXT::operator=(
    const safe_VkMutableDescriptorTypeCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pMutableDescriptorTypeLists) delete[] pMutableDescriptorTypeLists;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    mutableDescriptorTypeListCount = copy_src.mutableDescriptorTypeListCount;
    pMutableDescriptorTypeLists = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (mutableDescriptorTypeListCount && copy_src.pMutableDescriptorTypeLists) {
        pMutableDescriptorTypeLists = new safe_VkMutableDescriptorTypeListEXT[mutableDescriptorTypeListCount];
        for (uint32_t i = 0; i < mutableDescriptorTypeListCount; ++i) {
            pMutableDescriptorTypeLists[i].initialize(&copy_src.pMutableDescriptorTypeLists[i]);
        }
    }

    return *this;
}

safe_VkMutableDescriptorTypeCreateInfoEXT::~safe_VkMutableDescriptorTypeCreateInfoEXT() {
    if (pMutableDescriptorTypeLists) delete[] pMutableDescriptorTypeLists;
    FreePnextChain(pNext);
}

void safe_VkMutableDescriptorTypeCreateInfoEXT::initialize(const VkMutableDescriptorTypeCreateInfoEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pMutableDescriptorTypeLists) delete[] pMutableDescriptorTypeLists;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mutableDescriptorTypeListCount = in_struct->mutableDescriptorTypeListCount;
    pMutableDescriptorTypeLists = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (mutableDescriptorTypeListCount && in_struct->pMutableDescriptorTypeLists) {
        pMutableDescriptorTypeLists = new safe_VkMutableDescriptorTypeListEXT[mutableDescriptorTypeListCount];
        for (uint32_t i = 0; i < mutableDescriptorTypeListCount; ++i) {
            pMutableDescriptorTypeLists[i].initialize(&in_struct->pMutableDescriptorTypeLists[i]);
        }
    }
}

void safe_VkMutableDescriptorTypeCreateInfoEXT::initialize(const safe_VkMutableDescriptorTypeCreateInfoEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mutableDescriptorTypeListCount = copy_src->mutableDescriptorTypeListCount;
    pMutableDescriptorTypeLists = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (mutableDescriptorTypeListCount && copy_src->pMutableDescriptorTypeLists) {
        pMutableDescriptorTypeLists = new safe_VkMutableDescriptorTypeListEXT[mutableDescriptorTypeListCount];
        for (uint32_t i = 0; i < mutableDescriptorTypeListCount; ++i) {
            pMutableDescriptorTypeLists[i].initialize(&copy_src->pMutableDescriptorTypeLists[i]);
        }
    }
}

safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(
    const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), vertexInputDynamicState(in_struct->vertexInputDynamicState) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT), pNext(nullptr), vertexInputDynamicState() {}

safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(
    const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    vertexInputDynamicState = copy_src.vertexInputDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexInputDynamicState = copy_src.vertexInputDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::~safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::initialize(
    const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexInputDynamicState = in_struct->vertexInputDynamicState;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexInputDynamicState = copy_src->vertexInputDynamicState;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVertexInputBindingDescription2EXT::safe_VkVertexInputBindingDescription2EXT(
    const VkVertexInputBindingDescription2EXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      binding(in_struct->binding),
      stride(in_struct->stride),
      inputRate(in_struct->inputRate),
      divisor(in_struct->divisor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVertexInputBindingDescription2EXT::safe_VkVertexInputBindingDescription2EXT()
    : sType(VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT),
      pNext(nullptr),
      binding(),
      stride(),
      inputRate(),
      divisor() {}

safe_VkVertexInputBindingDescription2EXT::safe_VkVertexInputBindingDescription2EXT(
    const safe_VkVertexInputBindingDescription2EXT& copy_src) {
    sType = copy_src.sType;
    binding = copy_src.binding;
    stride = copy_src.stride;
    inputRate = copy_src.inputRate;
    divisor = copy_src.divisor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVertexInputBindingDescription2EXT& safe_VkVertexInputBindingDescription2EXT::operator=(
    const safe_VkVertexInputBindingDescription2EXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    binding = copy_src.binding;
    stride = copy_src.stride;
    inputRate = copy_src.inputRate;
    divisor = copy_src.divisor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVertexInputBindingDescription2EXT::~safe_VkVertexInputBindingDescription2EXT() { FreePnextChain(pNext); }

void safe_VkVertexInputBindingDescription2EXT::initialize(const VkVertexInputBindingDescription2EXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    binding = in_struct->binding;
    stride = in_struct->stride;
    inputRate = in_struct->inputRate;
    divisor = in_struct->divisor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVertexInputBindingDescription2EXT::initialize(const safe_VkVertexInputBindingDescription2EXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    binding = copy_src->binding;
    stride = copy_src->stride;
    inputRate = copy_src->inputRate;
    divisor = copy_src->divisor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVertexInputAttributeDescription2EXT::safe_VkVertexInputAttributeDescription2EXT(
    const VkVertexInputAttributeDescription2EXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      location(in_struct->location),
      binding(in_struct->binding),
      format(in_struct->format),
      offset(in_struct->offset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVertexInputAttributeDescription2EXT::safe_VkVertexInputAttributeDescription2EXT()
    : sType(VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT),
      pNext(nullptr),
      location(),
      binding(),
      format(),
      offset() {}

safe_VkVertexInputAttributeDescription2EXT::safe_VkVertexInputAttributeDescription2EXT(
    const safe_VkVertexInputAttributeDescription2EXT& copy_src) {
    sType = copy_src.sType;
    location = copy_src.location;
    binding = copy_src.binding;
    format = copy_src.format;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVertexInputAttributeDescription2EXT& safe_VkVertexInputAttributeDescription2EXT::operator=(
    const safe_VkVertexInputAttributeDescription2EXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    location = copy_src.location;
    binding = copy_src.binding;
    format = copy_src.format;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVertexInputAttributeDescription2EXT::~safe_VkVertexInputAttributeDescription2EXT() { FreePnextChain(pNext); }

void safe_VkVertexInputAttributeDescription2EXT::initialize(const VkVertexInputAttributeDescription2EXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    location = in_struct->location;
    binding = in_struct->binding;
    format = in_struct->format;
    offset = in_struct->offset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVertexInputAttributeDescription2EXT::initialize(const safe_VkVertexInputAttributeDescription2EXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    location = copy_src->location;
    binding = copy_src->binding;
    format = copy_src->format;
    offset = copy_src->offset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDrmPropertiesEXT::safe_VkPhysicalDeviceDrmPropertiesEXT(const VkPhysicalDeviceDrmPropertiesEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      hasPrimary(in_struct->hasPrimary),
      hasRender(in_struct->hasRender),
      primaryMajor(in_struct->primaryMajor),
      primaryMinor(in_struct->primaryMinor),
      renderMajor(in_struct->renderMajor),
      renderMinor(in_struct->renderMinor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDrmPropertiesEXT::safe_VkPhysicalDeviceDrmPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT),
      pNext(nullptr),
      hasPrimary(),
      hasRender(),
      primaryMajor(),
      primaryMinor(),
      renderMajor(),
      renderMinor() {}

safe_VkPhysicalDeviceDrmPropertiesEXT::safe_VkPhysicalDeviceDrmPropertiesEXT(
    const safe_VkPhysicalDeviceDrmPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    hasPrimary = copy_src.hasPrimary;
    hasRender = copy_src.hasRender;
    primaryMajor = copy_src.primaryMajor;
    primaryMinor = copy_src.primaryMinor;
    renderMajor = copy_src.renderMajor;
    renderMinor = copy_src.renderMinor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDrmPropertiesEXT& safe_VkPhysicalDeviceDrmPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceDrmPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hasPrimary = copy_src.hasPrimary;
    hasRender = copy_src.hasRender;
    primaryMajor = copy_src.primaryMajor;
    primaryMinor = copy_src.primaryMinor;
    renderMajor = copy_src.renderMajor;
    renderMinor = copy_src.renderMinor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDrmPropertiesEXT::~safe_VkPhysicalDeviceDrmPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDrmPropertiesEXT::initialize(const VkPhysicalDeviceDrmPropertiesEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hasPrimary = in_struct->hasPrimary;
    hasRender = in_struct->hasRender;
    primaryMajor = in_struct->primaryMajor;
    primaryMinor = in_struct->primaryMinor;
    renderMajor = in_struct->renderMajor;
    renderMinor = in_struct->renderMinor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDrmPropertiesEXT::initialize(const safe_VkPhysicalDeviceDrmPropertiesEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hasPrimary = copy_src->hasPrimary;
    hasRender = copy_src->hasRender;
    primaryMajor = copy_src->primaryMajor;
    primaryMinor = copy_src->primaryMinor;
    renderMajor = copy_src->renderMajor;
    renderMinor = copy_src->renderMinor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT(
    const VkPhysicalDeviceAddressBindingReportFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), reportAddressBinding(in_struct->reportAddressBinding) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT), pNext(nullptr), reportAddressBinding() {}

safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT(
    const safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    reportAddressBinding = copy_src.reportAddressBinding;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT& safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    reportAddressBinding = copy_src.reportAddressBinding;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::~safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::initialize(
    const VkPhysicalDeviceAddressBindingReportFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    reportAddressBinding = in_struct->reportAddressBinding;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    reportAddressBinding = copy_src->reportAddressBinding;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceAddressBindingCallbackDataEXT::safe_VkDeviceAddressBindingCallbackDataEXT(
    const VkDeviceAddressBindingCallbackDataEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      baseAddress(in_struct->baseAddress),
      size(in_struct->size),
      bindingType(in_struct->bindingType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceAddressBindingCallbackDataEXT::safe_VkDeviceAddressBindingCallbackDataEXT()
    : sType(VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT),
      pNext(nullptr),
      flags(),
      baseAddress(),
      size(),
      bindingType() {}

safe_VkDeviceAddressBindingCallbackDataEXT::safe_VkDeviceAddressBindingCallbackDataEXT(
    const safe_VkDeviceAddressBindingCallbackDataEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    baseAddress = copy_src.baseAddress;
    size = copy_src.size;
    bindingType = copy_src.bindingType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceAddressBindingCallbackDataEXT& safe_VkDeviceAddressBindingCallbackDataEXT::operator=(
    const safe_VkDeviceAddressBindingCallbackDataEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    baseAddress = copy_src.baseAddress;
    size = copy_src.size;
    bindingType = copy_src.bindingType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceAddressBindingCallbackDataEXT::~safe_VkDeviceAddressBindingCallbackDataEXT() { FreePnextChain(pNext); }

void safe_VkDeviceAddressBindingCallbackDataEXT::initialize(const VkDeviceAddressBindingCallbackDataEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    baseAddress = in_struct->baseAddress;
    size = in_struct->size;
    bindingType = in_struct->bindingType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceAddressBindingCallbackDataEXT::initialize(const safe_VkDeviceAddressBindingCallbackDataEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    baseAddress = copy_src->baseAddress;
    size = copy_src->size;
    bindingType = copy_src->bindingType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::safe_VkPhysicalDeviceDepthClipControlFeaturesEXT(
    const VkPhysicalDeviceDepthClipControlFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), depthClipControl(in_struct->depthClipControl) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::safe_VkPhysicalDeviceDepthClipControlFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT), pNext(nullptr), depthClipControl() {}

safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::safe_VkPhysicalDeviceDepthClipControlFeaturesEXT(
    const safe_VkPhysicalDeviceDepthClipControlFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    depthClipControl = copy_src.depthClipControl;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDepthClipControlFeaturesEXT& safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDepthClipControlFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthClipControl = copy_src.depthClipControl;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::~safe_VkPhysicalDeviceDepthClipControlFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::initialize(const VkPhysicalDeviceDepthClipControlFeaturesEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthClipControl = in_struct->depthClipControl;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDepthClipControlFeaturesEXT::initialize(const safe_VkPhysicalDeviceDepthClipControlFeaturesEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthClipControl = copy_src->depthClipControl;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineViewportDepthClipControlCreateInfoEXT::safe_VkPipelineViewportDepthClipControlCreateInfoEXT(
    const VkPipelineViewportDepthClipControlCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), negativeOneToOne(in_struct->negativeOneToOne) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineViewportDepthClipControlCreateInfoEXT::safe_VkPipelineViewportDepthClipControlCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT), pNext(nullptr), negativeOneToOne() {}

safe_VkPipelineViewportDepthClipControlCreateInfoEXT::safe_VkPipelineViewportDepthClipControlCreateInfoEXT(
    const safe_VkPipelineViewportDepthClipControlCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    negativeOneToOne = copy_src.negativeOneToOne;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineViewportDepthClipControlCreateInfoEXT& safe_VkPipelineViewportDepthClipControlCreateInfoEXT::operator=(
    const safe_VkPipelineViewportDepthClipControlCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    negativeOneToOne = copy_src.negativeOneToOne;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineViewportDepthClipControlCreateInfoEXT::~safe_VkPipelineViewportDepthClipControlCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportDepthClipControlCreateInfoEXT::initialize(
    const VkPipelineViewportDepthClipControlCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    negativeOneToOne = in_struct->negativeOneToOne;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineViewportDepthClipControlCreateInfoEXT::initialize(
    const safe_VkPipelineViewportDepthClipControlCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    negativeOneToOne = copy_src->negativeOneToOne;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(
    const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      primitiveTopologyListRestart(in_struct->primitiveTopologyListRestart),
      primitiveTopologyPatchListRestart(in_struct->primitiveTopologyPatchListRestart) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT),
      pNext(nullptr),
      primitiveTopologyListRestart(),
      primitiveTopologyPatchListRestart() {}

safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(
    const safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    primitiveTopologyListRestart = copy_src.primitiveTopologyListRestart;
    primitiveTopologyPatchListRestart = copy_src.primitiveTopologyPatchListRestart;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT&
safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::operator=(
    const safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    primitiveTopologyListRestart = copy_src.primitiveTopologyListRestart;
    primitiveTopologyPatchListRestart = copy_src.primitiveTopologyPatchListRestart;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::~safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::initialize(
    const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    primitiveTopologyListRestart = in_struct->primitiveTopologyListRestart;
    primitiveTopologyPatchListRestart = in_struct->primitiveTopologyPatchListRestart;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::initialize(
    const safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    primitiveTopologyListRestart = copy_src->primitiveTopologyListRestart;
    primitiveTopologyPatchListRestart = copy_src->primitiveTopologyPatchListRestart;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelinePropertiesIdentifierEXT::safe_VkPipelinePropertiesIdentifierEXT(const VkPipelinePropertiesIdentifierEXT* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        pipelineIdentifier[i] = in_struct->pipelineIdentifier[i];
    }
}

safe_VkPipelinePropertiesIdentifierEXT::safe_VkPipelinePropertiesIdentifierEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_PROPERTIES_IDENTIFIER_EXT), pNext(nullptr) {}

safe_VkPipelinePropertiesIdentifierEXT::safe_VkPipelinePropertiesIdentifierEXT(
    const safe_VkPipelinePropertiesIdentifierEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        pipelineIdentifier[i] = copy_src.pipelineIdentifier[i];
    }
}

safe_VkPipelinePropertiesIdentifierEXT& safe_VkPipelinePropertiesIdentifierEXT::operator=(
    const safe_VkPipelinePropertiesIdentifierEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        pipelineIdentifier[i] = copy_src.pipelineIdentifier[i];
    }

    return *this;
}

safe_VkPipelinePropertiesIdentifierEXT::~safe_VkPipelinePropertiesIdentifierEXT() { FreePnextChain(pNext); }

void safe_VkPipelinePropertiesIdentifierEXT::initialize(const VkPipelinePropertiesIdentifierEXT* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        pipelineIdentifier[i] = in_struct->pipelineIdentifier[i];
    }
}

void safe_VkPipelinePropertiesIdentifierEXT::initialize(const safe_VkPipelinePropertiesIdentifierEXT* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        pipelineIdentifier[i] = copy_src->pipelineIdentifier[i];
    }
}

safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT(
    const VkPhysicalDevicePipelinePropertiesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipelinePropertiesIdentifier(in_struct->pipelinePropertiesIdentifier) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT), pNext(nullptr), pipelinePropertiesIdentifier() {}

safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT(
    const safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    pipelinePropertiesIdentifier = copy_src.pipelinePropertiesIdentifier;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT& safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::operator=(
    const safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelinePropertiesIdentifier = copy_src.pipelinePropertiesIdentifier;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::~safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::initialize(const VkPhysicalDevicePipelinePropertiesFeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelinePropertiesIdentifier = in_struct->pipelinePropertiesIdentifier;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT::initialize(
    const safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelinePropertiesIdentifier = copy_src->pipelinePropertiesIdentifier;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT(
    const VkPhysicalDeviceFrameBoundaryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), frameBoundary(in_struct->frameBoundary) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT), pNext(nullptr), frameBoundary() {}

safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT(
    const safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    frameBoundary = copy_src.frameBoundary;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT& safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    frameBoundary = copy_src.frameBoundary;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::~safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::initialize(const VkPhysicalDeviceFrameBoundaryFeaturesEXT* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    frameBoundary = in_struct->frameBoundary;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT::initialize(const safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    frameBoundary = copy_src->frameBoundary;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkFrameBoundaryEXT::safe_VkFrameBoundaryEXT(const VkFrameBoundaryEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                                 bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      frameID(in_struct->frameID),
      imageCount(in_struct->imageCount),
      pImages(nullptr),
      bufferCount(in_struct->bufferCount),
      pBuffers(nullptr),
      tagName(in_struct->tagName),
      tagSize(in_struct->tagSize),
      pTag(in_struct->pTag) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (imageCount && in_struct->pImages) {
        pImages = new VkImage[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            pImages[i] = in_struct->pImages[i];
        }
    }
    if (bufferCount && in_struct->pBuffers) {
        pBuffers = new VkBuffer[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            pBuffers[i] = in_struct->pBuffers[i];
        }
    }
}

safe_VkFrameBoundaryEXT::safe_VkFrameBoundaryEXT()
    : sType(VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT),
      pNext(nullptr),
      flags(),
      frameID(),
      imageCount(),
      pImages(nullptr),
      bufferCount(),
      pBuffers(nullptr),
      tagName(),
      tagSize(),
      pTag(nullptr) {}

safe_VkFrameBoundaryEXT::safe_VkFrameBoundaryEXT(const safe_VkFrameBoundaryEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    frameID = copy_src.frameID;
    imageCount = copy_src.imageCount;
    pImages = nullptr;
    bufferCount = copy_src.bufferCount;
    pBuffers = nullptr;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);
    if (imageCount && copy_src.pImages) {
        pImages = new VkImage[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            pImages[i] = copy_src.pImages[i];
        }
    }
    if (bufferCount && copy_src.pBuffers) {
        pBuffers = new VkBuffer[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            pBuffers[i] = copy_src.pBuffers[i];
        }
    }
}

safe_VkFrameBoundaryEXT& safe_VkFrameBoundaryEXT::operator=(const safe_VkFrameBoundaryEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pImages) delete[] pImages;
    if (pBuffers) delete[] pBuffers;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    frameID = copy_src.frameID;
    imageCount = copy_src.imageCount;
    pImages = nullptr;
    bufferCount = copy_src.bufferCount;
    pBuffers = nullptr;
    tagName = copy_src.tagName;
    tagSize = copy_src.tagSize;
    pTag = copy_src.pTag;
    pNext = SafePnextCopy(copy_src.pNext);
    if (imageCount && copy_src.pImages) {
        pImages = new VkImage[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            pImages[i] = copy_src.pImages[i];
        }
    }
    if (bufferCount && copy_src.pBuffers) {
        pBuffers = new VkBuffer[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            pBuffers[i] = copy_src.pBuffers[i];
        }
    }

    return *this;
}

safe_VkFrameBoundaryEXT::~safe_VkFrameBoundaryEXT() {
    if (pImages) delete[] pImages;
    if (pBuffers) delete[] pBuffers;
    FreePnextChain(pNext);
}

void safe_VkFrameBoundaryEXT::initialize(const VkFrameBoundaryEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pImages) delete[] pImages;
    if (pBuffers) delete[] pBuffers;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    frameID = in_struct->frameID;
    imageCount = in_struct->imageCount;
    pImages = nullptr;
    bufferCount = in_struct->bufferCount;
    pBuffers = nullptr;
    tagName = in_struct->tagName;
    tagSize = in_struct->tagSize;
    pTag = in_struct->pTag;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (imageCount && in_struct->pImages) {
        pImages = new VkImage[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            pImages[i] = in_struct->pImages[i];
        }
    }
    if (bufferCount && in_struct->pBuffers) {
        pBuffers = new VkBuffer[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            pBuffers[i] = in_struct->pBuffers[i];
        }
    }
}

void safe_VkFrameBoundaryEXT::initialize(const safe_VkFrameBoundaryEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    frameID = copy_src->frameID;
    imageCount = copy_src->imageCount;
    pImages = nullptr;
    bufferCount = copy_src->bufferCount;
    pBuffers = nullptr;
    tagName = copy_src->tagName;
    tagSize = copy_src->tagSize;
    pTag = copy_src->pTag;
    pNext = SafePnextCopy(copy_src->pNext);
    if (imageCount && copy_src->pImages) {
        pImages = new VkImage[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            pImages[i] = copy_src->pImages[i];
        }
    }
    if (bufferCount && copy_src->pBuffers) {
        pBuffers = new VkBuffer[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            pBuffers[i] = copy_src->pBuffers[i];
        }
    }
}

safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::
    safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(
        const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType), multisampledRenderToSingleSampled(in_struct->multisampledRenderToSingleSampled) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::
    safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT),
      pNext(nullptr),
      multisampledRenderToSingleSampled() {}

safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::
    safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(
        const safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    multisampledRenderToSingleSampled = copy_src.multisampledRenderToSingleSampled;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT&
safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    multisampledRenderToSingleSampled = copy_src.multisampledRenderToSingleSampled;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::
    ~safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::initialize(
    const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    multisampledRenderToSingleSampled = in_struct->multisampledRenderToSingleSampled;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    multisampledRenderToSingleSampled = copy_src->multisampledRenderToSingleSampled;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSubpassResolvePerformanceQueryEXT::safe_VkSubpassResolvePerformanceQueryEXT(
    const VkSubpassResolvePerformanceQueryEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), optimal(in_struct->optimal) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSubpassResolvePerformanceQueryEXT::safe_VkSubpassResolvePerformanceQueryEXT()
    : sType(VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT), pNext(nullptr), optimal() {}

safe_VkSubpassResolvePerformanceQueryEXT::safe_VkSubpassResolvePerformanceQueryEXT(
    const safe_VkSubpassResolvePerformanceQueryEXT& copy_src) {
    sType = copy_src.sType;
    optimal = copy_src.optimal;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSubpassResolvePerformanceQueryEXT& safe_VkSubpassResolvePerformanceQueryEXT::operator=(
    const safe_VkSubpassResolvePerformanceQueryEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    optimal = copy_src.optimal;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSubpassResolvePerformanceQueryEXT::~safe_VkSubpassResolvePerformanceQueryEXT() { FreePnextChain(pNext); }

void safe_VkSubpassResolvePerformanceQueryEXT::initialize(const VkSubpassResolvePerformanceQueryEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    optimal = in_struct->optimal;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSubpassResolvePerformanceQueryEXT::initialize(const safe_VkSubpassResolvePerformanceQueryEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    optimal = copy_src->optimal;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMultisampledRenderToSingleSampledInfoEXT::safe_VkMultisampledRenderToSingleSampledInfoEXT(
    const VkMultisampledRenderToSingleSampledInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      multisampledRenderToSingleSampledEnable(in_struct->multisampledRenderToSingleSampledEnable),
      rasterizationSamples(in_struct->rasterizationSamples) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMultisampledRenderToSingleSampledInfoEXT::safe_VkMultisampledRenderToSingleSampledInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT),
      pNext(nullptr),
      multisampledRenderToSingleSampledEnable(),
      rasterizationSamples() {}

safe_VkMultisampledRenderToSingleSampledInfoEXT::safe_VkMultisampledRenderToSingleSampledInfoEXT(
    const safe_VkMultisampledRenderToSingleSampledInfoEXT& copy_src) {
    sType = copy_src.sType;
    multisampledRenderToSingleSampledEnable = copy_src.multisampledRenderToSingleSampledEnable;
    rasterizationSamples = copy_src.rasterizationSamples;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMultisampledRenderToSingleSampledInfoEXT& safe_VkMultisampledRenderToSingleSampledInfoEXT::operator=(
    const safe_VkMultisampledRenderToSingleSampledInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    multisampledRenderToSingleSampledEnable = copy_src.multisampledRenderToSingleSampledEnable;
    rasterizationSamples = copy_src.rasterizationSamples;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMultisampledRenderToSingleSampledInfoEXT::~safe_VkMultisampledRenderToSingleSampledInfoEXT() { FreePnextChain(pNext); }

void safe_VkMultisampledRenderToSingleSampledInfoEXT::initialize(const VkMultisampledRenderToSingleSampledInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    multisampledRenderToSingleSampledEnable = in_struct->multisampledRenderToSingleSampledEnable;
    rasterizationSamples = in_struct->rasterizationSamples;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMultisampledRenderToSingleSampledInfoEXT::initialize(const safe_VkMultisampledRenderToSingleSampledInfoEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    multisampledRenderToSingleSampledEnable = copy_src->multisampledRenderToSingleSampledEnable;
    rasterizationSamples = copy_src->rasterizationSamples;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(
    const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      extendedDynamicState2(in_struct->extendedDynamicState2),
      extendedDynamicState2LogicOp(in_struct->extendedDynamicState2LogicOp),
      extendedDynamicState2PatchControlPoints(in_struct->extendedDynamicState2PatchControlPoints) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT),
      pNext(nullptr),
      extendedDynamicState2(),
      extendedDynamicState2LogicOp(),
      extendedDynamicState2PatchControlPoints() {}

safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(
    const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    extendedDynamicState2 = copy_src.extendedDynamicState2;
    extendedDynamicState2LogicOp = copy_src.extendedDynamicState2LogicOp;
    extendedDynamicState2PatchControlPoints = copy_src.extendedDynamicState2PatchControlPoints;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    extendedDynamicState2 = copy_src.extendedDynamicState2;
    extendedDynamicState2LogicOp = copy_src.extendedDynamicState2LogicOp;
    extendedDynamicState2PatchControlPoints = copy_src.extendedDynamicState2PatchControlPoints;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::~safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::initialize(
    const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    extendedDynamicState2 = in_struct->extendedDynamicState2;
    extendedDynamicState2LogicOp = in_struct->extendedDynamicState2LogicOp;
    extendedDynamicState2PatchControlPoints = in_struct->extendedDynamicState2PatchControlPoints;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::initialize(
    const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    extendedDynamicState2 = copy_src->extendedDynamicState2;
    extendedDynamicState2LogicOp = copy_src->extendedDynamicState2LogicOp;
    extendedDynamicState2PatchControlPoints = copy_src->extendedDynamicState2PatchControlPoints;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(
    const VkPhysicalDeviceColorWriteEnableFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), colorWriteEnable(in_struct->colorWriteEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT), pNext(nullptr), colorWriteEnable() {}

safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(
    const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    colorWriteEnable = copy_src.colorWriteEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    colorWriteEnable = copy_src.colorWriteEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::~safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::initialize(const VkPhysicalDeviceColorWriteEnableFeaturesEXT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    colorWriteEnable = in_struct->colorWriteEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT::initialize(const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    colorWriteEnable = copy_src->colorWriteEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineColorWriteCreateInfoEXT::safe_VkPipelineColorWriteCreateInfoEXT(const VkPipelineColorWriteCreateInfoEXT* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), attachmentCount(in_struct->attachmentCount), pColorWriteEnables(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pColorWriteEnables) {
        pColorWriteEnables = new VkBool32[in_struct->attachmentCount];
        memcpy((void*)pColorWriteEnables, (void*)in_struct->pColorWriteEnables, sizeof(VkBool32) * in_struct->attachmentCount);
    }
}

safe_VkPipelineColorWriteCreateInfoEXT::safe_VkPipelineColorWriteCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT),
      pNext(nullptr),
      attachmentCount(),
      pColorWriteEnables(nullptr) {}

safe_VkPipelineColorWriteCreateInfoEXT::safe_VkPipelineColorWriteCreateInfoEXT(
    const safe_VkPipelineColorWriteCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    attachmentCount = copy_src.attachmentCount;
    pColorWriteEnables = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorWriteEnables) {
        pColorWriteEnables = new VkBool32[copy_src.attachmentCount];
        memcpy((void*)pColorWriteEnables, (void*)copy_src.pColorWriteEnables, sizeof(VkBool32) * copy_src.attachmentCount);
    }
}

safe_VkPipelineColorWriteCreateInfoEXT& safe_VkPipelineColorWriteCreateInfoEXT::operator=(
    const safe_VkPipelineColorWriteCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pColorWriteEnables) delete[] pColorWriteEnables;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    attachmentCount = copy_src.attachmentCount;
    pColorWriteEnables = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorWriteEnables) {
        pColorWriteEnables = new VkBool32[copy_src.attachmentCount];
        memcpy((void*)pColorWriteEnables, (void*)copy_src.pColorWriteEnables, sizeof(VkBool32) * copy_src.attachmentCount);
    }

    return *this;
}

safe_VkPipelineColorWriteCreateInfoEXT::~safe_VkPipelineColorWriteCreateInfoEXT() {
    if (pColorWriteEnables) delete[] pColorWriteEnables;
    FreePnextChain(pNext);
}

void safe_VkPipelineColorWriteCreateInfoEXT::initialize(const VkPipelineColorWriteCreateInfoEXT* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pColorWriteEnables) delete[] pColorWriteEnables;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    attachmentCount = in_struct->attachmentCount;
    pColorWriteEnables = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pColorWriteEnables) {
        pColorWriteEnables = new VkBool32[in_struct->attachmentCount];
        memcpy((void*)pColorWriteEnables, (void*)in_struct->pColorWriteEnables, sizeof(VkBool32) * in_struct->attachmentCount);
    }
}

void safe_VkPipelineColorWriteCreateInfoEXT::initialize(const safe_VkPipelineColorWriteCreateInfoEXT* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    attachmentCount = copy_src->attachmentCount;
    pColorWriteEnables = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pColorWriteEnables) {
        pColorWriteEnables = new VkBool32[copy_src->attachmentCount];
        memcpy((void*)pColorWriteEnables, (void*)copy_src->pColorWriteEnables, sizeof(VkBool32) * copy_src->attachmentCount);
    }
}

safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(
    const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      primitivesGeneratedQuery(in_struct->primitivesGeneratedQuery),
      primitivesGeneratedQueryWithRasterizerDiscard(in_struct->primitivesGeneratedQueryWithRasterizerDiscard),
      primitivesGeneratedQueryWithNonZeroStreams(in_struct->primitivesGeneratedQueryWithNonZeroStreams) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT),
      pNext(nullptr),
      primitivesGeneratedQuery(),
      primitivesGeneratedQueryWithRasterizerDiscard(),
      primitivesGeneratedQueryWithNonZeroStreams() {}

safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(
    const safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    primitivesGeneratedQuery = copy_src.primitivesGeneratedQuery;
    primitivesGeneratedQueryWithRasterizerDiscard = copy_src.primitivesGeneratedQueryWithRasterizerDiscard;
    primitivesGeneratedQueryWithNonZeroStreams = copy_src.primitivesGeneratedQueryWithNonZeroStreams;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT& safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::operator=(
    const safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    primitivesGeneratedQuery = copy_src.primitivesGeneratedQuery;
    primitivesGeneratedQueryWithRasterizerDiscard = copy_src.primitivesGeneratedQueryWithRasterizerDiscard;
    primitivesGeneratedQueryWithNonZeroStreams = copy_src.primitivesGeneratedQueryWithNonZeroStreams;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::~safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::initialize(
    const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    primitivesGeneratedQuery = in_struct->primitivesGeneratedQuery;
    primitivesGeneratedQueryWithRasterizerDiscard = in_struct->primitivesGeneratedQueryWithRasterizerDiscard;
    primitivesGeneratedQueryWithNonZeroStreams = in_struct->primitivesGeneratedQueryWithNonZeroStreams;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::initialize(
    const safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    primitivesGeneratedQuery = copy_src->primitivesGeneratedQuery;
    primitivesGeneratedQueryWithRasterizerDiscard = copy_src->primitivesGeneratedQueryWithRasterizerDiscard;
    primitivesGeneratedQueryWithNonZeroStreams = copy_src->primitivesGeneratedQueryWithNonZeroStreams;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT(
    const VkPhysicalDeviceImageViewMinLodFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minLod(in_struct->minLod) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT), pNext(nullptr), minLod() {}

safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT(
    const safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    minLod = copy_src.minLod;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT& safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minLod = copy_src.minLod;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::~safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::initialize(const VkPhysicalDeviceImageViewMinLodFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minLod = in_struct->minLod;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT::initialize(const safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minLod = copy_src->minLod;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewMinLodCreateInfoEXT::safe_VkImageViewMinLodCreateInfoEXT(const VkImageViewMinLodCreateInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), minLod(in_struct->minLod) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewMinLodCreateInfoEXT::safe_VkImageViewMinLodCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT), pNext(nullptr), minLod() {}

safe_VkImageViewMinLodCreateInfoEXT::safe_VkImageViewMinLodCreateInfoEXT(const safe_VkImageViewMinLodCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    minLod = copy_src.minLod;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewMinLodCreateInfoEXT& safe_VkImageViewMinLodCreateInfoEXT::operator=(
    const safe_VkImageViewMinLodCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minLod = copy_src.minLod;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewMinLodCreateInfoEXT::~safe_VkImageViewMinLodCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkImageViewMinLodCreateInfoEXT::initialize(const VkImageViewMinLodCreateInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minLod = in_struct->minLod;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewMinLodCreateInfoEXT::initialize(const safe_VkImageViewMinLodCreateInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minLod = copy_src->minLod;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMultiDrawFeaturesEXT::safe_VkPhysicalDeviceMultiDrawFeaturesEXT(
    const VkPhysicalDeviceMultiDrawFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), multiDraw(in_struct->multiDraw) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultiDrawFeaturesEXT::safe_VkPhysicalDeviceMultiDrawFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT), pNext(nullptr), multiDraw() {}

safe_VkPhysicalDeviceMultiDrawFeaturesEXT::safe_VkPhysicalDeviceMultiDrawFeaturesEXT(
    const safe_VkPhysicalDeviceMultiDrawFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    multiDraw = copy_src.multiDraw;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultiDrawFeaturesEXT& safe_VkPhysicalDeviceMultiDrawFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMultiDrawFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    multiDraw = copy_src.multiDraw;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultiDrawFeaturesEXT::~safe_VkPhysicalDeviceMultiDrawFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMultiDrawFeaturesEXT::initialize(const VkPhysicalDeviceMultiDrawFeaturesEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    multiDraw = in_struct->multiDraw;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultiDrawFeaturesEXT::initialize(const safe_VkPhysicalDeviceMultiDrawFeaturesEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    multiDraw = copy_src->multiDraw;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMultiDrawPropertiesEXT::safe_VkPhysicalDeviceMultiDrawPropertiesEXT(
    const VkPhysicalDeviceMultiDrawPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxMultiDrawCount(in_struct->maxMultiDrawCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultiDrawPropertiesEXT::safe_VkPhysicalDeviceMultiDrawPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT), pNext(nullptr), maxMultiDrawCount() {}

safe_VkPhysicalDeviceMultiDrawPropertiesEXT::safe_VkPhysicalDeviceMultiDrawPropertiesEXT(
    const safe_VkPhysicalDeviceMultiDrawPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxMultiDrawCount = copy_src.maxMultiDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultiDrawPropertiesEXT& safe_VkPhysicalDeviceMultiDrawPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceMultiDrawPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxMultiDrawCount = copy_src.maxMultiDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultiDrawPropertiesEXT::~safe_VkPhysicalDeviceMultiDrawPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMultiDrawPropertiesEXT::initialize(const VkPhysicalDeviceMultiDrawPropertiesEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxMultiDrawCount = in_struct->maxMultiDrawCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultiDrawPropertiesEXT::initialize(const safe_VkPhysicalDeviceMultiDrawPropertiesEXT* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxMultiDrawCount = copy_src->maxMultiDrawCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT(
    const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), image2DViewOf3D(in_struct->image2DViewOf3D), sampler2DViewOf3D(in_struct->sampler2DViewOf3D) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT),
      pNext(nullptr),
      image2DViewOf3D(),
      sampler2DViewOf3D() {}

safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT(
    const safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    image2DViewOf3D = copy_src.image2DViewOf3D;
    sampler2DViewOf3D = copy_src.sampler2DViewOf3D;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT& safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image2DViewOf3D = copy_src.image2DViewOf3D;
    sampler2DViewOf3D = copy_src.sampler2DViewOf3D;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::~safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::initialize(const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image2DViewOf3D = in_struct->image2DViewOf3D;
    sampler2DViewOf3D = in_struct->sampler2DViewOf3D;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::initialize(const safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image2DViewOf3D = copy_src->image2DViewOf3D;
    sampler2DViewOf3D = copy_src->sampler2DViewOf3D;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::safe_VkPhysicalDeviceShaderTileImageFeaturesEXT(
    const VkPhysicalDeviceShaderTileImageFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderTileImageColorReadAccess(in_struct->shaderTileImageColorReadAccess),
      shaderTileImageDepthReadAccess(in_struct->shaderTileImageDepthReadAccess),
      shaderTileImageStencilReadAccess(in_struct->shaderTileImageStencilReadAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::safe_VkPhysicalDeviceShaderTileImageFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT),
      pNext(nullptr),
      shaderTileImageColorReadAccess(),
      shaderTileImageDepthReadAccess(),
      shaderTileImageStencilReadAccess() {}

safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::safe_VkPhysicalDeviceShaderTileImageFeaturesEXT(
    const safe_VkPhysicalDeviceShaderTileImageFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderTileImageColorReadAccess = copy_src.shaderTileImageColorReadAccess;
    shaderTileImageDepthReadAccess = copy_src.shaderTileImageDepthReadAccess;
    shaderTileImageStencilReadAccess = copy_src.shaderTileImageStencilReadAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderTileImageFeaturesEXT& safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderTileImageFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderTileImageColorReadAccess = copy_src.shaderTileImageColorReadAccess;
    shaderTileImageDepthReadAccess = copy_src.shaderTileImageDepthReadAccess;
    shaderTileImageStencilReadAccess = copy_src.shaderTileImageStencilReadAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::~safe_VkPhysicalDeviceShaderTileImageFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::initialize(const VkPhysicalDeviceShaderTileImageFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderTileImageColorReadAccess = in_struct->shaderTileImageColorReadAccess;
    shaderTileImageDepthReadAccess = in_struct->shaderTileImageDepthReadAccess;
    shaderTileImageStencilReadAccess = in_struct->shaderTileImageStencilReadAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderTileImageFeaturesEXT::initialize(const safe_VkPhysicalDeviceShaderTileImageFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderTileImageColorReadAccess = copy_src->shaderTileImageColorReadAccess;
    shaderTileImageDepthReadAccess = copy_src->shaderTileImageDepthReadAccess;
    shaderTileImageStencilReadAccess = copy_src->shaderTileImageStencilReadAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::safe_VkPhysicalDeviceShaderTileImagePropertiesEXT(
    const VkPhysicalDeviceShaderTileImagePropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderTileImageCoherentReadAccelerated(in_struct->shaderTileImageCoherentReadAccelerated),
      shaderTileImageReadSampleFromPixelRateInvocation(in_struct->shaderTileImageReadSampleFromPixelRateInvocation),
      shaderTileImageReadFromHelperInvocation(in_struct->shaderTileImageReadFromHelperInvocation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::safe_VkPhysicalDeviceShaderTileImagePropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT),
      pNext(nullptr),
      shaderTileImageCoherentReadAccelerated(),
      shaderTileImageReadSampleFromPixelRateInvocation(),
      shaderTileImageReadFromHelperInvocation() {}

safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::safe_VkPhysicalDeviceShaderTileImagePropertiesEXT(
    const safe_VkPhysicalDeviceShaderTileImagePropertiesEXT& copy_src) {
    sType = copy_src.sType;
    shaderTileImageCoherentReadAccelerated = copy_src.shaderTileImageCoherentReadAccelerated;
    shaderTileImageReadSampleFromPixelRateInvocation = copy_src.shaderTileImageReadSampleFromPixelRateInvocation;
    shaderTileImageReadFromHelperInvocation = copy_src.shaderTileImageReadFromHelperInvocation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderTileImagePropertiesEXT& safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::operator=(
    const safe_VkPhysicalDeviceShaderTileImagePropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderTileImageCoherentReadAccelerated = copy_src.shaderTileImageCoherentReadAccelerated;
    shaderTileImageReadSampleFromPixelRateInvocation = copy_src.shaderTileImageReadSampleFromPixelRateInvocation;
    shaderTileImageReadFromHelperInvocation = copy_src.shaderTileImageReadFromHelperInvocation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::~safe_VkPhysicalDeviceShaderTileImagePropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::initialize(const VkPhysicalDeviceShaderTileImagePropertiesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderTileImageCoherentReadAccelerated = in_struct->shaderTileImageCoherentReadAccelerated;
    shaderTileImageReadSampleFromPixelRateInvocation = in_struct->shaderTileImageReadSampleFromPixelRateInvocation;
    shaderTileImageReadFromHelperInvocation = in_struct->shaderTileImageReadFromHelperInvocation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderTileImagePropertiesEXT::initialize(
    const safe_VkPhysicalDeviceShaderTileImagePropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderTileImageCoherentReadAccelerated = copy_src->shaderTileImageCoherentReadAccelerated;
    shaderTileImageReadSampleFromPixelRateInvocation = copy_src->shaderTileImageReadSampleFromPixelRateInvocation;
    shaderTileImageReadFromHelperInvocation = copy_src->shaderTileImageReadFromHelperInvocation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMicromapCreateInfoEXT::safe_VkMicromapCreateInfoEXT(const VkMicromapCreateInfoEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      createFlags(in_struct->createFlags),
      buffer(in_struct->buffer),
      offset(in_struct->offset),
      size(in_struct->size),
      type(in_struct->type),
      deviceAddress(in_struct->deviceAddress) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMicromapCreateInfoEXT::safe_VkMicromapCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MICROMAP_CREATE_INFO_EXT),
      pNext(nullptr),
      createFlags(),
      buffer(),
      offset(),
      size(),
      type(),
      deviceAddress() {}

safe_VkMicromapCreateInfoEXT::safe_VkMicromapCreateInfoEXT(const safe_VkMicromapCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    createFlags = copy_src.createFlags;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    size = copy_src.size;
    type = copy_src.type;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMicromapCreateInfoEXT& safe_VkMicromapCreateInfoEXT::operator=(const safe_VkMicromapCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    createFlags = copy_src.createFlags;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    size = copy_src.size;
    type = copy_src.type;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMicromapCreateInfoEXT::~safe_VkMicromapCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkMicromapCreateInfoEXT::initialize(const VkMicromapCreateInfoEXT* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    createFlags = in_struct->createFlags;
    buffer = in_struct->buffer;
    offset = in_struct->offset;
    size = in_struct->size;
    type = in_struct->type;
    deviceAddress = in_struct->deviceAddress;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMicromapCreateInfoEXT::initialize(const safe_VkMicromapCreateInfoEXT* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    createFlags = copy_src->createFlags;
    buffer = copy_src->buffer;
    offset = copy_src->offset;
    size = copy_src->size;
    type = copy_src->type;
    deviceAddress = copy_src->deviceAddress;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT(
    const VkPhysicalDeviceOpacityMicromapFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      micromap(in_struct->micromap),
      micromapCaptureReplay(in_struct->micromapCaptureReplay),
      micromapHostCommands(in_struct->micromapHostCommands) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT),
      pNext(nullptr),
      micromap(),
      micromapCaptureReplay(),
      micromapHostCommands() {}

safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT(
    const safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    micromap = copy_src.micromap;
    micromapCaptureReplay = copy_src.micromapCaptureReplay;
    micromapHostCommands = copy_src.micromapHostCommands;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT& safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    micromap = copy_src.micromap;
    micromapCaptureReplay = copy_src.micromapCaptureReplay;
    micromapHostCommands = copy_src.micromapHostCommands;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::~safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::initialize(const VkPhysicalDeviceOpacityMicromapFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    micromap = in_struct->micromap;
    micromapCaptureReplay = in_struct->micromapCaptureReplay;
    micromapHostCommands = in_struct->micromapHostCommands;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT::initialize(const safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    micromap = copy_src->micromap;
    micromapCaptureReplay = copy_src->micromapCaptureReplay;
    micromapHostCommands = copy_src->micromapHostCommands;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT(
    const VkPhysicalDeviceOpacityMicromapPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxOpacity2StateSubdivisionLevel(in_struct->maxOpacity2StateSubdivisionLevel),
      maxOpacity4StateSubdivisionLevel(in_struct->maxOpacity4StateSubdivisionLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT),
      pNext(nullptr),
      maxOpacity2StateSubdivisionLevel(),
      maxOpacity4StateSubdivisionLevel() {}

safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT(
    const safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxOpacity2StateSubdivisionLevel = copy_src.maxOpacity2StateSubdivisionLevel;
    maxOpacity4StateSubdivisionLevel = copy_src.maxOpacity4StateSubdivisionLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT& safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxOpacity2StateSubdivisionLevel = copy_src.maxOpacity2StateSubdivisionLevel;
    maxOpacity4StateSubdivisionLevel = copy_src.maxOpacity4StateSubdivisionLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::~safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::initialize(const VkPhysicalDeviceOpacityMicromapPropertiesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxOpacity2StateSubdivisionLevel = in_struct->maxOpacity2StateSubdivisionLevel;
    maxOpacity4StateSubdivisionLevel = in_struct->maxOpacity4StateSubdivisionLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxOpacity2StateSubdivisionLevel = copy_src->maxOpacity2StateSubdivisionLevel;
    maxOpacity4StateSubdivisionLevel = copy_src->maxOpacity4StateSubdivisionLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMicromapVersionInfoEXT::safe_VkMicromapVersionInfoEXT(const VkMicromapVersionInfoEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pVersionData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)in_struct->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkMicromapVersionInfoEXT::safe_VkMicromapVersionInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MICROMAP_VERSION_INFO_EXT), pNext(nullptr), pVersionData(nullptr) {}

safe_VkMicromapVersionInfoEXT::safe_VkMicromapVersionInfoEXT(const safe_VkMicromapVersionInfoEXT& copy_src) {
    sType = copy_src.sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src.pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkMicromapVersionInfoEXT& safe_VkMicromapVersionInfoEXT::operator=(const safe_VkMicromapVersionInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src.pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }

    return *this;
}

safe_VkMicromapVersionInfoEXT::~safe_VkMicromapVersionInfoEXT() {
    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);
}

void safe_VkMicromapVersionInfoEXT::initialize(const VkMicromapVersionInfoEXT* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)in_struct->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

void safe_VkMicromapVersionInfoEXT::initialize(const safe_VkMicromapVersionInfoEXT* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkCopyMicromapToMemoryInfoEXT::safe_VkCopyMicromapToMemoryInfoEXT(const VkCopyMicromapToMemoryInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(&in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyMicromapToMemoryInfoEXT::safe_VkCopyMicromapToMemoryInfoEXT()
    : sType(VK_STRUCTURE_TYPE_COPY_MICROMAP_TO_MEMORY_INFO_EXT), pNext(nullptr), src(), mode() {}

safe_VkCopyMicromapToMemoryInfoEXT::safe_VkCopyMicromapToMemoryInfoEXT(const safe_VkCopyMicromapToMemoryInfoEXT& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyMicromapToMemoryInfoEXT& safe_VkCopyMicromapToMemoryInfoEXT::operator=(
    const safe_VkCopyMicromapToMemoryInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyMicromapToMemoryInfoEXT::~safe_VkCopyMicromapToMemoryInfoEXT() { FreePnextChain(pNext); }

void safe_VkCopyMicromapToMemoryInfoEXT::initialize(const VkCopyMicromapToMemoryInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst.initialize(&in_struct->dst);
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyMicromapToMemoryInfoEXT::initialize(const safe_VkCopyMicromapToMemoryInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst.initialize(&copy_src->dst);
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyMemoryToMicromapInfoEXT::safe_VkCopyMemoryToMicromapInfoEXT(const VkCopyMemoryToMicromapInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(&in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyMemoryToMicromapInfoEXT::safe_VkCopyMemoryToMicromapInfoEXT()
    : sType(VK_STRUCTURE_TYPE_COPY_MEMORY_TO_MICROMAP_INFO_EXT), pNext(nullptr), dst(), mode() {}

safe_VkCopyMemoryToMicromapInfoEXT::safe_VkCopyMemoryToMicromapInfoEXT(const safe_VkCopyMemoryToMicromapInfoEXT& copy_src) {
    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyMemoryToMicromapInfoEXT& safe_VkCopyMemoryToMicromapInfoEXT::operator=(
    const safe_VkCopyMemoryToMicromapInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyMemoryToMicromapInfoEXT::~safe_VkCopyMemoryToMicromapInfoEXT() { FreePnextChain(pNext); }

void safe_VkCopyMemoryToMicromapInfoEXT::initialize(const VkCopyMemoryToMicromapInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src.initialize(&in_struct->src);
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyMemoryToMicromapInfoEXT::initialize(const safe_VkCopyMemoryToMicromapInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src.initialize(&copy_src->src);
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyMicromapInfoEXT::safe_VkCopyMicromapInfoEXT(const VkCopyMicromapInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyMicromapInfoEXT::safe_VkCopyMicromapInfoEXT()
    : sType(VK_STRUCTURE_TYPE_COPY_MICROMAP_INFO_EXT), pNext(nullptr), src(), dst(), mode() {}

safe_VkCopyMicromapInfoEXT::safe_VkCopyMicromapInfoEXT(const safe_VkCopyMicromapInfoEXT& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyMicromapInfoEXT& safe_VkCopyMicromapInfoEXT::operator=(const safe_VkCopyMicromapInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyMicromapInfoEXT::~safe_VkCopyMicromapInfoEXT() { FreePnextChain(pNext); }

void safe_VkCopyMicromapInfoEXT::initialize(const VkCopyMicromapInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyMicromapInfoEXT::initialize(const safe_VkCopyMicromapInfoEXT* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMicromapBuildSizesInfoEXT::safe_VkMicromapBuildSizesInfoEXT(const VkMicromapBuildSizesInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      micromapSize(in_struct->micromapSize),
      buildScratchSize(in_struct->buildScratchSize),
      discardable(in_struct->discardable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMicromapBuildSizesInfoEXT::safe_VkMicromapBuildSizesInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT), pNext(nullptr), micromapSize(), buildScratchSize(), discardable() {}

safe_VkMicromapBuildSizesInfoEXT::safe_VkMicromapBuildSizesInfoEXT(const safe_VkMicromapBuildSizesInfoEXT& copy_src) {
    sType = copy_src.sType;
    micromapSize = copy_src.micromapSize;
    buildScratchSize = copy_src.buildScratchSize;
    discardable = copy_src.discardable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMicromapBuildSizesInfoEXT& safe_VkMicromapBuildSizesInfoEXT::operator=(const safe_VkMicromapBuildSizesInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    micromapSize = copy_src.micromapSize;
    buildScratchSize = copy_src.buildScratchSize;
    discardable = copy_src.discardable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMicromapBuildSizesInfoEXT::~safe_VkMicromapBuildSizesInfoEXT() { FreePnextChain(pNext); }

void safe_VkMicromapBuildSizesInfoEXT::initialize(const VkMicromapBuildSizesInfoEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    micromapSize = in_struct->micromapSize;
    buildScratchSize = in_struct->buildScratchSize;
    discardable = in_struct->discardable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMicromapBuildSizesInfoEXT::initialize(const safe_VkMicromapBuildSizesInfoEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    micromapSize = copy_src->micromapSize;
    buildScratchSize = copy_src->buildScratchSize;
    discardable = copy_src->discardable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT(
    const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      borderColorSwizzle(in_struct->borderColorSwizzle),
      borderColorSwizzleFromImage(in_struct->borderColorSwizzleFromImage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT),
      pNext(nullptr),
      borderColorSwizzle(),
      borderColorSwizzleFromImage() {}

safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT(
    const safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    borderColorSwizzle = copy_src.borderColorSwizzle;
    borderColorSwizzleFromImage = copy_src.borderColorSwizzleFromImage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT& safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    borderColorSwizzle = copy_src.borderColorSwizzle;
    borderColorSwizzleFromImage = copy_src.borderColorSwizzleFromImage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::~safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::initialize(const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    borderColorSwizzle = in_struct->borderColorSwizzle;
    borderColorSwizzleFromImage = in_struct->borderColorSwizzleFromImage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    borderColorSwizzle = copy_src->borderColorSwizzle;
    borderColorSwizzleFromImage = copy_src->borderColorSwizzleFromImage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::safe_VkSamplerBorderColorComponentMappingCreateInfoEXT(
    const VkSamplerBorderColorComponentMappingCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), components(in_struct->components), srgb(in_struct->srgb) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::safe_VkSamplerBorderColorComponentMappingCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT), pNext(nullptr), components(), srgb() {}

safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::safe_VkSamplerBorderColorComponentMappingCreateInfoEXT(
    const safe_VkSamplerBorderColorComponentMappingCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    components = copy_src.components;
    srgb = copy_src.srgb;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerBorderColorComponentMappingCreateInfoEXT& safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::operator=(
    const safe_VkSamplerBorderColorComponentMappingCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    components = copy_src.components;
    srgb = copy_src.srgb;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::~safe_VkSamplerBorderColorComponentMappingCreateInfoEXT() {
    FreePnextChain(pNext);
}

void safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::initialize(
    const VkSamplerBorderColorComponentMappingCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    components = in_struct->components;
    srgb = in_struct->srgb;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerBorderColorComponentMappingCreateInfoEXT::initialize(
    const safe_VkSamplerBorderColorComponentMappingCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    components = copy_src->components;
    srgb = copy_src->srgb;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(
    const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), pageableDeviceLocalMemory(in_struct->pageableDeviceLocalMemory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT),
      pNext(nullptr),
      pageableDeviceLocalMemory() {}

safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(
    const safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    pageableDeviceLocalMemory = copy_src.pageableDeviceLocalMemory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT& safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::operator=(
    const safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pageableDeviceLocalMemory = copy_src.pageableDeviceLocalMemory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::~safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::initialize(
    const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pageableDeviceLocalMemory = in_struct->pageableDeviceLocalMemory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::initialize(
    const safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pageableDeviceLocalMemory = copy_src->pageableDeviceLocalMemory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(
    const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageSlicedViewOf3D(in_struct->imageSlicedViewOf3D) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT), pNext(nullptr), imageSlicedViewOf3D() {}

safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(
    const safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    imageSlicedViewOf3D = copy_src.imageSlicedViewOf3D;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT& safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageSlicedViewOf3D = copy_src.imageSlicedViewOf3D;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::~safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::initialize(
    const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageSlicedViewOf3D = in_struct->imageSlicedViewOf3D;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageSlicedViewOf3D = copy_src->imageSlicedViewOf3D;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewSlicedCreateInfoEXT::safe_VkImageViewSlicedCreateInfoEXT(const VkImageViewSlicedCreateInfoEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), sliceOffset(in_struct->sliceOffset), sliceCount(in_struct->sliceCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewSlicedCreateInfoEXT::safe_VkImageViewSlicedCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT), pNext(nullptr), sliceOffset(), sliceCount() {}

safe_VkImageViewSlicedCreateInfoEXT::safe_VkImageViewSlicedCreateInfoEXT(const safe_VkImageViewSlicedCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    sliceOffset = copy_src.sliceOffset;
    sliceCount = copy_src.sliceCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewSlicedCreateInfoEXT& safe_VkImageViewSlicedCreateInfoEXT::operator=(
    const safe_VkImageViewSlicedCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sliceOffset = copy_src.sliceOffset;
    sliceCount = copy_src.sliceCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewSlicedCreateInfoEXT::~safe_VkImageViewSlicedCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkImageViewSlicedCreateInfoEXT::initialize(const VkImageViewSlicedCreateInfoEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sliceOffset = in_struct->sliceOffset;
    sliceCount = in_struct->sliceCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewSlicedCreateInfoEXT::initialize(const safe_VkImageViewSlicedCreateInfoEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sliceOffset = copy_src->sliceOffset;
    sliceCount = copy_src->sliceCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(
    const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), nonSeamlessCubeMap(in_struct->nonSeamlessCubeMap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT), pNext(nullptr), nonSeamlessCubeMap() {}

safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(
    const safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    nonSeamlessCubeMap = copy_src.nonSeamlessCubeMap;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT& safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    nonSeamlessCubeMap = copy_src.nonSeamlessCubeMap;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::~safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::initialize(const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    nonSeamlessCubeMap = in_struct->nonSeamlessCubeMap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    nonSeamlessCubeMap = copy_src->nonSeamlessCubeMap;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT(
    const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityMapOffset(in_struct->fragmentDensityMapOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_EXT),
      pNext(nullptr),
      fragmentDensityMapOffset() {}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityMapOffset = copy_src.fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT& safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityMapOffset = copy_src.fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::~safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::initialize(
    const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityMapOffset = in_struct->fragmentDensityMapOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityMapOffset = copy_src->fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT(
    const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityOffsetGranularity(in_struct->fragmentDensityOffsetGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_EXT),
      pNext(nullptr),
      fragmentDensityOffsetGranularity() {}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityOffsetGranularity = copy_src.fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT& safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityOffsetGranularity = copy_src.fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::~safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::initialize(
    const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityOffsetGranularity = in_struct->fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityOffsetGranularity = copy_src->fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT(
    const VkRenderPassFragmentDensityMapOffsetEndInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityOffsetCount(in_struct->fragmentDensityOffsetCount), pFragmentDensityOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[in_struct->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)in_struct->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * in_struct->fragmentDensityOffsetCount);
    }
}

safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_EXT),
      pNext(nullptr),
      fragmentDensityOffsetCount(),
      pFragmentDensityOffsets(nullptr) {}

safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT(
    const safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT& copy_src) {
    sType = copy_src.sType;
    fragmentDensityOffsetCount = copy_src.fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src.fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src.pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src.fragmentDensityOffsetCount);
    }
}

safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT& safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::operator=(
    const safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityOffsetCount = copy_src.fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src.fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src.pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src.fragmentDensityOffsetCount);
    }

    return *this;
}

safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::~safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT() {
    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);
}

void safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::initialize(
    const VkRenderPassFragmentDensityMapOffsetEndInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityOffsetCount = in_struct->fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[in_struct->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)in_struct->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * in_struct->fragmentDensityOffsetCount);
    }
}

void safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT::initialize(
    const safe_VkRenderPassFragmentDensityMapOffsetEndInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityOffsetCount = copy_src->fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src->fragmentDensityOffsetCount);
    }
}

safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(
    const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), imageCompressionControlSwapchain(in_struct->imageCompressionControlSwapchain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT),
      pNext(nullptr),
      imageCompressionControlSwapchain() {}

safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(
    const safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    imageCompressionControlSwapchain = copy_src.imageCompressionControlSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT&
safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageCompressionControlSwapchain = copy_src.imageCompressionControlSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::
    ~safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::initialize(
    const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageCompressionControlSwapchain = in_struct->imageCompressionControlSwapchain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageCompressionControlSwapchain = copy_src->imageCompressionControlSwapchain;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT(
    const VkPhysicalDeviceNestedCommandBufferFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      nestedCommandBuffer(in_struct->nestedCommandBuffer),
      nestedCommandBufferRendering(in_struct->nestedCommandBufferRendering),
      nestedCommandBufferSimultaneousUse(in_struct->nestedCommandBufferSimultaneousUse) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT),
      pNext(nullptr),
      nestedCommandBuffer(),
      nestedCommandBufferRendering(),
      nestedCommandBufferSimultaneousUse() {}

safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT(
    const safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    nestedCommandBuffer = copy_src.nestedCommandBuffer;
    nestedCommandBufferRendering = copy_src.nestedCommandBufferRendering;
    nestedCommandBufferSimultaneousUse = copy_src.nestedCommandBufferSimultaneousUse;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT& safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    nestedCommandBuffer = copy_src.nestedCommandBuffer;
    nestedCommandBufferRendering = copy_src.nestedCommandBufferRendering;
    nestedCommandBufferSimultaneousUse = copy_src.nestedCommandBufferSimultaneousUse;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::~safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::initialize(
    const VkPhysicalDeviceNestedCommandBufferFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    nestedCommandBuffer = in_struct->nestedCommandBuffer;
    nestedCommandBufferRendering = in_struct->nestedCommandBufferRendering;
    nestedCommandBufferSimultaneousUse = in_struct->nestedCommandBufferSimultaneousUse;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    nestedCommandBuffer = copy_src->nestedCommandBuffer;
    nestedCommandBufferRendering = copy_src->nestedCommandBufferRendering;
    nestedCommandBufferSimultaneousUse = copy_src->nestedCommandBufferSimultaneousUse;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT(
    const VkPhysicalDeviceNestedCommandBufferPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxCommandBufferNestingLevel(in_struct->maxCommandBufferNestingLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT),
      pNext(nullptr),
      maxCommandBufferNestingLevel() {}

safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT(
    const safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxCommandBufferNestingLevel = copy_src.maxCommandBufferNestingLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT& safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxCommandBufferNestingLevel = copy_src.maxCommandBufferNestingLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::~safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::initialize(
    const VkPhysicalDeviceNestedCommandBufferPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxCommandBufferNestingLevel = in_struct->maxCommandBufferNestingLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxCommandBufferNestingLevel = copy_src->maxCommandBufferNestingLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExternalMemoryAcquireUnmodifiedEXT::safe_VkExternalMemoryAcquireUnmodifiedEXT(
    const VkExternalMemoryAcquireUnmodifiedEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), acquireUnmodifiedMemory(in_struct->acquireUnmodifiedMemory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExternalMemoryAcquireUnmodifiedEXT::safe_VkExternalMemoryAcquireUnmodifiedEXT()
    : sType(VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT), pNext(nullptr), acquireUnmodifiedMemory() {}

safe_VkExternalMemoryAcquireUnmodifiedEXT::safe_VkExternalMemoryAcquireUnmodifiedEXT(
    const safe_VkExternalMemoryAcquireUnmodifiedEXT& copy_src) {
    sType = copy_src.sType;
    acquireUnmodifiedMemory = copy_src.acquireUnmodifiedMemory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExternalMemoryAcquireUnmodifiedEXT& safe_VkExternalMemoryAcquireUnmodifiedEXT::operator=(
    const safe_VkExternalMemoryAcquireUnmodifiedEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    acquireUnmodifiedMemory = copy_src.acquireUnmodifiedMemory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExternalMemoryAcquireUnmodifiedEXT::~safe_VkExternalMemoryAcquireUnmodifiedEXT() { FreePnextChain(pNext); }

void safe_VkExternalMemoryAcquireUnmodifiedEXT::initialize(const VkExternalMemoryAcquireUnmodifiedEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    acquireUnmodifiedMemory = in_struct->acquireUnmodifiedMemory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExternalMemoryAcquireUnmodifiedEXT::initialize(const safe_VkExternalMemoryAcquireUnmodifiedEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    acquireUnmodifiedMemory = copy_src->acquireUnmodifiedMemory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT(
    const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      extendedDynamicState3TessellationDomainOrigin(in_struct->extendedDynamicState3TessellationDomainOrigin),
      extendedDynamicState3DepthClampEnable(in_struct->extendedDynamicState3DepthClampEnable),
      extendedDynamicState3PolygonMode(in_struct->extendedDynamicState3PolygonMode),
      extendedDynamicState3RasterizationSamples(in_struct->extendedDynamicState3RasterizationSamples),
      extendedDynamicState3SampleMask(in_struct->extendedDynamicState3SampleMask),
      extendedDynamicState3AlphaToCoverageEnable(in_struct->extendedDynamicState3AlphaToCoverageEnable),
      extendedDynamicState3AlphaToOneEnable(in_struct->extendedDynamicState3AlphaToOneEnable),
      extendedDynamicState3LogicOpEnable(in_struct->extendedDynamicState3LogicOpEnable),
      extendedDynamicState3ColorBlendEnable(in_struct->extendedDynamicState3ColorBlendEnable),
      extendedDynamicState3ColorBlendEquation(in_struct->extendedDynamicState3ColorBlendEquation),
      extendedDynamicState3ColorWriteMask(in_struct->extendedDynamicState3ColorWriteMask),
      extendedDynamicState3RasterizationStream(in_struct->extendedDynamicState3RasterizationStream),
      extendedDynamicState3ConservativeRasterizationMode(in_struct->extendedDynamicState3ConservativeRasterizationMode),
      extendedDynamicState3ExtraPrimitiveOverestimationSize(in_struct->extendedDynamicState3ExtraPrimitiveOverestimationSize),
      extendedDynamicState3DepthClipEnable(in_struct->extendedDynamicState3DepthClipEnable),
      extendedDynamicState3SampleLocationsEnable(in_struct->extendedDynamicState3SampleLocationsEnable),
      extendedDynamicState3ColorBlendAdvanced(in_struct->extendedDynamicState3ColorBlendAdvanced),
      extendedDynamicState3ProvokingVertexMode(in_struct->extendedDynamicState3ProvokingVertexMode),
      extendedDynamicState3LineRasterizationMode(in_struct->extendedDynamicState3LineRasterizationMode),
      extendedDynamicState3LineStippleEnable(in_struct->extendedDynamicState3LineStippleEnable),
      extendedDynamicState3DepthClipNegativeOneToOne(in_struct->extendedDynamicState3DepthClipNegativeOneToOne),
      extendedDynamicState3ViewportWScalingEnable(in_struct->extendedDynamicState3ViewportWScalingEnable),
      extendedDynamicState3ViewportSwizzle(in_struct->extendedDynamicState3ViewportSwizzle),
      extendedDynamicState3CoverageToColorEnable(in_struct->extendedDynamicState3CoverageToColorEnable),
      extendedDynamicState3CoverageToColorLocation(in_struct->extendedDynamicState3CoverageToColorLocation),
      extendedDynamicState3CoverageModulationMode(in_struct->extendedDynamicState3CoverageModulationMode),
      extendedDynamicState3CoverageModulationTableEnable(in_struct->extendedDynamicState3CoverageModulationTableEnable),
      extendedDynamicState3CoverageModulationTable(in_struct->extendedDynamicState3CoverageModulationTable),
      extendedDynamicState3CoverageReductionMode(in_struct->extendedDynamicState3CoverageReductionMode),
      extendedDynamicState3RepresentativeFragmentTestEnable(in_struct->extendedDynamicState3RepresentativeFragmentTestEnable),
      extendedDynamicState3ShadingRateImageEnable(in_struct->extendedDynamicState3ShadingRateImageEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT),
      pNext(nullptr),
      extendedDynamicState3TessellationDomainOrigin(),
      extendedDynamicState3DepthClampEnable(),
      extendedDynamicState3PolygonMode(),
      extendedDynamicState3RasterizationSamples(),
      extendedDynamicState3SampleMask(),
      extendedDynamicState3AlphaToCoverageEnable(),
      extendedDynamicState3AlphaToOneEnable(),
      extendedDynamicState3LogicOpEnable(),
      extendedDynamicState3ColorBlendEnable(),
      extendedDynamicState3ColorBlendEquation(),
      extendedDynamicState3ColorWriteMask(),
      extendedDynamicState3RasterizationStream(),
      extendedDynamicState3ConservativeRasterizationMode(),
      extendedDynamicState3ExtraPrimitiveOverestimationSize(),
      extendedDynamicState3DepthClipEnable(),
      extendedDynamicState3SampleLocationsEnable(),
      extendedDynamicState3ColorBlendAdvanced(),
      extendedDynamicState3ProvokingVertexMode(),
      extendedDynamicState3LineRasterizationMode(),
      extendedDynamicState3LineStippleEnable(),
      extendedDynamicState3DepthClipNegativeOneToOne(),
      extendedDynamicState3ViewportWScalingEnable(),
      extendedDynamicState3ViewportSwizzle(),
      extendedDynamicState3CoverageToColorEnable(),
      extendedDynamicState3CoverageToColorLocation(),
      extendedDynamicState3CoverageModulationMode(),
      extendedDynamicState3CoverageModulationTableEnable(),
      extendedDynamicState3CoverageModulationTable(),
      extendedDynamicState3CoverageReductionMode(),
      extendedDynamicState3RepresentativeFragmentTestEnable(),
      extendedDynamicState3ShadingRateImageEnable() {}

safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT(
    const safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    extendedDynamicState3TessellationDomainOrigin = copy_src.extendedDynamicState3TessellationDomainOrigin;
    extendedDynamicState3DepthClampEnable = copy_src.extendedDynamicState3DepthClampEnable;
    extendedDynamicState3PolygonMode = copy_src.extendedDynamicState3PolygonMode;
    extendedDynamicState3RasterizationSamples = copy_src.extendedDynamicState3RasterizationSamples;
    extendedDynamicState3SampleMask = copy_src.extendedDynamicState3SampleMask;
    extendedDynamicState3AlphaToCoverageEnable = copy_src.extendedDynamicState3AlphaToCoverageEnable;
    extendedDynamicState3AlphaToOneEnable = copy_src.extendedDynamicState3AlphaToOneEnable;
    extendedDynamicState3LogicOpEnable = copy_src.extendedDynamicState3LogicOpEnable;
    extendedDynamicState3ColorBlendEnable = copy_src.extendedDynamicState3ColorBlendEnable;
    extendedDynamicState3ColorBlendEquation = copy_src.extendedDynamicState3ColorBlendEquation;
    extendedDynamicState3ColorWriteMask = copy_src.extendedDynamicState3ColorWriteMask;
    extendedDynamicState3RasterizationStream = copy_src.extendedDynamicState3RasterizationStream;
    extendedDynamicState3ConservativeRasterizationMode = copy_src.extendedDynamicState3ConservativeRasterizationMode;
    extendedDynamicState3ExtraPrimitiveOverestimationSize = copy_src.extendedDynamicState3ExtraPrimitiveOverestimationSize;
    extendedDynamicState3DepthClipEnable = copy_src.extendedDynamicState3DepthClipEnable;
    extendedDynamicState3SampleLocationsEnable = copy_src.extendedDynamicState3SampleLocationsEnable;
    extendedDynamicState3ColorBlendAdvanced = copy_src.extendedDynamicState3ColorBlendAdvanced;
    extendedDynamicState3ProvokingVertexMode = copy_src.extendedDynamicState3ProvokingVertexMode;
    extendedDynamicState3LineRasterizationMode = copy_src.extendedDynamicState3LineRasterizationMode;
    extendedDynamicState3LineStippleEnable = copy_src.extendedDynamicState3LineStippleEnable;
    extendedDynamicState3DepthClipNegativeOneToOne = copy_src.extendedDynamicState3DepthClipNegativeOneToOne;
    extendedDynamicState3ViewportWScalingEnable = copy_src.extendedDynamicState3ViewportWScalingEnable;
    extendedDynamicState3ViewportSwizzle = copy_src.extendedDynamicState3ViewportSwizzle;
    extendedDynamicState3CoverageToColorEnable = copy_src.extendedDynamicState3CoverageToColorEnable;
    extendedDynamicState3CoverageToColorLocation = copy_src.extendedDynamicState3CoverageToColorLocation;
    extendedDynamicState3CoverageModulationMode = copy_src.extendedDynamicState3CoverageModulationMode;
    extendedDynamicState3CoverageModulationTableEnable = copy_src.extendedDynamicState3CoverageModulationTableEnable;
    extendedDynamicState3CoverageModulationTable = copy_src.extendedDynamicState3CoverageModulationTable;
    extendedDynamicState3CoverageReductionMode = copy_src.extendedDynamicState3CoverageReductionMode;
    extendedDynamicState3RepresentativeFragmentTestEnable = copy_src.extendedDynamicState3RepresentativeFragmentTestEnable;
    extendedDynamicState3ShadingRateImageEnable = copy_src.extendedDynamicState3ShadingRateImageEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT& safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    extendedDynamicState3TessellationDomainOrigin = copy_src.extendedDynamicState3TessellationDomainOrigin;
    extendedDynamicState3DepthClampEnable = copy_src.extendedDynamicState3DepthClampEnable;
    extendedDynamicState3PolygonMode = copy_src.extendedDynamicState3PolygonMode;
    extendedDynamicState3RasterizationSamples = copy_src.extendedDynamicState3RasterizationSamples;
    extendedDynamicState3SampleMask = copy_src.extendedDynamicState3SampleMask;
    extendedDynamicState3AlphaToCoverageEnable = copy_src.extendedDynamicState3AlphaToCoverageEnable;
    extendedDynamicState3AlphaToOneEnable = copy_src.extendedDynamicState3AlphaToOneEnable;
    extendedDynamicState3LogicOpEnable = copy_src.extendedDynamicState3LogicOpEnable;
    extendedDynamicState3ColorBlendEnable = copy_src.extendedDynamicState3ColorBlendEnable;
    extendedDynamicState3ColorBlendEquation = copy_src.extendedDynamicState3ColorBlendEquation;
    extendedDynamicState3ColorWriteMask = copy_src.extendedDynamicState3ColorWriteMask;
    extendedDynamicState3RasterizationStream = copy_src.extendedDynamicState3RasterizationStream;
    extendedDynamicState3ConservativeRasterizationMode = copy_src.extendedDynamicState3ConservativeRasterizationMode;
    extendedDynamicState3ExtraPrimitiveOverestimationSize = copy_src.extendedDynamicState3ExtraPrimitiveOverestimationSize;
    extendedDynamicState3DepthClipEnable = copy_src.extendedDynamicState3DepthClipEnable;
    extendedDynamicState3SampleLocationsEnable = copy_src.extendedDynamicState3SampleLocationsEnable;
    extendedDynamicState3ColorBlendAdvanced = copy_src.extendedDynamicState3ColorBlendAdvanced;
    extendedDynamicState3ProvokingVertexMode = copy_src.extendedDynamicState3ProvokingVertexMode;
    extendedDynamicState3LineRasterizationMode = copy_src.extendedDynamicState3LineRasterizationMode;
    extendedDynamicState3LineStippleEnable = copy_src.extendedDynamicState3LineStippleEnable;
    extendedDynamicState3DepthClipNegativeOneToOne = copy_src.extendedDynamicState3DepthClipNegativeOneToOne;
    extendedDynamicState3ViewportWScalingEnable = copy_src.extendedDynamicState3ViewportWScalingEnable;
    extendedDynamicState3ViewportSwizzle = copy_src.extendedDynamicState3ViewportSwizzle;
    extendedDynamicState3CoverageToColorEnable = copy_src.extendedDynamicState3CoverageToColorEnable;
    extendedDynamicState3CoverageToColorLocation = copy_src.extendedDynamicState3CoverageToColorLocation;
    extendedDynamicState3CoverageModulationMode = copy_src.extendedDynamicState3CoverageModulationMode;
    extendedDynamicState3CoverageModulationTableEnable = copy_src.extendedDynamicState3CoverageModulationTableEnable;
    extendedDynamicState3CoverageModulationTable = copy_src.extendedDynamicState3CoverageModulationTable;
    extendedDynamicState3CoverageReductionMode = copy_src.extendedDynamicState3CoverageReductionMode;
    extendedDynamicState3RepresentativeFragmentTestEnable = copy_src.extendedDynamicState3RepresentativeFragmentTestEnable;
    extendedDynamicState3ShadingRateImageEnable = copy_src.extendedDynamicState3ShadingRateImageEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::~safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::initialize(
    const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    extendedDynamicState3TessellationDomainOrigin = in_struct->extendedDynamicState3TessellationDomainOrigin;
    extendedDynamicState3DepthClampEnable = in_struct->extendedDynamicState3DepthClampEnable;
    extendedDynamicState3PolygonMode = in_struct->extendedDynamicState3PolygonMode;
    extendedDynamicState3RasterizationSamples = in_struct->extendedDynamicState3RasterizationSamples;
    extendedDynamicState3SampleMask = in_struct->extendedDynamicState3SampleMask;
    extendedDynamicState3AlphaToCoverageEnable = in_struct->extendedDynamicState3AlphaToCoverageEnable;
    extendedDynamicState3AlphaToOneEnable = in_struct->extendedDynamicState3AlphaToOneEnable;
    extendedDynamicState3LogicOpEnable = in_struct->extendedDynamicState3LogicOpEnable;
    extendedDynamicState3ColorBlendEnable = in_struct->extendedDynamicState3ColorBlendEnable;
    extendedDynamicState3ColorBlendEquation = in_struct->extendedDynamicState3ColorBlendEquation;
    extendedDynamicState3ColorWriteMask = in_struct->extendedDynamicState3ColorWriteMask;
    extendedDynamicState3RasterizationStream = in_struct->extendedDynamicState3RasterizationStream;
    extendedDynamicState3ConservativeRasterizationMode = in_struct->extendedDynamicState3ConservativeRasterizationMode;
    extendedDynamicState3ExtraPrimitiveOverestimationSize = in_struct->extendedDynamicState3ExtraPrimitiveOverestimationSize;
    extendedDynamicState3DepthClipEnable = in_struct->extendedDynamicState3DepthClipEnable;
    extendedDynamicState3SampleLocationsEnable = in_struct->extendedDynamicState3SampleLocationsEnable;
    extendedDynamicState3ColorBlendAdvanced = in_struct->extendedDynamicState3ColorBlendAdvanced;
    extendedDynamicState3ProvokingVertexMode = in_struct->extendedDynamicState3ProvokingVertexMode;
    extendedDynamicState3LineRasterizationMode = in_struct->extendedDynamicState3LineRasterizationMode;
    extendedDynamicState3LineStippleEnable = in_struct->extendedDynamicState3LineStippleEnable;
    extendedDynamicState3DepthClipNegativeOneToOne = in_struct->extendedDynamicState3DepthClipNegativeOneToOne;
    extendedDynamicState3ViewportWScalingEnable = in_struct->extendedDynamicState3ViewportWScalingEnable;
    extendedDynamicState3ViewportSwizzle = in_struct->extendedDynamicState3ViewportSwizzle;
    extendedDynamicState3CoverageToColorEnable = in_struct->extendedDynamicState3CoverageToColorEnable;
    extendedDynamicState3CoverageToColorLocation = in_struct->extendedDynamicState3CoverageToColorLocation;
    extendedDynamicState3CoverageModulationMode = in_struct->extendedDynamicState3CoverageModulationMode;
    extendedDynamicState3CoverageModulationTableEnable = in_struct->extendedDynamicState3CoverageModulationTableEnable;
    extendedDynamicState3CoverageModulationTable = in_struct->extendedDynamicState3CoverageModulationTable;
    extendedDynamicState3CoverageReductionMode = in_struct->extendedDynamicState3CoverageReductionMode;
    extendedDynamicState3RepresentativeFragmentTestEnable = in_struct->extendedDynamicState3RepresentativeFragmentTestEnable;
    extendedDynamicState3ShadingRateImageEnable = in_struct->extendedDynamicState3ShadingRateImageEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::initialize(
    const safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    extendedDynamicState3TessellationDomainOrigin = copy_src->extendedDynamicState3TessellationDomainOrigin;
    extendedDynamicState3DepthClampEnable = copy_src->extendedDynamicState3DepthClampEnable;
    extendedDynamicState3PolygonMode = copy_src->extendedDynamicState3PolygonMode;
    extendedDynamicState3RasterizationSamples = copy_src->extendedDynamicState3RasterizationSamples;
    extendedDynamicState3SampleMask = copy_src->extendedDynamicState3SampleMask;
    extendedDynamicState3AlphaToCoverageEnable = copy_src->extendedDynamicState3AlphaToCoverageEnable;
    extendedDynamicState3AlphaToOneEnable = copy_src->extendedDynamicState3AlphaToOneEnable;
    extendedDynamicState3LogicOpEnable = copy_src->extendedDynamicState3LogicOpEnable;
    extendedDynamicState3ColorBlendEnable = copy_src->extendedDynamicState3ColorBlendEnable;
    extendedDynamicState3ColorBlendEquation = copy_src->extendedDynamicState3ColorBlendEquation;
    extendedDynamicState3ColorWriteMask = copy_src->extendedDynamicState3ColorWriteMask;
    extendedDynamicState3RasterizationStream = copy_src->extendedDynamicState3RasterizationStream;
    extendedDynamicState3ConservativeRasterizationMode = copy_src->extendedDynamicState3ConservativeRasterizationMode;
    extendedDynamicState3ExtraPrimitiveOverestimationSize = copy_src->extendedDynamicState3ExtraPrimitiveOverestimationSize;
    extendedDynamicState3DepthClipEnable = copy_src->extendedDynamicState3DepthClipEnable;
    extendedDynamicState3SampleLocationsEnable = copy_src->extendedDynamicState3SampleLocationsEnable;
    extendedDynamicState3ColorBlendAdvanced = copy_src->extendedDynamicState3ColorBlendAdvanced;
    extendedDynamicState3ProvokingVertexMode = copy_src->extendedDynamicState3ProvokingVertexMode;
    extendedDynamicState3LineRasterizationMode = copy_src->extendedDynamicState3LineRasterizationMode;
    extendedDynamicState3LineStippleEnable = copy_src->extendedDynamicState3LineStippleEnable;
    extendedDynamicState3DepthClipNegativeOneToOne = copy_src->extendedDynamicState3DepthClipNegativeOneToOne;
    extendedDynamicState3ViewportWScalingEnable = copy_src->extendedDynamicState3ViewportWScalingEnable;
    extendedDynamicState3ViewportSwizzle = copy_src->extendedDynamicState3ViewportSwizzle;
    extendedDynamicState3CoverageToColorEnable = copy_src->extendedDynamicState3CoverageToColorEnable;
    extendedDynamicState3CoverageToColorLocation = copy_src->extendedDynamicState3CoverageToColorLocation;
    extendedDynamicState3CoverageModulationMode = copy_src->extendedDynamicState3CoverageModulationMode;
    extendedDynamicState3CoverageModulationTableEnable = copy_src->extendedDynamicState3CoverageModulationTableEnable;
    extendedDynamicState3CoverageModulationTable = copy_src->extendedDynamicState3CoverageModulationTable;
    extendedDynamicState3CoverageReductionMode = copy_src->extendedDynamicState3CoverageReductionMode;
    extendedDynamicState3RepresentativeFragmentTestEnable = copy_src->extendedDynamicState3RepresentativeFragmentTestEnable;
    extendedDynamicState3ShadingRateImageEnable = copy_src->extendedDynamicState3ShadingRateImageEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT(
    const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), dynamicPrimitiveTopologyUnrestricted(in_struct->dynamicPrimitiveTopologyUnrestricted) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT),
      pNext(nullptr),
      dynamicPrimitiveTopologyUnrestricted() {}

safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT(
    const safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT& copy_src) {
    sType = copy_src.sType;
    dynamicPrimitiveTopologyUnrestricted = copy_src.dynamicPrimitiveTopologyUnrestricted;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT& safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::operator=(
    const safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dynamicPrimitiveTopologyUnrestricted = copy_src.dynamicPrimitiveTopologyUnrestricted;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::~safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::initialize(
    const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dynamicPrimitiveTopologyUnrestricted = in_struct->dynamicPrimitiveTopologyUnrestricted;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT::initialize(
    const safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dynamicPrimitiveTopologyUnrestricted = copy_src->dynamicPrimitiveTopologyUnrestricted;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(
    const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), subpassMergeFeedback(in_struct->subpassMergeFeedback) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT), pNext(nullptr), subpassMergeFeedback() {}

safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(
    const safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    subpassMergeFeedback = copy_src.subpassMergeFeedback;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT& safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    subpassMergeFeedback = copy_src.subpassMergeFeedback;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::~safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::initialize(
    const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    subpassMergeFeedback = in_struct->subpassMergeFeedback;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    subpassMergeFeedback = copy_src->subpassMergeFeedback;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassCreationControlEXT::safe_VkRenderPassCreationControlEXT(const VkRenderPassCreationControlEXT* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), disallowMerging(in_struct->disallowMerging) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderPassCreationControlEXT::safe_VkRenderPassCreationControlEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT), pNext(nullptr), disallowMerging() {}

safe_VkRenderPassCreationControlEXT::safe_VkRenderPassCreationControlEXT(const safe_VkRenderPassCreationControlEXT& copy_src) {
    sType = copy_src.sType;
    disallowMerging = copy_src.disallowMerging;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderPassCreationControlEXT& safe_VkRenderPassCreationControlEXT::operator=(
    const safe_VkRenderPassCreationControlEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    disallowMerging = copy_src.disallowMerging;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderPassCreationControlEXT::~safe_VkRenderPassCreationControlEXT() { FreePnextChain(pNext); }

void safe_VkRenderPassCreationControlEXT::initialize(const VkRenderPassCreationControlEXT* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    disallowMerging = in_struct->disallowMerging;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderPassCreationControlEXT::initialize(const safe_VkRenderPassCreationControlEXT* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    disallowMerging = copy_src->disallowMerging;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassCreationFeedbackCreateInfoEXT::safe_VkRenderPassCreationFeedbackCreateInfoEXT(
    const VkRenderPassCreationFeedbackCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pRenderPassFeedback(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pRenderPassFeedback) {
        pRenderPassFeedback = new VkRenderPassCreationFeedbackInfoEXT(*in_struct->pRenderPassFeedback);
    }
}

safe_VkRenderPassCreationFeedbackCreateInfoEXT::safe_VkRenderPassCreationFeedbackCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT), pNext(nullptr), pRenderPassFeedback(nullptr) {}

safe_VkRenderPassCreationFeedbackCreateInfoEXT::safe_VkRenderPassCreationFeedbackCreateInfoEXT(
    const safe_VkRenderPassCreationFeedbackCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    pRenderPassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pRenderPassFeedback) {
        pRenderPassFeedback = new VkRenderPassCreationFeedbackInfoEXT(*copy_src.pRenderPassFeedback);
    }
}

safe_VkRenderPassCreationFeedbackCreateInfoEXT& safe_VkRenderPassCreationFeedbackCreateInfoEXT::operator=(
    const safe_VkRenderPassCreationFeedbackCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pRenderPassFeedback) delete pRenderPassFeedback;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pRenderPassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pRenderPassFeedback) {
        pRenderPassFeedback = new VkRenderPassCreationFeedbackInfoEXT(*copy_src.pRenderPassFeedback);
    }

    return *this;
}

safe_VkRenderPassCreationFeedbackCreateInfoEXT::~safe_VkRenderPassCreationFeedbackCreateInfoEXT() {
    if (pRenderPassFeedback) delete pRenderPassFeedback;
    FreePnextChain(pNext);
}

void safe_VkRenderPassCreationFeedbackCreateInfoEXT::initialize(const VkRenderPassCreationFeedbackCreateInfoEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRenderPassFeedback) delete pRenderPassFeedback;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pRenderPassFeedback = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pRenderPassFeedback) {
        pRenderPassFeedback = new VkRenderPassCreationFeedbackInfoEXT(*in_struct->pRenderPassFeedback);
    }
}

void safe_VkRenderPassCreationFeedbackCreateInfoEXT::initialize(const safe_VkRenderPassCreationFeedbackCreateInfoEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pRenderPassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pRenderPassFeedback) {
        pRenderPassFeedback = new VkRenderPassCreationFeedbackInfoEXT(*copy_src->pRenderPassFeedback);
    }
}

safe_VkRenderPassSubpassFeedbackCreateInfoEXT::safe_VkRenderPassSubpassFeedbackCreateInfoEXT(
    const VkRenderPassSubpassFeedbackCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pSubpassFeedback(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSubpassFeedback) {
        pSubpassFeedback = new VkRenderPassSubpassFeedbackInfoEXT(*in_struct->pSubpassFeedback);
    }
}

safe_VkRenderPassSubpassFeedbackCreateInfoEXT::safe_VkRenderPassSubpassFeedbackCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT), pNext(nullptr), pSubpassFeedback(nullptr) {}

safe_VkRenderPassSubpassFeedbackCreateInfoEXT::safe_VkRenderPassSubpassFeedbackCreateInfoEXT(
    const safe_VkRenderPassSubpassFeedbackCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    pSubpassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pSubpassFeedback) {
        pSubpassFeedback = new VkRenderPassSubpassFeedbackInfoEXT(*copy_src.pSubpassFeedback);
    }
}

safe_VkRenderPassSubpassFeedbackCreateInfoEXT& safe_VkRenderPassSubpassFeedbackCreateInfoEXT::operator=(
    const safe_VkRenderPassSubpassFeedbackCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pSubpassFeedback) delete pSubpassFeedback;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pSubpassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pSubpassFeedback) {
        pSubpassFeedback = new VkRenderPassSubpassFeedbackInfoEXT(*copy_src.pSubpassFeedback);
    }

    return *this;
}

safe_VkRenderPassSubpassFeedbackCreateInfoEXT::~safe_VkRenderPassSubpassFeedbackCreateInfoEXT() {
    if (pSubpassFeedback) delete pSubpassFeedback;
    FreePnextChain(pNext);
}

void safe_VkRenderPassSubpassFeedbackCreateInfoEXT::initialize(const VkRenderPassSubpassFeedbackCreateInfoEXT* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSubpassFeedback) delete pSubpassFeedback;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pSubpassFeedback = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pSubpassFeedback) {
        pSubpassFeedback = new VkRenderPassSubpassFeedbackInfoEXT(*in_struct->pSubpassFeedback);
    }
}

void safe_VkRenderPassSubpassFeedbackCreateInfoEXT::initialize(const safe_VkRenderPassSubpassFeedbackCreateInfoEXT* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pSubpassFeedback = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pSubpassFeedback) {
        pSubpassFeedback = new VkRenderPassSubpassFeedbackInfoEXT(*copy_src->pSubpassFeedback);
    }
}

safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(
    const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderModuleIdentifier(in_struct->shaderModuleIdentifier) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT), pNext(nullptr), shaderModuleIdentifier() {}

safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(
    const safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderModuleIdentifier = copy_src.shaderModuleIdentifier;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT& safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderModuleIdentifier = copy_src.shaderModuleIdentifier;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::~safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::initialize(
    const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderModuleIdentifier = in_struct->shaderModuleIdentifier;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderModuleIdentifier = copy_src->shaderModuleIdentifier;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(
    const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderModuleIdentifierAlgorithmUUID[i] = in_struct->shaderModuleIdentifierAlgorithmUUID[i];
    }
}

safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT), pNext(nullptr) {}

safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(
    const safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderModuleIdentifierAlgorithmUUID[i] = copy_src.shaderModuleIdentifierAlgorithmUUID[i];
    }
}

safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT& safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderModuleIdentifierAlgorithmUUID[i] = copy_src.shaderModuleIdentifierAlgorithmUUID[i];
    }

    return *this;
}

safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::~safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::initialize(
    const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderModuleIdentifierAlgorithmUUID[i] = in_struct->shaderModuleIdentifierAlgorithmUUID[i];
    }
}

void safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderModuleIdentifierAlgorithmUUID[i] = copy_src->shaderModuleIdentifierAlgorithmUUID[i];
    }
}

safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT(
    const VkPipelineShaderStageModuleIdentifierCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), identifierSize(in_struct->identifierSize), pIdentifier(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pIdentifier) {
        pIdentifier = new uint8_t[in_struct->identifierSize];
        memcpy((void*)pIdentifier, (void*)in_struct->pIdentifier, sizeof(uint8_t) * in_struct->identifierSize);
    }
}

safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT),
      pNext(nullptr),
      identifierSize(),
      pIdentifier(nullptr) {}

safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT(
    const safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    identifierSize = copy_src.identifierSize;
    pIdentifier = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pIdentifier) {
        pIdentifier = new uint8_t[copy_src.identifierSize];
        memcpy((void*)pIdentifier, (void*)copy_src.pIdentifier, sizeof(uint8_t) * copy_src.identifierSize);
    }
}

safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT& safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::operator=(
    const safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pIdentifier) delete[] pIdentifier;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    identifierSize = copy_src.identifierSize;
    pIdentifier = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pIdentifier) {
        pIdentifier = new uint8_t[copy_src.identifierSize];
        memcpy((void*)pIdentifier, (void*)copy_src.pIdentifier, sizeof(uint8_t) * copy_src.identifierSize);
    }

    return *this;
}

safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::~safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT() {
    if (pIdentifier) delete[] pIdentifier;
    FreePnextChain(pNext);
}

void safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::initialize(
    const VkPipelineShaderStageModuleIdentifierCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pIdentifier) delete[] pIdentifier;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    identifierSize = in_struct->identifierSize;
    pIdentifier = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pIdentifier) {
        pIdentifier = new uint8_t[in_struct->identifierSize];
        memcpy((void*)pIdentifier, (void*)in_struct->pIdentifier, sizeof(uint8_t) * in_struct->identifierSize);
    }
}

void safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT::initialize(
    const safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    identifierSize = copy_src->identifierSize;
    pIdentifier = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pIdentifier) {
        pIdentifier = new uint8_t[copy_src->identifierSize];
        memcpy((void*)pIdentifier, (void*)copy_src->pIdentifier, sizeof(uint8_t) * copy_src->identifierSize);
    }
}

safe_VkShaderModuleIdentifierEXT::safe_VkShaderModuleIdentifierEXT(const VkShaderModuleIdentifierEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), identifierSize(in_struct->identifierSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT; ++i) {
        identifier[i] = in_struct->identifier[i];
    }
}

safe_VkShaderModuleIdentifierEXT::safe_VkShaderModuleIdentifierEXT()
    : sType(VK_STRUCTURE_TYPE_SHADER_MODULE_IDENTIFIER_EXT), pNext(nullptr), identifierSize() {}

safe_VkShaderModuleIdentifierEXT::safe_VkShaderModuleIdentifierEXT(const safe_VkShaderModuleIdentifierEXT& copy_src) {
    sType = copy_src.sType;
    identifierSize = copy_src.identifierSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT; ++i) {
        identifier[i] = copy_src.identifier[i];
    }
}

safe_VkShaderModuleIdentifierEXT& safe_VkShaderModuleIdentifierEXT::operator=(const safe_VkShaderModuleIdentifierEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    identifierSize = copy_src.identifierSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT; ++i) {
        identifier[i] = copy_src.identifier[i];
    }

    return *this;
}

safe_VkShaderModuleIdentifierEXT::~safe_VkShaderModuleIdentifierEXT() { FreePnextChain(pNext); }

void safe_VkShaderModuleIdentifierEXT::initialize(const VkShaderModuleIdentifierEXT* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    identifierSize = in_struct->identifierSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT; ++i) {
        identifier[i] = in_struct->identifier[i];
    }
}

void safe_VkShaderModuleIdentifierEXT::initialize(const safe_VkShaderModuleIdentifierEXT* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    identifierSize = copy_src->identifierSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT; ++i) {
        identifier[i] = copy_src->identifier[i];
    }
}

safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT(
    const VkPhysicalDeviceLegacyDitheringFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), legacyDithering(in_struct->legacyDithering) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT), pNext(nullptr), legacyDithering() {}

safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT(
    const safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    legacyDithering = copy_src.legacyDithering;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT& safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    legacyDithering = copy_src.legacyDithering;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::~safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::initialize(const VkPhysicalDeviceLegacyDitheringFeaturesEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    legacyDithering = in_struct->legacyDithering;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT::initialize(const safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    legacyDithering = copy_src->legacyDithering;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderObjectFeaturesEXT::safe_VkPhysicalDeviceShaderObjectFeaturesEXT(
    const VkPhysicalDeviceShaderObjectFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderObject(in_struct->shaderObject) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderObjectFeaturesEXT::safe_VkPhysicalDeviceShaderObjectFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT), pNext(nullptr), shaderObject() {}

safe_VkPhysicalDeviceShaderObjectFeaturesEXT::safe_VkPhysicalDeviceShaderObjectFeaturesEXT(
    const safe_VkPhysicalDeviceShaderObjectFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderObject = copy_src.shaderObject;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderObjectFeaturesEXT& safe_VkPhysicalDeviceShaderObjectFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderObjectFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderObject = copy_src.shaderObject;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderObjectFeaturesEXT::~safe_VkPhysicalDeviceShaderObjectFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderObjectFeaturesEXT::initialize(const VkPhysicalDeviceShaderObjectFeaturesEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderObject = in_struct->shaderObject;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderObjectFeaturesEXT::initialize(const safe_VkPhysicalDeviceShaderObjectFeaturesEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderObject = copy_src->shaderObject;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderObjectPropertiesEXT::safe_VkPhysicalDeviceShaderObjectPropertiesEXT(
    const VkPhysicalDeviceShaderObjectPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderBinaryVersion(in_struct->shaderBinaryVersion) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderBinaryUUID[i] = in_struct->shaderBinaryUUID[i];
    }
}

safe_VkPhysicalDeviceShaderObjectPropertiesEXT::safe_VkPhysicalDeviceShaderObjectPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT), pNext(nullptr), shaderBinaryVersion() {}

safe_VkPhysicalDeviceShaderObjectPropertiesEXT::safe_VkPhysicalDeviceShaderObjectPropertiesEXT(
    const safe_VkPhysicalDeviceShaderObjectPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    shaderBinaryVersion = copy_src.shaderBinaryVersion;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderBinaryUUID[i] = copy_src.shaderBinaryUUID[i];
    }
}

safe_VkPhysicalDeviceShaderObjectPropertiesEXT& safe_VkPhysicalDeviceShaderObjectPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceShaderObjectPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderBinaryVersion = copy_src.shaderBinaryVersion;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderBinaryUUID[i] = copy_src.shaderBinaryUUID[i];
    }

    return *this;
}

safe_VkPhysicalDeviceShaderObjectPropertiesEXT::~safe_VkPhysicalDeviceShaderObjectPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderObjectPropertiesEXT::initialize(const VkPhysicalDeviceShaderObjectPropertiesEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderBinaryVersion = in_struct->shaderBinaryVersion;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderBinaryUUID[i] = in_struct->shaderBinaryUUID[i];
    }
}

void safe_VkPhysicalDeviceShaderObjectPropertiesEXT::initialize(const safe_VkPhysicalDeviceShaderObjectPropertiesEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderBinaryVersion = copy_src->shaderBinaryVersion;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        shaderBinaryUUID[i] = copy_src->shaderBinaryUUID[i];
    }
}

safe_VkShaderCreateInfoEXT::safe_VkShaderCreateInfoEXT(const VkShaderCreateInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stage(in_struct->stage),
      nextStage(in_struct->nextStage),
      codeType(in_struct->codeType),
      codeSize(in_struct->codeSize),
      pCode(in_struct->pCode),
      setLayoutCount(in_struct->setLayoutCount),
      pSetLayouts(nullptr),
      pushConstantRangeCount(in_struct->pushConstantRangeCount),
      pPushConstantRanges(nullptr),
      pSpecializationInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pName = SafeStringCopy(in_struct->pName);
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }

    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)in_struct->pPushConstantRanges,
               sizeof(VkPushConstantRange) * in_struct->pushConstantRangeCount);
    }
    if (in_struct->pSpecializationInfo) pSpecializationInfo = new safe_VkSpecializationInfo(in_struct->pSpecializationInfo);
}

safe_VkShaderCreateInfoEXT::safe_VkShaderCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      stage(),
      nextStage(),
      codeType(),
      codeSize(),
      pCode(nullptr),
      pName(nullptr),
      setLayoutCount(),
      pSetLayouts(nullptr),
      pushConstantRangeCount(),
      pPushConstantRanges(nullptr),
      pSpecializationInfo(nullptr) {}

safe_VkShaderCreateInfoEXT::safe_VkShaderCreateInfoEXT(const safe_VkShaderCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stage = copy_src.stage;
    nextStage = copy_src.nextStage;
    codeType = copy_src.codeType;
    codeSize = copy_src.codeSize;
    pCode = copy_src.pCode;
    setLayoutCount = copy_src.setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = copy_src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pSpecializationInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);
    if (setLayoutCount && copy_src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src.pSetLayouts[i];
        }
    }

    if (copy_src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src.pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src.pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src.pushConstantRangeCount);
    }
    if (copy_src.pSpecializationInfo) pSpecializationInfo = new safe_VkSpecializationInfo(*copy_src.pSpecializationInfo);
}

safe_VkShaderCreateInfoEXT& safe_VkShaderCreateInfoEXT::operator=(const safe_VkShaderCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pName) delete[] pName;
    if (pSetLayouts) delete[] pSetLayouts;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    if (pSpecializationInfo) delete pSpecializationInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stage = copy_src.stage;
    nextStage = copy_src.nextStage;
    codeType = copy_src.codeType;
    codeSize = copy_src.codeSize;
    pCode = copy_src.pCode;
    setLayoutCount = copy_src.setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = copy_src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pSpecializationInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);
    if (setLayoutCount && copy_src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src.pSetLayouts[i];
        }
    }

    if (copy_src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src.pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src.pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src.pushConstantRangeCount);
    }
    if (copy_src.pSpecializationInfo) pSpecializationInfo = new safe_VkSpecializationInfo(*copy_src.pSpecializationInfo);

    return *this;
}

safe_VkShaderCreateInfoEXT::~safe_VkShaderCreateInfoEXT() {
    if (pName) delete[] pName;
    if (pSetLayouts) delete[] pSetLayouts;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    if (pSpecializationInfo) delete pSpecializationInfo;
    FreePnextChain(pNext);
}

void safe_VkShaderCreateInfoEXT::initialize(const VkShaderCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pName) delete[] pName;
    if (pSetLayouts) delete[] pSetLayouts;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    if (pSpecializationInfo) delete pSpecializationInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stage = in_struct->stage;
    nextStage = in_struct->nextStage;
    codeType = in_struct->codeType;
    codeSize = in_struct->codeSize;
    pCode = in_struct->pCode;
    setLayoutCount = in_struct->setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = in_struct->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pSpecializationInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pName = SafeStringCopy(in_struct->pName);
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }

    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)in_struct->pPushConstantRanges,
               sizeof(VkPushConstantRange) * in_struct->pushConstantRangeCount);
    }
    if (in_struct->pSpecializationInfo) pSpecializationInfo = new safe_VkSpecializationInfo(in_struct->pSpecializationInfo);
}

void safe_VkShaderCreateInfoEXT::initialize(const safe_VkShaderCreateInfoEXT* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stage = copy_src->stage;
    nextStage = copy_src->nextStage;
    codeType = copy_src->codeType;
    codeSize = copy_src->codeSize;
    pCode = copy_src->pCode;
    setLayoutCount = copy_src->setLayoutCount;
    pSetLayouts = nullptr;
    pushConstantRangeCount = copy_src->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pSpecializationInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    pName = SafeStringCopy(copy_src->pName);
    if (setLayoutCount && copy_src->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src->pSetLayouts[i];
        }
    }

    if (copy_src->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src->pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src->pushConstantRangeCount);
    }
    if (copy_src->pSpecializationInfo) pSpecializationInfo = new safe_VkSpecializationInfo(*copy_src->pSpecializationInfo);
}

safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(
    const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), legacyVertexAttributes(in_struct->legacyVertexAttributes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_FEATURES_EXT), pNext(nullptr), legacyVertexAttributes() {}

safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT(
    const safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    legacyVertexAttributes = copy_src.legacyVertexAttributes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT& safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    legacyVertexAttributes = copy_src.legacyVertexAttributes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::~safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::initialize(
    const VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    legacyVertexAttributes = in_struct->legacyVertexAttributes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    legacyVertexAttributes = copy_src->legacyVertexAttributes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(
    const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), nativeUnalignedPerformance(in_struct->nativeUnalignedPerformance) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_VERTEX_ATTRIBUTES_PROPERTIES_EXT),
      pNext(nullptr),
      nativeUnalignedPerformance() {}

safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT(
    const safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    nativeUnalignedPerformance = copy_src.nativeUnalignedPerformance;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT& safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    nativeUnalignedPerformance = copy_src.nativeUnalignedPerformance;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::~safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::initialize(
    const VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    nativeUnalignedPerformance = in_struct->nativeUnalignedPerformance;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceLegacyVertexAttributesPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    nativeUnalignedPerformance = copy_src->nativeUnalignedPerformance;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkLayerSettingEXT::safe_VkLayerSettingEXT(const VkLayerSettingEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state)
    : type(in_struct->type), valueCount(in_struct->valueCount), pValues(in_struct->pValues) {
    pLayerName = SafeStringCopy(in_struct->pLayerName);
    pSettingName = SafeStringCopy(in_struct->pSettingName);
}

safe_VkLayerSettingEXT::safe_VkLayerSettingEXT()
    : pLayerName(nullptr), pSettingName(nullptr), type(), valueCount(), pValues(nullptr) {}

safe_VkLayerSettingEXT::safe_VkLayerSettingEXT(const safe_VkLayerSettingEXT& copy_src) {
    type = copy_src.type;
    valueCount = copy_src.valueCount;
    pValues = copy_src.pValues;
    pLayerName = SafeStringCopy(copy_src.pLayerName);
    pSettingName = SafeStringCopy(copy_src.pSettingName);
}

safe_VkLayerSettingEXT& safe_VkLayerSettingEXT::operator=(const safe_VkLayerSettingEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pLayerName) delete[] pLayerName;
    if (pSettingName) delete[] pSettingName;

    type = copy_src.type;
    valueCount = copy_src.valueCount;
    pValues = copy_src.pValues;
    pLayerName = SafeStringCopy(copy_src.pLayerName);
    pSettingName = SafeStringCopy(copy_src.pSettingName);

    return *this;
}

safe_VkLayerSettingEXT::~safe_VkLayerSettingEXT() {
    if (pLayerName) delete[] pLayerName;
    if (pSettingName) delete[] pSettingName;
}

void safe_VkLayerSettingEXT::initialize(const VkLayerSettingEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLayerName) delete[] pLayerName;
    if (pSettingName) delete[] pSettingName;
    type = in_struct->type;
    valueCount = in_struct->valueCount;
    pValues = in_struct->pValues;
    pLayerName = SafeStringCopy(in_struct->pLayerName);
    pSettingName = SafeStringCopy(in_struct->pSettingName);
}

void safe_VkLayerSettingEXT::initialize(const safe_VkLayerSettingEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    type = copy_src->type;
    valueCount = copy_src->valueCount;
    pValues = copy_src->pValues;
    pLayerName = SafeStringCopy(copy_src->pLayerName);
    pSettingName = SafeStringCopy(copy_src->pSettingName);
}

safe_VkLayerSettingsCreateInfoEXT::safe_VkLayerSettingsCreateInfoEXT(const VkLayerSettingsCreateInfoEXT* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), settingCount(in_struct->settingCount), pSettings(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (settingCount && in_struct->pSettings) {
        pSettings = new safe_VkLayerSettingEXT[settingCount];
        for (uint32_t i = 0; i < settingCount; ++i) {
            pSettings[i].initialize(&in_struct->pSettings[i]);
        }
    }
}

safe_VkLayerSettingsCreateInfoEXT::safe_VkLayerSettingsCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT), pNext(nullptr), settingCount(), pSettings(nullptr) {}

safe_VkLayerSettingsCreateInfoEXT::safe_VkLayerSettingsCreateInfoEXT(const safe_VkLayerSettingsCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    settingCount = copy_src.settingCount;
    pSettings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (settingCount && copy_src.pSettings) {
        pSettings = new safe_VkLayerSettingEXT[settingCount];
        for (uint32_t i = 0; i < settingCount; ++i) {
            pSettings[i].initialize(&copy_src.pSettings[i]);
        }
    }
}

safe_VkLayerSettingsCreateInfoEXT& safe_VkLayerSettingsCreateInfoEXT::operator=(const safe_VkLayerSettingsCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pSettings) delete[] pSettings;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    settingCount = copy_src.settingCount;
    pSettings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (settingCount && copy_src.pSettings) {
        pSettings = new safe_VkLayerSettingEXT[settingCount];
        for (uint32_t i = 0; i < settingCount; ++i) {
            pSettings[i].initialize(&copy_src.pSettings[i]);
        }
    }

    return *this;
}

safe_VkLayerSettingsCreateInfoEXT::~safe_VkLayerSettingsCreateInfoEXT() {
    if (pSettings) delete[] pSettings;
    FreePnextChain(pNext);
}

void safe_VkLayerSettingsCreateInfoEXT::initialize(const VkLayerSettingsCreateInfoEXT* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSettings) delete[] pSettings;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    settingCount = in_struct->settingCount;
    pSettings = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (settingCount && in_struct->pSettings) {
        pSettings = new safe_VkLayerSettingEXT[settingCount];
        for (uint32_t i = 0; i < settingCount; ++i) {
            pSettings[i].initialize(&in_struct->pSettings[i]);
        }
    }
}

void safe_VkLayerSettingsCreateInfoEXT::initialize(const safe_VkLayerSettingsCreateInfoEXT* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    settingCount = copy_src->settingCount;
    pSettings = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (settingCount && copy_src->pSettings) {
        pSettings = new safe_VkLayerSettingEXT[settingCount];
        for (uint32_t i = 0; i < settingCount; ++i) {
            pSettings[i].initialize(&copy_src->pSettings[i]);
        }
    }
}

safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(
    const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), pipelineLibraryGroupHandles(in_struct->pipelineLibraryGroupHandles) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT),
      pNext(nullptr),
      pipelineLibraryGroupHandles() {}

safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(
    const safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    pipelineLibraryGroupHandles = copy_src.pipelineLibraryGroupHandles;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT& safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::operator=(
    const safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineLibraryGroupHandles = copy_src.pipelineLibraryGroupHandles;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::~safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::initialize(
    const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineLibraryGroupHandles = in_struct->pipelineLibraryGroupHandles;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::initialize(
    const safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineLibraryGroupHandles = copy_src->pipelineLibraryGroupHandles;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::
    safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(
        const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType), dynamicRenderingUnusedAttachments(in_struct->dynamicRenderingUnusedAttachments) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::
    safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT),
      pNext(nullptr),
      dynamicRenderingUnusedAttachments() {}

safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::
    safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(
        const safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    dynamicRenderingUnusedAttachments = copy_src.dynamicRenderingUnusedAttachments;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT&
safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dynamicRenderingUnusedAttachments = copy_src.dynamicRenderingUnusedAttachments;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::
    ~safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::initialize(
    const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dynamicRenderingUnusedAttachments = in_struct->dynamicRenderingUnusedAttachments;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dynamicRenderingUnusedAttachments = copy_src->dynamicRenderingUnusedAttachments;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::
    safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(
        const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType), attachmentFeedbackLoopDynamicState(in_struct->attachmentFeedbackLoopDynamicState) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::
    safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT),
      pNext(nullptr),
      attachmentFeedbackLoopDynamicState() {}

safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::
    safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(
        const safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    attachmentFeedbackLoopDynamicState = copy_src.attachmentFeedbackLoopDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT&
safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    attachmentFeedbackLoopDynamicState = copy_src.attachmentFeedbackLoopDynamicState;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::
    ~safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::initialize(
    const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    attachmentFeedbackLoopDynamicState = in_struct->attachmentFeedbackLoopDynamicState;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    attachmentFeedbackLoopDynamicState = copy_src->attachmentFeedbackLoopDynamicState;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(
    const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderReplicatedComposites(in_struct->shaderReplicatedComposites) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_REPLICATED_COMPOSITES_FEATURES_EXT),
      pNext(nullptr),
      shaderReplicatedComposites() {}

safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT(
    const safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderReplicatedComposites = copy_src.shaderReplicatedComposites;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT& safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderReplicatedComposites = copy_src.shaderReplicatedComposites;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::~safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::initialize(
    const VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderReplicatedComposites = in_struct->shaderReplicatedComposites;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderReplicatedComposites = copy_src->shaderReplicatedComposites;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::safe_VkPhysicalDeviceShaderFloat8FeaturesEXT(
    const VkPhysicalDeviceShaderFloat8FeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderFloat8(in_struct->shaderFloat8),
      shaderFloat8CooperativeMatrix(in_struct->shaderFloat8CooperativeMatrix) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::safe_VkPhysicalDeviceShaderFloat8FeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT8_FEATURES_EXT),
      pNext(nullptr),
      shaderFloat8(),
      shaderFloat8CooperativeMatrix() {}

safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::safe_VkPhysicalDeviceShaderFloat8FeaturesEXT(
    const safe_VkPhysicalDeviceShaderFloat8FeaturesEXT& copy_src) {
    sType = copy_src.sType;
    shaderFloat8 = copy_src.shaderFloat8;
    shaderFloat8CooperativeMatrix = copy_src.shaderFloat8CooperativeMatrix;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderFloat8FeaturesEXT& safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::operator=(
    const safe_VkPhysicalDeviceShaderFloat8FeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderFloat8 = copy_src.shaderFloat8;
    shaderFloat8CooperativeMatrix = copy_src.shaderFloat8CooperativeMatrix;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::~safe_VkPhysicalDeviceShaderFloat8FeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::initialize(const VkPhysicalDeviceShaderFloat8FeaturesEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderFloat8 = in_struct->shaderFloat8;
    shaderFloat8CooperativeMatrix = in_struct->shaderFloat8CooperativeMatrix;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderFloat8FeaturesEXT::initialize(const safe_VkPhysicalDeviceShaderFloat8FeaturesEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderFloat8 = copy_src->shaderFloat8;
    shaderFloat8CooperativeMatrix = copy_src->shaderFloat8CooperativeMatrix;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(
    const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      deviceGeneratedCommands(in_struct->deviceGeneratedCommands),
      dynamicGeneratedPipelineLayout(in_struct->dynamicGeneratedPipelineLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_EXT),
      pNext(nullptr),
      deviceGeneratedCommands(),
      dynamicGeneratedPipelineLayout() {}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    deviceGeneratedCommands = copy_src.deviceGeneratedCommands;
    dynamicGeneratedPipelineLayout = copy_src.dynamicGeneratedPipelineLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT& safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceGeneratedCommands = copy_src.deviceGeneratedCommands;
    dynamicGeneratedPipelineLayout = copy_src.dynamicGeneratedPipelineLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::~safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::initialize(
    const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceGeneratedCommands = in_struct->deviceGeneratedCommands;
    dynamicGeneratedPipelineLayout = in_struct->dynamicGeneratedPipelineLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceGeneratedCommands = copy_src->deviceGeneratedCommands;
    dynamicGeneratedPipelineLayout = copy_src->dynamicGeneratedPipelineLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(
    const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxIndirectPipelineCount(in_struct->maxIndirectPipelineCount),
      maxIndirectShaderObjectCount(in_struct->maxIndirectShaderObjectCount),
      maxIndirectSequenceCount(in_struct->maxIndirectSequenceCount),
      maxIndirectCommandsTokenCount(in_struct->maxIndirectCommandsTokenCount),
      maxIndirectCommandsTokenOffset(in_struct->maxIndirectCommandsTokenOffset),
      maxIndirectCommandsIndirectStride(in_struct->maxIndirectCommandsIndirectStride),
      supportedIndirectCommandsInputModes(in_struct->supportedIndirectCommandsInputModes),
      supportedIndirectCommandsShaderStages(in_struct->supportedIndirectCommandsShaderStages),
      supportedIndirectCommandsShaderStagesPipelineBinding(in_struct->supportedIndirectCommandsShaderStagesPipelineBinding),
      supportedIndirectCommandsShaderStagesShaderBinding(in_struct->supportedIndirectCommandsShaderStagesShaderBinding),
      deviceGeneratedCommandsTransformFeedback(in_struct->deviceGeneratedCommandsTransformFeedback),
      deviceGeneratedCommandsMultiDrawIndirectCount(in_struct->deviceGeneratedCommandsMultiDrawIndirectCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_EXT),
      pNext(nullptr),
      maxIndirectPipelineCount(),
      maxIndirectShaderObjectCount(),
      maxIndirectSequenceCount(),
      maxIndirectCommandsTokenCount(),
      maxIndirectCommandsTokenOffset(),
      maxIndirectCommandsIndirectStride(),
      supportedIndirectCommandsInputModes(),
      supportedIndirectCommandsShaderStages(),
      supportedIndirectCommandsShaderStagesPipelineBinding(),
      supportedIndirectCommandsShaderStagesShaderBinding(),
      deviceGeneratedCommandsTransformFeedback(),
      deviceGeneratedCommandsMultiDrawIndirectCount() {}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxIndirectPipelineCount = copy_src.maxIndirectPipelineCount;
    maxIndirectShaderObjectCount = copy_src.maxIndirectShaderObjectCount;
    maxIndirectSequenceCount = copy_src.maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src.maxIndirectCommandsTokenCount;
    maxIndirectCommandsTokenOffset = copy_src.maxIndirectCommandsTokenOffset;
    maxIndirectCommandsIndirectStride = copy_src.maxIndirectCommandsIndirectStride;
    supportedIndirectCommandsInputModes = copy_src.supportedIndirectCommandsInputModes;
    supportedIndirectCommandsShaderStages = copy_src.supportedIndirectCommandsShaderStages;
    supportedIndirectCommandsShaderStagesPipelineBinding = copy_src.supportedIndirectCommandsShaderStagesPipelineBinding;
    supportedIndirectCommandsShaderStagesShaderBinding = copy_src.supportedIndirectCommandsShaderStagesShaderBinding;
    deviceGeneratedCommandsTransformFeedback = copy_src.deviceGeneratedCommandsTransformFeedback;
    deviceGeneratedCommandsMultiDrawIndirectCount = copy_src.deviceGeneratedCommandsMultiDrawIndirectCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT& safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxIndirectPipelineCount = copy_src.maxIndirectPipelineCount;
    maxIndirectShaderObjectCount = copy_src.maxIndirectShaderObjectCount;
    maxIndirectSequenceCount = copy_src.maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src.maxIndirectCommandsTokenCount;
    maxIndirectCommandsTokenOffset = copy_src.maxIndirectCommandsTokenOffset;
    maxIndirectCommandsIndirectStride = copy_src.maxIndirectCommandsIndirectStride;
    supportedIndirectCommandsInputModes = copy_src.supportedIndirectCommandsInputModes;
    supportedIndirectCommandsShaderStages = copy_src.supportedIndirectCommandsShaderStages;
    supportedIndirectCommandsShaderStagesPipelineBinding = copy_src.supportedIndirectCommandsShaderStagesPipelineBinding;
    supportedIndirectCommandsShaderStagesShaderBinding = copy_src.supportedIndirectCommandsShaderStagesShaderBinding;
    deviceGeneratedCommandsTransformFeedback = copy_src.deviceGeneratedCommandsTransformFeedback;
    deviceGeneratedCommandsMultiDrawIndirectCount = copy_src.deviceGeneratedCommandsMultiDrawIndirectCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::~safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::initialize(
    const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxIndirectPipelineCount = in_struct->maxIndirectPipelineCount;
    maxIndirectShaderObjectCount = in_struct->maxIndirectShaderObjectCount;
    maxIndirectSequenceCount = in_struct->maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = in_struct->maxIndirectCommandsTokenCount;
    maxIndirectCommandsTokenOffset = in_struct->maxIndirectCommandsTokenOffset;
    maxIndirectCommandsIndirectStride = in_struct->maxIndirectCommandsIndirectStride;
    supportedIndirectCommandsInputModes = in_struct->supportedIndirectCommandsInputModes;
    supportedIndirectCommandsShaderStages = in_struct->supportedIndirectCommandsShaderStages;
    supportedIndirectCommandsShaderStagesPipelineBinding = in_struct->supportedIndirectCommandsShaderStagesPipelineBinding;
    supportedIndirectCommandsShaderStagesShaderBinding = in_struct->supportedIndirectCommandsShaderStagesShaderBinding;
    deviceGeneratedCommandsTransformFeedback = in_struct->deviceGeneratedCommandsTransformFeedback;
    deviceGeneratedCommandsMultiDrawIndirectCount = in_struct->deviceGeneratedCommandsMultiDrawIndirectCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT::initialize(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxIndirectPipelineCount = copy_src->maxIndirectPipelineCount;
    maxIndirectShaderObjectCount = copy_src->maxIndirectShaderObjectCount;
    maxIndirectSequenceCount = copy_src->maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src->maxIndirectCommandsTokenCount;
    maxIndirectCommandsTokenOffset = copy_src->maxIndirectCommandsTokenOffset;
    maxIndirectCommandsIndirectStride = copy_src->maxIndirectCommandsIndirectStride;
    supportedIndirectCommandsInputModes = copy_src->supportedIndirectCommandsInputModes;
    supportedIndirectCommandsShaderStages = copy_src->supportedIndirectCommandsShaderStages;
    supportedIndirectCommandsShaderStagesPipelineBinding = copy_src->supportedIndirectCommandsShaderStagesPipelineBinding;
    supportedIndirectCommandsShaderStagesShaderBinding = copy_src->supportedIndirectCommandsShaderStagesShaderBinding;
    deviceGeneratedCommandsTransformFeedback = copy_src->deviceGeneratedCommandsTransformFeedback;
    deviceGeneratedCommandsMultiDrawIndirectCount = copy_src->deviceGeneratedCommandsMultiDrawIndirectCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::safe_VkGeneratedCommandsMemoryRequirementsInfoEXT(
    const VkGeneratedCommandsMemoryRequirementsInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      indirectExecutionSet(in_struct->indirectExecutionSet),
      indirectCommandsLayout(in_struct->indirectCommandsLayout),
      maxSequenceCount(in_struct->maxSequenceCount),
      maxDrawCount(in_struct->maxDrawCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::safe_VkGeneratedCommandsMemoryRequirementsInfoEXT()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT),
      pNext(nullptr),
      indirectExecutionSet(),
      indirectCommandsLayout(),
      maxSequenceCount(),
      maxDrawCount() {}

safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::safe_VkGeneratedCommandsMemoryRequirementsInfoEXT(
    const safe_VkGeneratedCommandsMemoryRequirementsInfoEXT& copy_src) {
    sType = copy_src.sType;
    indirectExecutionSet = copy_src.indirectExecutionSet;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    maxSequenceCount = copy_src.maxSequenceCount;
    maxDrawCount = copy_src.maxDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeneratedCommandsMemoryRequirementsInfoEXT& safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::operator=(
    const safe_VkGeneratedCommandsMemoryRequirementsInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    indirectExecutionSet = copy_src.indirectExecutionSet;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    maxSequenceCount = copy_src.maxSequenceCount;
    maxDrawCount = copy_src.maxDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::~safe_VkGeneratedCommandsMemoryRequirementsInfoEXT() { FreePnextChain(pNext); }

void safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::initialize(const VkGeneratedCommandsMemoryRequirementsInfoEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    indirectExecutionSet = in_struct->indirectExecutionSet;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    maxSequenceCount = in_struct->maxSequenceCount;
    maxDrawCount = in_struct->maxDrawCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeneratedCommandsMemoryRequirementsInfoEXT::initialize(
    const safe_VkGeneratedCommandsMemoryRequirementsInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    indirectExecutionSet = copy_src->indirectExecutionSet;
    indirectCommandsLayout = copy_src->indirectCommandsLayout;
    maxSequenceCount = copy_src->maxSequenceCount;
    maxDrawCount = copy_src->maxDrawCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkIndirectExecutionSetPipelineInfoEXT::safe_VkIndirectExecutionSetPipelineInfoEXT(
    const VkIndirectExecutionSetPipelineInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), initialPipeline(in_struct->initialPipeline), maxPipelineCount(in_struct->maxPipelineCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkIndirectExecutionSetPipelineInfoEXT::safe_VkIndirectExecutionSetPipelineInfoEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_PIPELINE_INFO_EXT), pNext(nullptr), initialPipeline(), maxPipelineCount() {}

safe_VkIndirectExecutionSetPipelineInfoEXT::safe_VkIndirectExecutionSetPipelineInfoEXT(
    const safe_VkIndirectExecutionSetPipelineInfoEXT& copy_src) {
    sType = copy_src.sType;
    initialPipeline = copy_src.initialPipeline;
    maxPipelineCount = copy_src.maxPipelineCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkIndirectExecutionSetPipelineInfoEXT& safe_VkIndirectExecutionSetPipelineInfoEXT::operator=(
    const safe_VkIndirectExecutionSetPipelineInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    initialPipeline = copy_src.initialPipeline;
    maxPipelineCount = copy_src.maxPipelineCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkIndirectExecutionSetPipelineInfoEXT::~safe_VkIndirectExecutionSetPipelineInfoEXT() { FreePnextChain(pNext); }

void safe_VkIndirectExecutionSetPipelineInfoEXT::initialize(const VkIndirectExecutionSetPipelineInfoEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    initialPipeline = in_struct->initialPipeline;
    maxPipelineCount = in_struct->maxPipelineCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkIndirectExecutionSetPipelineInfoEXT::initialize(const safe_VkIndirectExecutionSetPipelineInfoEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    initialPipeline = copy_src->initialPipeline;
    maxPipelineCount = copy_src->maxPipelineCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkIndirectExecutionSetShaderLayoutInfoEXT::safe_VkIndirectExecutionSetShaderLayoutInfoEXT(
    const VkIndirectExecutionSetShaderLayoutInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), setLayoutCount(in_struct->setLayoutCount), pSetLayouts(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
}

safe_VkIndirectExecutionSetShaderLayoutInfoEXT::safe_VkIndirectExecutionSetShaderLayoutInfoEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_LAYOUT_INFO_EXT),
      pNext(nullptr),
      setLayoutCount(),
      pSetLayouts(nullptr) {}

safe_VkIndirectExecutionSetShaderLayoutInfoEXT::safe_VkIndirectExecutionSetShaderLayoutInfoEXT(
    const safe_VkIndirectExecutionSetShaderLayoutInfoEXT& copy_src) {
    sType = copy_src.sType;
    setLayoutCount = copy_src.setLayoutCount;
    pSetLayouts = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (setLayoutCount && copy_src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src.pSetLayouts[i];
        }
    }
}

safe_VkIndirectExecutionSetShaderLayoutInfoEXT& safe_VkIndirectExecutionSetShaderLayoutInfoEXT::operator=(
    const safe_VkIndirectExecutionSetShaderLayoutInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pSetLayouts) delete[] pSetLayouts;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    setLayoutCount = copy_src.setLayoutCount;
    pSetLayouts = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (setLayoutCount && copy_src.pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src.pSetLayouts[i];
        }
    }

    return *this;
}

safe_VkIndirectExecutionSetShaderLayoutInfoEXT::~safe_VkIndirectExecutionSetShaderLayoutInfoEXT() {
    if (pSetLayouts) delete[] pSetLayouts;
    FreePnextChain(pNext);
}

void safe_VkIndirectExecutionSetShaderLayoutInfoEXT::initialize(const VkIndirectExecutionSetShaderLayoutInfoEXT* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSetLayouts) delete[] pSetLayouts;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    setLayoutCount = in_struct->setLayoutCount;
    pSetLayouts = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (setLayoutCount && in_struct->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = in_struct->pSetLayouts[i];
        }
    }
}

void safe_VkIndirectExecutionSetShaderLayoutInfoEXT::initialize(const safe_VkIndirectExecutionSetShaderLayoutInfoEXT* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    setLayoutCount = copy_src->setLayoutCount;
    pSetLayouts = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (setLayoutCount && copy_src->pSetLayouts) {
        pSetLayouts = new VkDescriptorSetLayout[setLayoutCount];
        for (uint32_t i = 0; i < setLayoutCount; ++i) {
            pSetLayouts[i] = copy_src->pSetLayouts[i];
        }
    }
}

safe_VkIndirectExecutionSetShaderInfoEXT::safe_VkIndirectExecutionSetShaderInfoEXT(
    const VkIndirectExecutionSetShaderInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderCount(in_struct->shaderCount),
      pInitialShaders(nullptr),
      pSetLayoutInfos(nullptr),
      maxShaderCount(in_struct->maxShaderCount),
      pushConstantRangeCount(in_struct->pushConstantRangeCount),
      pPushConstantRanges(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (shaderCount && in_struct->pInitialShaders) {
        pInitialShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pInitialShaders[i] = in_struct->pInitialShaders[i];
        }
    }
    if (shaderCount && in_struct->pSetLayoutInfos) {
        pSetLayoutInfos = new safe_VkIndirectExecutionSetShaderLayoutInfoEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pSetLayoutInfos[i].initialize(&in_struct->pSetLayoutInfos[i]);
        }
    }

    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)in_struct->pPushConstantRanges,
               sizeof(VkPushConstantRange) * in_struct->pushConstantRangeCount);
    }
}

safe_VkIndirectExecutionSetShaderInfoEXT::safe_VkIndirectExecutionSetShaderInfoEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_SHADER_INFO_EXT),
      pNext(nullptr),
      shaderCount(),
      pInitialShaders(nullptr),
      pSetLayoutInfos(nullptr),
      maxShaderCount(),
      pushConstantRangeCount(),
      pPushConstantRanges(nullptr) {}

safe_VkIndirectExecutionSetShaderInfoEXT::safe_VkIndirectExecutionSetShaderInfoEXT(
    const safe_VkIndirectExecutionSetShaderInfoEXT& copy_src) {
    sType = copy_src.sType;
    shaderCount = copy_src.shaderCount;
    pInitialShaders = nullptr;
    pSetLayoutInfos = nullptr;
    maxShaderCount = copy_src.maxShaderCount;
    pushConstantRangeCount = copy_src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (shaderCount && copy_src.pInitialShaders) {
        pInitialShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pInitialShaders[i] = copy_src.pInitialShaders[i];
        }
    }
    if (shaderCount && copy_src.pSetLayoutInfos) {
        pSetLayoutInfos = new safe_VkIndirectExecutionSetShaderLayoutInfoEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pSetLayoutInfos[i].initialize(&copy_src.pSetLayoutInfos[i]);
        }
    }

    if (copy_src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src.pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src.pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src.pushConstantRangeCount);
    }
}

safe_VkIndirectExecutionSetShaderInfoEXT& safe_VkIndirectExecutionSetShaderInfoEXT::operator=(
    const safe_VkIndirectExecutionSetShaderInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pInitialShaders) delete[] pInitialShaders;
    if (pSetLayoutInfos) delete[] pSetLayoutInfos;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCount = copy_src.shaderCount;
    pInitialShaders = nullptr;
    pSetLayoutInfos = nullptr;
    maxShaderCount = copy_src.maxShaderCount;
    pushConstantRangeCount = copy_src.pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (shaderCount && copy_src.pInitialShaders) {
        pInitialShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pInitialShaders[i] = copy_src.pInitialShaders[i];
        }
    }
    if (shaderCount && copy_src.pSetLayoutInfos) {
        pSetLayoutInfos = new safe_VkIndirectExecutionSetShaderLayoutInfoEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pSetLayoutInfos[i].initialize(&copy_src.pSetLayoutInfos[i]);
        }
    }

    if (copy_src.pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src.pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src.pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src.pushConstantRangeCount);
    }

    return *this;
}

safe_VkIndirectExecutionSetShaderInfoEXT::~safe_VkIndirectExecutionSetShaderInfoEXT() {
    if (pInitialShaders) delete[] pInitialShaders;
    if (pSetLayoutInfos) delete[] pSetLayoutInfos;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    FreePnextChain(pNext);
}

void safe_VkIndirectExecutionSetShaderInfoEXT::initialize(const VkIndirectExecutionSetShaderInfoEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pInitialShaders) delete[] pInitialShaders;
    if (pSetLayoutInfos) delete[] pSetLayoutInfos;
    if (pPushConstantRanges) delete[] pPushConstantRanges;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCount = in_struct->shaderCount;
    pInitialShaders = nullptr;
    pSetLayoutInfos = nullptr;
    maxShaderCount = in_struct->maxShaderCount;
    pushConstantRangeCount = in_struct->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (shaderCount && in_struct->pInitialShaders) {
        pInitialShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pInitialShaders[i] = in_struct->pInitialShaders[i];
        }
    }
    if (shaderCount && in_struct->pSetLayoutInfos) {
        pSetLayoutInfos = new safe_VkIndirectExecutionSetShaderLayoutInfoEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pSetLayoutInfos[i].initialize(&in_struct->pSetLayoutInfos[i]);
        }
    }

    if (in_struct->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[in_struct->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)in_struct->pPushConstantRanges,
               sizeof(VkPushConstantRange) * in_struct->pushConstantRangeCount);
    }
}

void safe_VkIndirectExecutionSetShaderInfoEXT::initialize(const safe_VkIndirectExecutionSetShaderInfoEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCount = copy_src->shaderCount;
    pInitialShaders = nullptr;
    pSetLayoutInfos = nullptr;
    maxShaderCount = copy_src->maxShaderCount;
    pushConstantRangeCount = copy_src->pushConstantRangeCount;
    pPushConstantRanges = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (shaderCount && copy_src->pInitialShaders) {
        pInitialShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pInitialShaders[i] = copy_src->pInitialShaders[i];
        }
    }
    if (shaderCount && copy_src->pSetLayoutInfos) {
        pSetLayoutInfos = new safe_VkIndirectExecutionSetShaderLayoutInfoEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pSetLayoutInfos[i].initialize(&copy_src->pSetLayoutInfos[i]);
        }
    }

    if (copy_src->pPushConstantRanges) {
        pPushConstantRanges = new VkPushConstantRange[copy_src->pushConstantRangeCount];
        memcpy((void*)pPushConstantRanges, (void*)copy_src->pPushConstantRanges,
               sizeof(VkPushConstantRange) * copy_src->pushConstantRangeCount);
    }
}

safe_VkIndirectExecutionSetCreateInfoEXT::safe_VkIndirectExecutionSetCreateInfoEXT(
    const VkIndirectExecutionSetCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type), info(in_struct->info) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkIndirectExecutionSetCreateInfoEXT::safe_VkIndirectExecutionSetCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_EXECUTION_SET_CREATE_INFO_EXT), pNext(nullptr), type(), info() {}

safe_VkIndirectExecutionSetCreateInfoEXT::safe_VkIndirectExecutionSetCreateInfoEXT(
    const safe_VkIndirectExecutionSetCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    info = copy_src.info;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkIndirectExecutionSetCreateInfoEXT& safe_VkIndirectExecutionSetCreateInfoEXT::operator=(
    const safe_VkIndirectExecutionSetCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    info = copy_src.info;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkIndirectExecutionSetCreateInfoEXT::~safe_VkIndirectExecutionSetCreateInfoEXT() { FreePnextChain(pNext); }

void safe_VkIndirectExecutionSetCreateInfoEXT::initialize(const VkIndirectExecutionSetCreateInfoEXT* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    info = in_struct->info;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkIndirectExecutionSetCreateInfoEXT::initialize(const safe_VkIndirectExecutionSetCreateInfoEXT* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    info = copy_src->info;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGeneratedCommandsInfoEXT::safe_VkGeneratedCommandsInfoEXT(const VkGeneratedCommandsInfoEXT* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderStages(in_struct->shaderStages),
      indirectExecutionSet(in_struct->indirectExecutionSet),
      indirectCommandsLayout(in_struct->indirectCommandsLayout),
      indirectAddress(in_struct->indirectAddress),
      indirectAddressSize(in_struct->indirectAddressSize),
      preprocessAddress(in_struct->preprocessAddress),
      preprocessSize(in_struct->preprocessSize),
      maxSequenceCount(in_struct->maxSequenceCount),
      sequenceCountAddress(in_struct->sequenceCountAddress),
      maxDrawCount(in_struct->maxDrawCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeneratedCommandsInfoEXT::safe_VkGeneratedCommandsInfoEXT()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT),
      pNext(nullptr),
      shaderStages(),
      indirectExecutionSet(),
      indirectCommandsLayout(),
      indirectAddress(),
      indirectAddressSize(),
      preprocessAddress(),
      preprocessSize(),
      maxSequenceCount(),
      sequenceCountAddress(),
      maxDrawCount() {}

safe_VkGeneratedCommandsInfoEXT::safe_VkGeneratedCommandsInfoEXT(const safe_VkGeneratedCommandsInfoEXT& copy_src) {
    sType = copy_src.sType;
    shaderStages = copy_src.shaderStages;
    indirectExecutionSet = copy_src.indirectExecutionSet;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    indirectAddress = copy_src.indirectAddress;
    indirectAddressSize = copy_src.indirectAddressSize;
    preprocessAddress = copy_src.preprocessAddress;
    preprocessSize = copy_src.preprocessSize;
    maxSequenceCount = copy_src.maxSequenceCount;
    sequenceCountAddress = copy_src.sequenceCountAddress;
    maxDrawCount = copy_src.maxDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeneratedCommandsInfoEXT& safe_VkGeneratedCommandsInfoEXT::operator=(const safe_VkGeneratedCommandsInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderStages = copy_src.shaderStages;
    indirectExecutionSet = copy_src.indirectExecutionSet;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    indirectAddress = copy_src.indirectAddress;
    indirectAddressSize = copy_src.indirectAddressSize;
    preprocessAddress = copy_src.preprocessAddress;
    preprocessSize = copy_src.preprocessSize;
    maxSequenceCount = copy_src.maxSequenceCount;
    sequenceCountAddress = copy_src.sequenceCountAddress;
    maxDrawCount = copy_src.maxDrawCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeneratedCommandsInfoEXT::~safe_VkGeneratedCommandsInfoEXT() { FreePnextChain(pNext); }

void safe_VkGeneratedCommandsInfoEXT::initialize(const VkGeneratedCommandsInfoEXT* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderStages = in_struct->shaderStages;
    indirectExecutionSet = in_struct->indirectExecutionSet;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    indirectAddress = in_struct->indirectAddress;
    indirectAddressSize = in_struct->indirectAddressSize;
    preprocessAddress = in_struct->preprocessAddress;
    preprocessSize = in_struct->preprocessSize;
    maxSequenceCount = in_struct->maxSequenceCount;
    sequenceCountAddress = in_struct->sequenceCountAddress;
    maxDrawCount = in_struct->maxDrawCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeneratedCommandsInfoEXT::initialize(const safe_VkGeneratedCommandsInfoEXT* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderStages = copy_src->shaderStages;
    indirectExecutionSet = copy_src->indirectExecutionSet;
    indirectCommandsLayout = copy_src->indirectCommandsLayout;
    indirectAddress = copy_src->indirectAddress;
    indirectAddressSize = copy_src->indirectAddressSize;
    preprocessAddress = copy_src->preprocessAddress;
    preprocessSize = copy_src->preprocessSize;
    maxSequenceCount = copy_src->maxSequenceCount;
    sequenceCountAddress = copy_src->sequenceCountAddress;
    maxDrawCount = copy_src->maxDrawCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkWriteIndirectExecutionSetPipelineEXT::safe_VkWriteIndirectExecutionSetPipelineEXT(
    const VkWriteIndirectExecutionSetPipelineEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), index(in_struct->index), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkWriteIndirectExecutionSetPipelineEXT::safe_VkWriteIndirectExecutionSetPipelineEXT()
    : sType(VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_PIPELINE_EXT), pNext(nullptr), index(), pipeline() {}

safe_VkWriteIndirectExecutionSetPipelineEXT::safe_VkWriteIndirectExecutionSetPipelineEXT(
    const safe_VkWriteIndirectExecutionSetPipelineEXT& copy_src) {
    sType = copy_src.sType;
    index = copy_src.index;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkWriteIndirectExecutionSetPipelineEXT& safe_VkWriteIndirectExecutionSetPipelineEXT::operator=(
    const safe_VkWriteIndirectExecutionSetPipelineEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    index = copy_src.index;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkWriteIndirectExecutionSetPipelineEXT::~safe_VkWriteIndirectExecutionSetPipelineEXT() { FreePnextChain(pNext); }

void safe_VkWriteIndirectExecutionSetPipelineEXT::initialize(const VkWriteIndirectExecutionSetPipelineEXT* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    index = in_struct->index;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkWriteIndirectExecutionSetPipelineEXT::initialize(const safe_VkWriteIndirectExecutionSetPipelineEXT* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    index = copy_src->index;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkIndirectCommandsLayoutTokenEXT::safe_VkIndirectCommandsLayoutTokenEXT(const VkIndirectCommandsLayoutTokenEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type), data(in_struct->data), offset(in_struct->offset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkIndirectCommandsLayoutTokenEXT::safe_VkIndirectCommandsLayoutTokenEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT), pNext(nullptr), type(), data(), offset() {}

safe_VkIndirectCommandsLayoutTokenEXT::safe_VkIndirectCommandsLayoutTokenEXT(
    const safe_VkIndirectCommandsLayoutTokenEXT& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    data = copy_src.data;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkIndirectCommandsLayoutTokenEXT& safe_VkIndirectCommandsLayoutTokenEXT::operator=(
    const safe_VkIndirectCommandsLayoutTokenEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    data = copy_src.data;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkIndirectCommandsLayoutTokenEXT::~safe_VkIndirectCommandsLayoutTokenEXT() { FreePnextChain(pNext); }

void safe_VkIndirectCommandsLayoutTokenEXT::initialize(const VkIndirectCommandsLayoutTokenEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    data = in_struct->data;
    offset = in_struct->offset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkIndirectCommandsLayoutTokenEXT::initialize(const safe_VkIndirectCommandsLayoutTokenEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    data = copy_src->data;
    offset = copy_src->offset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkIndirectCommandsLayoutCreateInfoEXT::safe_VkIndirectCommandsLayoutCreateInfoEXT(
    const VkIndirectCommandsLayoutCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      shaderStages(in_struct->shaderStages),
      indirectStride(in_struct->indirectStride),
      pipelineLayout(in_struct->pipelineLayout),
      tokenCount(in_struct->tokenCount),
      pTokens(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (tokenCount && in_struct->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenEXT[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&in_struct->pTokens[i]);
        }
    }
}

safe_VkIndirectCommandsLayoutCreateInfoEXT::safe_VkIndirectCommandsLayoutCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT),
      pNext(nullptr),
      flags(),
      shaderStages(),
      indirectStride(),
      pipelineLayout(),
      tokenCount(),
      pTokens(nullptr) {}

safe_VkIndirectCommandsLayoutCreateInfoEXT::safe_VkIndirectCommandsLayoutCreateInfoEXT(
    const safe_VkIndirectCommandsLayoutCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    shaderStages = copy_src.shaderStages;
    indirectStride = copy_src.indirectStride;
    pipelineLayout = copy_src.pipelineLayout;
    tokenCount = copy_src.tokenCount;
    pTokens = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (tokenCount && copy_src.pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenEXT[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src.pTokens[i]);
        }
    }
}

safe_VkIndirectCommandsLayoutCreateInfoEXT& safe_VkIndirectCommandsLayoutCreateInfoEXT::operator=(
    const safe_VkIndirectCommandsLayoutCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pTokens) delete[] pTokens;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    shaderStages = copy_src.shaderStages;
    indirectStride = copy_src.indirectStride;
    pipelineLayout = copy_src.pipelineLayout;
    tokenCount = copy_src.tokenCount;
    pTokens = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (tokenCount && copy_src.pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenEXT[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src.pTokens[i]);
        }
    }

    return *this;
}

safe_VkIndirectCommandsLayoutCreateInfoEXT::~safe_VkIndirectCommandsLayoutCreateInfoEXT() {
    if (pTokens) delete[] pTokens;
    FreePnextChain(pNext);
}

void safe_VkIndirectCommandsLayoutCreateInfoEXT::initialize(const VkIndirectCommandsLayoutCreateInfoEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    if (pTokens) delete[] pTokens;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    shaderStages = in_struct->shaderStages;
    indirectStride = in_struct->indirectStride;
    pipelineLayout = in_struct->pipelineLayout;
    tokenCount = in_struct->tokenCount;
    pTokens = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (tokenCount && in_struct->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenEXT[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&in_struct->pTokens[i]);
        }
    }
}

void safe_VkIndirectCommandsLayoutCreateInfoEXT::initialize(const safe_VkIndirectCommandsLayoutCreateInfoEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    shaderStages = copy_src->shaderStages;
    indirectStride = copy_src->indirectStride;
    pipelineLayout = copy_src->pipelineLayout;
    tokenCount = copy_src->tokenCount;
    pTokens = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (tokenCount && copy_src->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenEXT[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src->pTokens[i]);
        }
    }
}

safe_VkGeneratedCommandsPipelineInfoEXT::safe_VkGeneratedCommandsPipelineInfoEXT(
    const VkGeneratedCommandsPipelineInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeneratedCommandsPipelineInfoEXT::safe_VkGeneratedCommandsPipelineInfoEXT()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT), pNext(nullptr), pipeline() {}

safe_VkGeneratedCommandsPipelineInfoEXT::safe_VkGeneratedCommandsPipelineInfoEXT(
    const safe_VkGeneratedCommandsPipelineInfoEXT& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeneratedCommandsPipelineInfoEXT& safe_VkGeneratedCommandsPipelineInfoEXT::operator=(
    const safe_VkGeneratedCommandsPipelineInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeneratedCommandsPipelineInfoEXT::~safe_VkGeneratedCommandsPipelineInfoEXT() { FreePnextChain(pNext); }

void safe_VkGeneratedCommandsPipelineInfoEXT::initialize(const VkGeneratedCommandsPipelineInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeneratedCommandsPipelineInfoEXT::initialize(const safe_VkGeneratedCommandsPipelineInfoEXT* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGeneratedCommandsShaderInfoEXT::safe_VkGeneratedCommandsShaderInfoEXT(const VkGeneratedCommandsShaderInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), shaderCount(in_struct->shaderCount), pShaders(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (shaderCount && in_struct->pShaders) {
        pShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pShaders[i] = in_struct->pShaders[i];
        }
    }
}

safe_VkGeneratedCommandsShaderInfoEXT::safe_VkGeneratedCommandsShaderInfoEXT()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_SHADER_INFO_EXT), pNext(nullptr), shaderCount(), pShaders(nullptr) {}

safe_VkGeneratedCommandsShaderInfoEXT::safe_VkGeneratedCommandsShaderInfoEXT(
    const safe_VkGeneratedCommandsShaderInfoEXT& copy_src) {
    sType = copy_src.sType;
    shaderCount = copy_src.shaderCount;
    pShaders = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (shaderCount && copy_src.pShaders) {
        pShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pShaders[i] = copy_src.pShaders[i];
        }
    }
}

safe_VkGeneratedCommandsShaderInfoEXT& safe_VkGeneratedCommandsShaderInfoEXT::operator=(
    const safe_VkGeneratedCommandsShaderInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pShaders) delete[] pShaders;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCount = copy_src.shaderCount;
    pShaders = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (shaderCount && copy_src.pShaders) {
        pShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pShaders[i] = copy_src.pShaders[i];
        }
    }

    return *this;
}

safe_VkGeneratedCommandsShaderInfoEXT::~safe_VkGeneratedCommandsShaderInfoEXT() {
    if (pShaders) delete[] pShaders;
    FreePnextChain(pNext);
}

void safe_VkGeneratedCommandsShaderInfoEXT::initialize(const VkGeneratedCommandsShaderInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    if (pShaders) delete[] pShaders;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCount = in_struct->shaderCount;
    pShaders = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (shaderCount && in_struct->pShaders) {
        pShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pShaders[i] = in_struct->pShaders[i];
        }
    }
}

void safe_VkGeneratedCommandsShaderInfoEXT::initialize(const safe_VkGeneratedCommandsShaderInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCount = copy_src->shaderCount;
    pShaders = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (shaderCount && copy_src->pShaders) {
        pShaders = new VkShaderEXT[shaderCount];
        for (uint32_t i = 0; i < shaderCount; ++i) {
            pShaders[i] = copy_src->pShaders[i];
        }
    }
}

safe_VkWriteIndirectExecutionSetShaderEXT::safe_VkWriteIndirectExecutionSetShaderEXT(
    const VkWriteIndirectExecutionSetShaderEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), index(in_struct->index), shader(in_struct->shader) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkWriteIndirectExecutionSetShaderEXT::safe_VkWriteIndirectExecutionSetShaderEXT()
    : sType(VK_STRUCTURE_TYPE_WRITE_INDIRECT_EXECUTION_SET_SHADER_EXT), pNext(nullptr), index(), shader() {}

safe_VkWriteIndirectExecutionSetShaderEXT::safe_VkWriteIndirectExecutionSetShaderEXT(
    const safe_VkWriteIndirectExecutionSetShaderEXT& copy_src) {
    sType = copy_src.sType;
    index = copy_src.index;
    shader = copy_src.shader;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkWriteIndirectExecutionSetShaderEXT& safe_VkWriteIndirectExecutionSetShaderEXT::operator=(
    const safe_VkWriteIndirectExecutionSetShaderEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    index = copy_src.index;
    shader = copy_src.shader;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkWriteIndirectExecutionSetShaderEXT::~safe_VkWriteIndirectExecutionSetShaderEXT() { FreePnextChain(pNext); }

void safe_VkWriteIndirectExecutionSetShaderEXT::initialize(const VkWriteIndirectExecutionSetShaderEXT* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    index = in_struct->index;
    shader = in_struct->shader;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkWriteIndirectExecutionSetShaderEXT::initialize(const safe_VkWriteIndirectExecutionSetShaderEXT* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    index = copy_src->index;
    shader = copy_src->shader;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::safe_VkPhysicalDeviceDepthClampControlFeaturesEXT(
    const VkPhysicalDeviceDepthClampControlFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), depthClampControl(in_struct->depthClampControl) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::safe_VkPhysicalDeviceDepthClampControlFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_CONTROL_FEATURES_EXT), pNext(nullptr), depthClampControl() {}

safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::safe_VkPhysicalDeviceDepthClampControlFeaturesEXT(
    const safe_VkPhysicalDeviceDepthClampControlFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    depthClampControl = copy_src.depthClampControl;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDepthClampControlFeaturesEXT& safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceDepthClampControlFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthClampControl = copy_src.depthClampControl;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::~safe_VkPhysicalDeviceDepthClampControlFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::initialize(const VkPhysicalDeviceDepthClampControlFeaturesEXT* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthClampControl = in_struct->depthClampControl;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDepthClampControlFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceDepthClampControlFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthClampControl = copy_src->depthClampControl;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineViewportDepthClampControlCreateInfoEXT::safe_VkPipelineViewportDepthClampControlCreateInfoEXT(
    const VkPipelineViewportDepthClampControlCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), depthClampMode(in_struct->depthClampMode), pDepthClampRange(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDepthClampRange) {
        pDepthClampRange = new VkDepthClampRangeEXT(*in_struct->pDepthClampRange);
    }
}

safe_VkPipelineViewportDepthClampControlCreateInfoEXT::safe_VkPipelineViewportDepthClampControlCreateInfoEXT()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLAMP_CONTROL_CREATE_INFO_EXT),
      pNext(nullptr),
      depthClampMode(),
      pDepthClampRange(nullptr) {}

safe_VkPipelineViewportDepthClampControlCreateInfoEXT::safe_VkPipelineViewportDepthClampControlCreateInfoEXT(
    const safe_VkPipelineViewportDepthClampControlCreateInfoEXT& copy_src) {
    sType = copy_src.sType;
    depthClampMode = copy_src.depthClampMode;
    pDepthClampRange = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDepthClampRange) {
        pDepthClampRange = new VkDepthClampRangeEXT(*copy_src.pDepthClampRange);
    }
}

safe_VkPipelineViewportDepthClampControlCreateInfoEXT& safe_VkPipelineViewportDepthClampControlCreateInfoEXT::operator=(
    const safe_VkPipelineViewportDepthClampControlCreateInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pDepthClampRange) delete pDepthClampRange;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthClampMode = copy_src.depthClampMode;
    pDepthClampRange = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDepthClampRange) {
        pDepthClampRange = new VkDepthClampRangeEXT(*copy_src.pDepthClampRange);
    }

    return *this;
}

safe_VkPipelineViewportDepthClampControlCreateInfoEXT::~safe_VkPipelineViewportDepthClampControlCreateInfoEXT() {
    if (pDepthClampRange) delete pDepthClampRange;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportDepthClampControlCreateInfoEXT::initialize(
    const VkPipelineViewportDepthClampControlCreateInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDepthClampRange) delete pDepthClampRange;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthClampMode = in_struct->depthClampMode;
    pDepthClampRange = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDepthClampRange) {
        pDepthClampRange = new VkDepthClampRangeEXT(*in_struct->pDepthClampRange);
    }
}

void safe_VkPipelineViewportDepthClampControlCreateInfoEXT::initialize(
    const safe_VkPipelineViewportDepthClampControlCreateInfoEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthClampMode = copy_src->depthClampMode;
    pDepthClampRange = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDepthClampRange) {
        pDepthClampRange = new VkDepthClampRangeEXT(*copy_src->pDepthClampRange);
    }
}
#ifdef VK_USE_PLATFORM_METAL_EXT

safe_VkImportMemoryMetalHandleInfoEXT::safe_VkImportMemoryMetalHandleInfoEXT(const VkImportMemoryMetalHandleInfoEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), handle(in_struct->handle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryMetalHandleInfoEXT::safe_VkImportMemoryMetalHandleInfoEXT()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_METAL_HANDLE_INFO_EXT), pNext(nullptr), handleType(), handle(nullptr) {}

safe_VkImportMemoryMetalHandleInfoEXT::safe_VkImportMemoryMetalHandleInfoEXT(
    const safe_VkImportMemoryMetalHandleInfoEXT& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryMetalHandleInfoEXT& safe_VkImportMemoryMetalHandleInfoEXT::operator=(
    const safe_VkImportMemoryMetalHandleInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryMetalHandleInfoEXT::~safe_VkImportMemoryMetalHandleInfoEXT() { FreePnextChain(pNext); }

void safe_VkImportMemoryMetalHandleInfoEXT::initialize(const VkImportMemoryMetalHandleInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryMetalHandleInfoEXT::initialize(const safe_VkImportMemoryMetalHandleInfoEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryMetalHandlePropertiesEXT::safe_VkMemoryMetalHandlePropertiesEXT(const VkMemoryMetalHandlePropertiesEXT* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryMetalHandlePropertiesEXT::safe_VkMemoryMetalHandlePropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_MEMORY_METAL_HANDLE_PROPERTIES_EXT), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryMetalHandlePropertiesEXT::safe_VkMemoryMetalHandlePropertiesEXT(
    const safe_VkMemoryMetalHandlePropertiesEXT& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryMetalHandlePropertiesEXT& safe_VkMemoryMetalHandlePropertiesEXT::operator=(
    const safe_VkMemoryMetalHandlePropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryMetalHandlePropertiesEXT::~safe_VkMemoryMetalHandlePropertiesEXT() { FreePnextChain(pNext); }

void safe_VkMemoryMetalHandlePropertiesEXT::initialize(const VkMemoryMetalHandlePropertiesEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryMetalHandlePropertiesEXT::initialize(const safe_VkMemoryMetalHandlePropertiesEXT* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetMetalHandleInfoEXT::safe_VkMemoryGetMetalHandleInfoEXT(const VkMemoryGetMetalHandleInfoEXT* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetMetalHandleInfoEXT::safe_VkMemoryGetMetalHandleInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_METAL_HANDLE_INFO_EXT), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetMetalHandleInfoEXT::safe_VkMemoryGetMetalHandleInfoEXT(const safe_VkMemoryGetMetalHandleInfoEXT& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetMetalHandleInfoEXT& safe_VkMemoryGetMetalHandleInfoEXT::operator=(
    const safe_VkMemoryGetMetalHandleInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetMetalHandleInfoEXT::~safe_VkMemoryGetMetalHandleInfoEXT() { FreePnextChain(pNext); }

void safe_VkMemoryGetMetalHandleInfoEXT::initialize(const VkMemoryGetMetalHandleInfoEXT* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetMetalHandleInfoEXT::initialize(const safe_VkMemoryGetMetalHandleInfoEXT* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(
    const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), vertexAttributeRobustness(in_struct->vertexAttributeRobustness) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_ROBUSTNESS_FEATURES_EXT),
      pNext(nullptr),
      vertexAttributeRobustness() {}

safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT(
    const safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    vertexAttributeRobustness = copy_src.vertexAttributeRobustness;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT& safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexAttributeRobustness = copy_src.vertexAttributeRobustness;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::~safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::initialize(
    const VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexAttributeRobustness = in_struct->vertexAttributeRobustness;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexAttributeRobustness = copy_src->vertexAttributeRobustness;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderingEndInfoEXT::safe_VkRenderingEndInfoEXT(const VkRenderingEndInfoEXT* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderingEndInfoEXT::safe_VkRenderingEndInfoEXT() : sType(VK_STRUCTURE_TYPE_RENDERING_END_INFO_EXT), pNext(nullptr) {}

safe_VkRenderingEndInfoEXT::safe_VkRenderingEndInfoEXT(const safe_VkRenderingEndInfoEXT& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderingEndInfoEXT& safe_VkRenderingEndInfoEXT::operator=(const safe_VkRenderingEndInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderingEndInfoEXT::~safe_VkRenderingEndInfoEXT() { FreePnextChain(pNext); }

void safe_VkRenderingEndInfoEXT::initialize(const VkRenderingEndInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderingEndInfoEXT::initialize(const safe_VkRenderingEndInfoEXT* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT(
    const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), zeroInitializeDeviceMemory(in_struct->zeroInitializeDeviceMemory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_DEVICE_MEMORY_FEATURES_EXT),
      pNext(nullptr),
      zeroInitializeDeviceMemory() {}

safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT(
    const safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    zeroInitializeDeviceMemory = copy_src.zeroInitializeDeviceMemory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT& safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    zeroInitializeDeviceMemory = copy_src.zeroInitializeDeviceMemory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::~safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::initialize(
    const VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    zeroInitializeDeviceMemory = in_struct->zeroInitializeDeviceMemory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT::initialize(
    const safe_VkPhysicalDeviceZeroInitializeDeviceMemoryFeaturesEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    zeroInitializeDeviceMemory = copy_src->zeroInitializeDeviceMemory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMeshShaderFeaturesEXT::safe_VkPhysicalDeviceMeshShaderFeaturesEXT(
    const VkPhysicalDeviceMeshShaderFeaturesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      taskShader(in_struct->taskShader),
      meshShader(in_struct->meshShader),
      multiviewMeshShader(in_struct->multiviewMeshShader),
      primitiveFragmentShadingRateMeshShader(in_struct->primitiveFragmentShadingRateMeshShader),
      meshShaderQueries(in_struct->meshShaderQueries) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMeshShaderFeaturesEXT::safe_VkPhysicalDeviceMeshShaderFeaturesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT),
      pNext(nullptr),
      taskShader(),
      meshShader(),
      multiviewMeshShader(),
      primitiveFragmentShadingRateMeshShader(),
      meshShaderQueries() {}

safe_VkPhysicalDeviceMeshShaderFeaturesEXT::safe_VkPhysicalDeviceMeshShaderFeaturesEXT(
    const safe_VkPhysicalDeviceMeshShaderFeaturesEXT& copy_src) {
    sType = copy_src.sType;
    taskShader = copy_src.taskShader;
    meshShader = copy_src.meshShader;
    multiviewMeshShader = copy_src.multiviewMeshShader;
    primitiveFragmentShadingRateMeshShader = copy_src.primitiveFragmentShadingRateMeshShader;
    meshShaderQueries = copy_src.meshShaderQueries;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMeshShaderFeaturesEXT& safe_VkPhysicalDeviceMeshShaderFeaturesEXT::operator=(
    const safe_VkPhysicalDeviceMeshShaderFeaturesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    taskShader = copy_src.taskShader;
    meshShader = copy_src.meshShader;
    multiviewMeshShader = copy_src.multiviewMeshShader;
    primitiveFragmentShadingRateMeshShader = copy_src.primitiveFragmentShadingRateMeshShader;
    meshShaderQueries = copy_src.meshShaderQueries;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMeshShaderFeaturesEXT::~safe_VkPhysicalDeviceMeshShaderFeaturesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMeshShaderFeaturesEXT::initialize(const VkPhysicalDeviceMeshShaderFeaturesEXT* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    taskShader = in_struct->taskShader;
    meshShader = in_struct->meshShader;
    multiviewMeshShader = in_struct->multiviewMeshShader;
    primitiveFragmentShadingRateMeshShader = in_struct->primitiveFragmentShadingRateMeshShader;
    meshShaderQueries = in_struct->meshShaderQueries;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMeshShaderFeaturesEXT::initialize(const safe_VkPhysicalDeviceMeshShaderFeaturesEXT* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    taskShader = copy_src->taskShader;
    meshShader = copy_src->meshShader;
    multiviewMeshShader = copy_src->multiviewMeshShader;
    primitiveFragmentShadingRateMeshShader = copy_src->primitiveFragmentShadingRateMeshShader;
    meshShaderQueries = copy_src->meshShaderQueries;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMeshShaderPropertiesEXT::safe_VkPhysicalDeviceMeshShaderPropertiesEXT(
    const VkPhysicalDeviceMeshShaderPropertiesEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxTaskWorkGroupTotalCount(in_struct->maxTaskWorkGroupTotalCount),
      maxTaskWorkGroupInvocations(in_struct->maxTaskWorkGroupInvocations),
      maxTaskPayloadSize(in_struct->maxTaskPayloadSize),
      maxTaskSharedMemorySize(in_struct->maxTaskSharedMemorySize),
      maxTaskPayloadAndSharedMemorySize(in_struct->maxTaskPayloadAndSharedMemorySize),
      maxMeshWorkGroupTotalCount(in_struct->maxMeshWorkGroupTotalCount),
      maxMeshWorkGroupInvocations(in_struct->maxMeshWorkGroupInvocations),
      maxMeshSharedMemorySize(in_struct->maxMeshSharedMemorySize),
      maxMeshPayloadAndSharedMemorySize(in_struct->maxMeshPayloadAndSharedMemorySize),
      maxMeshOutputMemorySize(in_struct->maxMeshOutputMemorySize),
      maxMeshPayloadAndOutputMemorySize(in_struct->maxMeshPayloadAndOutputMemorySize),
      maxMeshOutputComponents(in_struct->maxMeshOutputComponents),
      maxMeshOutputVertices(in_struct->maxMeshOutputVertices),
      maxMeshOutputPrimitives(in_struct->maxMeshOutputPrimitives),
      maxMeshOutputLayers(in_struct->maxMeshOutputLayers),
      maxMeshMultiviewViewCount(in_struct->maxMeshMultiviewViewCount),
      meshOutputPerVertexGranularity(in_struct->meshOutputPerVertexGranularity),
      meshOutputPerPrimitiveGranularity(in_struct->meshOutputPerPrimitiveGranularity),
      maxPreferredTaskWorkGroupInvocations(in_struct->maxPreferredTaskWorkGroupInvocations),
      maxPreferredMeshWorkGroupInvocations(in_struct->maxPreferredMeshWorkGroupInvocations),
      prefersLocalInvocationVertexOutput(in_struct->prefersLocalInvocationVertexOutput),
      prefersLocalInvocationPrimitiveOutput(in_struct->prefersLocalInvocationPrimitiveOutput),
      prefersCompactVertexOutput(in_struct->prefersCompactVertexOutput),
      prefersCompactPrimitiveOutput(in_struct->prefersCompactPrimitiveOutput) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupCount[i] = in_struct->maxTaskWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupCount[i] = in_struct->maxMeshWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesEXT::safe_VkPhysicalDeviceMeshShaderPropertiesEXT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT),
      pNext(nullptr),
      maxTaskWorkGroupTotalCount(),
      maxTaskWorkGroupInvocations(),
      maxTaskPayloadSize(),
      maxTaskSharedMemorySize(),
      maxTaskPayloadAndSharedMemorySize(),
      maxMeshWorkGroupTotalCount(),
      maxMeshWorkGroupInvocations(),
      maxMeshSharedMemorySize(),
      maxMeshPayloadAndSharedMemorySize(),
      maxMeshOutputMemorySize(),
      maxMeshPayloadAndOutputMemorySize(),
      maxMeshOutputComponents(),
      maxMeshOutputVertices(),
      maxMeshOutputPrimitives(),
      maxMeshOutputLayers(),
      maxMeshMultiviewViewCount(),
      meshOutputPerVertexGranularity(),
      meshOutputPerPrimitiveGranularity(),
      maxPreferredTaskWorkGroupInvocations(),
      maxPreferredMeshWorkGroupInvocations(),
      prefersLocalInvocationVertexOutput(),
      prefersLocalInvocationPrimitiveOutput(),
      prefersCompactVertexOutput(),
      prefersCompactPrimitiveOutput() {}

safe_VkPhysicalDeviceMeshShaderPropertiesEXT::safe_VkPhysicalDeviceMeshShaderPropertiesEXT(
    const safe_VkPhysicalDeviceMeshShaderPropertiesEXT& copy_src) {
    sType = copy_src.sType;
    maxTaskWorkGroupTotalCount = copy_src.maxTaskWorkGroupTotalCount;
    maxTaskWorkGroupInvocations = copy_src.maxTaskWorkGroupInvocations;
    maxTaskPayloadSize = copy_src.maxTaskPayloadSize;
    maxTaskSharedMemorySize = copy_src.maxTaskSharedMemorySize;
    maxTaskPayloadAndSharedMemorySize = copy_src.maxTaskPayloadAndSharedMemorySize;
    maxMeshWorkGroupTotalCount = copy_src.maxMeshWorkGroupTotalCount;
    maxMeshWorkGroupInvocations = copy_src.maxMeshWorkGroupInvocations;
    maxMeshSharedMemorySize = copy_src.maxMeshSharedMemorySize;
    maxMeshPayloadAndSharedMemorySize = copy_src.maxMeshPayloadAndSharedMemorySize;
    maxMeshOutputMemorySize = copy_src.maxMeshOutputMemorySize;
    maxMeshPayloadAndOutputMemorySize = copy_src.maxMeshPayloadAndOutputMemorySize;
    maxMeshOutputComponents = copy_src.maxMeshOutputComponents;
    maxMeshOutputVertices = copy_src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src.maxMeshOutputPrimitives;
    maxMeshOutputLayers = copy_src.maxMeshOutputLayers;
    maxMeshMultiviewViewCount = copy_src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src.meshOutputPerPrimitiveGranularity;
    maxPreferredTaskWorkGroupInvocations = copy_src.maxPreferredTaskWorkGroupInvocations;
    maxPreferredMeshWorkGroupInvocations = copy_src.maxPreferredMeshWorkGroupInvocations;
    prefersLocalInvocationVertexOutput = copy_src.prefersLocalInvocationVertexOutput;
    prefersLocalInvocationPrimitiveOutput = copy_src.prefersLocalInvocationPrimitiveOutput;
    prefersCompactVertexOutput = copy_src.prefersCompactVertexOutput;
    prefersCompactPrimitiveOutput = copy_src.prefersCompactPrimitiveOutput;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupCount[i] = copy_src.maxTaskWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src.maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupCount[i] = copy_src.maxMeshWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src.maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesEXT& safe_VkPhysicalDeviceMeshShaderPropertiesEXT::operator=(
    const safe_VkPhysicalDeviceMeshShaderPropertiesEXT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxTaskWorkGroupTotalCount = copy_src.maxTaskWorkGroupTotalCount;
    maxTaskWorkGroupInvocations = copy_src.maxTaskWorkGroupInvocations;
    maxTaskPayloadSize = copy_src.maxTaskPayloadSize;
    maxTaskSharedMemorySize = copy_src.maxTaskSharedMemorySize;
    maxTaskPayloadAndSharedMemorySize = copy_src.maxTaskPayloadAndSharedMemorySize;
    maxMeshWorkGroupTotalCount = copy_src.maxMeshWorkGroupTotalCount;
    maxMeshWorkGroupInvocations = copy_src.maxMeshWorkGroupInvocations;
    maxMeshSharedMemorySize = copy_src.maxMeshSharedMemorySize;
    maxMeshPayloadAndSharedMemorySize = copy_src.maxMeshPayloadAndSharedMemorySize;
    maxMeshOutputMemorySize = copy_src.maxMeshOutputMemorySize;
    maxMeshPayloadAndOutputMemorySize = copy_src.maxMeshPayloadAndOutputMemorySize;
    maxMeshOutputComponents = copy_src.maxMeshOutputComponents;
    maxMeshOutputVertices = copy_src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src.maxMeshOutputPrimitives;
    maxMeshOutputLayers = copy_src.maxMeshOutputLayers;
    maxMeshMultiviewViewCount = copy_src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src.meshOutputPerPrimitiveGranularity;
    maxPreferredTaskWorkGroupInvocations = copy_src.maxPreferredTaskWorkGroupInvocations;
    maxPreferredMeshWorkGroupInvocations = copy_src.maxPreferredMeshWorkGroupInvocations;
    prefersLocalInvocationVertexOutput = copy_src.prefersLocalInvocationVertexOutput;
    prefersLocalInvocationPrimitiveOutput = copy_src.prefersLocalInvocationPrimitiveOutput;
    prefersCompactVertexOutput = copy_src.prefersCompactVertexOutput;
    prefersCompactPrimitiveOutput = copy_src.prefersCompactPrimitiveOutput;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupCount[i] = copy_src.maxTaskWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src.maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupCount[i] = copy_src.maxMeshWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src.maxMeshWorkGroupSize[i];
    }

    return *this;
}

safe_VkPhysicalDeviceMeshShaderPropertiesEXT::~safe_VkPhysicalDeviceMeshShaderPropertiesEXT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMeshShaderPropertiesEXT::initialize(const VkPhysicalDeviceMeshShaderPropertiesEXT* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxTaskWorkGroupTotalCount = in_struct->maxTaskWorkGroupTotalCount;
    maxTaskWorkGroupInvocations = in_struct->maxTaskWorkGroupInvocations;
    maxTaskPayloadSize = in_struct->maxTaskPayloadSize;
    maxTaskSharedMemorySize = in_struct->maxTaskSharedMemorySize;
    maxTaskPayloadAndSharedMemorySize = in_struct->maxTaskPayloadAndSharedMemorySize;
    maxMeshWorkGroupTotalCount = in_struct->maxMeshWorkGroupTotalCount;
    maxMeshWorkGroupInvocations = in_struct->maxMeshWorkGroupInvocations;
    maxMeshSharedMemorySize = in_struct->maxMeshSharedMemorySize;
    maxMeshPayloadAndSharedMemorySize = in_struct->maxMeshPayloadAndSharedMemorySize;
    maxMeshOutputMemorySize = in_struct->maxMeshOutputMemorySize;
    maxMeshPayloadAndOutputMemorySize = in_struct->maxMeshPayloadAndOutputMemorySize;
    maxMeshOutputComponents = in_struct->maxMeshOutputComponents;
    maxMeshOutputVertices = in_struct->maxMeshOutputVertices;
    maxMeshOutputPrimitives = in_struct->maxMeshOutputPrimitives;
    maxMeshOutputLayers = in_struct->maxMeshOutputLayers;
    maxMeshMultiviewViewCount = in_struct->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = in_struct->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = in_struct->meshOutputPerPrimitiveGranularity;
    maxPreferredTaskWorkGroupInvocations = in_struct->maxPreferredTaskWorkGroupInvocations;
    maxPreferredMeshWorkGroupInvocations = in_struct->maxPreferredMeshWorkGroupInvocations;
    prefersLocalInvocationVertexOutput = in_struct->prefersLocalInvocationVertexOutput;
    prefersLocalInvocationPrimitiveOutput = in_struct->prefersLocalInvocationPrimitiveOutput;
    prefersCompactVertexOutput = in_struct->prefersCompactVertexOutput;
    prefersCompactPrimitiveOutput = in_struct->prefersCompactPrimitiveOutput;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupCount[i] = in_struct->maxTaskWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupCount[i] = in_struct->maxMeshWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

void safe_VkPhysicalDeviceMeshShaderPropertiesEXT::initialize(const safe_VkPhysicalDeviceMeshShaderPropertiesEXT* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxTaskWorkGroupTotalCount = copy_src->maxTaskWorkGroupTotalCount;
    maxTaskWorkGroupInvocations = copy_src->maxTaskWorkGroupInvocations;
    maxTaskPayloadSize = copy_src->maxTaskPayloadSize;
    maxTaskSharedMemorySize = copy_src->maxTaskSharedMemorySize;
    maxTaskPayloadAndSharedMemorySize = copy_src->maxTaskPayloadAndSharedMemorySize;
    maxMeshWorkGroupTotalCount = copy_src->maxMeshWorkGroupTotalCount;
    maxMeshWorkGroupInvocations = copy_src->maxMeshWorkGroupInvocations;
    maxMeshSharedMemorySize = copy_src->maxMeshSharedMemorySize;
    maxMeshPayloadAndSharedMemorySize = copy_src->maxMeshPayloadAndSharedMemorySize;
    maxMeshOutputMemorySize = copy_src->maxMeshOutputMemorySize;
    maxMeshPayloadAndOutputMemorySize = copy_src->maxMeshPayloadAndOutputMemorySize;
    maxMeshOutputComponents = copy_src->maxMeshOutputComponents;
    maxMeshOutputVertices = copy_src->maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src->maxMeshOutputPrimitives;
    maxMeshOutputLayers = copy_src->maxMeshOutputLayers;
    maxMeshMultiviewViewCount = copy_src->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src->meshOutputPerPrimitiveGranularity;
    maxPreferredTaskWorkGroupInvocations = copy_src->maxPreferredTaskWorkGroupInvocations;
    maxPreferredMeshWorkGroupInvocations = copy_src->maxPreferredMeshWorkGroupInvocations;
    prefersLocalInvocationVertexOutput = copy_src->prefersLocalInvocationVertexOutput;
    prefersLocalInvocationPrimitiveOutput = copy_src->prefersLocalInvocationPrimitiveOutput;
    prefersCompactVertexOutput = copy_src->prefersCompactVertexOutput;
    prefersCompactPrimitiveOutput = copy_src->prefersCompactPrimitiveOutput;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupCount[i] = copy_src->maxTaskWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupCount[i] = copy_src->maxMeshWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src->maxMeshWorkGroupSize[i];
    }
}

}  // namespace vku

// NOLINTEND
