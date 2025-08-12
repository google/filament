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

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const VkSwapchainCreateInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      surface(in_struct->surface),
      minImageCount(in_struct->minImageCount),
      imageFormat(in_struct->imageFormat),
      imageColorSpace(in_struct->imageColorSpace),
      imageExtent(in_struct->imageExtent),
      imageArrayLayers(in_struct->imageArrayLayers),
      imageUsage(in_struct->imageUsage),
      imageSharingMode(in_struct->imageSharingMode),
      queueFamilyIndexCount(0),
      pQueueFamilyIndices(nullptr),
      preTransform(in_struct->preTransform),
      compositeAlpha(in_struct->compositeAlpha),
      presentMode(in_struct->presentMode),
      clipped(in_struct->clipped),
      oldSwapchain(in_struct->oldSwapchain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if ((in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      surface(),
      minImageCount(),
      imageFormat(),
      imageColorSpace(),
      imageExtent(),
      imageArrayLayers(),
      imageUsage(),
      imageSharingMode(),
      queueFamilyIndexCount(),
      pQueueFamilyIndices(nullptr),
      preTransform(),
      compositeAlpha(),
      presentMode(),
      clipped(),
      oldSwapchain() {}

safe_VkSwapchainCreateInfoKHR::safe_VkSwapchainCreateInfoKHR(const safe_VkSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    surface = copy_src.surface;
    minImageCount = copy_src.minImageCount;
    imageFormat = copy_src.imageFormat;
    imageColorSpace = copy_src.imageColorSpace;
    imageExtent = copy_src.imageExtent;
    imageArrayLayers = copy_src.imageArrayLayers;
    imageUsage = copy_src.imageUsage;
    imageSharingMode = copy_src.imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src.preTransform;
    compositeAlpha = copy_src.compositeAlpha;
    presentMode = copy_src.presentMode;
    clipped = copy_src.clipped;
    oldSwapchain = copy_src.oldSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkSwapchainCreateInfoKHR& safe_VkSwapchainCreateInfoKHR::operator=(const safe_VkSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    surface = copy_src.surface;
    minImageCount = copy_src.minImageCount;
    imageFormat = copy_src.imageFormat;
    imageColorSpace = copy_src.imageColorSpace;
    imageExtent = copy_src.imageExtent;
    imageArrayLayers = copy_src.imageArrayLayers;
    imageUsage = copy_src.imageUsage;
    imageSharingMode = copy_src.imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src.preTransform;
    compositeAlpha = copy_src.compositeAlpha;
    presentMode = copy_src.presentMode;
    clipped = copy_src.clipped;
    oldSwapchain = copy_src.oldSwapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    if ((copy_src.imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src.pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src.queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src.pQueueFamilyIndices, sizeof(uint32_t) * copy_src.queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src.queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }

    return *this;
}

safe_VkSwapchainCreateInfoKHR::~safe_VkSwapchainCreateInfoKHR() {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
}

void safe_VkSwapchainCreateInfoKHR::initialize(const VkSwapchainCreateInfoKHR* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pQueueFamilyIndices) delete[] pQueueFamilyIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    surface = in_struct->surface;
    minImageCount = in_struct->minImageCount;
    imageFormat = in_struct->imageFormat;
    imageColorSpace = in_struct->imageColorSpace;
    imageExtent = in_struct->imageExtent;
    imageArrayLayers = in_struct->imageArrayLayers;
    imageUsage = in_struct->imageUsage;
    imageSharingMode = in_struct->imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = in_struct->preTransform;
    compositeAlpha = in_struct->compositeAlpha;
    presentMode = in_struct->presentMode;
    clipped = in_struct->clipped;
    oldSwapchain = in_struct->oldSwapchain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if ((in_struct->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && in_struct->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[in_struct->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)in_struct->pQueueFamilyIndices,
               sizeof(uint32_t) * in_struct->queueFamilyIndexCount);
        queueFamilyIndexCount = in_struct->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

void safe_VkSwapchainCreateInfoKHR::initialize(const safe_VkSwapchainCreateInfoKHR* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    surface = copy_src->surface;
    minImageCount = copy_src->minImageCount;
    imageFormat = copy_src->imageFormat;
    imageColorSpace = copy_src->imageColorSpace;
    imageExtent = copy_src->imageExtent;
    imageArrayLayers = copy_src->imageArrayLayers;
    imageUsage = copy_src->imageUsage;
    imageSharingMode = copy_src->imageSharingMode;
    pQueueFamilyIndices = nullptr;
    preTransform = copy_src->preTransform;
    compositeAlpha = copy_src->compositeAlpha;
    presentMode = copy_src->presentMode;
    clipped = copy_src->clipped;
    oldSwapchain = copy_src->oldSwapchain;
    pNext = SafePnextCopy(copy_src->pNext);

    if ((copy_src->imageSharingMode == VK_SHARING_MODE_CONCURRENT) && copy_src->pQueueFamilyIndices) {
        pQueueFamilyIndices = new uint32_t[copy_src->queueFamilyIndexCount];
        memcpy((void*)pQueueFamilyIndices, (void*)copy_src->pQueueFamilyIndices,
               sizeof(uint32_t) * copy_src->queueFamilyIndexCount);
        queueFamilyIndexCount = copy_src->queueFamilyIndexCount;
    } else {
        queueFamilyIndexCount = 0;
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const VkPresentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                             bool copy_pnext)
    : sType(in_struct->sType),
      waitSemaphoreCount(in_struct->waitSemaphoreCount),
      pWaitSemaphores(nullptr),
      swapchainCount(in_struct->swapchainCount),
      pSwapchains(nullptr),
      pImageIndices(nullptr),
      pResults(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }

    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->swapchainCount);
    }

    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy((void*)pResults, (void*)in_struct->pResults, sizeof(VkResult) * in_struct->swapchainCount);
    }
}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_INFO_KHR),
      pNext(nullptr),
      waitSemaphoreCount(),
      pWaitSemaphores(nullptr),
      swapchainCount(),
      pSwapchains(nullptr),
      pImageIndices(nullptr),
      pResults(nullptr) {}

safe_VkPresentInfoKHR::safe_VkPresentInfoKHR(const safe_VkPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    waitSemaphoreCount = copy_src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (waitSemaphoreCount && copy_src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src.pSwapchains[i];
        }
    }

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    if (copy_src.pResults) {
        pResults = new VkResult[copy_src.swapchainCount];
        memcpy((void*)pResults, (void*)copy_src.pResults, sizeof(VkResult) * copy_src.swapchainCount);
    }
}

safe_VkPresentInfoKHR& safe_VkPresentInfoKHR::operator=(const safe_VkPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    waitSemaphoreCount = copy_src.waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src.swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (waitSemaphoreCount && copy_src.pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src.pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src.pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src.pSwapchains[i];
        }
    }

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    if (copy_src.pResults) {
        pResults = new VkResult[copy_src.swapchainCount];
        memcpy((void*)pResults, (void*)copy_src.pResults, sizeof(VkResult) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentInfoKHR::~safe_VkPresentInfoKHR() {
    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);
}

void safe_VkPresentInfoKHR::initialize(const VkPresentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pWaitSemaphores) delete[] pWaitSemaphores;
    if (pSwapchains) delete[] pSwapchains;
    if (pImageIndices) delete[] pImageIndices;
    if (pResults) delete[] pResults;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    waitSemaphoreCount = in_struct->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = in_struct->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (waitSemaphoreCount && in_struct->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = in_struct->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && in_struct->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = in_struct->pSwapchains[i];
        }
    }

    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->swapchainCount);
    }

    if (in_struct->pResults) {
        pResults = new VkResult[in_struct->swapchainCount];
        memcpy((void*)pResults, (void*)in_struct->pResults, sizeof(VkResult) * in_struct->swapchainCount);
    }
}

void safe_VkPresentInfoKHR::initialize(const safe_VkPresentInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    waitSemaphoreCount = copy_src->waitSemaphoreCount;
    pWaitSemaphores = nullptr;
    swapchainCount = copy_src->swapchainCount;
    pSwapchains = nullptr;
    pImageIndices = nullptr;
    pResults = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (waitSemaphoreCount && copy_src->pWaitSemaphores) {
        pWaitSemaphores = new VkSemaphore[waitSemaphoreCount];
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i) {
            pWaitSemaphores[i] = copy_src->pWaitSemaphores[i];
        }
    }
    if (swapchainCount && copy_src->pSwapchains) {
        pSwapchains = new VkSwapchainKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pSwapchains[i] = copy_src->pSwapchains[i];
        }
    }

    if (copy_src->pImageIndices) {
        pImageIndices = new uint32_t[copy_src->swapchainCount];
        memcpy((void*)pImageIndices, (void*)copy_src->pImageIndices, sizeof(uint32_t) * copy_src->swapchainCount);
    }

    if (copy_src->pResults) {
        pResults = new VkResult[copy_src->swapchainCount];
        memcpy((void*)pResults, (void*)copy_src->pResults, sizeof(VkResult) * copy_src->swapchainCount);
    }
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const VkImageSwapchainCreateInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchain(in_struct->swapchain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR), pNext(nullptr), swapchain() {}

safe_VkImageSwapchainCreateInfoKHR::safe_VkImageSwapchainCreateInfoKHR(const safe_VkImageSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageSwapchainCreateInfoKHR& safe_VkImageSwapchainCreateInfoKHR::operator=(
    const safe_VkImageSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageSwapchainCreateInfoKHR::~safe_VkImageSwapchainCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkImageSwapchainCreateInfoKHR::initialize(const VkImageSwapchainCreateInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageSwapchainCreateInfoKHR::initialize(const safe_VkImageSwapchainCreateInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(const VkBindImageMemorySwapchainInfoKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), swapchain(in_struct->swapchain), imageIndex(in_struct->imageIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR()
    : sType(VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR), pNext(nullptr), swapchain(), imageIndex() {}

safe_VkBindImageMemorySwapchainInfoKHR::safe_VkBindImageMemorySwapchainInfoKHR(
    const safe_VkBindImageMemorySwapchainInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndex = copy_src.imageIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBindImageMemorySwapchainInfoKHR& safe_VkBindImageMemorySwapchainInfoKHR::operator=(
    const safe_VkBindImageMemorySwapchainInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndex = copy_src.imageIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBindImageMemorySwapchainInfoKHR::~safe_VkBindImageMemorySwapchainInfoKHR() { FreePnextChain(pNext); }

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const VkBindImageMemorySwapchainInfoKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    imageIndex = in_struct->imageIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBindImageMemorySwapchainInfoKHR::initialize(const safe_VkBindImageMemorySwapchainInfoKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    imageIndex = copy_src->imageIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const VkAcquireNextImageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      swapchain(in_struct->swapchain),
      timeout(in_struct->timeout),
      semaphore(in_struct->semaphore),
      fence(in_struct->fence),
      deviceMask(in_struct->deviceMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR),
      pNext(nullptr),
      swapchain(),
      timeout(),
      semaphore(),
      fence(),
      deviceMask() {}

safe_VkAcquireNextImageInfoKHR::safe_VkAcquireNextImageInfoKHR(const safe_VkAcquireNextImageInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    timeout = copy_src.timeout;
    semaphore = copy_src.semaphore;
    fence = copy_src.fence;
    deviceMask = copy_src.deviceMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAcquireNextImageInfoKHR& safe_VkAcquireNextImageInfoKHR::operator=(const safe_VkAcquireNextImageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    timeout = copy_src.timeout;
    semaphore = copy_src.semaphore;
    fence = copy_src.fence;
    deviceMask = copy_src.deviceMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAcquireNextImageInfoKHR::~safe_VkAcquireNextImageInfoKHR() { FreePnextChain(pNext); }

void safe_VkAcquireNextImageInfoKHR::initialize(const VkAcquireNextImageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    timeout = in_struct->timeout;
    semaphore = in_struct->semaphore;
    fence = in_struct->fence;
    deviceMask = in_struct->deviceMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAcquireNextImageInfoKHR::initialize(const safe_VkAcquireNextImageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    timeout = copy_src->timeout;
    semaphore = copy_src->semaphore;
    fence = copy_src->fence;
    deviceMask = copy_src->deviceMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(
    const VkDeviceGroupPresentCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), modes(in_struct->modes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR), pNext(nullptr), modes() {}

safe_VkDeviceGroupPresentCapabilitiesKHR::safe_VkDeviceGroupPresentCapabilitiesKHR(
    const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src.presentMask[i];
    }
}

safe_VkDeviceGroupPresentCapabilitiesKHR& safe_VkDeviceGroupPresentCapabilitiesKHR::operator=(
    const safe_VkDeviceGroupPresentCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src.presentMask[i];
    }

    return *this;
}

safe_VkDeviceGroupPresentCapabilitiesKHR::~safe_VkDeviceGroupPresentCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const VkDeviceGroupPresentCapabilitiesKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    modes = in_struct->modes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = in_struct->presentMask[i];
    }
}

void safe_VkDeviceGroupPresentCapabilitiesKHR::initialize(const safe_VkDeviceGroupPresentCapabilitiesKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    modes = copy_src->modes;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DEVICE_GROUP_SIZE; ++i) {
        presentMask[i] = copy_src->presentMask[i];
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const VkDeviceGroupPresentInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pDeviceMasks(nullptr), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)in_struct->pDeviceMasks, sizeof(uint32_t) * in_struct->swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR), pNext(nullptr), swapchainCount(), pDeviceMasks(nullptr), mode() {}

safe_VkDeviceGroupPresentInfoKHR::safe_VkDeviceGroupPresentInfoKHR(const safe_VkDeviceGroupPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src.pDeviceMasks, sizeof(uint32_t) * copy_src.swapchainCount);
    }
}

safe_VkDeviceGroupPresentInfoKHR& safe_VkDeviceGroupPresentInfoKHR::operator=(const safe_VkDeviceGroupPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src.swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src.pDeviceMasks, sizeof(uint32_t) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkDeviceGroupPresentInfoKHR::~safe_VkDeviceGroupPresentInfoKHR() {
    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const VkDeviceGroupPresentInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDeviceMasks) delete[] pDeviceMasks;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pDeviceMasks = nullptr;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDeviceMasks) {
        pDeviceMasks = new uint32_t[in_struct->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)in_struct->pDeviceMasks, sizeof(uint32_t) * in_struct->swapchainCount);
    }
}

void safe_VkDeviceGroupPresentInfoKHR::initialize(const safe_VkDeviceGroupPresentInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pDeviceMasks = nullptr;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDeviceMasks) {
        pDeviceMasks = new uint32_t[copy_src->swapchainCount];
        memcpy((void*)pDeviceMasks, (void*)copy_src->pDeviceMasks, sizeof(uint32_t) * copy_src->swapchainCount);
    }
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(
    const VkDeviceGroupSwapchainCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), modes(in_struct->modes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR), pNext(nullptr), modes() {}

safe_VkDeviceGroupSwapchainCreateInfoKHR::safe_VkDeviceGroupSwapchainCreateInfoKHR(
    const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceGroupSwapchainCreateInfoKHR& safe_VkDeviceGroupSwapchainCreateInfoKHR::operator=(
    const safe_VkDeviceGroupSwapchainCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    modes = copy_src.modes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceGroupSwapchainCreateInfoKHR::~safe_VkDeviceGroupSwapchainCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const VkDeviceGroupSwapchainCreateInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    modes = in_struct->modes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceGroupSwapchainCreateInfoKHR::initialize(const safe_VkDeviceGroupSwapchainCreateInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    modes = copy_src->modes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const VkDisplayModeCreateInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), parameters(in_struct->parameters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR), pNext(nullptr), flags(), parameters() {}

safe_VkDisplayModeCreateInfoKHR::safe_VkDisplayModeCreateInfoKHR(const safe_VkDisplayModeCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    parameters = copy_src.parameters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayModeCreateInfoKHR& safe_VkDisplayModeCreateInfoKHR::operator=(const safe_VkDisplayModeCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    parameters = copy_src.parameters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayModeCreateInfoKHR::~safe_VkDisplayModeCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplayModeCreateInfoKHR::initialize(const VkDisplayModeCreateInfoKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    parameters = in_struct->parameters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayModeCreateInfoKHR::initialize(const safe_VkDisplayModeCreateInfoKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    parameters = copy_src->parameters;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const VkDisplayPropertiesKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : display(in_struct->display),
      physicalDimensions(in_struct->physicalDimensions),
      physicalResolution(in_struct->physicalResolution),
      supportedTransforms(in_struct->supportedTransforms),
      planeReorderPossible(in_struct->planeReorderPossible),
      persistentContent(in_struct->persistentContent) {
    displayName = SafeStringCopy(in_struct->displayName);
}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR()
    : display(),
      displayName(nullptr),
      physicalDimensions(),
      physicalResolution(),
      supportedTransforms(),
      planeReorderPossible(),
      persistentContent() {}

safe_VkDisplayPropertiesKHR::safe_VkDisplayPropertiesKHR(const safe_VkDisplayPropertiesKHR& copy_src) {
    display = copy_src.display;
    physicalDimensions = copy_src.physicalDimensions;
    physicalResolution = copy_src.physicalResolution;
    supportedTransforms = copy_src.supportedTransforms;
    planeReorderPossible = copy_src.planeReorderPossible;
    persistentContent = copy_src.persistentContent;
    displayName = SafeStringCopy(copy_src.displayName);
}

safe_VkDisplayPropertiesKHR& safe_VkDisplayPropertiesKHR::operator=(const safe_VkDisplayPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (displayName) delete[] displayName;

    display = copy_src.display;
    physicalDimensions = copy_src.physicalDimensions;
    physicalResolution = copy_src.physicalResolution;
    supportedTransforms = copy_src.supportedTransforms;
    planeReorderPossible = copy_src.planeReorderPossible;
    persistentContent = copy_src.persistentContent;
    displayName = SafeStringCopy(copy_src.displayName);

    return *this;
}

safe_VkDisplayPropertiesKHR::~safe_VkDisplayPropertiesKHR() {
    if (displayName) delete[] displayName;
}

void safe_VkDisplayPropertiesKHR::initialize(const VkDisplayPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (displayName) delete[] displayName;
    display = in_struct->display;
    physicalDimensions = in_struct->physicalDimensions;
    physicalResolution = in_struct->physicalResolution;
    supportedTransforms = in_struct->supportedTransforms;
    planeReorderPossible = in_struct->planeReorderPossible;
    persistentContent = in_struct->persistentContent;
    displayName = SafeStringCopy(in_struct->displayName);
}

void safe_VkDisplayPropertiesKHR::initialize(const safe_VkDisplayPropertiesKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    display = copy_src->display;
    physicalDimensions = copy_src->physicalDimensions;
    physicalResolution = copy_src->physicalResolution;
    supportedTransforms = copy_src->supportedTransforms;
    planeReorderPossible = copy_src->planeReorderPossible;
    persistentContent = copy_src->persistentContent;
    displayName = SafeStringCopy(copy_src->displayName);
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const VkDisplaySurfaceCreateInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      displayMode(in_struct->displayMode),
      planeIndex(in_struct->planeIndex),
      planeStackIndex(in_struct->planeStackIndex),
      transform(in_struct->transform),
      globalAlpha(in_struct->globalAlpha),
      alphaMode(in_struct->alphaMode),
      imageExtent(in_struct->imageExtent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      displayMode(),
      planeIndex(),
      planeStackIndex(),
      transform(),
      globalAlpha(),
      alphaMode(),
      imageExtent() {}

safe_VkDisplaySurfaceCreateInfoKHR::safe_VkDisplaySurfaceCreateInfoKHR(const safe_VkDisplaySurfaceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    displayMode = copy_src.displayMode;
    planeIndex = copy_src.planeIndex;
    planeStackIndex = copy_src.planeStackIndex;
    transform = copy_src.transform;
    globalAlpha = copy_src.globalAlpha;
    alphaMode = copy_src.alphaMode;
    imageExtent = copy_src.imageExtent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplaySurfaceCreateInfoKHR& safe_VkDisplaySurfaceCreateInfoKHR::operator=(
    const safe_VkDisplaySurfaceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    displayMode = copy_src.displayMode;
    planeIndex = copy_src.planeIndex;
    planeStackIndex = copy_src.planeStackIndex;
    transform = copy_src.transform;
    globalAlpha = copy_src.globalAlpha;
    alphaMode = copy_src.alphaMode;
    imageExtent = copy_src.imageExtent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplaySurfaceCreateInfoKHR::~safe_VkDisplaySurfaceCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const VkDisplaySurfaceCreateInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    displayMode = in_struct->displayMode;
    planeIndex = in_struct->planeIndex;
    planeStackIndex = in_struct->planeStackIndex;
    transform = in_struct->transform;
    globalAlpha = in_struct->globalAlpha;
    alphaMode = in_struct->alphaMode;
    imageExtent = in_struct->imageExtent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplaySurfaceCreateInfoKHR::initialize(const safe_VkDisplaySurfaceCreateInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    displayMode = copy_src->displayMode;
    planeIndex = copy_src->planeIndex;
    planeStackIndex = copy_src->planeStackIndex;
    transform = copy_src->transform;
    globalAlpha = copy_src->globalAlpha;
    alphaMode = copy_src->alphaMode;
    imageExtent = copy_src->imageExtent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const VkDisplayPresentInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), srcRect(in_struct->srcRect), dstRect(in_struct->dstRect), persistent(in_struct->persistent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR), pNext(nullptr), srcRect(), dstRect(), persistent() {}

safe_VkDisplayPresentInfoKHR::safe_VkDisplayPresentInfoKHR(const safe_VkDisplayPresentInfoKHR& copy_src) {
    sType = copy_src.sType;
    srcRect = copy_src.srcRect;
    dstRect = copy_src.dstRect;
    persistent = copy_src.persistent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPresentInfoKHR& safe_VkDisplayPresentInfoKHR::operator=(const safe_VkDisplayPresentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    srcRect = copy_src.srcRect;
    dstRect = copy_src.dstRect;
    persistent = copy_src.persistent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPresentInfoKHR::~safe_VkDisplayPresentInfoKHR() { FreePnextChain(pNext); }

void safe_VkDisplayPresentInfoKHR::initialize(const VkDisplayPresentInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    srcRect = in_struct->srcRect;
    dstRect = in_struct->dstRect;
    persistent = in_struct->persistent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPresentInfoKHR::initialize(const safe_VkDisplayPresentInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    srcRect = copy_src->srcRect;
    dstRect = copy_src->dstRect;
    persistent = copy_src->persistent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR(
    const VkQueueFamilyQueryResultStatusPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), queryResultStatusSupport(in_struct->queryResultStatusSupport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR), pNext(nullptr), queryResultStatusSupport() {}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::safe_VkQueueFamilyQueryResultStatusPropertiesKHR(
    const safe_VkQueueFamilyQueryResultStatusPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    queryResultStatusSupport = copy_src.queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR& safe_VkQueueFamilyQueryResultStatusPropertiesKHR::operator=(
    const safe_VkQueueFamilyQueryResultStatusPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    queryResultStatusSupport = copy_src.queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyQueryResultStatusPropertiesKHR::~safe_VkQueueFamilyQueryResultStatusPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyQueryResultStatusPropertiesKHR::initialize(const VkQueueFamilyQueryResultStatusPropertiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queryResultStatusSupport = in_struct->queryResultStatusSupport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyQueryResultStatusPropertiesKHR::initialize(const safe_VkQueueFamilyQueryResultStatusPropertiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queryResultStatusSupport = copy_src->queryResultStatusSupport;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR(const VkQueueFamilyVideoPropertiesKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), videoCodecOperations(in_struct->videoCodecOperations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR), pNext(nullptr), videoCodecOperations() {}

safe_VkQueueFamilyVideoPropertiesKHR::safe_VkQueueFamilyVideoPropertiesKHR(const safe_VkQueueFamilyVideoPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    videoCodecOperations = copy_src.videoCodecOperations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyVideoPropertiesKHR& safe_VkQueueFamilyVideoPropertiesKHR::operator=(
    const safe_VkQueueFamilyVideoPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoCodecOperations = copy_src.videoCodecOperations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyVideoPropertiesKHR::~safe_VkQueueFamilyVideoPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyVideoPropertiesKHR::initialize(const VkQueueFamilyVideoPropertiesKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoCodecOperations = in_struct->videoCodecOperations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyVideoPropertiesKHR::initialize(const safe_VkQueueFamilyVideoPropertiesKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoCodecOperations = copy_src->videoCodecOperations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR(const VkVideoProfileInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      videoCodecOperation(in_struct->videoCodecOperation),
      chromaSubsampling(in_struct->chromaSubsampling),
      lumaBitDepth(in_struct->lumaBitDepth),
      chromaBitDepth(in_struct->chromaBitDepth) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR),
      pNext(nullptr),
      videoCodecOperation(),
      chromaSubsampling(),
      lumaBitDepth(),
      chromaBitDepth() {}

safe_VkVideoProfileInfoKHR::safe_VkVideoProfileInfoKHR(const safe_VkVideoProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoCodecOperation = copy_src.videoCodecOperation;
    chromaSubsampling = copy_src.chromaSubsampling;
    lumaBitDepth = copy_src.lumaBitDepth;
    chromaBitDepth = copy_src.chromaBitDepth;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoProfileInfoKHR& safe_VkVideoProfileInfoKHR::operator=(const safe_VkVideoProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoCodecOperation = copy_src.videoCodecOperation;
    chromaSubsampling = copy_src.chromaSubsampling;
    lumaBitDepth = copy_src.lumaBitDepth;
    chromaBitDepth = copy_src.chromaBitDepth;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoProfileInfoKHR::~safe_VkVideoProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoProfileInfoKHR::initialize(const VkVideoProfileInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoCodecOperation = in_struct->videoCodecOperation;
    chromaSubsampling = in_struct->chromaSubsampling;
    lumaBitDepth = in_struct->lumaBitDepth;
    chromaBitDepth = in_struct->chromaBitDepth;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoProfileInfoKHR::initialize(const safe_VkVideoProfileInfoKHR* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoCodecOperation = copy_src->videoCodecOperation;
    chromaSubsampling = copy_src->chromaSubsampling;
    lumaBitDepth = copy_src->lumaBitDepth;
    chromaBitDepth = copy_src->chromaBitDepth;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR(const VkVideoProfileListInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), profileCount(in_struct->profileCount), pProfiles(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (profileCount && in_struct->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&in_struct->pProfiles[i]);
        }
    }
}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR), pNext(nullptr), profileCount(), pProfiles(nullptr) {}

safe_VkVideoProfileListInfoKHR::safe_VkVideoProfileListInfoKHR(const safe_VkVideoProfileListInfoKHR& copy_src) {
    sType = copy_src.sType;
    profileCount = copy_src.profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (profileCount && copy_src.pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src.pProfiles[i]);
        }
    }
}

safe_VkVideoProfileListInfoKHR& safe_VkVideoProfileListInfoKHR::operator=(const safe_VkVideoProfileListInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    profileCount = copy_src.profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (profileCount && copy_src.pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src.pProfiles[i]);
        }
    }

    return *this;
}

safe_VkVideoProfileListInfoKHR::~safe_VkVideoProfileListInfoKHR() {
    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);
}

void safe_VkVideoProfileListInfoKHR::initialize(const VkVideoProfileListInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pProfiles) delete[] pProfiles;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    profileCount = in_struct->profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (profileCount && in_struct->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&in_struct->pProfiles[i]);
        }
    }
}

void safe_VkVideoProfileListInfoKHR::initialize(const safe_VkVideoProfileListInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    profileCount = copy_src->profileCount;
    pProfiles = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (profileCount && copy_src->pProfiles) {
        pProfiles = new safe_VkVideoProfileInfoKHR[profileCount];
        for (uint32_t i = 0; i < profileCount; ++i) {
            pProfiles[i].initialize(&copy_src->pProfiles[i]);
        }
    }
}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR(const VkVideoCapabilitiesKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      minBitstreamBufferOffsetAlignment(in_struct->minBitstreamBufferOffsetAlignment),
      minBitstreamBufferSizeAlignment(in_struct->minBitstreamBufferSizeAlignment),
      pictureAccessGranularity(in_struct->pictureAccessGranularity),
      minCodedExtent(in_struct->minCodedExtent),
      maxCodedExtent(in_struct->maxCodedExtent),
      maxDpbSlots(in_struct->maxDpbSlots),
      maxActiveReferencePictures(in_struct->maxActiveReferencePictures),
      stdHeaderVersion(in_struct->stdHeaderVersion) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      minBitstreamBufferOffsetAlignment(),
      minBitstreamBufferSizeAlignment(),
      pictureAccessGranularity(),
      minCodedExtent(),
      maxCodedExtent(),
      maxDpbSlots(),
      maxActiveReferencePictures(),
      stdHeaderVersion() {}

safe_VkVideoCapabilitiesKHR::safe_VkVideoCapabilitiesKHR(const safe_VkVideoCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    minBitstreamBufferOffsetAlignment = copy_src.minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src.minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src.pictureAccessGranularity;
    minCodedExtent = copy_src.minCodedExtent;
    maxCodedExtent = copy_src.maxCodedExtent;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    stdHeaderVersion = copy_src.stdHeaderVersion;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoCapabilitiesKHR& safe_VkVideoCapabilitiesKHR::operator=(const safe_VkVideoCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    minBitstreamBufferOffsetAlignment = copy_src.minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src.minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src.pictureAccessGranularity;
    minCodedExtent = copy_src.minCodedExtent;
    maxCodedExtent = copy_src.maxCodedExtent;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    stdHeaderVersion = copy_src.stdHeaderVersion;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoCapabilitiesKHR::~safe_VkVideoCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoCapabilitiesKHR::initialize(const VkVideoCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    minBitstreamBufferOffsetAlignment = in_struct->minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = in_struct->minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = in_struct->pictureAccessGranularity;
    minCodedExtent = in_struct->minCodedExtent;
    maxCodedExtent = in_struct->maxCodedExtent;
    maxDpbSlots = in_struct->maxDpbSlots;
    maxActiveReferencePictures = in_struct->maxActiveReferencePictures;
    stdHeaderVersion = in_struct->stdHeaderVersion;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoCapabilitiesKHR::initialize(const safe_VkVideoCapabilitiesKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    minBitstreamBufferOffsetAlignment = copy_src->minBitstreamBufferOffsetAlignment;
    minBitstreamBufferSizeAlignment = copy_src->minBitstreamBufferSizeAlignment;
    pictureAccessGranularity = copy_src->pictureAccessGranularity;
    minCodedExtent = copy_src->minCodedExtent;
    maxCodedExtent = copy_src->maxCodedExtent;
    maxDpbSlots = copy_src->maxDpbSlots;
    maxActiveReferencePictures = copy_src->maxActiveReferencePictures;
    stdHeaderVersion = copy_src->stdHeaderVersion;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR(
    const VkPhysicalDeviceVideoFormatInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageUsage(in_struct->imageUsage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_FORMAT_INFO_KHR), pNext(nullptr), imageUsage() {}

safe_VkPhysicalDeviceVideoFormatInfoKHR::safe_VkPhysicalDeviceVideoFormatInfoKHR(
    const safe_VkPhysicalDeviceVideoFormatInfoKHR& copy_src) {
    sType = copy_src.sType;
    imageUsage = copy_src.imageUsage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoFormatInfoKHR& safe_VkPhysicalDeviceVideoFormatInfoKHR::operator=(
    const safe_VkPhysicalDeviceVideoFormatInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageUsage = copy_src.imageUsage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoFormatInfoKHR::~safe_VkPhysicalDeviceVideoFormatInfoKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoFormatInfoKHR::initialize(const VkPhysicalDeviceVideoFormatInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageUsage = in_struct->imageUsage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoFormatInfoKHR::initialize(const safe_VkPhysicalDeviceVideoFormatInfoKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageUsage = copy_src->imageUsage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR(const VkVideoFormatPropertiesKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      format(in_struct->format),
      componentMapping(in_struct->componentMapping),
      imageCreateFlags(in_struct->imageCreateFlags),
      imageType(in_struct->imageType),
      imageTiling(in_struct->imageTiling),
      imageUsageFlags(in_struct->imageUsageFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_FORMAT_PROPERTIES_KHR),
      pNext(nullptr),
      format(),
      componentMapping(),
      imageCreateFlags(),
      imageType(),
      imageTiling(),
      imageUsageFlags() {}

safe_VkVideoFormatPropertiesKHR::safe_VkVideoFormatPropertiesKHR(const safe_VkVideoFormatPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    componentMapping = copy_src.componentMapping;
    imageCreateFlags = copy_src.imageCreateFlags;
    imageType = copy_src.imageType;
    imageTiling = copy_src.imageTiling;
    imageUsageFlags = copy_src.imageUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoFormatPropertiesKHR& safe_VkVideoFormatPropertiesKHR::operator=(const safe_VkVideoFormatPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    componentMapping = copy_src.componentMapping;
    imageCreateFlags = copy_src.imageCreateFlags;
    imageType = copy_src.imageType;
    imageTiling = copy_src.imageTiling;
    imageUsageFlags = copy_src.imageUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoFormatPropertiesKHR::~safe_VkVideoFormatPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoFormatPropertiesKHR::initialize(const VkVideoFormatPropertiesKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    componentMapping = in_struct->componentMapping;
    imageCreateFlags = in_struct->imageCreateFlags;
    imageType = in_struct->imageType;
    imageTiling = in_struct->imageTiling;
    imageUsageFlags = in_struct->imageUsageFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoFormatPropertiesKHR::initialize(const safe_VkVideoFormatPropertiesKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    componentMapping = copy_src->componentMapping;
    imageCreateFlags = copy_src->imageCreateFlags;
    imageType = copy_src->imageType;
    imageTiling = copy_src->imageTiling;
    imageUsageFlags = copy_src->imageUsageFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR(const VkVideoPictureResourceInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      codedOffset(in_struct->codedOffset),
      codedExtent(in_struct->codedExtent),
      baseArrayLayer(in_struct->baseArrayLayer),
      imageViewBinding(in_struct->imageViewBinding) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR),
      pNext(nullptr),
      codedOffset(),
      codedExtent(),
      baseArrayLayer(),
      imageViewBinding() {}

safe_VkVideoPictureResourceInfoKHR::safe_VkVideoPictureResourceInfoKHR(const safe_VkVideoPictureResourceInfoKHR& copy_src) {
    sType = copy_src.sType;
    codedOffset = copy_src.codedOffset;
    codedExtent = copy_src.codedExtent;
    baseArrayLayer = copy_src.baseArrayLayer;
    imageViewBinding = copy_src.imageViewBinding;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoPictureResourceInfoKHR& safe_VkVideoPictureResourceInfoKHR::operator=(
    const safe_VkVideoPictureResourceInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    codedOffset = copy_src.codedOffset;
    codedExtent = copy_src.codedExtent;
    baseArrayLayer = copy_src.baseArrayLayer;
    imageViewBinding = copy_src.imageViewBinding;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoPictureResourceInfoKHR::~safe_VkVideoPictureResourceInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoPictureResourceInfoKHR::initialize(const VkVideoPictureResourceInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    codedOffset = in_struct->codedOffset;
    codedExtent = in_struct->codedExtent;
    baseArrayLayer = in_struct->baseArrayLayer;
    imageViewBinding = in_struct->imageViewBinding;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoPictureResourceInfoKHR::initialize(const safe_VkVideoPictureResourceInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    codedOffset = copy_src->codedOffset;
    codedExtent = copy_src->codedExtent;
    baseArrayLayer = copy_src->baseArrayLayer;
    imageViewBinding = copy_src->imageViewBinding;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR(const VkVideoReferenceSlotInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), slotIndex(in_struct->slotIndex), pPictureResource(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(in_struct->pPictureResource);
}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR), pNext(nullptr), slotIndex(), pPictureResource(nullptr) {}

safe_VkVideoReferenceSlotInfoKHR::safe_VkVideoReferenceSlotInfoKHR(const safe_VkVideoReferenceSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    slotIndex = copy_src.slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src.pPictureResource);
}

safe_VkVideoReferenceSlotInfoKHR& safe_VkVideoReferenceSlotInfoKHR::operator=(const safe_VkVideoReferenceSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    slotIndex = copy_src.slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src.pPictureResource);

    return *this;
}

safe_VkVideoReferenceSlotInfoKHR::~safe_VkVideoReferenceSlotInfoKHR() {
    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);
}

void safe_VkVideoReferenceSlotInfoKHR::initialize(const VkVideoReferenceSlotInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPictureResource) delete pPictureResource;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    slotIndex = in_struct->slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(in_struct->pPictureResource);
}

void safe_VkVideoReferenceSlotInfoKHR::initialize(const safe_VkVideoReferenceSlotInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    slotIndex = copy_src->slotIndex;
    pPictureResource = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pPictureResource) pPictureResource = new safe_VkVideoPictureResourceInfoKHR(*copy_src->pPictureResource);
}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR(
    const VkVideoSessionMemoryRequirementsKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryBindIndex(in_struct->memoryBindIndex), memoryRequirements(in_struct->memoryRequirements) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR), pNext(nullptr), memoryBindIndex(), memoryRequirements() {}

safe_VkVideoSessionMemoryRequirementsKHR::safe_VkVideoSessionMemoryRequirementsKHR(
    const safe_VkVideoSessionMemoryRequirementsKHR& copy_src) {
    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memoryRequirements = copy_src.memoryRequirements;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionMemoryRequirementsKHR& safe_VkVideoSessionMemoryRequirementsKHR::operator=(
    const safe_VkVideoSessionMemoryRequirementsKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memoryRequirements = copy_src.memoryRequirements;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionMemoryRequirementsKHR::~safe_VkVideoSessionMemoryRequirementsKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionMemoryRequirementsKHR::initialize(const VkVideoSessionMemoryRequirementsKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryBindIndex = in_struct->memoryBindIndex;
    memoryRequirements = in_struct->memoryRequirements;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionMemoryRequirementsKHR::initialize(const safe_VkVideoSessionMemoryRequirementsKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryBindIndex = copy_src->memoryBindIndex;
    memoryRequirements = copy_src->memoryRequirements;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR(const VkBindVideoSessionMemoryInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      memoryBindIndex(in_struct->memoryBindIndex),
      memory(in_struct->memory),
      memoryOffset(in_struct->memoryOffset),
      memorySize(in_struct->memorySize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR),
      pNext(nullptr),
      memoryBindIndex(),
      memory(),
      memoryOffset(),
      memorySize() {}

safe_VkBindVideoSessionMemoryInfoKHR::safe_VkBindVideoSessionMemoryInfoKHR(const safe_VkBindVideoSessionMemoryInfoKHR& copy_src) {
    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    memorySize = copy_src.memorySize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBindVideoSessionMemoryInfoKHR& safe_VkBindVideoSessionMemoryInfoKHR::operator=(
    const safe_VkBindVideoSessionMemoryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryBindIndex = copy_src.memoryBindIndex;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    memorySize = copy_src.memorySize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBindVideoSessionMemoryInfoKHR::~safe_VkBindVideoSessionMemoryInfoKHR() { FreePnextChain(pNext); }

void safe_VkBindVideoSessionMemoryInfoKHR::initialize(const VkBindVideoSessionMemoryInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryBindIndex = in_struct->memoryBindIndex;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
    memorySize = in_struct->memorySize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBindVideoSessionMemoryInfoKHR::initialize(const safe_VkBindVideoSessionMemoryInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryBindIndex = copy_src->memoryBindIndex;
    memory = copy_src->memory;
    memoryOffset = copy_src->memoryOffset;
    memorySize = copy_src->memorySize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR(const VkVideoSessionCreateInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      queueFamilyIndex(in_struct->queueFamilyIndex),
      flags(in_struct->flags),
      pVideoProfile(nullptr),
      pictureFormat(in_struct->pictureFormat),
      maxCodedExtent(in_struct->maxCodedExtent),
      referencePictureFormat(in_struct->referencePictureFormat),
      maxDpbSlots(in_struct->maxDpbSlots),
      maxActiveReferencePictures(in_struct->maxActiveReferencePictures),
      pStdHeaderVersion(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);

    if (in_struct->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*in_struct->pStdHeaderVersion);
    }
}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR),
      pNext(nullptr),
      queueFamilyIndex(),
      flags(),
      pVideoProfile(nullptr),
      pictureFormat(),
      maxCodedExtent(),
      referencePictureFormat(),
      maxDpbSlots(),
      maxActiveReferencePictures(),
      pStdHeaderVersion(nullptr) {}

safe_VkVideoSessionCreateInfoKHR::safe_VkVideoSessionCreateInfoKHR(const safe_VkVideoSessionCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    flags = copy_src.flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src.pictureFormat;
    maxCodedExtent = copy_src.maxCodedExtent;
    referencePictureFormat = copy_src.referencePictureFormat;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    if (copy_src.pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src.pStdHeaderVersion);
    }
}

safe_VkVideoSessionCreateInfoKHR& safe_VkVideoSessionCreateInfoKHR::operator=(const safe_VkVideoSessionCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    flags = copy_src.flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src.pictureFormat;
    maxCodedExtent = copy_src.maxCodedExtent;
    referencePictureFormat = copy_src.referencePictureFormat;
    maxDpbSlots = copy_src.maxDpbSlots;
    maxActiveReferencePictures = copy_src.maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    if (copy_src.pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src.pStdHeaderVersion);
    }

    return *this;
}

safe_VkVideoSessionCreateInfoKHR::~safe_VkVideoSessionCreateInfoKHR() {
    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);
}

void safe_VkVideoSessionCreateInfoKHR::initialize(const VkVideoSessionCreateInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVideoProfile) delete pVideoProfile;
    if (pStdHeaderVersion) delete pStdHeaderVersion;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    flags = in_struct->flags;
    pVideoProfile = nullptr;
    pictureFormat = in_struct->pictureFormat;
    maxCodedExtent = in_struct->maxCodedExtent;
    referencePictureFormat = in_struct->referencePictureFormat;
    maxDpbSlots = in_struct->maxDpbSlots;
    maxActiveReferencePictures = in_struct->maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);

    if (in_struct->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*in_struct->pStdHeaderVersion);
    }
}

void safe_VkVideoSessionCreateInfoKHR::initialize(const safe_VkVideoSessionCreateInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queueFamilyIndex = copy_src->queueFamilyIndex;
    flags = copy_src->flags;
    pVideoProfile = nullptr;
    pictureFormat = copy_src->pictureFormat;
    maxCodedExtent = copy_src->maxCodedExtent;
    referencePictureFormat = copy_src->referencePictureFormat;
    maxDpbSlots = copy_src->maxDpbSlots;
    maxActiveReferencePictures = copy_src->maxActiveReferencePictures;
    pStdHeaderVersion = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src->pVideoProfile);

    if (copy_src->pStdHeaderVersion) {
        pStdHeaderVersion = new VkExtensionProperties(*copy_src->pStdHeaderVersion);
    }
}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR(
    const VkVideoSessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      videoSessionParametersTemplate(in_struct->videoSessionParametersTemplate),
      videoSession(in_struct->videoSession) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      videoSessionParametersTemplate(),
      videoSession() {}

safe_VkVideoSessionParametersCreateInfoKHR::safe_VkVideoSessionParametersCreateInfoKHR(
    const safe_VkVideoSessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSessionParametersTemplate = copy_src.videoSessionParametersTemplate;
    videoSession = copy_src.videoSession;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionParametersCreateInfoKHR& safe_VkVideoSessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoSessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSessionParametersTemplate = copy_src.videoSessionParametersTemplate;
    videoSession = copy_src.videoSession;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionParametersCreateInfoKHR::~safe_VkVideoSessionParametersCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionParametersCreateInfoKHR::initialize(const VkVideoSessionParametersCreateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    videoSessionParametersTemplate = in_struct->videoSessionParametersTemplate;
    videoSession = in_struct->videoSession;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionParametersCreateInfoKHR::initialize(const safe_VkVideoSessionParametersCreateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    videoSessionParametersTemplate = copy_src->videoSessionParametersTemplate;
    videoSession = copy_src->videoSession;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR(
    const VkVideoSessionParametersUpdateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), updateSequenceCount(in_struct->updateSequenceCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_UPDATE_INFO_KHR), pNext(nullptr), updateSequenceCount() {}

safe_VkVideoSessionParametersUpdateInfoKHR::safe_VkVideoSessionParametersUpdateInfoKHR(
    const safe_VkVideoSessionParametersUpdateInfoKHR& copy_src) {
    sType = copy_src.sType;
    updateSequenceCount = copy_src.updateSequenceCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoSessionParametersUpdateInfoKHR& safe_VkVideoSessionParametersUpdateInfoKHR::operator=(
    const safe_VkVideoSessionParametersUpdateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    updateSequenceCount = copy_src.updateSequenceCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoSessionParametersUpdateInfoKHR::~safe_VkVideoSessionParametersUpdateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoSessionParametersUpdateInfoKHR::initialize(const VkVideoSessionParametersUpdateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    updateSequenceCount = in_struct->updateSequenceCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoSessionParametersUpdateInfoKHR::initialize(const safe_VkVideoSessionParametersUpdateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    updateSequenceCount = copy_src->updateSequenceCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR(const VkVideoBeginCodingInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      videoSession(in_struct->videoSession),
      videoSessionParameters(in_struct->videoSessionParameters),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR),
      pNext(nullptr),
      flags(),
      videoSession(),
      videoSessionParameters(),
      referenceSlotCount(),
      pReferenceSlots(nullptr) {}

safe_VkVideoBeginCodingInfoKHR::safe_VkVideoBeginCodingInfoKHR(const safe_VkVideoBeginCodingInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSession = copy_src.videoSession;
    videoSessionParameters = copy_src.videoSessionParameters;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoBeginCodingInfoKHR& safe_VkVideoBeginCodingInfoKHR::operator=(const safe_VkVideoBeginCodingInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    videoSession = copy_src.videoSession;
    videoSessionParameters = copy_src.videoSessionParameters;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoBeginCodingInfoKHR::~safe_VkVideoBeginCodingInfoKHR() {
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoBeginCodingInfoKHR::initialize(const VkVideoBeginCodingInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    videoSession = in_struct->videoSession;
    videoSessionParameters = in_struct->videoSessionParameters;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoBeginCodingInfoKHR::initialize(const safe_VkVideoBeginCodingInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    videoSession = copy_src->videoSession;
    videoSessionParameters = copy_src->videoSessionParameters;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR(const VkVideoEndCodingInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR), pNext(nullptr), flags() {}

safe_VkVideoEndCodingInfoKHR::safe_VkVideoEndCodingInfoKHR(const safe_VkVideoEndCodingInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEndCodingInfoKHR& safe_VkVideoEndCodingInfoKHR::operator=(const safe_VkVideoEndCodingInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEndCodingInfoKHR::~safe_VkVideoEndCodingInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEndCodingInfoKHR::initialize(const VkVideoEndCodingInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEndCodingInfoKHR::initialize(const safe_VkVideoEndCodingInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR(const VkVideoCodingControlInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR), pNext(nullptr), flags() {}

safe_VkVideoCodingControlInfoKHR::safe_VkVideoCodingControlInfoKHR(const safe_VkVideoCodingControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoCodingControlInfoKHR& safe_VkVideoCodingControlInfoKHR::operator=(const safe_VkVideoCodingControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoCodingControlInfoKHR::~safe_VkVideoCodingControlInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoCodingControlInfoKHR::initialize(const VkVideoCodingControlInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoCodingControlInfoKHR::initialize(const safe_VkVideoCodingControlInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR(const VkVideoDecodeCapabilitiesKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR), pNext(nullptr), flags() {}

safe_VkVideoDecodeCapabilitiesKHR::safe_VkVideoDecodeCapabilitiesKHR(const safe_VkVideoDecodeCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeCapabilitiesKHR& safe_VkVideoDecodeCapabilitiesKHR::operator=(const safe_VkVideoDecodeCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeCapabilitiesKHR::~safe_VkVideoDecodeCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeCapabilitiesKHR::initialize(const VkVideoDecodeCapabilitiesKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeCapabilitiesKHR::initialize(const safe_VkVideoDecodeCapabilitiesKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR(const VkVideoDecodeUsageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoUsageHints(in_struct->videoUsageHints) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR), pNext(nullptr), videoUsageHints() {}

safe_VkVideoDecodeUsageInfoKHR::safe_VkVideoDecodeUsageInfoKHR(const safe_VkVideoDecodeUsageInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeUsageInfoKHR& safe_VkVideoDecodeUsageInfoKHR::operator=(const safe_VkVideoDecodeUsageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeUsageInfoKHR::~safe_VkVideoDecodeUsageInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeUsageInfoKHR::initialize(const VkVideoDecodeUsageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoUsageHints = in_struct->videoUsageHints;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeUsageInfoKHR::initialize(const safe_VkVideoDecodeUsageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoUsageHints = copy_src->videoUsageHints;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR(const VkVideoDecodeInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      srcBuffer(in_struct->srcBuffer),
      srcBufferOffset(in_struct->srcBufferOffset),
      srcBufferRange(in_struct->srcBufferRange),
      dstPictureResource(&in_struct->dstPictureResource),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR),
      pNext(nullptr),
      flags(),
      srcBuffer(),
      srcBufferOffset(),
      srcBufferRange(),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(),
      pReferenceSlots(nullptr) {}

safe_VkVideoDecodeInfoKHR::safe_VkVideoDecodeInfoKHR(const safe_VkVideoDecodeInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    srcBuffer = copy_src.srcBuffer;
    srcBufferOffset = copy_src.srcBufferOffset;
    srcBufferRange = copy_src.srcBufferRange;
    dstPictureResource.initialize(&copy_src.dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoDecodeInfoKHR& safe_VkVideoDecodeInfoKHR::operator=(const safe_VkVideoDecodeInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    srcBuffer = copy_src.srcBuffer;
    srcBufferOffset = copy_src.srcBufferOffset;
    srcBufferRange = copy_src.srcBufferRange;
    dstPictureResource.initialize(&copy_src.dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoDecodeInfoKHR::~safe_VkVideoDecodeInfoKHR() {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeInfoKHR::initialize(const VkVideoDecodeInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    srcBuffer = in_struct->srcBuffer;
    srcBufferOffset = in_struct->srcBufferOffset;
    srcBufferRange = in_struct->srcBufferRange;
    dstPictureResource.initialize(&in_struct->dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoDecodeInfoKHR::initialize(const safe_VkVideoDecodeInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    srcBuffer = copy_src->srcBuffer;
    srcBufferOffset = copy_src->srcBufferOffset;
    srcBufferRange = copy_src->srcBufferRange;
    dstPictureResource.initialize(&copy_src->dstPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src->pSetupReferenceSlot);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeH264CapabilitiesKHR::safe_VkVideoEncodeH264CapabilitiesKHR(const VkVideoEncodeH264CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      maxLevelIdc(in_struct->maxLevelIdc),
      maxSliceCount(in_struct->maxSliceCount),
      maxPPictureL0ReferenceCount(in_struct->maxPPictureL0ReferenceCount),
      maxBPictureL0ReferenceCount(in_struct->maxBPictureL0ReferenceCount),
      maxL1ReferenceCount(in_struct->maxL1ReferenceCount),
      maxTemporalLayerCount(in_struct->maxTemporalLayerCount),
      expectDyadicTemporalLayerPattern(in_struct->expectDyadicTemporalLayerPattern),
      minQp(in_struct->minQp),
      maxQp(in_struct->maxQp),
      prefersGopRemainingFrames(in_struct->prefersGopRemainingFrames),
      requiresGopRemainingFrames(in_struct->requiresGopRemainingFrames),
      stdSyntaxFlags(in_struct->stdSyntaxFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264CapabilitiesKHR::safe_VkVideoEncodeH264CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      maxLevelIdc(),
      maxSliceCount(),
      maxPPictureL0ReferenceCount(),
      maxBPictureL0ReferenceCount(),
      maxL1ReferenceCount(),
      maxTemporalLayerCount(),
      expectDyadicTemporalLayerPattern(),
      minQp(),
      maxQp(),
      prefersGopRemainingFrames(),
      requiresGopRemainingFrames(),
      stdSyntaxFlags() {}

safe_VkVideoEncodeH264CapabilitiesKHR::safe_VkVideoEncodeH264CapabilitiesKHR(
    const safe_VkVideoEncodeH264CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevelIdc = copy_src.maxLevelIdc;
    maxSliceCount = copy_src.maxSliceCount;
    maxPPictureL0ReferenceCount = copy_src.maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src.maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src.maxL1ReferenceCount;
    maxTemporalLayerCount = copy_src.maxTemporalLayerCount;
    expectDyadicTemporalLayerPattern = copy_src.expectDyadicTemporalLayerPattern;
    minQp = copy_src.minQp;
    maxQp = copy_src.maxQp;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264CapabilitiesKHR& safe_VkVideoEncodeH264CapabilitiesKHR::operator=(
    const safe_VkVideoEncodeH264CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevelIdc = copy_src.maxLevelIdc;
    maxSliceCount = copy_src.maxSliceCount;
    maxPPictureL0ReferenceCount = copy_src.maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src.maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src.maxL1ReferenceCount;
    maxTemporalLayerCount = copy_src.maxTemporalLayerCount;
    expectDyadicTemporalLayerPattern = copy_src.expectDyadicTemporalLayerPattern;
    minQp = copy_src.minQp;
    maxQp = copy_src.maxQp;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264CapabilitiesKHR::~safe_VkVideoEncodeH264CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264CapabilitiesKHR::initialize(const VkVideoEncodeH264CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    maxLevelIdc = in_struct->maxLevelIdc;
    maxSliceCount = in_struct->maxSliceCount;
    maxPPictureL0ReferenceCount = in_struct->maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = in_struct->maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = in_struct->maxL1ReferenceCount;
    maxTemporalLayerCount = in_struct->maxTemporalLayerCount;
    expectDyadicTemporalLayerPattern = in_struct->expectDyadicTemporalLayerPattern;
    minQp = in_struct->minQp;
    maxQp = in_struct->maxQp;
    prefersGopRemainingFrames = in_struct->prefersGopRemainingFrames;
    requiresGopRemainingFrames = in_struct->requiresGopRemainingFrames;
    stdSyntaxFlags = in_struct->stdSyntaxFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264CapabilitiesKHR::initialize(const safe_VkVideoEncodeH264CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    maxLevelIdc = copy_src->maxLevelIdc;
    maxSliceCount = copy_src->maxSliceCount;
    maxPPictureL0ReferenceCount = copy_src->maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src->maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src->maxL1ReferenceCount;
    maxTemporalLayerCount = copy_src->maxTemporalLayerCount;
    expectDyadicTemporalLayerPattern = copy_src->expectDyadicTemporalLayerPattern;
    minQp = copy_src->minQp;
    maxQp = copy_src->maxQp;
    prefersGopRemainingFrames = copy_src->prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src->requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src->stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264QualityLevelPropertiesKHR::safe_VkVideoEncodeH264QualityLevelPropertiesKHR(
    const VkVideoEncodeH264QualityLevelPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      preferredRateControlFlags(in_struct->preferredRateControlFlags),
      preferredGopFrameCount(in_struct->preferredGopFrameCount),
      preferredIdrPeriod(in_struct->preferredIdrPeriod),
      preferredConsecutiveBFrameCount(in_struct->preferredConsecutiveBFrameCount),
      preferredTemporalLayerCount(in_struct->preferredTemporalLayerCount),
      preferredConstantQp(in_struct->preferredConstantQp),
      preferredMaxL0ReferenceCount(in_struct->preferredMaxL0ReferenceCount),
      preferredMaxL1ReferenceCount(in_struct->preferredMaxL1ReferenceCount),
      preferredStdEntropyCodingModeFlag(in_struct->preferredStdEntropyCodingModeFlag) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264QualityLevelPropertiesKHR::safe_VkVideoEncodeH264QualityLevelPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_KHR),
      pNext(nullptr),
      preferredRateControlFlags(),
      preferredGopFrameCount(),
      preferredIdrPeriod(),
      preferredConsecutiveBFrameCount(),
      preferredTemporalLayerCount(),
      preferredConstantQp(),
      preferredMaxL0ReferenceCount(),
      preferredMaxL1ReferenceCount(),
      preferredStdEntropyCodingModeFlag() {}

safe_VkVideoEncodeH264QualityLevelPropertiesKHR::safe_VkVideoEncodeH264QualityLevelPropertiesKHR(
    const safe_VkVideoEncodeH264QualityLevelPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredIdrPeriod = copy_src.preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src.preferredConsecutiveBFrameCount;
    preferredTemporalLayerCount = copy_src.preferredTemporalLayerCount;
    preferredConstantQp = copy_src.preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src.preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src.preferredMaxL1ReferenceCount;
    preferredStdEntropyCodingModeFlag = copy_src.preferredStdEntropyCodingModeFlag;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264QualityLevelPropertiesKHR& safe_VkVideoEncodeH264QualityLevelPropertiesKHR::operator=(
    const safe_VkVideoEncodeH264QualityLevelPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredIdrPeriod = copy_src.preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src.preferredConsecutiveBFrameCount;
    preferredTemporalLayerCount = copy_src.preferredTemporalLayerCount;
    preferredConstantQp = copy_src.preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src.preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src.preferredMaxL1ReferenceCount;
    preferredStdEntropyCodingModeFlag = copy_src.preferredStdEntropyCodingModeFlag;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264QualityLevelPropertiesKHR::~safe_VkVideoEncodeH264QualityLevelPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264QualityLevelPropertiesKHR::initialize(const VkVideoEncodeH264QualityLevelPropertiesKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    preferredRateControlFlags = in_struct->preferredRateControlFlags;
    preferredGopFrameCount = in_struct->preferredGopFrameCount;
    preferredIdrPeriod = in_struct->preferredIdrPeriod;
    preferredConsecutiveBFrameCount = in_struct->preferredConsecutiveBFrameCount;
    preferredTemporalLayerCount = in_struct->preferredTemporalLayerCount;
    preferredConstantQp = in_struct->preferredConstantQp;
    preferredMaxL0ReferenceCount = in_struct->preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = in_struct->preferredMaxL1ReferenceCount;
    preferredStdEntropyCodingModeFlag = in_struct->preferredStdEntropyCodingModeFlag;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264QualityLevelPropertiesKHR::initialize(const safe_VkVideoEncodeH264QualityLevelPropertiesKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    preferredRateControlFlags = copy_src->preferredRateControlFlags;
    preferredGopFrameCount = copy_src->preferredGopFrameCount;
    preferredIdrPeriod = copy_src->preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src->preferredConsecutiveBFrameCount;
    preferredTemporalLayerCount = copy_src->preferredTemporalLayerCount;
    preferredConstantQp = copy_src->preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src->preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src->preferredMaxL1ReferenceCount;
    preferredStdEntropyCodingModeFlag = copy_src->preferredStdEntropyCodingModeFlag;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264SessionCreateInfoKHR::safe_VkVideoEncodeH264SessionCreateInfoKHR(
    const VkVideoEncodeH264SessionCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), useMaxLevelIdc(in_struct->useMaxLevelIdc), maxLevelIdc(in_struct->maxLevelIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264SessionCreateInfoKHR::safe_VkVideoEncodeH264SessionCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_KHR), pNext(nullptr), useMaxLevelIdc(), maxLevelIdc() {}

safe_VkVideoEncodeH264SessionCreateInfoKHR::safe_VkVideoEncodeH264SessionCreateInfoKHR(
    const safe_VkVideoEncodeH264SessionCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMaxLevelIdc = copy_src.useMaxLevelIdc;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264SessionCreateInfoKHR& safe_VkVideoEncodeH264SessionCreateInfoKHR::operator=(
    const safe_VkVideoEncodeH264SessionCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMaxLevelIdc = copy_src.useMaxLevelIdc;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264SessionCreateInfoKHR::~safe_VkVideoEncodeH264SessionCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264SessionCreateInfoKHR::initialize(const VkVideoEncodeH264SessionCreateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMaxLevelIdc = in_struct->useMaxLevelIdc;
    maxLevelIdc = in_struct->maxLevelIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264SessionCreateInfoKHR::initialize(const safe_VkVideoEncodeH264SessionCreateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMaxLevelIdc = copy_src->useMaxLevelIdc;
    maxLevelIdc = copy_src->maxLevelIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264SessionParametersAddInfoKHR::safe_VkVideoEncodeH264SessionParametersAddInfoKHR(
    const VkVideoEncodeH264SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoEncodeH264SessionParametersAddInfoKHR::safe_VkVideoEncodeH264SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoEncodeH264SessionParametersAddInfoKHR::safe_VkVideoEncodeH264SessionParametersAddInfoKHR(
    const safe_VkVideoEncodeH264SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoEncodeH264SessionParametersAddInfoKHR& safe_VkVideoEncodeH264SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoEncodeH264SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoEncodeH264SessionParametersAddInfoKHR::~safe_VkVideoEncodeH264SessionParametersAddInfoKHR() {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264SessionParametersAddInfoKHR::initialize(const VkVideoEncodeH264SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoEncodeH264SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoEncodeH264SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::safe_VkVideoEncodeH264SessionParametersCreateInfoKHR(
    const VkVideoEncodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::safe_VkVideoEncodeH264SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::safe_VkVideoEncodeH264SessionParametersCreateInfoKHR(
    const safe_VkVideoEncodeH264SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoEncodeH264SessionParametersCreateInfoKHR& safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoEncodeH264SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::~safe_VkVideoEncodeH264SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::initialize(
    const VkVideoEncodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoEncodeH264SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoEncodeH264SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH264SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoEncodeH264SessionParametersGetInfoKHR::safe_VkVideoEncodeH264SessionParametersGetInfoKHR(
    const VkVideoEncodeH264SessionParametersGetInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      writeStdSPS(in_struct->writeStdSPS),
      writeStdPPS(in_struct->writeStdPPS),
      stdSPSId(in_struct->stdSPSId),
      stdPPSId(in_struct->stdPPSId) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264SessionParametersGetInfoKHR::safe_VkVideoEncodeH264SessionParametersGetInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_KHR),
      pNext(nullptr),
      writeStdSPS(),
      writeStdPPS(),
      stdSPSId(),
      stdPPSId() {}

safe_VkVideoEncodeH264SessionParametersGetInfoKHR::safe_VkVideoEncodeH264SessionParametersGetInfoKHR(
    const safe_VkVideoEncodeH264SessionParametersGetInfoKHR& copy_src) {
    sType = copy_src.sType;
    writeStdSPS = copy_src.writeStdSPS;
    writeStdPPS = copy_src.writeStdPPS;
    stdSPSId = copy_src.stdSPSId;
    stdPPSId = copy_src.stdPPSId;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264SessionParametersGetInfoKHR& safe_VkVideoEncodeH264SessionParametersGetInfoKHR::operator=(
    const safe_VkVideoEncodeH264SessionParametersGetInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    writeStdSPS = copy_src.writeStdSPS;
    writeStdPPS = copy_src.writeStdPPS;
    stdSPSId = copy_src.stdSPSId;
    stdPPSId = copy_src.stdPPSId;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264SessionParametersGetInfoKHR::~safe_VkVideoEncodeH264SessionParametersGetInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264SessionParametersGetInfoKHR::initialize(const VkVideoEncodeH264SessionParametersGetInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    writeStdSPS = in_struct->writeStdSPS;
    writeStdPPS = in_struct->writeStdPPS;
    stdSPSId = in_struct->stdSPSId;
    stdPPSId = in_struct->stdPPSId;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264SessionParametersGetInfoKHR::initialize(
    const safe_VkVideoEncodeH264SessionParametersGetInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    writeStdSPS = copy_src->writeStdSPS;
    writeStdPPS = copy_src->writeStdPPS;
    stdSPSId = copy_src->stdSPSId;
    stdPPSId = copy_src->stdPPSId;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR(
    const VkVideoEncodeH264SessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      hasStdSPSOverrides(in_struct->hasStdSPSOverrides),
      hasStdPPSOverrides(in_struct->hasStdPPSOverrides) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_KHR),
      pNext(nullptr),
      hasStdSPSOverrides(),
      hasStdPPSOverrides() {}

safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR(
    const safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR& copy_src) {
    sType = copy_src.sType;
    hasStdSPSOverrides = copy_src.hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src.hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR& safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::operator=(
    const safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hasStdSPSOverrides = copy_src.hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src.hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::~safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::initialize(
    const VkVideoEncodeH264SessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hasStdSPSOverrides = in_struct->hasStdSPSOverrides;
    hasStdPPSOverrides = in_struct->hasStdPPSOverrides;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR::initialize(
    const safe_VkVideoEncodeH264SessionParametersFeedbackInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hasStdSPSOverrides = copy_src->hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src->hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264NaluSliceInfoKHR::safe_VkVideoEncodeH264NaluSliceInfoKHR(const VkVideoEncodeH264NaluSliceInfoKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), constantQp(in_struct->constantQp), pStdSliceHeader(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSliceHeader) {
        pStdSliceHeader = new StdVideoEncodeH264SliceHeader(*in_struct->pStdSliceHeader);
    }
}

safe_VkVideoEncodeH264NaluSliceInfoKHR::safe_VkVideoEncodeH264NaluSliceInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_NALU_SLICE_INFO_KHR), pNext(nullptr), constantQp(), pStdSliceHeader(nullptr) {}

safe_VkVideoEncodeH264NaluSliceInfoKHR::safe_VkVideoEncodeH264NaluSliceInfoKHR(
    const safe_VkVideoEncodeH264NaluSliceInfoKHR& copy_src) {
    sType = copy_src.sType;
    constantQp = copy_src.constantQp;
    pStdSliceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSliceHeader) {
        pStdSliceHeader = new StdVideoEncodeH264SliceHeader(*copy_src.pStdSliceHeader);
    }
}

safe_VkVideoEncodeH264NaluSliceInfoKHR& safe_VkVideoEncodeH264NaluSliceInfoKHR::operator=(
    const safe_VkVideoEncodeH264NaluSliceInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSliceHeader) delete pStdSliceHeader;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    constantQp = copy_src.constantQp;
    pStdSliceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSliceHeader) {
        pStdSliceHeader = new StdVideoEncodeH264SliceHeader(*copy_src.pStdSliceHeader);
    }

    return *this;
}

safe_VkVideoEncodeH264NaluSliceInfoKHR::~safe_VkVideoEncodeH264NaluSliceInfoKHR() {
    if (pStdSliceHeader) delete pStdSliceHeader;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264NaluSliceInfoKHR::initialize(const VkVideoEncodeH264NaluSliceInfoKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSliceHeader) delete pStdSliceHeader;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    constantQp = in_struct->constantQp;
    pStdSliceHeader = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSliceHeader) {
        pStdSliceHeader = new StdVideoEncodeH264SliceHeader(*in_struct->pStdSliceHeader);
    }
}

void safe_VkVideoEncodeH264NaluSliceInfoKHR::initialize(const safe_VkVideoEncodeH264NaluSliceInfoKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    constantQp = copy_src->constantQp;
    pStdSliceHeader = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSliceHeader) {
        pStdSliceHeader = new StdVideoEncodeH264SliceHeader(*copy_src->pStdSliceHeader);
    }
}

safe_VkVideoEncodeH264PictureInfoKHR::safe_VkVideoEncodeH264PictureInfoKHR(const VkVideoEncodeH264PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      naluSliceEntryCount(in_struct->naluSliceEntryCount),
      pNaluSliceEntries(nullptr),
      pStdPictureInfo(nullptr),
      generatePrefixNalu(in_struct->generatePrefixNalu) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (naluSliceEntryCount && in_struct->pNaluSliceEntries) {
        pNaluSliceEntries = new safe_VkVideoEncodeH264NaluSliceInfoKHR[naluSliceEntryCount];
        for (uint32_t i = 0; i < naluSliceEntryCount; ++i) {
            pNaluSliceEntries[i].initialize(&in_struct->pNaluSliceEntries[i]);
        }
    }

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }
}

safe_VkVideoEncodeH264PictureInfoKHR::safe_VkVideoEncodeH264PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_KHR),
      pNext(nullptr),
      naluSliceEntryCount(),
      pNaluSliceEntries(nullptr),
      pStdPictureInfo(nullptr),
      generatePrefixNalu() {}

safe_VkVideoEncodeH264PictureInfoKHR::safe_VkVideoEncodeH264PictureInfoKHR(const safe_VkVideoEncodeH264PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    naluSliceEntryCount = copy_src.naluSliceEntryCount;
    pNaluSliceEntries = nullptr;
    pStdPictureInfo = nullptr;
    generatePrefixNalu = copy_src.generatePrefixNalu;
    pNext = SafePnextCopy(copy_src.pNext);
    if (naluSliceEntryCount && copy_src.pNaluSliceEntries) {
        pNaluSliceEntries = new safe_VkVideoEncodeH264NaluSliceInfoKHR[naluSliceEntryCount];
        for (uint32_t i = 0; i < naluSliceEntryCount; ++i) {
            pNaluSliceEntries[i].initialize(&copy_src.pNaluSliceEntries[i]);
        }
    }

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }
}

safe_VkVideoEncodeH264PictureInfoKHR& safe_VkVideoEncodeH264PictureInfoKHR::operator=(
    const safe_VkVideoEncodeH264PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pNaluSliceEntries) delete[] pNaluSliceEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    naluSliceEntryCount = copy_src.naluSliceEntryCount;
    pNaluSliceEntries = nullptr;
    pStdPictureInfo = nullptr;
    generatePrefixNalu = copy_src.generatePrefixNalu;
    pNext = SafePnextCopy(copy_src.pNext);
    if (naluSliceEntryCount && copy_src.pNaluSliceEntries) {
        pNaluSliceEntries = new safe_VkVideoEncodeH264NaluSliceInfoKHR[naluSliceEntryCount];
        for (uint32_t i = 0; i < naluSliceEntryCount; ++i) {
            pNaluSliceEntries[i].initialize(&copy_src.pNaluSliceEntries[i]);
        }
    }

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }

    return *this;
}

safe_VkVideoEncodeH264PictureInfoKHR::~safe_VkVideoEncodeH264PictureInfoKHR() {
    if (pNaluSliceEntries) delete[] pNaluSliceEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264PictureInfoKHR::initialize(const VkVideoEncodeH264PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pNaluSliceEntries) delete[] pNaluSliceEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    naluSliceEntryCount = in_struct->naluSliceEntryCount;
    pNaluSliceEntries = nullptr;
    pStdPictureInfo = nullptr;
    generatePrefixNalu = in_struct->generatePrefixNalu;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (naluSliceEntryCount && in_struct->pNaluSliceEntries) {
        pNaluSliceEntries = new safe_VkVideoEncodeH264NaluSliceInfoKHR[naluSliceEntryCount];
        for (uint32_t i = 0; i < naluSliceEntryCount; ++i) {
            pNaluSliceEntries[i].initialize(&in_struct->pNaluSliceEntries[i]);
        }
    }

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }
}

void safe_VkVideoEncodeH264PictureInfoKHR::initialize(const safe_VkVideoEncodeH264PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    naluSliceEntryCount = copy_src->naluSliceEntryCount;
    pNaluSliceEntries = nullptr;
    pStdPictureInfo = nullptr;
    generatePrefixNalu = copy_src->generatePrefixNalu;
    pNext = SafePnextCopy(copy_src->pNext);
    if (naluSliceEntryCount && copy_src->pNaluSliceEntries) {
        pNaluSliceEntries = new safe_VkVideoEncodeH264NaluSliceInfoKHR[naluSliceEntryCount];
        for (uint32_t i = 0; i < naluSliceEntryCount; ++i) {
            pNaluSliceEntries[i].initialize(&copy_src->pNaluSliceEntries[i]);
        }
    }

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH264PictureInfo(*copy_src->pStdPictureInfo);
    }
}

safe_VkVideoEncodeH264DpbSlotInfoKHR::safe_VkVideoEncodeH264DpbSlotInfoKHR(const VkVideoEncodeH264DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH264DpbSlotInfoKHR::safe_VkVideoEncodeH264DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoEncodeH264DpbSlotInfoKHR::safe_VkVideoEncodeH264DpbSlotInfoKHR(const safe_VkVideoEncodeH264DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH264DpbSlotInfoKHR& safe_VkVideoEncodeH264DpbSlotInfoKHR::operator=(
    const safe_VkVideoEncodeH264DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoEncodeH264DpbSlotInfoKHR::~safe_VkVideoEncodeH264DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264DpbSlotInfoKHR::initialize(const VkVideoEncodeH264DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoEncodeH264DpbSlotInfoKHR::initialize(const safe_VkVideoEncodeH264DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH264ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH264ProfileInfoKHR::safe_VkVideoEncodeH264ProfileInfoKHR(const VkVideoEncodeH264ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264ProfileInfoKHR::safe_VkVideoEncodeH264ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc() {}

safe_VkVideoEncodeH264ProfileInfoKHR::safe_VkVideoEncodeH264ProfileInfoKHR(const safe_VkVideoEncodeH264ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264ProfileInfoKHR& safe_VkVideoEncodeH264ProfileInfoKHR::operator=(
    const safe_VkVideoEncodeH264ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264ProfileInfoKHR::~safe_VkVideoEncodeH264ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264ProfileInfoKHR::initialize(const VkVideoEncodeH264ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264ProfileInfoKHR::initialize(const safe_VkVideoEncodeH264ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264RateControlInfoKHR::safe_VkVideoEncodeH264RateControlInfoKHR(
    const VkVideoEncodeH264RateControlInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      gopFrameCount(in_struct->gopFrameCount),
      idrPeriod(in_struct->idrPeriod),
      consecutiveBFrameCount(in_struct->consecutiveBFrameCount),
      temporalLayerCount(in_struct->temporalLayerCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264RateControlInfoKHR::safe_VkVideoEncodeH264RateControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_KHR),
      pNext(nullptr),
      flags(),
      gopFrameCount(),
      idrPeriod(),
      consecutiveBFrameCount(),
      temporalLayerCount() {}

safe_VkVideoEncodeH264RateControlInfoKHR::safe_VkVideoEncodeH264RateControlInfoKHR(
    const safe_VkVideoEncodeH264RateControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    idrPeriod = copy_src.idrPeriod;
    consecutiveBFrameCount = copy_src.consecutiveBFrameCount;
    temporalLayerCount = copy_src.temporalLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264RateControlInfoKHR& safe_VkVideoEncodeH264RateControlInfoKHR::operator=(
    const safe_VkVideoEncodeH264RateControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    idrPeriod = copy_src.idrPeriod;
    consecutiveBFrameCount = copy_src.consecutiveBFrameCount;
    temporalLayerCount = copy_src.temporalLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264RateControlInfoKHR::~safe_VkVideoEncodeH264RateControlInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264RateControlInfoKHR::initialize(const VkVideoEncodeH264RateControlInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    gopFrameCount = in_struct->gopFrameCount;
    idrPeriod = in_struct->idrPeriod;
    consecutiveBFrameCount = in_struct->consecutiveBFrameCount;
    temporalLayerCount = in_struct->temporalLayerCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264RateControlInfoKHR::initialize(const safe_VkVideoEncodeH264RateControlInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    gopFrameCount = copy_src->gopFrameCount;
    idrPeriod = copy_src->idrPeriod;
    consecutiveBFrameCount = copy_src->consecutiveBFrameCount;
    temporalLayerCount = copy_src->temporalLayerCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264RateControlLayerInfoKHR::safe_VkVideoEncodeH264RateControlLayerInfoKHR(
    const VkVideoEncodeH264RateControlLayerInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useMinQp(in_struct->useMinQp),
      minQp(in_struct->minQp),
      useMaxQp(in_struct->useMaxQp),
      maxQp(in_struct->maxQp),
      useMaxFrameSize(in_struct->useMaxFrameSize),
      maxFrameSize(in_struct->maxFrameSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264RateControlLayerInfoKHR::safe_VkVideoEncodeH264RateControlLayerInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_KHR),
      pNext(nullptr),
      useMinQp(),
      minQp(),
      useMaxQp(),
      maxQp(),
      useMaxFrameSize(),
      maxFrameSize() {}

safe_VkVideoEncodeH264RateControlLayerInfoKHR::safe_VkVideoEncodeH264RateControlLayerInfoKHR(
    const safe_VkVideoEncodeH264RateControlLayerInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMinQp = copy_src.useMinQp;
    minQp = copy_src.minQp;
    useMaxQp = copy_src.useMaxQp;
    maxQp = copy_src.maxQp;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264RateControlLayerInfoKHR& safe_VkVideoEncodeH264RateControlLayerInfoKHR::operator=(
    const safe_VkVideoEncodeH264RateControlLayerInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMinQp = copy_src.useMinQp;
    minQp = copy_src.minQp;
    useMaxQp = copy_src.useMaxQp;
    maxQp = copy_src.maxQp;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264RateControlLayerInfoKHR::~safe_VkVideoEncodeH264RateControlLayerInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264RateControlLayerInfoKHR::initialize(const VkVideoEncodeH264RateControlLayerInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMinQp = in_struct->useMinQp;
    minQp = in_struct->minQp;
    useMaxQp = in_struct->useMaxQp;
    maxQp = in_struct->maxQp;
    useMaxFrameSize = in_struct->useMaxFrameSize;
    maxFrameSize = in_struct->maxFrameSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264RateControlLayerInfoKHR::initialize(const safe_VkVideoEncodeH264RateControlLayerInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMinQp = copy_src->useMinQp;
    minQp = copy_src->minQp;
    useMaxQp = copy_src->useMaxQp;
    maxQp = copy_src->maxQp;
    useMaxFrameSize = copy_src->useMaxFrameSize;
    maxFrameSize = copy_src->maxFrameSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::safe_VkVideoEncodeH264GopRemainingFrameInfoKHR(
    const VkVideoEncodeH264GopRemainingFrameInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useGopRemainingFrames(in_struct->useGopRemainingFrames),
      gopRemainingI(in_struct->gopRemainingI),
      gopRemainingP(in_struct->gopRemainingP),
      gopRemainingB(in_struct->gopRemainingB) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::safe_VkVideoEncodeH264GopRemainingFrameInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_KHR),
      pNext(nullptr),
      useGopRemainingFrames(),
      gopRemainingI(),
      gopRemainingP(),
      gopRemainingB() {}

safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::safe_VkVideoEncodeH264GopRemainingFrameInfoKHR(
    const safe_VkVideoEncodeH264GopRemainingFrameInfoKHR& copy_src) {
    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingI = copy_src.gopRemainingI;
    gopRemainingP = copy_src.gopRemainingP;
    gopRemainingB = copy_src.gopRemainingB;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264GopRemainingFrameInfoKHR& safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::operator=(
    const safe_VkVideoEncodeH264GopRemainingFrameInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingI = copy_src.gopRemainingI;
    gopRemainingP = copy_src.gopRemainingP;
    gopRemainingB = copy_src.gopRemainingB;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::~safe_VkVideoEncodeH264GopRemainingFrameInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::initialize(const VkVideoEncodeH264GopRemainingFrameInfoKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useGopRemainingFrames = in_struct->useGopRemainingFrames;
    gopRemainingI = in_struct->gopRemainingI;
    gopRemainingP = in_struct->gopRemainingP;
    gopRemainingB = in_struct->gopRemainingB;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264GopRemainingFrameInfoKHR::initialize(const safe_VkVideoEncodeH264GopRemainingFrameInfoKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useGopRemainingFrames = copy_src->useGopRemainingFrames;
    gopRemainingI = copy_src->gopRemainingI;
    gopRemainingP = copy_src->gopRemainingP;
    gopRemainingB = copy_src->gopRemainingB;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265CapabilitiesKHR::safe_VkVideoEncodeH265CapabilitiesKHR(const VkVideoEncodeH265CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      maxLevelIdc(in_struct->maxLevelIdc),
      maxSliceSegmentCount(in_struct->maxSliceSegmentCount),
      maxTiles(in_struct->maxTiles),
      ctbSizes(in_struct->ctbSizes),
      transformBlockSizes(in_struct->transformBlockSizes),
      maxPPictureL0ReferenceCount(in_struct->maxPPictureL0ReferenceCount),
      maxBPictureL0ReferenceCount(in_struct->maxBPictureL0ReferenceCount),
      maxL1ReferenceCount(in_struct->maxL1ReferenceCount),
      maxSubLayerCount(in_struct->maxSubLayerCount),
      expectDyadicTemporalSubLayerPattern(in_struct->expectDyadicTemporalSubLayerPattern),
      minQp(in_struct->minQp),
      maxQp(in_struct->maxQp),
      prefersGopRemainingFrames(in_struct->prefersGopRemainingFrames),
      requiresGopRemainingFrames(in_struct->requiresGopRemainingFrames),
      stdSyntaxFlags(in_struct->stdSyntaxFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265CapabilitiesKHR::safe_VkVideoEncodeH265CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      maxLevelIdc(),
      maxSliceSegmentCount(),
      maxTiles(),
      ctbSizes(),
      transformBlockSizes(),
      maxPPictureL0ReferenceCount(),
      maxBPictureL0ReferenceCount(),
      maxL1ReferenceCount(),
      maxSubLayerCount(),
      expectDyadicTemporalSubLayerPattern(),
      minQp(),
      maxQp(),
      prefersGopRemainingFrames(),
      requiresGopRemainingFrames(),
      stdSyntaxFlags() {}

safe_VkVideoEncodeH265CapabilitiesKHR::safe_VkVideoEncodeH265CapabilitiesKHR(
    const safe_VkVideoEncodeH265CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevelIdc = copy_src.maxLevelIdc;
    maxSliceSegmentCount = copy_src.maxSliceSegmentCount;
    maxTiles = copy_src.maxTiles;
    ctbSizes = copy_src.ctbSizes;
    transformBlockSizes = copy_src.transformBlockSizes;
    maxPPictureL0ReferenceCount = copy_src.maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src.maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src.maxL1ReferenceCount;
    maxSubLayerCount = copy_src.maxSubLayerCount;
    expectDyadicTemporalSubLayerPattern = copy_src.expectDyadicTemporalSubLayerPattern;
    minQp = copy_src.minQp;
    maxQp = copy_src.maxQp;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265CapabilitiesKHR& safe_VkVideoEncodeH265CapabilitiesKHR::operator=(
    const safe_VkVideoEncodeH265CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevelIdc = copy_src.maxLevelIdc;
    maxSliceSegmentCount = copy_src.maxSliceSegmentCount;
    maxTiles = copy_src.maxTiles;
    ctbSizes = copy_src.ctbSizes;
    transformBlockSizes = copy_src.transformBlockSizes;
    maxPPictureL0ReferenceCount = copy_src.maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src.maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src.maxL1ReferenceCount;
    maxSubLayerCount = copy_src.maxSubLayerCount;
    expectDyadicTemporalSubLayerPattern = copy_src.expectDyadicTemporalSubLayerPattern;
    minQp = copy_src.minQp;
    maxQp = copy_src.maxQp;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265CapabilitiesKHR::~safe_VkVideoEncodeH265CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265CapabilitiesKHR::initialize(const VkVideoEncodeH265CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    maxLevelIdc = in_struct->maxLevelIdc;
    maxSliceSegmentCount = in_struct->maxSliceSegmentCount;
    maxTiles = in_struct->maxTiles;
    ctbSizes = in_struct->ctbSizes;
    transformBlockSizes = in_struct->transformBlockSizes;
    maxPPictureL0ReferenceCount = in_struct->maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = in_struct->maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = in_struct->maxL1ReferenceCount;
    maxSubLayerCount = in_struct->maxSubLayerCount;
    expectDyadicTemporalSubLayerPattern = in_struct->expectDyadicTemporalSubLayerPattern;
    minQp = in_struct->minQp;
    maxQp = in_struct->maxQp;
    prefersGopRemainingFrames = in_struct->prefersGopRemainingFrames;
    requiresGopRemainingFrames = in_struct->requiresGopRemainingFrames;
    stdSyntaxFlags = in_struct->stdSyntaxFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265CapabilitiesKHR::initialize(const safe_VkVideoEncodeH265CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    maxLevelIdc = copy_src->maxLevelIdc;
    maxSliceSegmentCount = copy_src->maxSliceSegmentCount;
    maxTiles = copy_src->maxTiles;
    ctbSizes = copy_src->ctbSizes;
    transformBlockSizes = copy_src->transformBlockSizes;
    maxPPictureL0ReferenceCount = copy_src->maxPPictureL0ReferenceCount;
    maxBPictureL0ReferenceCount = copy_src->maxBPictureL0ReferenceCount;
    maxL1ReferenceCount = copy_src->maxL1ReferenceCount;
    maxSubLayerCount = copy_src->maxSubLayerCount;
    expectDyadicTemporalSubLayerPattern = copy_src->expectDyadicTemporalSubLayerPattern;
    minQp = copy_src->minQp;
    maxQp = copy_src->maxQp;
    prefersGopRemainingFrames = copy_src->prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src->requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src->stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265SessionCreateInfoKHR::safe_VkVideoEncodeH265SessionCreateInfoKHR(
    const VkVideoEncodeH265SessionCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), useMaxLevelIdc(in_struct->useMaxLevelIdc), maxLevelIdc(in_struct->maxLevelIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265SessionCreateInfoKHR::safe_VkVideoEncodeH265SessionCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_KHR), pNext(nullptr), useMaxLevelIdc(), maxLevelIdc() {}

safe_VkVideoEncodeH265SessionCreateInfoKHR::safe_VkVideoEncodeH265SessionCreateInfoKHR(
    const safe_VkVideoEncodeH265SessionCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMaxLevelIdc = copy_src.useMaxLevelIdc;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265SessionCreateInfoKHR& safe_VkVideoEncodeH265SessionCreateInfoKHR::operator=(
    const safe_VkVideoEncodeH265SessionCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMaxLevelIdc = copy_src.useMaxLevelIdc;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265SessionCreateInfoKHR::~safe_VkVideoEncodeH265SessionCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265SessionCreateInfoKHR::initialize(const VkVideoEncodeH265SessionCreateInfoKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMaxLevelIdc = in_struct->useMaxLevelIdc;
    maxLevelIdc = in_struct->maxLevelIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265SessionCreateInfoKHR::initialize(const safe_VkVideoEncodeH265SessionCreateInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMaxLevelIdc = copy_src->useMaxLevelIdc;
    maxLevelIdc = copy_src->maxLevelIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265QualityLevelPropertiesKHR::safe_VkVideoEncodeH265QualityLevelPropertiesKHR(
    const VkVideoEncodeH265QualityLevelPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      preferredRateControlFlags(in_struct->preferredRateControlFlags),
      preferredGopFrameCount(in_struct->preferredGopFrameCount),
      preferredIdrPeriod(in_struct->preferredIdrPeriod),
      preferredConsecutiveBFrameCount(in_struct->preferredConsecutiveBFrameCount),
      preferredSubLayerCount(in_struct->preferredSubLayerCount),
      preferredConstantQp(in_struct->preferredConstantQp),
      preferredMaxL0ReferenceCount(in_struct->preferredMaxL0ReferenceCount),
      preferredMaxL1ReferenceCount(in_struct->preferredMaxL1ReferenceCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265QualityLevelPropertiesKHR::safe_VkVideoEncodeH265QualityLevelPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_KHR),
      pNext(nullptr),
      preferredRateControlFlags(),
      preferredGopFrameCount(),
      preferredIdrPeriod(),
      preferredConsecutiveBFrameCount(),
      preferredSubLayerCount(),
      preferredConstantQp(),
      preferredMaxL0ReferenceCount(),
      preferredMaxL1ReferenceCount() {}

safe_VkVideoEncodeH265QualityLevelPropertiesKHR::safe_VkVideoEncodeH265QualityLevelPropertiesKHR(
    const safe_VkVideoEncodeH265QualityLevelPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredIdrPeriod = copy_src.preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src.preferredConsecutiveBFrameCount;
    preferredSubLayerCount = copy_src.preferredSubLayerCount;
    preferredConstantQp = copy_src.preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src.preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src.preferredMaxL1ReferenceCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265QualityLevelPropertiesKHR& safe_VkVideoEncodeH265QualityLevelPropertiesKHR::operator=(
    const safe_VkVideoEncodeH265QualityLevelPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredIdrPeriod = copy_src.preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src.preferredConsecutiveBFrameCount;
    preferredSubLayerCount = copy_src.preferredSubLayerCount;
    preferredConstantQp = copy_src.preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src.preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src.preferredMaxL1ReferenceCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265QualityLevelPropertiesKHR::~safe_VkVideoEncodeH265QualityLevelPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265QualityLevelPropertiesKHR::initialize(const VkVideoEncodeH265QualityLevelPropertiesKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    preferredRateControlFlags = in_struct->preferredRateControlFlags;
    preferredGopFrameCount = in_struct->preferredGopFrameCount;
    preferredIdrPeriod = in_struct->preferredIdrPeriod;
    preferredConsecutiveBFrameCount = in_struct->preferredConsecutiveBFrameCount;
    preferredSubLayerCount = in_struct->preferredSubLayerCount;
    preferredConstantQp = in_struct->preferredConstantQp;
    preferredMaxL0ReferenceCount = in_struct->preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = in_struct->preferredMaxL1ReferenceCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265QualityLevelPropertiesKHR::initialize(const safe_VkVideoEncodeH265QualityLevelPropertiesKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    preferredRateControlFlags = copy_src->preferredRateControlFlags;
    preferredGopFrameCount = copy_src->preferredGopFrameCount;
    preferredIdrPeriod = copy_src->preferredIdrPeriod;
    preferredConsecutiveBFrameCount = copy_src->preferredConsecutiveBFrameCount;
    preferredSubLayerCount = copy_src->preferredSubLayerCount;
    preferredConstantQp = copy_src->preferredConstantQp;
    preferredMaxL0ReferenceCount = copy_src->preferredMaxL0ReferenceCount;
    preferredMaxL1ReferenceCount = copy_src->preferredMaxL1ReferenceCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265SessionParametersAddInfoKHR::safe_VkVideoEncodeH265SessionParametersAddInfoKHR(
    const VkVideoEncodeH265SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdVPSCount(in_struct->stdVPSCount),
      pStdVPSs(nullptr),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoEncodeH265SessionParametersAddInfoKHR::safe_VkVideoEncodeH265SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdVPSCount(),
      pStdVPSs(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoEncodeH265SessionParametersAddInfoKHR::safe_VkVideoEncodeH265SessionParametersAddInfoKHR(
    const safe_VkVideoEncodeH265SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoEncodeH265SessionParametersAddInfoKHR& safe_VkVideoEncodeH265SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoEncodeH265SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoEncodeH265SessionParametersAddInfoKHR::~safe_VkVideoEncodeH265SessionParametersAddInfoKHR() {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265SessionParametersAddInfoKHR::initialize(const VkVideoEncodeH265SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdVPSCount = in_struct->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoEncodeH265SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoEncodeH265SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdVPSCount = copy_src->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src->stdVPSCount);
    }

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::safe_VkVideoEncodeH265SessionParametersCreateInfoKHR(
    const VkVideoEncodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdVPSCount(in_struct->maxStdVPSCount),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::safe_VkVideoEncodeH265SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdVPSCount(),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::safe_VkVideoEncodeH265SessionParametersCreateInfoKHR(
    const safe_VkVideoEncodeH265SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoEncodeH265SessionParametersCreateInfoKHR& safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoEncodeH265SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::~safe_VkVideoEncodeH265SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::initialize(
    const VkVideoEncodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdVPSCount = in_struct->maxStdVPSCount;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoEncodeH265SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoEncodeH265SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdVPSCount = copy_src->maxStdVPSCount;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoEncodeH265SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoEncodeH265SessionParametersGetInfoKHR::safe_VkVideoEncodeH265SessionParametersGetInfoKHR(
    const VkVideoEncodeH265SessionParametersGetInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      writeStdVPS(in_struct->writeStdVPS),
      writeStdSPS(in_struct->writeStdSPS),
      writeStdPPS(in_struct->writeStdPPS),
      stdVPSId(in_struct->stdVPSId),
      stdSPSId(in_struct->stdSPSId),
      stdPPSId(in_struct->stdPPSId) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265SessionParametersGetInfoKHR::safe_VkVideoEncodeH265SessionParametersGetInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_KHR),
      pNext(nullptr),
      writeStdVPS(),
      writeStdSPS(),
      writeStdPPS(),
      stdVPSId(),
      stdSPSId(),
      stdPPSId() {}

safe_VkVideoEncodeH265SessionParametersGetInfoKHR::safe_VkVideoEncodeH265SessionParametersGetInfoKHR(
    const safe_VkVideoEncodeH265SessionParametersGetInfoKHR& copy_src) {
    sType = copy_src.sType;
    writeStdVPS = copy_src.writeStdVPS;
    writeStdSPS = copy_src.writeStdSPS;
    writeStdPPS = copy_src.writeStdPPS;
    stdVPSId = copy_src.stdVPSId;
    stdSPSId = copy_src.stdSPSId;
    stdPPSId = copy_src.stdPPSId;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265SessionParametersGetInfoKHR& safe_VkVideoEncodeH265SessionParametersGetInfoKHR::operator=(
    const safe_VkVideoEncodeH265SessionParametersGetInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    writeStdVPS = copy_src.writeStdVPS;
    writeStdSPS = copy_src.writeStdSPS;
    writeStdPPS = copy_src.writeStdPPS;
    stdVPSId = copy_src.stdVPSId;
    stdSPSId = copy_src.stdSPSId;
    stdPPSId = copy_src.stdPPSId;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265SessionParametersGetInfoKHR::~safe_VkVideoEncodeH265SessionParametersGetInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265SessionParametersGetInfoKHR::initialize(const VkVideoEncodeH265SessionParametersGetInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    writeStdVPS = in_struct->writeStdVPS;
    writeStdSPS = in_struct->writeStdSPS;
    writeStdPPS = in_struct->writeStdPPS;
    stdVPSId = in_struct->stdVPSId;
    stdSPSId = in_struct->stdSPSId;
    stdPPSId = in_struct->stdPPSId;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265SessionParametersGetInfoKHR::initialize(
    const safe_VkVideoEncodeH265SessionParametersGetInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    writeStdVPS = copy_src->writeStdVPS;
    writeStdSPS = copy_src->writeStdSPS;
    writeStdPPS = copy_src->writeStdPPS;
    stdVPSId = copy_src->stdVPSId;
    stdSPSId = copy_src->stdSPSId;
    stdPPSId = copy_src->stdPPSId;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR(
    const VkVideoEncodeH265SessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      hasStdVPSOverrides(in_struct->hasStdVPSOverrides),
      hasStdSPSOverrides(in_struct->hasStdSPSOverrides),
      hasStdPPSOverrides(in_struct->hasStdPPSOverrides) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_KHR),
      pNext(nullptr),
      hasStdVPSOverrides(),
      hasStdSPSOverrides(),
      hasStdPPSOverrides() {}

safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR(
    const safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR& copy_src) {
    sType = copy_src.sType;
    hasStdVPSOverrides = copy_src.hasStdVPSOverrides;
    hasStdSPSOverrides = copy_src.hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src.hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR& safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::operator=(
    const safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hasStdVPSOverrides = copy_src.hasStdVPSOverrides;
    hasStdSPSOverrides = copy_src.hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src.hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::~safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::initialize(
    const VkVideoEncodeH265SessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hasStdVPSOverrides = in_struct->hasStdVPSOverrides;
    hasStdSPSOverrides = in_struct->hasStdSPSOverrides;
    hasStdPPSOverrides = in_struct->hasStdPPSOverrides;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR::initialize(
    const safe_VkVideoEncodeH265SessionParametersFeedbackInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hasStdVPSOverrides = copy_src->hasStdVPSOverrides;
    hasStdSPSOverrides = copy_src->hasStdSPSOverrides;
    hasStdPPSOverrides = copy_src->hasStdPPSOverrides;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR(
    const VkVideoEncodeH265NaluSliceSegmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), constantQp(in_struct->constantQp), pStdSliceSegmentHeader(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSliceSegmentHeader) {
        pStdSliceSegmentHeader = new StdVideoEncodeH265SliceSegmentHeader(*in_struct->pStdSliceSegmentHeader);
    }
}

safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_NALU_SLICE_SEGMENT_INFO_KHR),
      pNext(nullptr),
      constantQp(),
      pStdSliceSegmentHeader(nullptr) {}

safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR(
    const safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR& copy_src) {
    sType = copy_src.sType;
    constantQp = copy_src.constantQp;
    pStdSliceSegmentHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSliceSegmentHeader) {
        pStdSliceSegmentHeader = new StdVideoEncodeH265SliceSegmentHeader(*copy_src.pStdSliceSegmentHeader);
    }
}

safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR& safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::operator=(
    const safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSliceSegmentHeader) delete pStdSliceSegmentHeader;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    constantQp = copy_src.constantQp;
    pStdSliceSegmentHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSliceSegmentHeader) {
        pStdSliceSegmentHeader = new StdVideoEncodeH265SliceSegmentHeader(*copy_src.pStdSliceSegmentHeader);
    }

    return *this;
}

safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::~safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR() {
    if (pStdSliceSegmentHeader) delete pStdSliceSegmentHeader;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::initialize(const VkVideoEncodeH265NaluSliceSegmentInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSliceSegmentHeader) delete pStdSliceSegmentHeader;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    constantQp = in_struct->constantQp;
    pStdSliceSegmentHeader = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSliceSegmentHeader) {
        pStdSliceSegmentHeader = new StdVideoEncodeH265SliceSegmentHeader(*in_struct->pStdSliceSegmentHeader);
    }
}

void safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR::initialize(const safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    constantQp = copy_src->constantQp;
    pStdSliceSegmentHeader = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSliceSegmentHeader) {
        pStdSliceSegmentHeader = new StdVideoEncodeH265SliceSegmentHeader(*copy_src->pStdSliceSegmentHeader);
    }
}

safe_VkVideoEncodeH265PictureInfoKHR::safe_VkVideoEncodeH265PictureInfoKHR(const VkVideoEncodeH265PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      naluSliceSegmentEntryCount(in_struct->naluSliceSegmentEntryCount),
      pNaluSliceSegmentEntries(nullptr),
      pStdPictureInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (naluSliceSegmentEntryCount && in_struct->pNaluSliceSegmentEntries) {
        pNaluSliceSegmentEntries = new safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR[naluSliceSegmentEntryCount];
        for (uint32_t i = 0; i < naluSliceSegmentEntryCount; ++i) {
            pNaluSliceSegmentEntries[i].initialize(&in_struct->pNaluSliceSegmentEntries[i]);
        }
    }

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }
}

safe_VkVideoEncodeH265PictureInfoKHR::safe_VkVideoEncodeH265PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_KHR),
      pNext(nullptr),
      naluSliceSegmentEntryCount(),
      pNaluSliceSegmentEntries(nullptr),
      pStdPictureInfo(nullptr) {}

safe_VkVideoEncodeH265PictureInfoKHR::safe_VkVideoEncodeH265PictureInfoKHR(const safe_VkVideoEncodeH265PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    naluSliceSegmentEntryCount = copy_src.naluSliceSegmentEntryCount;
    pNaluSliceSegmentEntries = nullptr;
    pStdPictureInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (naluSliceSegmentEntryCount && copy_src.pNaluSliceSegmentEntries) {
        pNaluSliceSegmentEntries = new safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR[naluSliceSegmentEntryCount];
        for (uint32_t i = 0; i < naluSliceSegmentEntryCount; ++i) {
            pNaluSliceSegmentEntries[i].initialize(&copy_src.pNaluSliceSegmentEntries[i]);
        }
    }

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }
}

safe_VkVideoEncodeH265PictureInfoKHR& safe_VkVideoEncodeH265PictureInfoKHR::operator=(
    const safe_VkVideoEncodeH265PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pNaluSliceSegmentEntries) delete[] pNaluSliceSegmentEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    naluSliceSegmentEntryCount = copy_src.naluSliceSegmentEntryCount;
    pNaluSliceSegmentEntries = nullptr;
    pStdPictureInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (naluSliceSegmentEntryCount && copy_src.pNaluSliceSegmentEntries) {
        pNaluSliceSegmentEntries = new safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR[naluSliceSegmentEntryCount];
        for (uint32_t i = 0; i < naluSliceSegmentEntryCount; ++i) {
            pNaluSliceSegmentEntries[i].initialize(&copy_src.pNaluSliceSegmentEntries[i]);
        }
    }

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }

    return *this;
}

safe_VkVideoEncodeH265PictureInfoKHR::~safe_VkVideoEncodeH265PictureInfoKHR() {
    if (pNaluSliceSegmentEntries) delete[] pNaluSliceSegmentEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265PictureInfoKHR::initialize(const VkVideoEncodeH265PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pNaluSliceSegmentEntries) delete[] pNaluSliceSegmentEntries;
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    naluSliceSegmentEntryCount = in_struct->naluSliceSegmentEntryCount;
    pNaluSliceSegmentEntries = nullptr;
    pStdPictureInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (naluSliceSegmentEntryCount && in_struct->pNaluSliceSegmentEntries) {
        pNaluSliceSegmentEntries = new safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR[naluSliceSegmentEntryCount];
        for (uint32_t i = 0; i < naluSliceSegmentEntryCount; ++i) {
            pNaluSliceSegmentEntries[i].initialize(&in_struct->pNaluSliceSegmentEntries[i]);
        }
    }

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }
}

void safe_VkVideoEncodeH265PictureInfoKHR::initialize(const safe_VkVideoEncodeH265PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    naluSliceSegmentEntryCount = copy_src->naluSliceSegmentEntryCount;
    pNaluSliceSegmentEntries = nullptr;
    pStdPictureInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (naluSliceSegmentEntryCount && copy_src->pNaluSliceSegmentEntries) {
        pNaluSliceSegmentEntries = new safe_VkVideoEncodeH265NaluSliceSegmentInfoKHR[naluSliceSegmentEntryCount];
        for (uint32_t i = 0; i < naluSliceSegmentEntryCount; ++i) {
            pNaluSliceSegmentEntries[i].initialize(&copy_src->pNaluSliceSegmentEntries[i]);
        }
    }

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeH265PictureInfo(*copy_src->pStdPictureInfo);
    }
}

safe_VkVideoEncodeH265DpbSlotInfoKHR::safe_VkVideoEncodeH265DpbSlotInfoKHR(const VkVideoEncodeH265DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH265DpbSlotInfoKHR::safe_VkVideoEncodeH265DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoEncodeH265DpbSlotInfoKHR::safe_VkVideoEncodeH265DpbSlotInfoKHR(const safe_VkVideoEncodeH265DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH265DpbSlotInfoKHR& safe_VkVideoEncodeH265DpbSlotInfoKHR::operator=(
    const safe_VkVideoEncodeH265DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoEncodeH265DpbSlotInfoKHR::~safe_VkVideoEncodeH265DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265DpbSlotInfoKHR::initialize(const VkVideoEncodeH265DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoEncodeH265DpbSlotInfoKHR::initialize(const safe_VkVideoEncodeH265DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeH265ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeH265ProfileInfoKHR::safe_VkVideoEncodeH265ProfileInfoKHR(const VkVideoEncodeH265ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265ProfileInfoKHR::safe_VkVideoEncodeH265ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc() {}

safe_VkVideoEncodeH265ProfileInfoKHR::safe_VkVideoEncodeH265ProfileInfoKHR(const safe_VkVideoEncodeH265ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265ProfileInfoKHR& safe_VkVideoEncodeH265ProfileInfoKHR::operator=(
    const safe_VkVideoEncodeH265ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265ProfileInfoKHR::~safe_VkVideoEncodeH265ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265ProfileInfoKHR::initialize(const VkVideoEncodeH265ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265ProfileInfoKHR::initialize(const safe_VkVideoEncodeH265ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265RateControlInfoKHR::safe_VkVideoEncodeH265RateControlInfoKHR(
    const VkVideoEncodeH265RateControlInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      gopFrameCount(in_struct->gopFrameCount),
      idrPeriod(in_struct->idrPeriod),
      consecutiveBFrameCount(in_struct->consecutiveBFrameCount),
      subLayerCount(in_struct->subLayerCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265RateControlInfoKHR::safe_VkVideoEncodeH265RateControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_KHR),
      pNext(nullptr),
      flags(),
      gopFrameCount(),
      idrPeriod(),
      consecutiveBFrameCount(),
      subLayerCount() {}

safe_VkVideoEncodeH265RateControlInfoKHR::safe_VkVideoEncodeH265RateControlInfoKHR(
    const safe_VkVideoEncodeH265RateControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    idrPeriod = copy_src.idrPeriod;
    consecutiveBFrameCount = copy_src.consecutiveBFrameCount;
    subLayerCount = copy_src.subLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265RateControlInfoKHR& safe_VkVideoEncodeH265RateControlInfoKHR::operator=(
    const safe_VkVideoEncodeH265RateControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    idrPeriod = copy_src.idrPeriod;
    consecutiveBFrameCount = copy_src.consecutiveBFrameCount;
    subLayerCount = copy_src.subLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265RateControlInfoKHR::~safe_VkVideoEncodeH265RateControlInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265RateControlInfoKHR::initialize(const VkVideoEncodeH265RateControlInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    gopFrameCount = in_struct->gopFrameCount;
    idrPeriod = in_struct->idrPeriod;
    consecutiveBFrameCount = in_struct->consecutiveBFrameCount;
    subLayerCount = in_struct->subLayerCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265RateControlInfoKHR::initialize(const safe_VkVideoEncodeH265RateControlInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    gopFrameCount = copy_src->gopFrameCount;
    idrPeriod = copy_src->idrPeriod;
    consecutiveBFrameCount = copy_src->consecutiveBFrameCount;
    subLayerCount = copy_src->subLayerCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265RateControlLayerInfoKHR::safe_VkVideoEncodeH265RateControlLayerInfoKHR(
    const VkVideoEncodeH265RateControlLayerInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useMinQp(in_struct->useMinQp),
      minQp(in_struct->minQp),
      useMaxQp(in_struct->useMaxQp),
      maxQp(in_struct->maxQp),
      useMaxFrameSize(in_struct->useMaxFrameSize),
      maxFrameSize(in_struct->maxFrameSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265RateControlLayerInfoKHR::safe_VkVideoEncodeH265RateControlLayerInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_KHR),
      pNext(nullptr),
      useMinQp(),
      minQp(),
      useMaxQp(),
      maxQp(),
      useMaxFrameSize(),
      maxFrameSize() {}

safe_VkVideoEncodeH265RateControlLayerInfoKHR::safe_VkVideoEncodeH265RateControlLayerInfoKHR(
    const safe_VkVideoEncodeH265RateControlLayerInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMinQp = copy_src.useMinQp;
    minQp = copy_src.minQp;
    useMaxQp = copy_src.useMaxQp;
    maxQp = copy_src.maxQp;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265RateControlLayerInfoKHR& safe_VkVideoEncodeH265RateControlLayerInfoKHR::operator=(
    const safe_VkVideoEncodeH265RateControlLayerInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMinQp = copy_src.useMinQp;
    minQp = copy_src.minQp;
    useMaxQp = copy_src.useMaxQp;
    maxQp = copy_src.maxQp;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265RateControlLayerInfoKHR::~safe_VkVideoEncodeH265RateControlLayerInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265RateControlLayerInfoKHR::initialize(const VkVideoEncodeH265RateControlLayerInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMinQp = in_struct->useMinQp;
    minQp = in_struct->minQp;
    useMaxQp = in_struct->useMaxQp;
    maxQp = in_struct->maxQp;
    useMaxFrameSize = in_struct->useMaxFrameSize;
    maxFrameSize = in_struct->maxFrameSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265RateControlLayerInfoKHR::initialize(const safe_VkVideoEncodeH265RateControlLayerInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMinQp = copy_src->useMinQp;
    minQp = copy_src->minQp;
    useMaxQp = copy_src->useMaxQp;
    maxQp = copy_src->maxQp;
    useMaxFrameSize = copy_src->useMaxFrameSize;
    maxFrameSize = copy_src->maxFrameSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::safe_VkVideoEncodeH265GopRemainingFrameInfoKHR(
    const VkVideoEncodeH265GopRemainingFrameInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useGopRemainingFrames(in_struct->useGopRemainingFrames),
      gopRemainingI(in_struct->gopRemainingI),
      gopRemainingP(in_struct->gopRemainingP),
      gopRemainingB(in_struct->gopRemainingB) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::safe_VkVideoEncodeH265GopRemainingFrameInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_KHR),
      pNext(nullptr),
      useGopRemainingFrames(),
      gopRemainingI(),
      gopRemainingP(),
      gopRemainingB() {}

safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::safe_VkVideoEncodeH265GopRemainingFrameInfoKHR(
    const safe_VkVideoEncodeH265GopRemainingFrameInfoKHR& copy_src) {
    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingI = copy_src.gopRemainingI;
    gopRemainingP = copy_src.gopRemainingP;
    gopRemainingB = copy_src.gopRemainingB;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265GopRemainingFrameInfoKHR& safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::operator=(
    const safe_VkVideoEncodeH265GopRemainingFrameInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingI = copy_src.gopRemainingI;
    gopRemainingP = copy_src.gopRemainingP;
    gopRemainingB = copy_src.gopRemainingB;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::~safe_VkVideoEncodeH265GopRemainingFrameInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::initialize(const VkVideoEncodeH265GopRemainingFrameInfoKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useGopRemainingFrames = in_struct->useGopRemainingFrames;
    gopRemainingI = in_struct->gopRemainingI;
    gopRemainingP = in_struct->gopRemainingP;
    gopRemainingB = in_struct->gopRemainingB;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265GopRemainingFrameInfoKHR::initialize(const safe_VkVideoEncodeH265GopRemainingFrameInfoKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useGopRemainingFrames = copy_src->useGopRemainingFrames;
    gopRemainingI = copy_src->gopRemainingI;
    gopRemainingP = copy_src->gopRemainingP;
    gopRemainingB = copy_src->gopRemainingB;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR(const VkVideoDecodeH264ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc), pictureLayout(in_struct->pictureLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc(), pictureLayout() {}

safe_VkVideoDecodeH264ProfileInfoKHR::safe_VkVideoDecodeH264ProfileInfoKHR(const safe_VkVideoDecodeH264ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pictureLayout = copy_src.pictureLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH264ProfileInfoKHR& safe_VkVideoDecodeH264ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeH264ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pictureLayout = copy_src.pictureLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH264ProfileInfoKHR::~safe_VkVideoDecodeH264ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH264ProfileInfoKHR::initialize(const VkVideoDecodeH264ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pictureLayout = in_struct->pictureLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH264ProfileInfoKHR::initialize(const safe_VkVideoDecodeH264ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pictureLayout = copy_src->pictureLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR(const VkVideoDecodeH264CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), maxLevelIdc(in_struct->maxLevelIdc), fieldOffsetGranularity(in_struct->fieldOffsetGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR), pNext(nullptr), maxLevelIdc(), fieldOffsetGranularity() {}

safe_VkVideoDecodeH264CapabilitiesKHR::safe_VkVideoDecodeH264CapabilitiesKHR(
    const safe_VkVideoDecodeH264CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    fieldOffsetGranularity = copy_src.fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH264CapabilitiesKHR& safe_VkVideoDecodeH264CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeH264CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    fieldOffsetGranularity = copy_src.fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH264CapabilitiesKHR::~safe_VkVideoDecodeH264CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH264CapabilitiesKHR::initialize(const VkVideoDecodeH264CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevelIdc = in_struct->maxLevelIdc;
    fieldOffsetGranularity = in_struct->fieldOffsetGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH264CapabilitiesKHR::initialize(const safe_VkVideoDecodeH264CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevelIdc = copy_src->maxLevelIdc;
    fieldOffsetGranularity = copy_src->fieldOffsetGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR(
    const VkVideoDecodeH264SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::safe_VkVideoDecodeH264SessionParametersAddInfoKHR(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR& safe_VkVideoDecodeH264SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoDecodeH264SessionParametersAddInfoKHR::~safe_VkVideoDecodeH264SessionParametersAddInfoKHR() {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264SessionParametersAddInfoKHR::initialize(const VkVideoDecodeH264SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoDecodeH264SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoDecodeH264SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH264SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH264SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH264PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH264PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(
    const VkVideoDecodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::~safe_VkVideoDecodeH264SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::initialize(
    const VkVideoDecodeH264SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoDecodeH264SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR(const VkVideoDecodeH264PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdPictureInfo(nullptr), sliceCount(in_struct->sliceCount), pSliceOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceOffsets) {
        pSliceOffsets = new uint32_t[in_struct->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)in_struct->pSliceOffsets, sizeof(uint32_t) * in_struct->sliceCount);
    }
}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      sliceCount(),
      pSliceOffsets(nullptr) {}

safe_VkVideoDecodeH264PictureInfoKHR::safe_VkVideoDecodeH264PictureInfoKHR(const safe_VkVideoDecodeH264PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src.sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src.sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src.pSliceOffsets, sizeof(uint32_t) * copy_src.sliceCount);
    }
}

safe_VkVideoDecodeH264PictureInfoKHR& safe_VkVideoDecodeH264PictureInfoKHR::operator=(
    const safe_VkVideoDecodeH264PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src.sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src.sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src.pSliceOffsets, sizeof(uint32_t) * copy_src.sliceCount);
    }

    return *this;
}

safe_VkVideoDecodeH264PictureInfoKHR::~safe_VkVideoDecodeH264PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264PictureInfoKHR::initialize(const VkVideoDecodeH264PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceOffsets) delete[] pSliceOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    sliceCount = in_struct->sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceOffsets) {
        pSliceOffsets = new uint32_t[in_struct->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)in_struct->pSliceOffsets, sizeof(uint32_t) * in_struct->sliceCount);
    }
}

void safe_VkVideoDecodeH264PictureInfoKHR::initialize(const safe_VkVideoDecodeH264PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    sliceCount = copy_src->sliceCount;
    pSliceOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH264PictureInfo(*copy_src->pStdPictureInfo);
    }

    if (copy_src->pSliceOffsets) {
        pSliceOffsets = new uint32_t[copy_src->sliceCount];
        memcpy((void*)pSliceOffsets, (void*)copy_src->pSliceOffsets, sizeof(uint32_t) * copy_src->sliceCount);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR(const VkVideoDecodeH264DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoDecodeH264DpbSlotInfoKHR::safe_VkVideoDecodeH264DpbSlotInfoKHR(const safe_VkVideoDecodeH264DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH264DpbSlotInfoKHR& safe_VkVideoDecodeH264DpbSlotInfoKHR::operator=(
    const safe_VkVideoDecodeH264DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoDecodeH264DpbSlotInfoKHR::~safe_VkVideoDecodeH264DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264DpbSlotInfoKHR::initialize(const VkVideoDecodeH264DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoDecodeH264DpbSlotInfoKHR::initialize(const safe_VkVideoDecodeH264DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH264ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(const VkImportMemoryWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), handle(in_struct->handle), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR), pNext(nullptr), handleType(), handle(), name() {}

safe_VkImportMemoryWin32HandleInfoKHR::safe_VkImportMemoryWin32HandleInfoKHR(
    const safe_VkImportMemoryWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryWin32HandleInfoKHR& safe_VkImportMemoryWin32HandleInfoKHR::operator=(
    const safe_VkImportMemoryWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryWin32HandleInfoKHR::~safe_VkImportMemoryWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const VkImportMemoryWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryWin32HandleInfoKHR::initialize(const safe_VkImportMemoryWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(const VkExportMemoryWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportMemoryWin32HandleInfoKHR::safe_VkExportMemoryWin32HandleInfoKHR(
    const safe_VkExportMemoryWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoKHR& safe_VkExportMemoryWin32HandleInfoKHR::operator=(
    const safe_VkExportMemoryWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportMemoryWin32HandleInfoKHR::~safe_VkExportMemoryWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const VkExportMemoryWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportMemoryWin32HandleInfoKHR::initialize(const safe_VkExportMemoryWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(const VkMemoryWin32HandlePropertiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryWin32HandlePropertiesKHR::safe_VkMemoryWin32HandlePropertiesKHR(
    const safe_VkMemoryWin32HandlePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryWin32HandlePropertiesKHR& safe_VkMemoryWin32HandlePropertiesKHR::operator=(
    const safe_VkMemoryWin32HandlePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryWin32HandlePropertiesKHR::~safe_VkMemoryWin32HandlePropertiesKHR() { FreePnextChain(pNext); }

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const VkMemoryWin32HandlePropertiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryWin32HandlePropertiesKHR::initialize(const safe_VkMemoryWin32HandlePropertiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const VkMemoryGetWin32HandleInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetWin32HandleInfoKHR::safe_VkMemoryGetWin32HandleInfoKHR(const safe_VkMemoryGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetWin32HandleInfoKHR& safe_VkMemoryGetWin32HandleInfoKHR::operator=(
    const safe_VkMemoryGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetWin32HandleInfoKHR::~safe_VkMemoryGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const VkMemoryGetWin32HandleInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetWin32HandleInfoKHR::initialize(const safe_VkMemoryGetWin32HandleInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const VkImportMemoryFdInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR), pNext(nullptr), handleType(), fd() {}

safe_VkImportMemoryFdInfoKHR::safe_VkImportMemoryFdInfoKHR(const safe_VkImportMemoryFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryFdInfoKHR& safe_VkImportMemoryFdInfoKHR::operator=(const safe_VkImportMemoryFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryFdInfoKHR::~safe_VkImportMemoryFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportMemoryFdInfoKHR::initialize(const VkImportMemoryFdInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryFdInfoKHR::initialize(const safe_VkImportMemoryFdInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const VkMemoryFdPropertiesKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryFdPropertiesKHR::safe_VkMemoryFdPropertiesKHR(const safe_VkMemoryFdPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryFdPropertiesKHR& safe_VkMemoryFdPropertiesKHR::operator=(const safe_VkMemoryFdPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryFdPropertiesKHR::~safe_VkMemoryFdPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkMemoryFdPropertiesKHR::initialize(const VkMemoryFdPropertiesKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryFdPropertiesKHR::initialize(const safe_VkMemoryFdPropertiesKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const VkMemoryGetFdInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetFdInfoKHR::safe_VkMemoryGetFdInfoKHR(const safe_VkMemoryGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetFdInfoKHR& safe_VkMemoryGetFdInfoKHR::operator=(const safe_VkMemoryGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetFdInfoKHR::~safe_VkMemoryGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkMemoryGetFdInfoKHR::initialize(const VkMemoryGetFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetFdInfoKHR::initialize(const safe_VkMemoryGetFdInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(
    const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      acquireCount(in_struct->acquireCount),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeouts(nullptr),
      releaseCount(in_struct->releaseCount),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)in_struct->pAcquireTimeouts, sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR()
    : sType(VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR),
      pNext(nullptr),
      acquireCount(),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeouts(nullptr),
      releaseCount(),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& copy_src) {
    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src.pAcquireTimeouts, sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::operator=(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src.pAcquireTimeouts, sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }

    return *this;
}

safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::~safe_VkWin32KeyedMutexAcquireReleaseInfoKHR() {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const VkWin32KeyedMutexAcquireReleaseInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeouts) delete[] pAcquireTimeouts;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    acquireCount = in_struct->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = in_struct->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)in_struct->pAcquireTimeouts, sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoKHR::initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    acquireCount = copy_src->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeouts = nullptr;
    releaseCount = copy_src->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (acquireCount && copy_src->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src->pAcquireSyncs[i];
        }
    }

    if (copy_src->pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src->pAcquireKeys, sizeof(uint64_t) * copy_src->acquireCount);
    }

    if (copy_src->pAcquireTimeouts) {
        pAcquireTimeouts = new uint32_t[copy_src->acquireCount];
        memcpy((void*)pAcquireTimeouts, (void*)copy_src->pAcquireTimeouts, sizeof(uint32_t) * copy_src->acquireCount);
    }
    if (releaseCount && copy_src->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src->pReleaseSyncs[i];
        }
    }

    if (copy_src->pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src->pReleaseKeys, sizeof(uint64_t) * copy_src->releaseCount);
    }
}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(
    const VkImportSemaphoreWin32HandleInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      semaphore(in_struct->semaphore),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      handle(in_struct->handle),
      name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR),
      pNext(nullptr),
      semaphore(),
      flags(),
      handleType(),
      handle(),
      name() {}

safe_VkImportSemaphoreWin32HandleInfoKHR::safe_VkImportSemaphoreWin32HandleInfoKHR(
    const safe_VkImportSemaphoreWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportSemaphoreWin32HandleInfoKHR& safe_VkImportSemaphoreWin32HandleInfoKHR::operator=(
    const safe_VkImportSemaphoreWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportSemaphoreWin32HandleInfoKHR::~safe_VkImportSemaphoreWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const VkImportSemaphoreWin32HandleInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkImportSemaphoreWin32HandleInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(
    const VkExportSemaphoreWin32HandleInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportSemaphoreWin32HandleInfoKHR::safe_VkExportSemaphoreWin32HandleInfoKHR(
    const safe_VkExportSemaphoreWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportSemaphoreWin32HandleInfoKHR& safe_VkExportSemaphoreWin32HandleInfoKHR::operator=(
    const safe_VkExportSemaphoreWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportSemaphoreWin32HandleInfoKHR::~safe_VkExportSemaphoreWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const VkExportSemaphoreWin32HandleInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportSemaphoreWin32HandleInfoKHR::initialize(const safe_VkExportSemaphoreWin32HandleInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const VkD3D12FenceSubmitInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      waitSemaphoreValuesCount(in_struct->waitSemaphoreValuesCount),
      pWaitSemaphoreValues(nullptr),
      signalSemaphoreValuesCount(in_struct->signalSemaphoreValuesCount),
      pSignalSemaphoreValues(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)in_struct->pWaitSemaphoreValues,
               sizeof(uint64_t) * in_struct->waitSemaphoreValuesCount);
    }

    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)in_struct->pSignalSemaphoreValues,
               sizeof(uint64_t) * in_struct->signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR()
    : sType(VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR),
      pNext(nullptr),
      waitSemaphoreValuesCount(),
      pWaitSemaphoreValues(nullptr),
      signalSemaphoreValuesCount(),
      pSignalSemaphoreValues(nullptr) {}

safe_VkD3D12FenceSubmitInfoKHR::safe_VkD3D12FenceSubmitInfoKHR(const safe_VkD3D12FenceSubmitInfoKHR& copy_src) {
    sType = copy_src.sType;
    waitSemaphoreValuesCount = copy_src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src.waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src.pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src.waitSemaphoreValuesCount);
    }

    if (copy_src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src.signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src.pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src.signalSemaphoreValuesCount);
    }
}

safe_VkD3D12FenceSubmitInfoKHR& safe_VkD3D12FenceSubmitInfoKHR::operator=(const safe_VkD3D12FenceSubmitInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    waitSemaphoreValuesCount = copy_src.waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src.signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src.waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src.pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src.waitSemaphoreValuesCount);
    }

    if (copy_src.pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src.signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src.pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src.signalSemaphoreValuesCount);
    }

    return *this;
}

safe_VkD3D12FenceSubmitInfoKHR::~safe_VkD3D12FenceSubmitInfoKHR() {
    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const VkD3D12FenceSubmitInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pWaitSemaphoreValues) delete[] pWaitSemaphoreValues;
    if (pSignalSemaphoreValues) delete[] pSignalSemaphoreValues;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    waitSemaphoreValuesCount = in_struct->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = in_struct->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[in_struct->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)in_struct->pWaitSemaphoreValues,
               sizeof(uint64_t) * in_struct->waitSemaphoreValuesCount);
    }

    if (in_struct->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[in_struct->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)in_struct->pSignalSemaphoreValues,
               sizeof(uint64_t) * in_struct->signalSemaphoreValuesCount);
    }
}

void safe_VkD3D12FenceSubmitInfoKHR::initialize(const safe_VkD3D12FenceSubmitInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    waitSemaphoreValuesCount = copy_src->waitSemaphoreValuesCount;
    pWaitSemaphoreValues = nullptr;
    signalSemaphoreValuesCount = copy_src->signalSemaphoreValuesCount;
    pSignalSemaphoreValues = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pWaitSemaphoreValues) {
        pWaitSemaphoreValues = new uint64_t[copy_src->waitSemaphoreValuesCount];
        memcpy((void*)pWaitSemaphoreValues, (void*)copy_src->pWaitSemaphoreValues,
               sizeof(uint64_t) * copy_src->waitSemaphoreValuesCount);
    }

    if (copy_src->pSignalSemaphoreValues) {
        pSignalSemaphoreValues = new uint64_t[copy_src->signalSemaphoreValuesCount];
        memcpy((void*)pSignalSemaphoreValues, (void*)copy_src->pSignalSemaphoreValues,
               sizeof(uint64_t) * copy_src->signalSemaphoreValuesCount);
    }
}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(const VkSemaphoreGetWin32HandleInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), semaphore(), handleType() {}

safe_VkSemaphoreGetWin32HandleInfoKHR::safe_VkSemaphoreGetWin32HandleInfoKHR(
    const safe_VkSemaphoreGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSemaphoreGetWin32HandleInfoKHR& safe_VkSemaphoreGetWin32HandleInfoKHR::operator=(
    const safe_VkSemaphoreGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSemaphoreGetWin32HandleInfoKHR::~safe_VkSemaphoreGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const VkSemaphoreGetWin32HandleInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSemaphoreGetWin32HandleInfoKHR::initialize(const safe_VkSemaphoreGetWin32HandleInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const VkImportSemaphoreFdInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      semaphore(in_struct->semaphore),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR), pNext(nullptr), semaphore(), flags(), handleType(), fd() {}

safe_VkImportSemaphoreFdInfoKHR::safe_VkImportSemaphoreFdInfoKHR(const safe_VkImportSemaphoreFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportSemaphoreFdInfoKHR& safe_VkImportSemaphoreFdInfoKHR::operator=(const safe_VkImportSemaphoreFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportSemaphoreFdInfoKHR::~safe_VkImportSemaphoreFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportSemaphoreFdInfoKHR::initialize(const VkImportSemaphoreFdInfoKHR* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportSemaphoreFdInfoKHR::initialize(const safe_VkImportSemaphoreFdInfoKHR* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const VkSemaphoreGetFdInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR), pNext(nullptr), semaphore(), handleType() {}

safe_VkSemaphoreGetFdInfoKHR::safe_VkSemaphoreGetFdInfoKHR(const safe_VkSemaphoreGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSemaphoreGetFdInfoKHR& safe_VkSemaphoreGetFdInfoKHR::operator=(const safe_VkSemaphoreGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSemaphoreGetFdInfoKHR::~safe_VkSemaphoreGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkSemaphoreGetFdInfoKHR::initialize(const VkSemaphoreGetFdInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSemaphoreGetFdInfoKHR::initialize(const safe_VkSemaphoreGetFdInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const VkPresentRegionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state)
    : rectangleCount(in_struct->rectangleCount), pRectangles(nullptr) {
    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy((void*)pRectangles, (void*)in_struct->pRectangles, sizeof(VkRectLayerKHR) * in_struct->rectangleCount);
    }
}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR() : rectangleCount(), pRectangles(nullptr) {}

safe_VkPresentRegionKHR::safe_VkPresentRegionKHR(const safe_VkPresentRegionKHR& copy_src) {
    rectangleCount = copy_src.rectangleCount;
    pRectangles = nullptr;

    if (copy_src.pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src.rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src.pRectangles, sizeof(VkRectLayerKHR) * copy_src.rectangleCount);
    }
}

safe_VkPresentRegionKHR& safe_VkPresentRegionKHR::operator=(const safe_VkPresentRegionKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pRectangles) delete[] pRectangles;

    rectangleCount = copy_src.rectangleCount;
    pRectangles = nullptr;

    if (copy_src.pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src.rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src.pRectangles, sizeof(VkRectLayerKHR) * copy_src.rectangleCount);
    }

    return *this;
}

safe_VkPresentRegionKHR::~safe_VkPresentRegionKHR() {
    if (pRectangles) delete[] pRectangles;
}

void safe_VkPresentRegionKHR::initialize(const VkPresentRegionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRectangles) delete[] pRectangles;
    rectangleCount = in_struct->rectangleCount;
    pRectangles = nullptr;

    if (in_struct->pRectangles) {
        pRectangles = new VkRectLayerKHR[in_struct->rectangleCount];
        memcpy((void*)pRectangles, (void*)in_struct->pRectangles, sizeof(VkRectLayerKHR) * in_struct->rectangleCount);
    }
}

void safe_VkPresentRegionKHR::initialize(const safe_VkPresentRegionKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    rectangleCount = copy_src->rectangleCount;
    pRectangles = nullptr;

    if (copy_src->pRectangles) {
        pRectangles = new VkRectLayerKHR[copy_src->rectangleCount];
        memcpy((void*)pRectangles, (void*)copy_src->pRectangles, sizeof(VkRectLayerKHR) * copy_src->rectangleCount);
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const VkPresentRegionsKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pRegions(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR), pNext(nullptr), swapchainCount(), pRegions(nullptr) {}

safe_VkPresentRegionsKHR::safe_VkPresentRegionsKHR(const safe_VkPresentRegionsKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src.pRegions[i]);
        }
    }
}

safe_VkPresentRegionsKHR& safe_VkPresentRegionsKHR::operator=(const safe_VkPresentRegionsKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src.pRegions[i]);
        }
    }

    return *this;
}

safe_VkPresentRegionsKHR::~safe_VkPresentRegionsKHR() {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
}

void safe_VkPresentRegionsKHR::initialize(const VkPresentRegionsKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (swapchainCount && in_struct->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&in_struct->pRegions[i]);
        }
    }
}

void safe_VkPresentRegionsKHR::initialize(const safe_VkPresentRegionsKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (swapchainCount && copy_src->pRegions) {
        pRegions = new safe_VkPresentRegionKHR[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pRegions[i].initialize(&copy_src->pRegions[i]);
        }
    }
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(
    const VkSharedPresentSurfaceCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), sharedPresentSupportedUsageFlags(in_struct->sharedPresentSupportedUsageFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR), pNext(nullptr), sharedPresentSupportedUsageFlags() {}

safe_VkSharedPresentSurfaceCapabilitiesKHR::safe_VkSharedPresentSurfaceCapabilitiesKHR(
    const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    sharedPresentSupportedUsageFlags = copy_src.sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSharedPresentSurfaceCapabilitiesKHR& safe_VkSharedPresentSurfaceCapabilitiesKHR::operator=(
    const safe_VkSharedPresentSurfaceCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sharedPresentSupportedUsageFlags = copy_src.sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSharedPresentSurfaceCapabilitiesKHR::~safe_VkSharedPresentSurfaceCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const VkSharedPresentSurfaceCapabilitiesKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sharedPresentSupportedUsageFlags = in_struct->sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSharedPresentSurfaceCapabilitiesKHR::initialize(const safe_VkSharedPresentSurfaceCapabilitiesKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sharedPresentSupportedUsageFlags = copy_src->sharedPresentSupportedUsageFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const VkImportFenceWin32HandleInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      fence(in_struct->fence),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      handle(in_struct->handle),
      name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR),
      pNext(nullptr),
      fence(),
      flags(),
      handleType(),
      handle(),
      name() {}

safe_VkImportFenceWin32HandleInfoKHR::safe_VkImportFenceWin32HandleInfoKHR(const safe_VkImportFenceWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportFenceWin32HandleInfoKHR& safe_VkImportFenceWin32HandleInfoKHR::operator=(
    const safe_VkImportFenceWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportFenceWin32HandleInfoKHR::~safe_VkImportFenceWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const VkImportFenceWin32HandleInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportFenceWin32HandleInfoKHR::initialize(const safe_VkImportFenceWin32HandleInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const VkExportFenceWin32HandleInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess), name(in_struct->name) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR), pNext(nullptr), pAttributes(nullptr), dwAccess(), name() {}

safe_VkExportFenceWin32HandleInfoKHR::safe_VkExportFenceWin32HandleInfoKHR(const safe_VkExportFenceWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportFenceWin32HandleInfoKHR& safe_VkExportFenceWin32HandleInfoKHR::operator=(
    const safe_VkExportFenceWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    name = copy_src.name;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportFenceWin32HandleInfoKHR::~safe_VkExportFenceWin32HandleInfoKHR() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const VkExportFenceWin32HandleInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    name = in_struct->name;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportFenceWin32HandleInfoKHR::initialize(const safe_VkExportFenceWin32HandleInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    name = copy_src->name;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const VkFenceGetWin32HandleInfoKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fence(in_struct->fence), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR), pNext(nullptr), fence(), handleType() {}

safe_VkFenceGetWin32HandleInfoKHR::safe_VkFenceGetWin32HandleInfoKHR(const safe_VkFenceGetWin32HandleInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFenceGetWin32HandleInfoKHR& safe_VkFenceGetWin32HandleInfoKHR::operator=(const safe_VkFenceGetWin32HandleInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFenceGetWin32HandleInfoKHR::~safe_VkFenceGetWin32HandleInfoKHR() { FreePnextChain(pNext); }

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const VkFenceGetWin32HandleInfoKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFenceGetWin32HandleInfoKHR::initialize(const safe_VkFenceGetWin32HandleInfoKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const VkImportFenceFdInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      fence(in_struct->fence),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      fd(in_struct->fd) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR), pNext(nullptr), fence(), flags(), handleType(), fd() {}

safe_VkImportFenceFdInfoKHR::safe_VkImportFenceFdInfoKHR(const safe_VkImportFenceFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportFenceFdInfoKHR& safe_VkImportFenceFdInfoKHR::operator=(const safe_VkImportFenceFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    fd = copy_src.fd;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportFenceFdInfoKHR::~safe_VkImportFenceFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkImportFenceFdInfoKHR::initialize(const VkImportFenceFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    fd = in_struct->fd;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportFenceFdInfoKHR::initialize(const safe_VkImportFenceFdInfoKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    fd = copy_src->fd;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const VkFenceGetFdInfoKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fence(in_struct->fence), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR), pNext(nullptr), fence(), handleType() {}

safe_VkFenceGetFdInfoKHR::safe_VkFenceGetFdInfoKHR(const safe_VkFenceGetFdInfoKHR& copy_src) {
    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFenceGetFdInfoKHR& safe_VkFenceGetFdInfoKHR::operator=(const safe_VkFenceGetFdInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fence = copy_src.fence;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFenceGetFdInfoKHR::~safe_VkFenceGetFdInfoKHR() { FreePnextChain(pNext); }

void safe_VkFenceGetFdInfoKHR::initialize(const VkFenceGetFdInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fence = in_struct->fence;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFenceGetFdInfoKHR::initialize(const safe_VkFenceGetFdInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fence = copy_src->fence;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(
    const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      performanceCounterQueryPools(in_struct->performanceCounterQueryPools),
      performanceCounterMultipleQueryPools(in_struct->performanceCounterMultipleQueryPools) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR),
      pNext(nullptr),
      performanceCounterQueryPools(),
      performanceCounterMultipleQueryPools() {}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(
    const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    performanceCounterQueryPools = copy_src.performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src.performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    performanceCounterQueryPools = copy_src.performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src.performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::~safe_VkPhysicalDevicePerformanceQueryFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::initialize(const VkPhysicalDevicePerformanceQueryFeaturesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    performanceCounterQueryPools = in_struct->performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = in_struct->performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePerformanceQueryFeaturesKHR::initialize(const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    performanceCounterQueryPools = copy_src->performanceCounterQueryPools;
    performanceCounterMultipleQueryPools = copy_src->performanceCounterMultipleQueryPools;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(
    const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), allowCommandBufferQueryCopies(in_struct->allowCommandBufferQueryCopies) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR), pNext(nullptr), allowCommandBufferQueryCopies() {}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    allowCommandBufferQueryCopies = copy_src.allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    allowCommandBufferQueryCopies = copy_src.allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::~safe_VkPhysicalDevicePerformanceQueryPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::initialize(const VkPhysicalDevicePerformanceQueryPropertiesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    allowCommandBufferQueryCopies = in_struct->allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePerformanceQueryPropertiesKHR::initialize(
    const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    allowCommandBufferQueryCopies = copy_src->allowCommandBufferQueryCopies;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR(const VkPerformanceCounterKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), unit(in_struct->unit), scope(in_struct->scope), storage(in_struct->storage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = in_struct->uuid[i];
    }
}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR), pNext(nullptr), unit(), scope(), storage() {}

safe_VkPerformanceCounterKHR::safe_VkPerformanceCounterKHR(const safe_VkPerformanceCounterKHR& copy_src) {
    sType = copy_src.sType;
    unit = copy_src.unit;
    scope = copy_src.scope;
    storage = copy_src.storage;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src.uuid[i];
    }
}

safe_VkPerformanceCounterKHR& safe_VkPerformanceCounterKHR::operator=(const safe_VkPerformanceCounterKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    unit = copy_src.unit;
    scope = copy_src.scope;
    storage = copy_src.storage;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src.uuid[i];
    }

    return *this;
}

safe_VkPerformanceCounterKHR::~safe_VkPerformanceCounterKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceCounterKHR::initialize(const VkPerformanceCounterKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    unit = in_struct->unit;
    scope = in_struct->scope;
    storage = in_struct->storage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = in_struct->uuid[i];
    }
}

void safe_VkPerformanceCounterKHR::initialize(const safe_VkPerformanceCounterKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    unit = copy_src->unit;
    scope = copy_src->scope;
    storage = copy_src->storage;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
        uuid[i] = copy_src->uuid[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR(
    const VkPerformanceCounterDescriptionKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = in_struct->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR), pNext(nullptr), flags() {}

safe_VkPerformanceCounterDescriptionKHR::safe_VkPerformanceCounterDescriptionKHR(
    const safe_VkPerformanceCounterDescriptionKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src.category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPerformanceCounterDescriptionKHR& safe_VkPerformanceCounterDescriptionKHR::operator=(
    const safe_VkPerformanceCounterDescriptionKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src.category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPerformanceCounterDescriptionKHR::~safe_VkPerformanceCounterDescriptionKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceCounterDescriptionKHR::initialize(const VkPerformanceCounterDescriptionKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = in_struct->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPerformanceCounterDescriptionKHR::initialize(const safe_VkPerformanceCounterDescriptionKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        category[i] = copy_src->category[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR(
    const VkQueryPoolPerformanceCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      queueFamilyIndex(in_struct->queueFamilyIndex),
      counterIndexCount(in_struct->counterIndexCount),
      pCounterIndices(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pCounterIndices) {
        pCounterIndices = new uint32_t[in_struct->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)in_struct->pCounterIndices, sizeof(uint32_t) * in_struct->counterIndexCount);
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR),
      pNext(nullptr),
      queueFamilyIndex(),
      counterIndexCount(),
      pCounterIndices(nullptr) {}

safe_VkQueryPoolPerformanceCreateInfoKHR::safe_VkQueryPoolPerformanceCreateInfoKHR(
    const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    counterIndexCount = copy_src.counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src.counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src.pCounterIndices, sizeof(uint32_t) * copy_src.counterIndexCount);
    }
}

safe_VkQueryPoolPerformanceCreateInfoKHR& safe_VkQueryPoolPerformanceCreateInfoKHR::operator=(
    const safe_VkQueryPoolPerformanceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    queueFamilyIndex = copy_src.queueFamilyIndex;
    counterIndexCount = copy_src.counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src.counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src.pCounterIndices, sizeof(uint32_t) * copy_src.counterIndexCount);
    }

    return *this;
}

safe_VkQueryPoolPerformanceCreateInfoKHR::~safe_VkQueryPoolPerformanceCreateInfoKHR() {
    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);
}

void safe_VkQueryPoolPerformanceCreateInfoKHR::initialize(const VkQueryPoolPerformanceCreateInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pCounterIndices) delete[] pCounterIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queueFamilyIndex = in_struct->queueFamilyIndex;
    counterIndexCount = in_struct->counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pCounterIndices) {
        pCounterIndices = new uint32_t[in_struct->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)in_struct->pCounterIndices, sizeof(uint32_t) * in_struct->counterIndexCount);
    }
}

void safe_VkQueryPoolPerformanceCreateInfoKHR::initialize(const safe_VkQueryPoolPerformanceCreateInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queueFamilyIndex = copy_src->queueFamilyIndex;
    counterIndexCount = copy_src->counterIndexCount;
    pCounterIndices = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pCounterIndices) {
        pCounterIndices = new uint32_t[copy_src->counterIndexCount];
        memcpy((void*)pCounterIndices, (void*)copy_src->pCounterIndices, sizeof(uint32_t) * copy_src->counterIndexCount);
    }
}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR(const VkAcquireProfilingLockInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), timeout(in_struct->timeout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR), pNext(nullptr), flags(), timeout() {}

safe_VkAcquireProfilingLockInfoKHR::safe_VkAcquireProfilingLockInfoKHR(const safe_VkAcquireProfilingLockInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAcquireProfilingLockInfoKHR& safe_VkAcquireProfilingLockInfoKHR::operator=(
    const safe_VkAcquireProfilingLockInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAcquireProfilingLockInfoKHR::~safe_VkAcquireProfilingLockInfoKHR() { FreePnextChain(pNext); }

void safe_VkAcquireProfilingLockInfoKHR::initialize(const VkAcquireProfilingLockInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    timeout = in_struct->timeout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAcquireProfilingLockInfoKHR::initialize(const safe_VkAcquireProfilingLockInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    timeout = copy_src->timeout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR(const VkPerformanceQuerySubmitInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), counterPassIndex(in_struct->counterPassIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR), pNext(nullptr), counterPassIndex() {}

safe_VkPerformanceQuerySubmitInfoKHR::safe_VkPerformanceQuerySubmitInfoKHR(const safe_VkPerformanceQuerySubmitInfoKHR& copy_src) {
    sType = copy_src.sType;
    counterPassIndex = copy_src.counterPassIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceQuerySubmitInfoKHR& safe_VkPerformanceQuerySubmitInfoKHR::operator=(
    const safe_VkPerformanceQuerySubmitInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    counterPassIndex = copy_src.counterPassIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceQuerySubmitInfoKHR::~safe_VkPerformanceQuerySubmitInfoKHR() { FreePnextChain(pNext); }

void safe_VkPerformanceQuerySubmitInfoKHR::initialize(const VkPerformanceQuerySubmitInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    counterPassIndex = in_struct->counterPassIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceQuerySubmitInfoKHR::initialize(const safe_VkPerformanceQuerySubmitInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    counterPassIndex = copy_src->counterPassIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), surface(in_struct->surface) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR), pNext(nullptr), surface() {}

safe_VkPhysicalDeviceSurfaceInfo2KHR::safe_VkPhysicalDeviceSurfaceInfo2KHR(const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src) {
    sType = copy_src.sType;
    surface = copy_src.surface;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSurfaceInfo2KHR& safe_VkPhysicalDeviceSurfaceInfo2KHR::operator=(
    const safe_VkPhysicalDeviceSurfaceInfo2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surface = copy_src.surface;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSurfaceInfo2KHR::~safe_VkPhysicalDeviceSurfaceInfo2KHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const VkPhysicalDeviceSurfaceInfo2KHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surface = in_struct->surface;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSurfaceInfo2KHR::initialize(const safe_VkPhysicalDeviceSurfaceInfo2KHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surface = copy_src->surface;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const VkSurfaceCapabilities2KHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), surfaceCapabilities(in_struct->surfaceCapabilities) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR), pNext(nullptr), surfaceCapabilities() {}

safe_VkSurfaceCapabilities2KHR::safe_VkSurfaceCapabilities2KHR(const safe_VkSurfaceCapabilities2KHR& copy_src) {
    sType = copy_src.sType;
    surfaceCapabilities = copy_src.surfaceCapabilities;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilities2KHR& safe_VkSurfaceCapabilities2KHR::operator=(const safe_VkSurfaceCapabilities2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surfaceCapabilities = copy_src.surfaceCapabilities;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilities2KHR::~safe_VkSurfaceCapabilities2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilities2KHR::initialize(const VkSurfaceCapabilities2KHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surfaceCapabilities = in_struct->surfaceCapabilities;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilities2KHR::initialize(const safe_VkSurfaceCapabilities2KHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surfaceCapabilities = copy_src->surfaceCapabilities;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const VkSurfaceFormat2KHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), surfaceFormat(in_struct->surfaceFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR), pNext(nullptr), surfaceFormat() {}

safe_VkSurfaceFormat2KHR::safe_VkSurfaceFormat2KHR(const safe_VkSurfaceFormat2KHR& copy_src) {
    sType = copy_src.sType;
    surfaceFormat = copy_src.surfaceFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceFormat2KHR& safe_VkSurfaceFormat2KHR::operator=(const safe_VkSurfaceFormat2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    surfaceFormat = copy_src.surfaceFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceFormat2KHR::~safe_VkSurfaceFormat2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceFormat2KHR::initialize(const VkSurfaceFormat2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    surfaceFormat = in_struct->surfaceFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceFormat2KHR::initialize(const safe_VkSurfaceFormat2KHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    surfaceFormat = copy_src->surfaceFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const VkDisplayProperties2KHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayProperties(&in_struct->displayProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR() : sType(VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR), pNext(nullptr) {}

safe_VkDisplayProperties2KHR::safe_VkDisplayProperties2KHR(const safe_VkDisplayProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayProperties.initialize(&copy_src.displayProperties);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayProperties2KHR& safe_VkDisplayProperties2KHR::operator=(const safe_VkDisplayProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayProperties.initialize(&copy_src.displayProperties);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayProperties2KHR::~safe_VkDisplayProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayProperties2KHR::initialize(const VkDisplayProperties2KHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayProperties.initialize(&in_struct->displayProperties);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayProperties2KHR::initialize(const safe_VkDisplayProperties2KHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayProperties.initialize(&copy_src->displayProperties);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const VkDisplayPlaneProperties2KHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayPlaneProperties(in_struct->displayPlaneProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR), pNext(nullptr), displayPlaneProperties() {}

safe_VkDisplayPlaneProperties2KHR::safe_VkDisplayPlaneProperties2KHR(const safe_VkDisplayPlaneProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayPlaneProperties = copy_src.displayPlaneProperties;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneProperties2KHR& safe_VkDisplayPlaneProperties2KHR::operator=(const safe_VkDisplayPlaneProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayPlaneProperties = copy_src.displayPlaneProperties;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneProperties2KHR::~safe_VkDisplayPlaneProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneProperties2KHR::initialize(const VkDisplayPlaneProperties2KHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayPlaneProperties = in_struct->displayPlaneProperties;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneProperties2KHR::initialize(const safe_VkDisplayPlaneProperties2KHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayPlaneProperties = copy_src->displayPlaneProperties;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const VkDisplayModeProperties2KHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displayModeProperties(in_struct->displayModeProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR), pNext(nullptr), displayModeProperties() {}

safe_VkDisplayModeProperties2KHR::safe_VkDisplayModeProperties2KHR(const safe_VkDisplayModeProperties2KHR& copy_src) {
    sType = copy_src.sType;
    displayModeProperties = copy_src.displayModeProperties;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayModeProperties2KHR& safe_VkDisplayModeProperties2KHR::operator=(const safe_VkDisplayModeProperties2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displayModeProperties = copy_src.displayModeProperties;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayModeProperties2KHR::~safe_VkDisplayModeProperties2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayModeProperties2KHR::initialize(const VkDisplayModeProperties2KHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displayModeProperties = in_struct->displayModeProperties;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayModeProperties2KHR::initialize(const safe_VkDisplayModeProperties2KHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displayModeProperties = copy_src->displayModeProperties;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const VkDisplayPlaneInfo2KHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), mode(in_struct->mode), planeIndex(in_struct->planeIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR), pNext(nullptr), mode(), planeIndex() {}

safe_VkDisplayPlaneInfo2KHR::safe_VkDisplayPlaneInfo2KHR(const safe_VkDisplayPlaneInfo2KHR& copy_src) {
    sType = copy_src.sType;
    mode = copy_src.mode;
    planeIndex = copy_src.planeIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneInfo2KHR& safe_VkDisplayPlaneInfo2KHR::operator=(const safe_VkDisplayPlaneInfo2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    mode = copy_src.mode;
    planeIndex = copy_src.planeIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneInfo2KHR::~safe_VkDisplayPlaneInfo2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneInfo2KHR::initialize(const VkDisplayPlaneInfo2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mode = in_struct->mode;
    planeIndex = in_struct->planeIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneInfo2KHR::initialize(const safe_VkDisplayPlaneInfo2KHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mode = copy_src->mode;
    planeIndex = copy_src->planeIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const VkDisplayPlaneCapabilities2KHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), capabilities(in_struct->capabilities) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR), pNext(nullptr), capabilities() {}

safe_VkDisplayPlaneCapabilities2KHR::safe_VkDisplayPlaneCapabilities2KHR(const safe_VkDisplayPlaneCapabilities2KHR& copy_src) {
    sType = copy_src.sType;
    capabilities = copy_src.capabilities;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayPlaneCapabilities2KHR& safe_VkDisplayPlaneCapabilities2KHR::operator=(
    const safe_VkDisplayPlaneCapabilities2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    capabilities = copy_src.capabilities;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayPlaneCapabilities2KHR::~safe_VkDisplayPlaneCapabilities2KHR() { FreePnextChain(pNext); }

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const VkDisplayPlaneCapabilities2KHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    capabilities = in_struct->capabilities;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayPlaneCapabilities2KHR::initialize(const safe_VkDisplayPlaneCapabilities2KHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    capabilities = copy_src->capabilities;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR(
    const VkPhysicalDeviceShaderBfloat16FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderBFloat16Type(in_struct->shaderBFloat16Type),
      shaderBFloat16DotProduct(in_struct->shaderBFloat16DotProduct),
      shaderBFloat16CooperativeMatrix(in_struct->shaderBFloat16CooperativeMatrix) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_BFLOAT16_FEATURES_KHR),
      pNext(nullptr),
      shaderBFloat16Type(),
      shaderBFloat16DotProduct(),
      shaderBFloat16CooperativeMatrix() {}

safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR(
    const safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderBFloat16Type = copy_src.shaderBFloat16Type;
    shaderBFloat16DotProduct = copy_src.shaderBFloat16DotProduct;
    shaderBFloat16CooperativeMatrix = copy_src.shaderBFloat16CooperativeMatrix;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR& safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderBFloat16Type = copy_src.shaderBFloat16Type;
    shaderBFloat16DotProduct = copy_src.shaderBFloat16DotProduct;
    shaderBFloat16CooperativeMatrix = copy_src.shaderBFloat16CooperativeMatrix;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::~safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::initialize(const VkPhysicalDeviceShaderBfloat16FeaturesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderBFloat16Type = in_struct->shaderBFloat16Type;
    shaderBFloat16DotProduct = in_struct->shaderBFloat16DotProduct;
    shaderBFloat16CooperativeMatrix = in_struct->shaderBFloat16CooperativeMatrix;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR::initialize(const safe_VkPhysicalDeviceShaderBfloat16FeaturesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderBFloat16Type = copy_src->shaderBFloat16Type;
    shaderBFloat16DotProduct = copy_src->shaderBFloat16DotProduct;
    shaderBFloat16CooperativeMatrix = copy_src->shaderBFloat16CooperativeMatrix;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(
    const VkPhysicalDevicePortabilitySubsetFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      constantAlphaColorBlendFactors(in_struct->constantAlphaColorBlendFactors),
      events(in_struct->events),
      imageViewFormatReinterpretation(in_struct->imageViewFormatReinterpretation),
      imageViewFormatSwizzle(in_struct->imageViewFormatSwizzle),
      imageView2DOn3DImage(in_struct->imageView2DOn3DImage),
      multisampleArrayImage(in_struct->multisampleArrayImage),
      mutableComparisonSamplers(in_struct->mutableComparisonSamplers),
      pointPolygons(in_struct->pointPolygons),
      samplerMipLodBias(in_struct->samplerMipLodBias),
      separateStencilMaskRef(in_struct->separateStencilMaskRef),
      shaderSampleRateInterpolationFunctions(in_struct->shaderSampleRateInterpolationFunctions),
      tessellationIsolines(in_struct->tessellationIsolines),
      tessellationPointMode(in_struct->tessellationPointMode),
      triangleFans(in_struct->triangleFans),
      vertexAttributeAccessBeyondStride(in_struct->vertexAttributeAccessBeyondStride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR),
      pNext(nullptr),
      constantAlphaColorBlendFactors(),
      events(),
      imageViewFormatReinterpretation(),
      imageViewFormatSwizzle(),
      imageView2DOn3DImage(),
      multisampleArrayImage(),
      mutableComparisonSamplers(),
      pointPolygons(),
      samplerMipLodBias(),
      separateStencilMaskRef(),
      shaderSampleRateInterpolationFunctions(),
      tessellationIsolines(),
      tessellationPointMode(),
      triangleFans(),
      vertexAttributeAccessBeyondStride() {}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    constantAlphaColorBlendFactors = copy_src.constantAlphaColorBlendFactors;
    events = copy_src.events;
    imageViewFormatReinterpretation = copy_src.imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src.imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src.imageView2DOn3DImage;
    multisampleArrayImage = copy_src.multisampleArrayImage;
    mutableComparisonSamplers = copy_src.mutableComparisonSamplers;
    pointPolygons = copy_src.pointPolygons;
    samplerMipLodBias = copy_src.samplerMipLodBias;
    separateStencilMaskRef = copy_src.separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src.shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src.tessellationIsolines;
    tessellationPointMode = copy_src.tessellationPointMode;
    triangleFans = copy_src.triangleFans;
    vertexAttributeAccessBeyondStride = copy_src.vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    constantAlphaColorBlendFactors = copy_src.constantAlphaColorBlendFactors;
    events = copy_src.events;
    imageViewFormatReinterpretation = copy_src.imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src.imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src.imageView2DOn3DImage;
    multisampleArrayImage = copy_src.multisampleArrayImage;
    mutableComparisonSamplers = copy_src.mutableComparisonSamplers;
    pointPolygons = copy_src.pointPolygons;
    samplerMipLodBias = copy_src.samplerMipLodBias;
    separateStencilMaskRef = copy_src.separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src.shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src.tessellationIsolines;
    tessellationPointMode = copy_src.tessellationPointMode;
    triangleFans = copy_src.triangleFans;
    vertexAttributeAccessBeyondStride = copy_src.vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::~safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::initialize(const VkPhysicalDevicePortabilitySubsetFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    constantAlphaColorBlendFactors = in_struct->constantAlphaColorBlendFactors;
    events = in_struct->events;
    imageViewFormatReinterpretation = in_struct->imageViewFormatReinterpretation;
    imageViewFormatSwizzle = in_struct->imageViewFormatSwizzle;
    imageView2DOn3DImage = in_struct->imageView2DOn3DImage;
    multisampleArrayImage = in_struct->multisampleArrayImage;
    mutableComparisonSamplers = in_struct->mutableComparisonSamplers;
    pointPolygons = in_struct->pointPolygons;
    samplerMipLodBias = in_struct->samplerMipLodBias;
    separateStencilMaskRef = in_struct->separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = in_struct->shaderSampleRateInterpolationFunctions;
    tessellationIsolines = in_struct->tessellationIsolines;
    tessellationPointMode = in_struct->tessellationPointMode;
    triangleFans = in_struct->triangleFans;
    vertexAttributeAccessBeyondStride = in_struct->vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR::initialize(
    const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    constantAlphaColorBlendFactors = copy_src->constantAlphaColorBlendFactors;
    events = copy_src->events;
    imageViewFormatReinterpretation = copy_src->imageViewFormatReinterpretation;
    imageViewFormatSwizzle = copy_src->imageViewFormatSwizzle;
    imageView2DOn3DImage = copy_src->imageView2DOn3DImage;
    multisampleArrayImage = copy_src->multisampleArrayImage;
    mutableComparisonSamplers = copy_src->mutableComparisonSamplers;
    pointPolygons = copy_src->pointPolygons;
    samplerMipLodBias = copy_src->samplerMipLodBias;
    separateStencilMaskRef = copy_src->separateStencilMaskRef;
    shaderSampleRateInterpolationFunctions = copy_src->shaderSampleRateInterpolationFunctions;
    tessellationIsolines = copy_src->tessellationIsolines;
    tessellationPointMode = copy_src->tessellationPointMode;
    triangleFans = copy_src->triangleFans;
    vertexAttributeAccessBeyondStride = copy_src->vertexAttributeAccessBeyondStride;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(
    const VkPhysicalDevicePortabilitySubsetPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minVertexInputBindingStrideAlignment(in_struct->minVertexInputBindingStrideAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR),
      pNext(nullptr),
      minVertexInputBindingStrideAlignment() {}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    minVertexInputBindingStrideAlignment = copy_src.minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minVertexInputBindingStrideAlignment = copy_src.minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::~safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::initialize(
    const VkPhysicalDevicePortabilitySubsetPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minVertexInputBindingStrideAlignment = in_struct->minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR::initialize(
    const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minVertexInputBindingStrideAlignment = copy_src->minVertexInputBindingStrideAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR(
    const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderSubgroupClock(in_struct->shaderSubgroupClock),
      shaderDeviceClock(in_struct->shaderDeviceClock) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR),
      pNext(nullptr),
      shaderSubgroupClock(),
      shaderDeviceClock() {}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::safe_VkPhysicalDeviceShaderClockFeaturesKHR(
    const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderSubgroupClock = copy_src.shaderSubgroupClock;
    shaderDeviceClock = copy_src.shaderDeviceClock;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR& safe_VkPhysicalDeviceShaderClockFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderClockFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSubgroupClock = copy_src.shaderSubgroupClock;
    shaderDeviceClock = copy_src.shaderDeviceClock;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderClockFeaturesKHR::~safe_VkPhysicalDeviceShaderClockFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderClockFeaturesKHR::initialize(const VkPhysicalDeviceShaderClockFeaturesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSubgroupClock = in_struct->shaderSubgroupClock;
    shaderDeviceClock = in_struct->shaderDeviceClock;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderClockFeaturesKHR::initialize(const safe_VkPhysicalDeviceShaderClockFeaturesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSubgroupClock = copy_src->shaderSubgroupClock;
    shaderDeviceClock = copy_src->shaderDeviceClock;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR(const VkVideoDecodeH265ProfileInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stdProfileIdc(in_struct->stdProfileIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR), pNext(nullptr), stdProfileIdc() {}

safe_VkVideoDecodeH265ProfileInfoKHR::safe_VkVideoDecodeH265ProfileInfoKHR(const safe_VkVideoDecodeH265ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH265ProfileInfoKHR& safe_VkVideoDecodeH265ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeH265ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfileIdc = copy_src.stdProfileIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH265ProfileInfoKHR::~safe_VkVideoDecodeH265ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH265ProfileInfoKHR::initialize(const VkVideoDecodeH265ProfileInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfileIdc = in_struct->stdProfileIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH265ProfileInfoKHR::initialize(const safe_VkVideoDecodeH265ProfileInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfileIdc = copy_src->stdProfileIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR(const VkVideoDecodeH265CapabilitiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), maxLevelIdc(in_struct->maxLevelIdc) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR), pNext(nullptr), maxLevelIdc() {}

safe_VkVideoDecodeH265CapabilitiesKHR::safe_VkVideoDecodeH265CapabilitiesKHR(
    const safe_VkVideoDecodeH265CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeH265CapabilitiesKHR& safe_VkVideoDecodeH265CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeH265CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevelIdc = copy_src.maxLevelIdc;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeH265CapabilitiesKHR::~safe_VkVideoDecodeH265CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeH265CapabilitiesKHR::initialize(const VkVideoDecodeH265CapabilitiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevelIdc = in_struct->maxLevelIdc;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeH265CapabilitiesKHR::initialize(const safe_VkVideoDecodeH265CapabilitiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevelIdc = copy_src->maxLevelIdc;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR(
    const VkVideoDecodeH265SessionParametersAddInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      stdVPSCount(in_struct->stdVPSCount),
      pStdVPSs(nullptr),
      stdSPSCount(in_struct->stdSPSCount),
      pStdSPSs(nullptr),
      stdPPSCount(in_struct->stdPPSCount),
      pStdPPSs(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR),
      pNext(nullptr),
      stdVPSCount(),
      pStdVPSs(nullptr),
      stdSPSCount(),
      pStdSPSs(nullptr),
      stdPPSCount(),
      pStdPPSs(nullptr) {}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::safe_VkVideoDecodeH265SessionParametersAddInfoKHR(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR& safe_VkVideoDecodeH265SessionParametersAddInfoKHR::operator=(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdVPSCount = copy_src.stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src.stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src.stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src.stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src.pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src.stdVPSCount);
    }

    if (copy_src.pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src.stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src.pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src.stdSPSCount);
    }

    if (copy_src.pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src.stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src.pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src.stdPPSCount);
    }

    return *this;
}

safe_VkVideoDecodeH265SessionParametersAddInfoKHR::~safe_VkVideoDecodeH265SessionParametersAddInfoKHR() {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265SessionParametersAddInfoKHR::initialize(const VkVideoDecodeH265SessionParametersAddInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdVPSs) delete[] pStdVPSs;
    if (pStdSPSs) delete[] pStdSPSs;
    if (pStdPPSs) delete[] pStdPPSs;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdVPSCount = in_struct->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = in_struct->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = in_struct->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[in_struct->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)in_struct->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * in_struct->stdVPSCount);
    }

    if (in_struct->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[in_struct->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)in_struct->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * in_struct->stdSPSCount);
    }

    if (in_struct->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[in_struct->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)in_struct->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * in_struct->stdPPSCount);
    }
}

void safe_VkVideoDecodeH265SessionParametersAddInfoKHR::initialize(
    const safe_VkVideoDecodeH265SessionParametersAddInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdVPSCount = copy_src->stdVPSCount;
    pStdVPSs = nullptr;
    stdSPSCount = copy_src->stdSPSCount;
    pStdSPSs = nullptr;
    stdPPSCount = copy_src->stdPPSCount;
    pStdPPSs = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdVPSs) {
        pStdVPSs = new StdVideoH265VideoParameterSet[copy_src->stdVPSCount];
        memcpy((void*)pStdVPSs, (void*)copy_src->pStdVPSs, sizeof(StdVideoH265VideoParameterSet) * copy_src->stdVPSCount);
    }

    if (copy_src->pStdSPSs) {
        pStdSPSs = new StdVideoH265SequenceParameterSet[copy_src->stdSPSCount];
        memcpy((void*)pStdSPSs, (void*)copy_src->pStdSPSs, sizeof(StdVideoH265SequenceParameterSet) * copy_src->stdSPSCount);
    }

    if (copy_src->pStdPPSs) {
        pStdPPSs = new StdVideoH265PictureParameterSet[copy_src->stdPPSCount];
        memcpy((void*)pStdPPSs, (void*)copy_src->pStdPPSs, sizeof(StdVideoH265PictureParameterSet) * copy_src->stdPPSCount);
    }
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(
    const VkVideoDecodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxStdVPSCount(in_struct->maxStdVPSCount),
      maxStdSPSCount(in_struct->maxStdSPSCount),
      maxStdPPSCount(in_struct->maxStdPPSCount),
      pParametersAddInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      maxStdVPSCount(),
      maxStdSPSCount(),
      maxStdPPSCount(),
      pParametersAddInfo(nullptr) {}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxStdVPSCount = copy_src.maxStdVPSCount;
    maxStdSPSCount = copy_src.maxStdSPSCount;
    maxStdPPSCount = copy_src.maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src.pParametersAddInfo);

    return *this;
}

safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::~safe_VkVideoDecodeH265SessionParametersCreateInfoKHR() {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::initialize(
    const VkVideoDecodeH265SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pParametersAddInfo) delete pParametersAddInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxStdVPSCount = in_struct->maxStdVPSCount;
    maxStdSPSCount = in_struct->maxStdSPSCount;
    maxStdPPSCount = in_struct->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(in_struct->pParametersAddInfo);
}

void safe_VkVideoDecodeH265SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxStdVPSCount = copy_src->maxStdVPSCount;
    maxStdSPSCount = copy_src->maxStdSPSCount;
    maxStdPPSCount = copy_src->maxStdPPSCount;
    pParametersAddInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pParametersAddInfo)
        pParametersAddInfo = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(*copy_src->pParametersAddInfo);
}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR(const VkVideoDecodeH265PictureInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      pStdPictureInfo(nullptr),
      sliceSegmentCount(in_struct->sliceSegmentCount),
      pSliceSegmentOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[in_struct->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)in_struct->pSliceSegmentOffsets,
               sizeof(uint32_t) * in_struct->sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      sliceSegmentCount(),
      pSliceSegmentOffsets(nullptr) {}

safe_VkVideoDecodeH265PictureInfoKHR::safe_VkVideoDecodeH265PictureInfoKHR(const safe_VkVideoDecodeH265PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src.sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src.sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src.pSliceSegmentOffsets, sizeof(uint32_t) * copy_src.sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265PictureInfoKHR& safe_VkVideoDecodeH265PictureInfoKHR::operator=(
    const safe_VkVideoDecodeH265PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src.sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src.pStdPictureInfo);
    }

    if (copy_src.pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src.sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src.pSliceSegmentOffsets, sizeof(uint32_t) * copy_src.sliceSegmentCount);
    }

    return *this;
}

safe_VkVideoDecodeH265PictureInfoKHR::~safe_VkVideoDecodeH265PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265PictureInfoKHR::initialize(const VkVideoDecodeH265PictureInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pSliceSegmentOffsets) delete[] pSliceSegmentOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = in_struct->sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*in_struct->pStdPictureInfo);
    }

    if (in_struct->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[in_struct->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)in_struct->pSliceSegmentOffsets,
               sizeof(uint32_t) * in_struct->sliceSegmentCount);
    }
}

void safe_VkVideoDecodeH265PictureInfoKHR::initialize(const safe_VkVideoDecodeH265PictureInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    sliceSegmentCount = copy_src->sliceSegmentCount;
    pSliceSegmentOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeH265PictureInfo(*copy_src->pStdPictureInfo);
    }

    if (copy_src->pSliceSegmentOffsets) {
        pSliceSegmentOffsets = new uint32_t[copy_src->sliceSegmentCount];
        memcpy((void*)pSliceSegmentOffsets, (void*)copy_src->pSliceSegmentOffsets, sizeof(uint32_t) * copy_src->sliceSegmentCount);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR(const VkVideoDecodeH265DpbSlotInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoDecodeH265DpbSlotInfoKHR::safe_VkVideoDecodeH265DpbSlotInfoKHR(const safe_VkVideoDecodeH265DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoDecodeH265DpbSlotInfoKHR& safe_VkVideoDecodeH265DpbSlotInfoKHR::operator=(
    const safe_VkVideoDecodeH265DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoDecodeH265DpbSlotInfoKHR::~safe_VkVideoDecodeH265DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265DpbSlotInfoKHR::initialize(const VkVideoDecodeH265DpbSlotInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoDecodeH265DpbSlotInfoKHR::initialize(const safe_VkVideoDecodeH265DpbSlotInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeH265ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR(
    const VkFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pFragmentShadingRateAttachment(nullptr),
      shadingRateAttachmentTexelSize(in_struct->shadingRateAttachmentTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(in_struct->pFragmentShadingRateAttachment);
}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR),
      pNext(nullptr),
      pFragmentShadingRateAttachment(nullptr),
      shadingRateAttachmentTexelSize() {}

safe_VkFragmentShadingRateAttachmentInfoKHR::safe_VkFragmentShadingRateAttachmentInfoKHR(
    const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src) {
    sType = copy_src.sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src.pFragmentShadingRateAttachment);
}

safe_VkFragmentShadingRateAttachmentInfoKHR& safe_VkFragmentShadingRateAttachmentInfoKHR::operator=(
    const safe_VkFragmentShadingRateAttachmentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src.pFragmentShadingRateAttachment);

    return *this;
}

safe_VkFragmentShadingRateAttachmentInfoKHR::~safe_VkFragmentShadingRateAttachmentInfoKHR() {
    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);
}

void safe_VkFragmentShadingRateAttachmentInfoKHR::initialize(const VkFragmentShadingRateAttachmentInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFragmentShadingRateAttachment) delete pFragmentShadingRateAttachment;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = in_struct->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(in_struct->pFragmentShadingRateAttachment);
}

void safe_VkFragmentShadingRateAttachmentInfoKHR::initialize(const safe_VkFragmentShadingRateAttachmentInfoKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pFragmentShadingRateAttachment = nullptr;
    shadingRateAttachmentTexelSize = copy_src->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pFragmentShadingRateAttachment)
        pFragmentShadingRateAttachment = new safe_VkAttachmentReference2(*copy_src->pFragmentShadingRateAttachment);
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(
    const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentSize(in_struct->fragmentSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR), pNext(nullptr), fragmentSize() {}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::operator=(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }

    return *this;
}

safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::~safe_VkPipelineFragmentShadingRateStateCreateInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::initialize(
    const VkPipelineFragmentShadingRateStateCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentSize = in_struct->fragmentSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

void safe_VkPipelineFragmentShadingRateStateCreateInfoKHR::initialize(
    const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentSize = copy_src->fragmentSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src->combinerOps[i];
    }
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(
    const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pipelineFragmentShadingRate(in_struct->pipelineFragmentShadingRate),
      primitiveFragmentShadingRate(in_struct->primitiveFragmentShadingRate),
      attachmentFragmentShadingRate(in_struct->attachmentFragmentShadingRate) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR),
      pNext(nullptr),
      pipelineFragmentShadingRate(),
      primitiveFragmentShadingRate(),
      attachmentFragmentShadingRate() {}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineFragmentShadingRate = copy_src.pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src.primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src.attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineFragmentShadingRate = copy_src.pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src.primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src.attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::~safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::initialize(
    const VkPhysicalDeviceFragmentShadingRateFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineFragmentShadingRate = in_struct->pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = in_struct->primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = in_struct->attachmentFragmentShadingRate;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineFragmentShadingRate = copy_src->pipelineFragmentShadingRate;
    primitiveFragmentShadingRate = copy_src->primitiveFragmentShadingRate;
    attachmentFragmentShadingRate = copy_src->attachmentFragmentShadingRate;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(
    const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minFragmentShadingRateAttachmentTexelSize(in_struct->minFragmentShadingRateAttachmentTexelSize),
      maxFragmentShadingRateAttachmentTexelSize(in_struct->maxFragmentShadingRateAttachmentTexelSize),
      maxFragmentShadingRateAttachmentTexelSizeAspectRatio(in_struct->maxFragmentShadingRateAttachmentTexelSizeAspectRatio),
      primitiveFragmentShadingRateWithMultipleViewports(in_struct->primitiveFragmentShadingRateWithMultipleViewports),
      layeredShadingRateAttachments(in_struct->layeredShadingRateAttachments),
      fragmentShadingRateNonTrivialCombinerOps(in_struct->fragmentShadingRateNonTrivialCombinerOps),
      maxFragmentSize(in_struct->maxFragmentSize),
      maxFragmentSizeAspectRatio(in_struct->maxFragmentSizeAspectRatio),
      maxFragmentShadingRateCoverageSamples(in_struct->maxFragmentShadingRateCoverageSamples),
      maxFragmentShadingRateRasterizationSamples(in_struct->maxFragmentShadingRateRasterizationSamples),
      fragmentShadingRateWithShaderDepthStencilWrites(in_struct->fragmentShadingRateWithShaderDepthStencilWrites),
      fragmentShadingRateWithSampleMask(in_struct->fragmentShadingRateWithSampleMask),
      fragmentShadingRateWithShaderSampleMask(in_struct->fragmentShadingRateWithShaderSampleMask),
      fragmentShadingRateWithConservativeRasterization(in_struct->fragmentShadingRateWithConservativeRasterization),
      fragmentShadingRateWithFragmentShaderInterlock(in_struct->fragmentShadingRateWithFragmentShaderInterlock),
      fragmentShadingRateWithCustomSampleLocations(in_struct->fragmentShadingRateWithCustomSampleLocations),
      fragmentShadingRateStrictMultiplyCombiner(in_struct->fragmentShadingRateStrictMultiplyCombiner) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR),
      pNext(nullptr),
      minFragmentShadingRateAttachmentTexelSize(),
      maxFragmentShadingRateAttachmentTexelSize(),
      maxFragmentShadingRateAttachmentTexelSizeAspectRatio(),
      primitiveFragmentShadingRateWithMultipleViewports(),
      layeredShadingRateAttachments(),
      fragmentShadingRateNonTrivialCombinerOps(),
      maxFragmentSize(),
      maxFragmentSizeAspectRatio(),
      maxFragmentShadingRateCoverageSamples(),
      maxFragmentShadingRateRasterizationSamples(),
      fragmentShadingRateWithShaderDepthStencilWrites(),
      fragmentShadingRateWithSampleMask(),
      fragmentShadingRateWithShaderSampleMask(),
      fragmentShadingRateWithConservativeRasterization(),
      fragmentShadingRateWithFragmentShaderInterlock(),
      fragmentShadingRateWithCustomSampleLocations(),
      fragmentShadingRateStrictMultiplyCombiner() {}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src.minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src.maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src.primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src.layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src.fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src.maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src.maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src.maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src.maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src.fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src.fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src.fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src.fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src.fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src.fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src.fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src.minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src.maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src.primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src.layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src.fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src.maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src.maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src.maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src.maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src.fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src.fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src.fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src.fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src.fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src.fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src.fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::~safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::initialize(
    const VkPhysicalDeviceFragmentShadingRatePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minFragmentShadingRateAttachmentTexelSize = in_struct->minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = in_struct->maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = in_struct->maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = in_struct->primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = in_struct->layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = in_struct->fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = in_struct->maxFragmentSize;
    maxFragmentSizeAspectRatio = in_struct->maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = in_struct->maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = in_struct->maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = in_struct->fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = in_struct->fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = in_struct->fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = in_struct->fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = in_struct->fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = in_struct->fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = in_struct->fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minFragmentShadingRateAttachmentTexelSize = copy_src->minFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSize = copy_src->maxFragmentShadingRateAttachmentTexelSize;
    maxFragmentShadingRateAttachmentTexelSizeAspectRatio = copy_src->maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
    primitiveFragmentShadingRateWithMultipleViewports = copy_src->primitiveFragmentShadingRateWithMultipleViewports;
    layeredShadingRateAttachments = copy_src->layeredShadingRateAttachments;
    fragmentShadingRateNonTrivialCombinerOps = copy_src->fragmentShadingRateNonTrivialCombinerOps;
    maxFragmentSize = copy_src->maxFragmentSize;
    maxFragmentSizeAspectRatio = copy_src->maxFragmentSizeAspectRatio;
    maxFragmentShadingRateCoverageSamples = copy_src->maxFragmentShadingRateCoverageSamples;
    maxFragmentShadingRateRasterizationSamples = copy_src->maxFragmentShadingRateRasterizationSamples;
    fragmentShadingRateWithShaderDepthStencilWrites = copy_src->fragmentShadingRateWithShaderDepthStencilWrites;
    fragmentShadingRateWithSampleMask = copy_src->fragmentShadingRateWithSampleMask;
    fragmentShadingRateWithShaderSampleMask = copy_src->fragmentShadingRateWithShaderSampleMask;
    fragmentShadingRateWithConservativeRasterization = copy_src->fragmentShadingRateWithConservativeRasterization;
    fragmentShadingRateWithFragmentShaderInterlock = copy_src->fragmentShadingRateWithFragmentShaderInterlock;
    fragmentShadingRateWithCustomSampleLocations = copy_src->fragmentShadingRateWithCustomSampleLocations;
    fragmentShadingRateStrictMultiplyCombiner = copy_src->fragmentShadingRateStrictMultiplyCombiner;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR(
    const VkPhysicalDeviceFragmentShadingRateKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), sampleCounts(in_struct->sampleCounts), fragmentSize(in_struct->fragmentSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR), pNext(nullptr), sampleCounts(), fragmentSize() {}

safe_VkPhysicalDeviceFragmentShadingRateKHR::safe_VkPhysicalDeviceFragmentShadingRateKHR(
    const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src) {
    sType = copy_src.sType;
    sampleCounts = copy_src.sampleCounts;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateKHR& safe_VkPhysicalDeviceFragmentShadingRateKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleCounts = copy_src.sampleCounts;
    fragmentSize = copy_src.fragmentSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateKHR::~safe_VkPhysicalDeviceFragmentShadingRateKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceFragmentShadingRateKHR::initialize(const VkPhysicalDeviceFragmentShadingRateKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleCounts = in_struct->sampleCounts;
    fragmentSize = in_struct->fragmentSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateKHR::initialize(const safe_VkPhysicalDeviceFragmentShadingRateKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleCounts = copy_src->sampleCounts;
    fragmentSize = copy_src->fragmentSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(
    const VkRenderingFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      imageView(in_struct->imageView),
      imageLayout(in_struct->imageLayout),
      shadingRateAttachmentTexelSize(in_struct->shadingRateAttachmentTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR),
      pNext(nullptr),
      imageView(),
      imageLayout(),
      shadingRateAttachmentTexelSize() {}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& copy_src) {
    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::operator=(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageView = copy_src.imageView;
    imageLayout = copy_src.imageLayout;
    shadingRateAttachmentTexelSize = copy_src.shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::~safe_VkRenderingFragmentShadingRateAttachmentInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::initialize(
    const VkRenderingFragmentShadingRateAttachmentInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageView = in_struct->imageView;
    imageLayout = in_struct->imageLayout;
    shadingRateAttachmentTexelSize = in_struct->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderingFragmentShadingRateAttachmentInfoKHR::initialize(
    const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageView = copy_src->imageView;
    imageLayout = copy_src->imageLayout;
    shadingRateAttachmentTexelSize = copy_src->shadingRateAttachmentTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR(
    const VkPhysicalDeviceShaderQuadControlFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderQuadControl(in_struct->shaderQuadControl) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_QUAD_CONTROL_FEATURES_KHR), pNext(nullptr), shaderQuadControl() {}

safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR(
    const safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderQuadControl = copy_src.shaderQuadControl;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR& safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderQuadControl = copy_src.shaderQuadControl;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::~safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::initialize(const VkPhysicalDeviceShaderQuadControlFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderQuadControl = in_struct->shaderQuadControl;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceShaderQuadControlFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderQuadControl = copy_src->shaderQuadControl;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(const VkSurfaceProtectedCapabilitiesKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), supportsProtected(in_struct->supportsProtected) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR), pNext(nullptr), supportsProtected() {}

safe_VkSurfaceProtectedCapabilitiesKHR::safe_VkSurfaceProtectedCapabilitiesKHR(
    const safe_VkSurfaceProtectedCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    supportsProtected = copy_src.supportsProtected;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceProtectedCapabilitiesKHR& safe_VkSurfaceProtectedCapabilitiesKHR::operator=(
    const safe_VkSurfaceProtectedCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportsProtected = copy_src.supportsProtected;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceProtectedCapabilitiesKHR::~safe_VkSurfaceProtectedCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const VkSurfaceProtectedCapabilitiesKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportsProtected = in_struct->supportsProtected;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceProtectedCapabilitiesKHR::initialize(const safe_VkSurfaceProtectedCapabilitiesKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportsProtected = copy_src->supportsProtected;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR(
    const VkPhysicalDevicePresentWaitFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentWait(in_struct->presentWait) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR), pNext(nullptr), presentWait() {}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::safe_VkPhysicalDevicePresentWaitFeaturesKHR(
    const safe_VkPhysicalDevicePresentWaitFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentWait = copy_src.presentWait;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR& safe_VkPhysicalDevicePresentWaitFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentWaitFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentWait = copy_src.presentWait;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentWaitFeaturesKHR::~safe_VkPhysicalDevicePresentWaitFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentWaitFeaturesKHR::initialize(const VkPhysicalDevicePresentWaitFeaturesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentWait = in_struct->presentWait;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentWaitFeaturesKHR::initialize(const safe_VkPhysicalDevicePresentWaitFeaturesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentWait = copy_src->presentWait;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(
    const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), pipelineExecutableInfo(in_struct->pipelineExecutableInfo) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR),
      pNext(nullptr),
      pipelineExecutableInfo() {}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineExecutableInfo = copy_src.pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR&
safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineExecutableInfo = copy_src.pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::~safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::initialize(
    const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineExecutableInfo = in_struct->pipelineExecutableInfo;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::initialize(
    const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineExecutableInfo = copy_src->pipelineExecutableInfo;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR(const VkPipelineInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                               bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR() : sType(VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR), pNext(nullptr), pipeline() {}

safe_VkPipelineInfoKHR::safe_VkPipelineInfoKHR(const safe_VkPipelineInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineInfoKHR& safe_VkPipelineInfoKHR::operator=(const safe_VkPipelineInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineInfoKHR::~safe_VkPipelineInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineInfoKHR::initialize(const VkPipelineInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineInfoKHR::initialize(const safe_VkPipelineInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR(const VkPipelineExecutablePropertiesKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), stages(in_struct->stages), subgroupSize(in_struct->subgroupSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR), pNext(nullptr), stages(), subgroupSize() {}

safe_VkPipelineExecutablePropertiesKHR::safe_VkPipelineExecutablePropertiesKHR(
    const safe_VkPipelineExecutablePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    stages = copy_src.stages;
    subgroupSize = copy_src.subgroupSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPipelineExecutablePropertiesKHR& safe_VkPipelineExecutablePropertiesKHR::operator=(
    const safe_VkPipelineExecutablePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stages = copy_src.stages;
    subgroupSize = copy_src.subgroupSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPipelineExecutablePropertiesKHR::~safe_VkPipelineExecutablePropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutablePropertiesKHR::initialize(const VkPipelineExecutablePropertiesKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stages = in_struct->stages;
    subgroupSize = in_struct->subgroupSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPipelineExecutablePropertiesKHR::initialize(const safe_VkPipelineExecutablePropertiesKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stages = copy_src->stages;
    subgroupSize = copy_src->subgroupSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR(const VkPipelineExecutableInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline), executableIndex(in_struct->executableIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR), pNext(nullptr), pipeline(), executableIndex() {}

safe_VkPipelineExecutableInfoKHR::safe_VkPipelineExecutableInfoKHR(const safe_VkPipelineExecutableInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    executableIndex = copy_src.executableIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineExecutableInfoKHR& safe_VkPipelineExecutableInfoKHR::operator=(const safe_VkPipelineExecutableInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    executableIndex = copy_src.executableIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineExecutableInfoKHR::~safe_VkPipelineExecutableInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutableInfoKHR::initialize(const VkPipelineExecutableInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    executableIndex = in_struct->executableIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineExecutableInfoKHR::initialize(const safe_VkPipelineExecutableInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    executableIndex = copy_src->executableIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR(const VkPipelineExecutableStatisticKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), format(in_struct->format), value(in_struct->value) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR), pNext(nullptr), format(), value() {}

safe_VkPipelineExecutableStatisticKHR::safe_VkPipelineExecutableStatisticKHR(
    const safe_VkPipelineExecutableStatisticKHR& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }
}

safe_VkPipelineExecutableStatisticKHR& safe_VkPipelineExecutableStatisticKHR::operator=(
    const safe_VkPipelineExecutableStatisticKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    return *this;
}

safe_VkPipelineExecutableStatisticKHR::~safe_VkPipelineExecutableStatisticKHR() { FreePnextChain(pNext); }

void safe_VkPipelineExecutableStatisticKHR::initialize(const VkPipelineExecutableStatisticKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    value = in_struct->value;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }
}

void safe_VkPipelineExecutableStatisticKHR::initialize(const safe_VkPipelineExecutableStatisticKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    value = copy_src->value;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR(
    const VkPipelineExecutableInternalRepresentationKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), isText(in_struct->isText), dataSize(in_struct->dataSize), pData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INTERNAL_REPRESENTATION_KHR),
      pNext(nullptr),
      isText(),
      dataSize(),
      pData(nullptr) {}

safe_VkPipelineExecutableInternalRepresentationKHR::safe_VkPipelineExecutableInternalRepresentationKHR(
    const safe_VkPipelineExecutableInternalRepresentationKHR& copy_src) {
    sType = copy_src.sType;
    isText = copy_src.isText;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }
}

safe_VkPipelineExecutableInternalRepresentationKHR& safe_VkPipelineExecutableInternalRepresentationKHR::operator=(
    const safe_VkPipelineExecutableInternalRepresentationKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    isText = copy_src.isText;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src.name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src.description[i];
    }

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }

    return *this;
}

safe_VkPipelineExecutableInternalRepresentationKHR::~safe_VkPipelineExecutableInternalRepresentationKHR() {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
}

void safe_VkPipelineExecutableInternalRepresentationKHR::initialize(const VkPipelineExecutableInternalRepresentationKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    isText = in_struct->isText;
    dataSize = in_struct->dataSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = in_struct->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = in_struct->description[i];
    }

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

void safe_VkPipelineExecutableInternalRepresentationKHR::initialize(
    const safe_VkPipelineExecutableInternalRepresentationKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    isText = copy_src->isText;
    dataSize = copy_src->dataSize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        name[i] = copy_src->name[i];
    }

    for (uint32_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; ++i) {
        description[i] = copy_src->description[i];
    }

    if (copy_src->pData != nullptr) {
        auto temp = new std::byte[copy_src->dataSize];
        std::memcpy(temp, copy_src->pData, copy_src->dataSize);
        pData = temp;
    }
}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR(const VkPipelineLibraryCreateInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), libraryCount(in_struct->libraryCount), pLibraries(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (libraryCount && in_struct->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = in_struct->pLibraries[i];
        }
    }
}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR), pNext(nullptr), libraryCount(), pLibraries(nullptr) {}

safe_VkPipelineLibraryCreateInfoKHR::safe_VkPipelineLibraryCreateInfoKHR(const safe_VkPipelineLibraryCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    libraryCount = copy_src.libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (libraryCount && copy_src.pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src.pLibraries[i];
        }
    }
}

safe_VkPipelineLibraryCreateInfoKHR& safe_VkPipelineLibraryCreateInfoKHR::operator=(
    const safe_VkPipelineLibraryCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    libraryCount = copy_src.libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (libraryCount && copy_src.pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src.pLibraries[i];
        }
    }

    return *this;
}

safe_VkPipelineLibraryCreateInfoKHR::~safe_VkPipelineLibraryCreateInfoKHR() {
    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);
}

void safe_VkPipelineLibraryCreateInfoKHR::initialize(const VkPipelineLibraryCreateInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLibraries) delete[] pLibraries;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    libraryCount = in_struct->libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (libraryCount && in_struct->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = in_struct->pLibraries[i];
        }
    }
}

void safe_VkPipelineLibraryCreateInfoKHR::initialize(const safe_VkPipelineLibraryCreateInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    libraryCount = copy_src->libraryCount;
    pLibraries = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (libraryCount && copy_src->pLibraries) {
        pLibraries = new VkPipeline[libraryCount];
        for (uint32_t i = 0; i < libraryCount; ++i) {
            pLibraries[i] = copy_src->pLibraries[i];
        }
    }
}

safe_VkPresentIdKHR::safe_VkPresentIdKHR(const VkPresentIdKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                         bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pPresentIds(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

safe_VkPresentIdKHR::safe_VkPresentIdKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_ID_KHR), pNext(nullptr), swapchainCount(), pPresentIds(nullptr) {}

safe_VkPresentIdKHR::safe_VkPresentIdKHR(const safe_VkPresentIdKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }
}

safe_VkPresentIdKHR& safe_VkPresentIdKHR::operator=(const safe_VkPresentIdKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentIdKHR::~safe_VkPresentIdKHR() {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
}

void safe_VkPresentIdKHR::initialize(const VkPresentIdKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

void safe_VkPresentIdKHR::initialize(const safe_VkPresentIdKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentIds) {
        pPresentIds = new uint64_t[copy_src->swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src->pPresentIds, sizeof(uint64_t) * copy_src->swapchainCount);
    }
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR(
    const VkPhysicalDevicePresentIdFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentId(in_struct->presentId) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR), pNext(nullptr), presentId() {}

safe_VkPhysicalDevicePresentIdFeaturesKHR::safe_VkPhysicalDevicePresentIdFeaturesKHR(
    const safe_VkPhysicalDevicePresentIdFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentId = copy_src.presentId;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentIdFeaturesKHR& safe_VkPhysicalDevicePresentIdFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentIdFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentId = copy_src.presentId;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentIdFeaturesKHR::~safe_VkPhysicalDevicePresentIdFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentIdFeaturesKHR::initialize(const VkPhysicalDevicePresentIdFeaturesKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentId = in_struct->presentId;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentIdFeaturesKHR::initialize(const safe_VkPhysicalDevicePresentIdFeaturesKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentId = copy_src->presentId;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR(const VkVideoEncodeInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      dstBuffer(in_struct->dstBuffer),
      dstBufferOffset(in_struct->dstBufferOffset),
      dstBufferRange(in_struct->dstBufferRange),
      srcPictureResource(&in_struct->srcPictureResource),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(in_struct->referenceSlotCount),
      pReferenceSlots(nullptr),
      precedingExternallyEncodedBytes(in_struct->precedingExternallyEncodedBytes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR),
      pNext(nullptr),
      flags(),
      dstBuffer(),
      dstBufferOffset(),
      dstBufferRange(),
      pSetupReferenceSlot(nullptr),
      referenceSlotCount(),
      pReferenceSlots(nullptr),
      precedingExternallyEncodedBytes() {}

safe_VkVideoEncodeInfoKHR::safe_VkVideoEncodeInfoKHR(const safe_VkVideoEncodeInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    dstBuffer = copy_src.dstBuffer;
    dstBufferOffset = copy_src.dstBufferOffset;
    dstBufferRange = copy_src.dstBufferRange;
    srcPictureResource.initialize(&copy_src.srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src.precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeInfoKHR& safe_VkVideoEncodeInfoKHR::operator=(const safe_VkVideoEncodeInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    dstBuffer = copy_src.dstBuffer;
    dstBufferOffset = copy_src.dstBufferOffset;
    dstBufferRange = copy_src.dstBufferRange;
    srcPictureResource.initialize(&copy_src.srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src.referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src.precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src.pSetupReferenceSlot);
    if (referenceSlotCount && copy_src.pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src.pReferenceSlots[i]);
        }
    }

    return *this;
}

safe_VkVideoEncodeInfoKHR::~safe_VkVideoEncodeInfoKHR() {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeInfoKHR::initialize(const VkVideoEncodeInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSetupReferenceSlot) delete pSetupReferenceSlot;
    if (pReferenceSlots) delete[] pReferenceSlots;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    dstBuffer = in_struct->dstBuffer;
    dstBufferOffset = in_struct->dstBufferOffset;
    dstBufferRange = in_struct->dstBufferRange;
    srcPictureResource.initialize(&in_struct->srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = in_struct->referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = in_struct->precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(in_struct->pSetupReferenceSlot);
    if (referenceSlotCount && in_struct->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&in_struct->pReferenceSlots[i]);
        }
    }
}

void safe_VkVideoEncodeInfoKHR::initialize(const safe_VkVideoEncodeInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    dstBuffer = copy_src->dstBuffer;
    dstBufferOffset = copy_src->dstBufferOffset;
    dstBufferRange = copy_src->dstBufferRange;
    srcPictureResource.initialize(&copy_src->srcPictureResource);
    pSetupReferenceSlot = nullptr;
    referenceSlotCount = copy_src->referenceSlotCount;
    pReferenceSlots = nullptr;
    precedingExternallyEncodedBytes = copy_src->precedingExternallyEncodedBytes;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pSetupReferenceSlot) pSetupReferenceSlot = new safe_VkVideoReferenceSlotInfoKHR(*copy_src->pSetupReferenceSlot);
    if (referenceSlotCount && copy_src->pReferenceSlots) {
        pReferenceSlots = new safe_VkVideoReferenceSlotInfoKHR[referenceSlotCount];
        for (uint32_t i = 0; i < referenceSlotCount; ++i) {
            pReferenceSlots[i].initialize(&copy_src->pReferenceSlots[i]);
        }
    }
}

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR(const VkVideoEncodeCapabilitiesKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      rateControlModes(in_struct->rateControlModes),
      maxRateControlLayers(in_struct->maxRateControlLayers),
      maxBitrate(in_struct->maxBitrate),
      maxQualityLevels(in_struct->maxQualityLevels),
      encodeInputPictureGranularity(in_struct->encodeInputPictureGranularity),
      supportedEncodeFeedbackFlags(in_struct->supportedEncodeFeedbackFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      rateControlModes(),
      maxRateControlLayers(),
      maxBitrate(),
      maxQualityLevels(),
      encodeInputPictureGranularity(),
      supportedEncodeFeedbackFlags() {}

safe_VkVideoEncodeCapabilitiesKHR::safe_VkVideoEncodeCapabilitiesKHR(const safe_VkVideoEncodeCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlModes = copy_src.rateControlModes;
    maxRateControlLayers = copy_src.maxRateControlLayers;
    maxBitrate = copy_src.maxBitrate;
    maxQualityLevels = copy_src.maxQualityLevels;
    encodeInputPictureGranularity = copy_src.encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src.supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeCapabilitiesKHR& safe_VkVideoEncodeCapabilitiesKHR::operator=(const safe_VkVideoEncodeCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlModes = copy_src.rateControlModes;
    maxRateControlLayers = copy_src.maxRateControlLayers;
    maxBitrate = copy_src.maxBitrate;
    maxQualityLevels = copy_src.maxQualityLevels;
    encodeInputPictureGranularity = copy_src.encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src.supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeCapabilitiesKHR::~safe_VkVideoEncodeCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeCapabilitiesKHR::initialize(const VkVideoEncodeCapabilitiesKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    rateControlModes = in_struct->rateControlModes;
    maxRateControlLayers = in_struct->maxRateControlLayers;
    maxBitrate = in_struct->maxBitrate;
    maxQualityLevels = in_struct->maxQualityLevels;
    encodeInputPictureGranularity = in_struct->encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = in_struct->supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeCapabilitiesKHR::initialize(const safe_VkVideoEncodeCapabilitiesKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    rateControlModes = copy_src->rateControlModes;
    maxRateControlLayers = copy_src->maxRateControlLayers;
    maxBitrate = copy_src->maxBitrate;
    maxQualityLevels = copy_src->maxQualityLevels;
    encodeInputPictureGranularity = copy_src->encodeInputPictureGranularity;
    supportedEncodeFeedbackFlags = copy_src->supportedEncodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(
    const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), encodeFeedbackFlags(in_struct->encodeFeedbackFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR), pNext(nullptr), encodeFeedbackFlags() {}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(
    const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    encodeFeedbackFlags = copy_src.encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::operator=(
    const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    encodeFeedbackFlags = copy_src.encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::~safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::initialize(const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    encodeFeedbackFlags = in_struct->encodeFeedbackFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR::initialize(const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    encodeFeedbackFlags = copy_src->encodeFeedbackFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR(const VkVideoEncodeUsageInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      videoUsageHints(in_struct->videoUsageHints),
      videoContentHints(in_struct->videoContentHints),
      tuningMode(in_struct->tuningMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR), pNext(nullptr), videoUsageHints(), videoContentHints(), tuningMode() {}

safe_VkVideoEncodeUsageInfoKHR::safe_VkVideoEncodeUsageInfoKHR(const safe_VkVideoEncodeUsageInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    videoContentHints = copy_src.videoContentHints;
    tuningMode = copy_src.tuningMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeUsageInfoKHR& safe_VkVideoEncodeUsageInfoKHR::operator=(const safe_VkVideoEncodeUsageInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoUsageHints = copy_src.videoUsageHints;
    videoContentHints = copy_src.videoContentHints;
    tuningMode = copy_src.tuningMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeUsageInfoKHR::~safe_VkVideoEncodeUsageInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeUsageInfoKHR::initialize(const VkVideoEncodeUsageInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoUsageHints = in_struct->videoUsageHints;
    videoContentHints = in_struct->videoContentHints;
    tuningMode = in_struct->tuningMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeUsageInfoKHR::initialize(const safe_VkVideoEncodeUsageInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoUsageHints = copy_src->videoUsageHints;
    videoContentHints = copy_src->videoContentHints;
    tuningMode = copy_src->tuningMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR(
    const VkVideoEncodeRateControlLayerInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      averageBitrate(in_struct->averageBitrate),
      maxBitrate(in_struct->maxBitrate),
      frameRateNumerator(in_struct->frameRateNumerator),
      frameRateDenominator(in_struct->frameRateDenominator) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_LAYER_INFO_KHR),
      pNext(nullptr),
      averageBitrate(),
      maxBitrate(),
      frameRateNumerator(),
      frameRateDenominator() {}

safe_VkVideoEncodeRateControlLayerInfoKHR::safe_VkVideoEncodeRateControlLayerInfoKHR(
    const safe_VkVideoEncodeRateControlLayerInfoKHR& copy_src) {
    sType = copy_src.sType;
    averageBitrate = copy_src.averageBitrate;
    maxBitrate = copy_src.maxBitrate;
    frameRateNumerator = copy_src.frameRateNumerator;
    frameRateDenominator = copy_src.frameRateDenominator;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeRateControlLayerInfoKHR& safe_VkVideoEncodeRateControlLayerInfoKHR::operator=(
    const safe_VkVideoEncodeRateControlLayerInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    averageBitrate = copy_src.averageBitrate;
    maxBitrate = copy_src.maxBitrate;
    frameRateNumerator = copy_src.frameRateNumerator;
    frameRateDenominator = copy_src.frameRateDenominator;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeRateControlLayerInfoKHR::~safe_VkVideoEncodeRateControlLayerInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeRateControlLayerInfoKHR::initialize(const VkVideoEncodeRateControlLayerInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    averageBitrate = in_struct->averageBitrate;
    maxBitrate = in_struct->maxBitrate;
    frameRateNumerator = in_struct->frameRateNumerator;
    frameRateDenominator = in_struct->frameRateDenominator;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeRateControlLayerInfoKHR::initialize(const safe_VkVideoEncodeRateControlLayerInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    averageBitrate = copy_src->averageBitrate;
    maxBitrate = copy_src->maxBitrate;
    frameRateNumerator = copy_src->frameRateNumerator;
    frameRateDenominator = copy_src->frameRateDenominator;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR(const VkVideoEncodeRateControlInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      rateControlMode(in_struct->rateControlMode),
      layerCount(in_struct->layerCount),
      pLayers(nullptr),
      virtualBufferSizeInMs(in_struct->virtualBufferSizeInMs),
      initialVirtualBufferSizeInMs(in_struct->initialVirtualBufferSizeInMs) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (layerCount && in_struct->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&in_struct->pLayers[i]);
        }
    }
}

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR),
      pNext(nullptr),
      flags(),
      rateControlMode(),
      layerCount(),
      pLayers(nullptr),
      virtualBufferSizeInMs(),
      initialVirtualBufferSizeInMs() {}

safe_VkVideoEncodeRateControlInfoKHR::safe_VkVideoEncodeRateControlInfoKHR(const safe_VkVideoEncodeRateControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlMode = copy_src.rateControlMode;
    layerCount = copy_src.layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src.virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src.initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layerCount && copy_src.pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src.pLayers[i]);
        }
    }
}

safe_VkVideoEncodeRateControlInfoKHR& safe_VkVideoEncodeRateControlInfoKHR::operator=(
    const safe_VkVideoEncodeRateControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    rateControlMode = copy_src.rateControlMode;
    layerCount = copy_src.layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src.virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src.initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layerCount && copy_src.pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src.pLayers[i]);
        }
    }

    return *this;
}

safe_VkVideoEncodeRateControlInfoKHR::~safe_VkVideoEncodeRateControlInfoKHR() {
    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeRateControlInfoKHR::initialize(const VkVideoEncodeRateControlInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLayers) delete[] pLayers;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    rateControlMode = in_struct->rateControlMode;
    layerCount = in_struct->layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = in_struct->virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = in_struct->initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (layerCount && in_struct->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&in_struct->pLayers[i]);
        }
    }
}

void safe_VkVideoEncodeRateControlInfoKHR::initialize(const safe_VkVideoEncodeRateControlInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    rateControlMode = copy_src->rateControlMode;
    layerCount = copy_src->layerCount;
    pLayers = nullptr;
    virtualBufferSizeInMs = copy_src->virtualBufferSizeInMs;
    initialVirtualBufferSizeInMs = copy_src->initialVirtualBufferSizeInMs;
    pNext = SafePnextCopy(copy_src->pNext);
    if (layerCount && copy_src->pLayers) {
        pLayers = new safe_VkVideoEncodeRateControlLayerInfoKHR[layerCount];
        for (uint32_t i = 0; i < layerCount; ++i) {
            pLayers[i].initialize(&copy_src->pLayers[i]);
        }
    }
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR(
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pVideoProfile(nullptr), qualityLevel(in_struct->qualityLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR),
      pNext(nullptr),
      pVideoProfile(nullptr),
      qualityLevel() {}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& copy_src) {
    sType = copy_src.sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::operator=(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src.pVideoProfile);

    return *this;
}

safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::~safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR() {
    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::initialize(
    const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pVideoProfile) delete pVideoProfile;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pVideoProfile = nullptr;
    qualityLevel = in_struct->qualityLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(in_struct->pVideoProfile);
}

void safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR::initialize(
    const safe_VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pVideoProfile = nullptr;
    qualityLevel = copy_src->qualityLevel;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pVideoProfile) pVideoProfile = new safe_VkVideoProfileInfoKHR(*copy_src->pVideoProfile);
}

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR(
    const VkVideoEncodeQualityLevelPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      preferredRateControlMode(in_struct->preferredRateControlMode),
      preferredRateControlLayerCount(in_struct->preferredRateControlLayerCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_PROPERTIES_KHR),
      pNext(nullptr),
      preferredRateControlMode(),
      preferredRateControlLayerCount() {}

safe_VkVideoEncodeQualityLevelPropertiesKHR::safe_VkVideoEncodeQualityLevelPropertiesKHR(
    const safe_VkVideoEncodeQualityLevelPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    preferredRateControlMode = copy_src.preferredRateControlMode;
    preferredRateControlLayerCount = copy_src.preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQualityLevelPropertiesKHR& safe_VkVideoEncodeQualityLevelPropertiesKHR::operator=(
    const safe_VkVideoEncodeQualityLevelPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    preferredRateControlMode = copy_src.preferredRateControlMode;
    preferredRateControlLayerCount = copy_src.preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQualityLevelPropertiesKHR::~safe_VkVideoEncodeQualityLevelPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQualityLevelPropertiesKHR::initialize(const VkVideoEncodeQualityLevelPropertiesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    preferredRateControlMode = in_struct->preferredRateControlMode;
    preferredRateControlLayerCount = in_struct->preferredRateControlLayerCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQualityLevelPropertiesKHR::initialize(const safe_VkVideoEncodeQualityLevelPropertiesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    preferredRateControlMode = copy_src->preferredRateControlMode;
    preferredRateControlLayerCount = copy_src->preferredRateControlLayerCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR(const VkVideoEncodeQualityLevelInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType), qualityLevel(in_struct->qualityLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR), pNext(nullptr), qualityLevel() {}

safe_VkVideoEncodeQualityLevelInfoKHR::safe_VkVideoEncodeQualityLevelInfoKHR(
    const safe_VkVideoEncodeQualityLevelInfoKHR& copy_src) {
    sType = copy_src.sType;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQualityLevelInfoKHR& safe_VkVideoEncodeQualityLevelInfoKHR::operator=(
    const safe_VkVideoEncodeQualityLevelInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    qualityLevel = copy_src.qualityLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQualityLevelInfoKHR::~safe_VkVideoEncodeQualityLevelInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQualityLevelInfoKHR::initialize(const VkVideoEncodeQualityLevelInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    qualityLevel = in_struct->qualityLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQualityLevelInfoKHR::initialize(const safe_VkVideoEncodeQualityLevelInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    qualityLevel = copy_src->qualityLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR(
    const VkVideoEncodeSessionParametersGetInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoSessionParameters(in_struct->videoSessionParameters) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_GET_INFO_KHR), pNext(nullptr), videoSessionParameters() {}

safe_VkVideoEncodeSessionParametersGetInfoKHR::safe_VkVideoEncodeSessionParametersGetInfoKHR(
    const safe_VkVideoEncodeSessionParametersGetInfoKHR& copy_src) {
    sType = copy_src.sType;
    videoSessionParameters = copy_src.videoSessionParameters;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeSessionParametersGetInfoKHR& safe_VkVideoEncodeSessionParametersGetInfoKHR::operator=(
    const safe_VkVideoEncodeSessionParametersGetInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoSessionParameters = copy_src.videoSessionParameters;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeSessionParametersGetInfoKHR::~safe_VkVideoEncodeSessionParametersGetInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeSessionParametersGetInfoKHR::initialize(const VkVideoEncodeSessionParametersGetInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoSessionParameters = in_struct->videoSessionParameters;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeSessionParametersGetInfoKHR::initialize(const safe_VkVideoEncodeSessionParametersGetInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoSessionParameters = copy_src->videoSessionParameters;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR(
    const VkVideoEncodeSessionParametersFeedbackInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), hasOverrides(in_struct->hasOverrides) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_PARAMETERS_FEEDBACK_INFO_KHR), pNext(nullptr), hasOverrides() {}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::safe_VkVideoEncodeSessionParametersFeedbackInfoKHR(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& copy_src) {
    sType = copy_src.sType;
    hasOverrides = copy_src.hasOverrides;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::operator=(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hasOverrides = copy_src.hasOverrides;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::~safe_VkVideoEncodeSessionParametersFeedbackInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::initialize(const VkVideoEncodeSessionParametersFeedbackInfoKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hasOverrides = in_struct->hasOverrides;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeSessionParametersFeedbackInfoKHR::initialize(
    const safe_VkVideoEncodeSessionParametersFeedbackInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hasOverrides = copy_src->hasOverrides;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(
    const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentShaderBarycentric(in_struct->fragmentShaderBarycentric) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR),
      pNext(nullptr),
      fragmentShaderBarycentric() {}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    fragmentShaderBarycentric = copy_src.fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentShaderBarycentric = copy_src.fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::~safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::initialize(
    const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentShaderBarycentric = in_struct->fragmentShaderBarycentric;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentShaderBarycentric = copy_src->fragmentShaderBarycentric;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(
    const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      triStripVertexOrderIndependentOfProvokingVertex(in_struct->triStripVertexOrderIndependentOfProvokingVertex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR),
      pNext(nullptr),
      triStripVertexOrderIndependentOfProvokingVertex() {}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src.triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src.triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::~safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::initialize(
    const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    triStripVertexOrderIndependentOfProvokingVertex = in_struct->triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    triStripVertexOrderIndependentOfProvokingVertex = copy_src->triStripVertexOrderIndependentOfProvokingVertex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(
    const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderSubgroupUniformControlFlow(in_struct->shaderSubgroupUniformControlFlow) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR),
      pNext(nullptr),
      shaderSubgroupUniformControlFlow() {}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderSubgroupUniformControlFlow = copy_src.shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR&
safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSubgroupUniformControlFlow = copy_src.shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::
    ~safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::initialize(
    const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSubgroupUniformControlFlow = in_struct->shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSubgroupUniformControlFlow = copy_src->shaderSubgroupUniformControlFlow;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(
    const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      workgroupMemoryExplicitLayout(in_struct->workgroupMemoryExplicitLayout),
      workgroupMemoryExplicitLayoutScalarBlockLayout(in_struct->workgroupMemoryExplicitLayoutScalarBlockLayout),
      workgroupMemoryExplicitLayout8BitAccess(in_struct->workgroupMemoryExplicitLayout8BitAccess),
      workgroupMemoryExplicitLayout16BitAccess(in_struct->workgroupMemoryExplicitLayout16BitAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR),
      pNext(nullptr),
      workgroupMemoryExplicitLayout(),
      workgroupMemoryExplicitLayoutScalarBlockLayout(),
      workgroupMemoryExplicitLayout8BitAccess(),
      workgroupMemoryExplicitLayout16BitAccess() {}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    workgroupMemoryExplicitLayout = copy_src.workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src.workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src.workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src.workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR&
safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    workgroupMemoryExplicitLayout = copy_src.workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src.workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src.workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src.workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::~safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::initialize(
    const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    workgroupMemoryExplicitLayout = in_struct->workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = in_struct->workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = in_struct->workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = in_struct->workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    workgroupMemoryExplicitLayout = copy_src->workgroupMemoryExplicitLayout;
    workgroupMemoryExplicitLayoutScalarBlockLayout = copy_src->workgroupMemoryExplicitLayoutScalarBlockLayout;
    workgroupMemoryExplicitLayout8BitAccess = copy_src->workgroupMemoryExplicitLayout8BitAccess;
    workgroupMemoryExplicitLayout16BitAccess = copy_src->workgroupMemoryExplicitLayout16BitAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(
    const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      rayTracingMaintenance1(in_struct->rayTracingMaintenance1),
      rayTracingPipelineTraceRaysIndirect2(in_struct->rayTracingPipelineTraceRaysIndirect2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR),
      pNext(nullptr),
      rayTracingMaintenance1(),
      rayTracingPipelineTraceRaysIndirect2() {}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingMaintenance1 = copy_src.rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src.rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingMaintenance1 = copy_src.rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src.rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::~safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::initialize(
    const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingMaintenance1 = in_struct->rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = in_struct->rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingMaintenance1 = copy_src->rayTracingMaintenance1;
    rayTracingPipelineTraceRaysIndirect2 = copy_src->rayTracingPipelineTraceRaysIndirect2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(
    const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderMaximalReconvergence(in_struct->shaderMaximalReconvergence) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MAXIMAL_RECONVERGENCE_FEATURES_KHR),
      pNext(nullptr),
      shaderMaximalReconvergence() {}

safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR(
    const safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderMaximalReconvergence = copy_src.shaderMaximalReconvergence;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR& safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderMaximalReconvergence = copy_src.shaderMaximalReconvergence;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::~safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::initialize(
    const VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderMaximalReconvergence = in_struct->shaderMaximalReconvergence;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderMaximalReconvergence = copy_src->shaderMaximalReconvergence;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilitiesPresentId2KHR::safe_VkSurfaceCapabilitiesPresentId2KHR(
    const VkSurfaceCapabilitiesPresentId2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentId2Supported(in_struct->presentId2Supported) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilitiesPresentId2KHR::safe_VkSurfaceCapabilitiesPresentId2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_ID_2_KHR), pNext(nullptr), presentId2Supported() {}

safe_VkSurfaceCapabilitiesPresentId2KHR::safe_VkSurfaceCapabilitiesPresentId2KHR(
    const safe_VkSurfaceCapabilitiesPresentId2KHR& copy_src) {
    sType = copy_src.sType;
    presentId2Supported = copy_src.presentId2Supported;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilitiesPresentId2KHR& safe_VkSurfaceCapabilitiesPresentId2KHR::operator=(
    const safe_VkSurfaceCapabilitiesPresentId2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentId2Supported = copy_src.presentId2Supported;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilitiesPresentId2KHR::~safe_VkSurfaceCapabilitiesPresentId2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilitiesPresentId2KHR::initialize(const VkSurfaceCapabilitiesPresentId2KHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentId2Supported = in_struct->presentId2Supported;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilitiesPresentId2KHR::initialize(const safe_VkSurfaceCapabilitiesPresentId2KHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentId2Supported = copy_src->presentId2Supported;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPresentId2KHR::safe_VkPresentId2KHR(const VkPresentId2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                           bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pPresentIds(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

safe_VkPresentId2KHR::safe_VkPresentId2KHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_ID_2_KHR), pNext(nullptr), swapchainCount(), pPresentIds(nullptr) {}

safe_VkPresentId2KHR::safe_VkPresentId2KHR(const safe_VkPresentId2KHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }
}

safe_VkPresentId2KHR& safe_VkPresentId2KHR::operator=(const safe_VkPresentId2KHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentIds) {
        pPresentIds = new uint64_t[copy_src.swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src.pPresentIds, sizeof(uint64_t) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentId2KHR::~safe_VkPresentId2KHR() {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
}

void safe_VkPresentId2KHR::initialize(const VkPresentId2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentIds) delete[] pPresentIds;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentIds) {
        pPresentIds = new uint64_t[in_struct->swapchainCount];
        memcpy((void*)pPresentIds, (void*)in_struct->pPresentIds, sizeof(uint64_t) * in_struct->swapchainCount);
    }
}

void safe_VkPresentId2KHR::initialize(const safe_VkPresentId2KHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pPresentIds = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentIds) {
        pPresentIds = new uint64_t[copy_src->swapchainCount];
        memcpy((void*)pPresentIds, (void*)copy_src->pPresentIds, sizeof(uint64_t) * copy_src->swapchainCount);
    }
}

safe_VkPhysicalDevicePresentId2FeaturesKHR::safe_VkPhysicalDevicePresentId2FeaturesKHR(
    const VkPhysicalDevicePresentId2FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentId2(in_struct->presentId2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentId2FeaturesKHR::safe_VkPhysicalDevicePresentId2FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_2_FEATURES_KHR), pNext(nullptr), presentId2() {}

safe_VkPhysicalDevicePresentId2FeaturesKHR::safe_VkPhysicalDevicePresentId2FeaturesKHR(
    const safe_VkPhysicalDevicePresentId2FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentId2 = copy_src.presentId2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentId2FeaturesKHR& safe_VkPhysicalDevicePresentId2FeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentId2FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentId2 = copy_src.presentId2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentId2FeaturesKHR::~safe_VkPhysicalDevicePresentId2FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentId2FeaturesKHR::initialize(const VkPhysicalDevicePresentId2FeaturesKHR* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentId2 = in_struct->presentId2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentId2FeaturesKHR::initialize(const safe_VkPhysicalDevicePresentId2FeaturesKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentId2 = copy_src->presentId2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilitiesPresentWait2KHR::safe_VkSurfaceCapabilitiesPresentWait2KHR(
    const VkSurfaceCapabilitiesPresentWait2KHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentWait2Supported(in_struct->presentWait2Supported) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilitiesPresentWait2KHR::safe_VkSurfaceCapabilitiesPresentWait2KHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_WAIT_2_KHR), pNext(nullptr), presentWait2Supported() {}

safe_VkSurfaceCapabilitiesPresentWait2KHR::safe_VkSurfaceCapabilitiesPresentWait2KHR(
    const safe_VkSurfaceCapabilitiesPresentWait2KHR& copy_src) {
    sType = copy_src.sType;
    presentWait2Supported = copy_src.presentWait2Supported;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilitiesPresentWait2KHR& safe_VkSurfaceCapabilitiesPresentWait2KHR::operator=(
    const safe_VkSurfaceCapabilitiesPresentWait2KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentWait2Supported = copy_src.presentWait2Supported;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilitiesPresentWait2KHR::~safe_VkSurfaceCapabilitiesPresentWait2KHR() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilitiesPresentWait2KHR::initialize(const VkSurfaceCapabilitiesPresentWait2KHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentWait2Supported = in_struct->presentWait2Supported;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilitiesPresentWait2KHR::initialize(const safe_VkSurfaceCapabilitiesPresentWait2KHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentWait2Supported = copy_src->presentWait2Supported;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePresentWait2FeaturesKHR::safe_VkPhysicalDevicePresentWait2FeaturesKHR(
    const VkPhysicalDevicePresentWait2FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentWait2(in_struct->presentWait2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentWait2FeaturesKHR::safe_VkPhysicalDevicePresentWait2FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_2_FEATURES_KHR), pNext(nullptr), presentWait2() {}

safe_VkPhysicalDevicePresentWait2FeaturesKHR::safe_VkPhysicalDevicePresentWait2FeaturesKHR(
    const safe_VkPhysicalDevicePresentWait2FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentWait2 = copy_src.presentWait2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentWait2FeaturesKHR& safe_VkPhysicalDevicePresentWait2FeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentWait2FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentWait2 = copy_src.presentWait2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentWait2FeaturesKHR::~safe_VkPhysicalDevicePresentWait2FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentWait2FeaturesKHR::initialize(const VkPhysicalDevicePresentWait2FeaturesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentWait2 = in_struct->presentWait2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentWait2FeaturesKHR::initialize(const safe_VkPhysicalDevicePresentWait2FeaturesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentWait2 = copy_src->presentWait2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPresentWait2InfoKHR::safe_VkPresentWait2InfoKHR(const VkPresentWait2InfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentId(in_struct->presentId), timeout(in_struct->timeout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPresentWait2InfoKHR::safe_VkPresentWait2InfoKHR()
    : sType(VK_STRUCTURE_TYPE_PRESENT_WAIT_2_INFO_KHR), pNext(nullptr), presentId(), timeout() {}

safe_VkPresentWait2InfoKHR::safe_VkPresentWait2InfoKHR(const safe_VkPresentWait2InfoKHR& copy_src) {
    sType = copy_src.sType;
    presentId = copy_src.presentId;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPresentWait2InfoKHR& safe_VkPresentWait2InfoKHR::operator=(const safe_VkPresentWait2InfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentId = copy_src.presentId;
    timeout = copy_src.timeout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPresentWait2InfoKHR::~safe_VkPresentWait2InfoKHR() { FreePnextChain(pNext); }

void safe_VkPresentWait2InfoKHR::initialize(const VkPresentWait2InfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentId = in_struct->presentId;
    timeout = in_struct->timeout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPresentWait2InfoKHR::initialize(const safe_VkPresentWait2InfoKHR* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentId = copy_src->presentId;
    timeout = copy_src->timeout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(
    const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), rayTracingPositionFetch(in_struct->rayTracingPositionFetch) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR), pNext(nullptr), rayTracingPositionFetch() {}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingPositionFetch = copy_src.rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingPositionFetch = copy_src.rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::~safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::initialize(
    const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingPositionFetch = in_struct->rayTracingPositionFetch;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingPositionFetch = copy_src->rayTracingPositionFetch;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::safe_VkPhysicalDevicePipelineBinaryFeaturesKHR(
    const VkPhysicalDevicePipelineBinaryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipelineBinaries(in_struct->pipelineBinaries) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::safe_VkPhysicalDevicePipelineBinaryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_FEATURES_KHR), pNext(nullptr), pipelineBinaries() {}

safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::safe_VkPhysicalDevicePipelineBinaryFeaturesKHR(
    const safe_VkPhysicalDevicePipelineBinaryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineBinaries = copy_src.pipelineBinaries;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineBinaryFeaturesKHR& safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePipelineBinaryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBinaries = copy_src.pipelineBinaries;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::~safe_VkPhysicalDevicePipelineBinaryFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::initialize(const VkPhysicalDevicePipelineBinaryFeaturesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBinaries = in_struct->pipelineBinaries;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineBinaryFeaturesKHR::initialize(const safe_VkPhysicalDevicePipelineBinaryFeaturesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBinaries = copy_src->pipelineBinaries;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::safe_VkPhysicalDevicePipelineBinaryPropertiesKHR(
    const VkPhysicalDevicePipelineBinaryPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pipelineBinaryInternalCache(in_struct->pipelineBinaryInternalCache),
      pipelineBinaryInternalCacheControl(in_struct->pipelineBinaryInternalCacheControl),
      pipelineBinaryPrefersInternalCache(in_struct->pipelineBinaryPrefersInternalCache),
      pipelineBinaryPrecompiledInternalCache(in_struct->pipelineBinaryPrecompiledInternalCache),
      pipelineBinaryCompressedData(in_struct->pipelineBinaryCompressedData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::safe_VkPhysicalDevicePipelineBinaryPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_BINARY_PROPERTIES_KHR),
      pNext(nullptr),
      pipelineBinaryInternalCache(),
      pipelineBinaryInternalCacheControl(),
      pipelineBinaryPrefersInternalCache(),
      pipelineBinaryPrecompiledInternalCache(),
      pipelineBinaryCompressedData() {}

safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::safe_VkPhysicalDevicePipelineBinaryPropertiesKHR(
    const safe_VkPhysicalDevicePipelineBinaryPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    pipelineBinaryInternalCache = copy_src.pipelineBinaryInternalCache;
    pipelineBinaryInternalCacheControl = copy_src.pipelineBinaryInternalCacheControl;
    pipelineBinaryPrefersInternalCache = copy_src.pipelineBinaryPrefersInternalCache;
    pipelineBinaryPrecompiledInternalCache = copy_src.pipelineBinaryPrecompiledInternalCache;
    pipelineBinaryCompressedData = copy_src.pipelineBinaryCompressedData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineBinaryPropertiesKHR& safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::operator=(
    const safe_VkPhysicalDevicePipelineBinaryPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBinaryInternalCache = copy_src.pipelineBinaryInternalCache;
    pipelineBinaryInternalCacheControl = copy_src.pipelineBinaryInternalCacheControl;
    pipelineBinaryPrefersInternalCache = copy_src.pipelineBinaryPrefersInternalCache;
    pipelineBinaryPrecompiledInternalCache = copy_src.pipelineBinaryPrecompiledInternalCache;
    pipelineBinaryCompressedData = copy_src.pipelineBinaryCompressedData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::~safe_VkPhysicalDevicePipelineBinaryPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::initialize(const VkPhysicalDevicePipelineBinaryPropertiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBinaryInternalCache = in_struct->pipelineBinaryInternalCache;
    pipelineBinaryInternalCacheControl = in_struct->pipelineBinaryInternalCacheControl;
    pipelineBinaryPrefersInternalCache = in_struct->pipelineBinaryPrefersInternalCache;
    pipelineBinaryPrecompiledInternalCache = in_struct->pipelineBinaryPrecompiledInternalCache;
    pipelineBinaryCompressedData = in_struct->pipelineBinaryCompressedData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineBinaryPropertiesKHR::initialize(const safe_VkPhysicalDevicePipelineBinaryPropertiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBinaryInternalCache = copy_src->pipelineBinaryInternalCache;
    pipelineBinaryInternalCacheControl = copy_src->pipelineBinaryInternalCacheControl;
    pipelineBinaryPrefersInternalCache = copy_src->pipelineBinaryPrefersInternalCache;
    pipelineBinaryPrecompiledInternalCache = copy_src->pipelineBinaryPrecompiledInternalCache;
    pipelineBinaryCompressedData = copy_src->pipelineBinaryCompressedData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDevicePipelineBinaryInternalCacheControlKHR::safe_VkDevicePipelineBinaryInternalCacheControlKHR(
    const VkDevicePipelineBinaryInternalCacheControlKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), disableInternalCache(in_struct->disableInternalCache) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDevicePipelineBinaryInternalCacheControlKHR::safe_VkDevicePipelineBinaryInternalCacheControlKHR()
    : sType(VK_STRUCTURE_TYPE_DEVICE_PIPELINE_BINARY_INTERNAL_CACHE_CONTROL_KHR), pNext(nullptr), disableInternalCache() {}

safe_VkDevicePipelineBinaryInternalCacheControlKHR::safe_VkDevicePipelineBinaryInternalCacheControlKHR(
    const safe_VkDevicePipelineBinaryInternalCacheControlKHR& copy_src) {
    sType = copy_src.sType;
    disableInternalCache = copy_src.disableInternalCache;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDevicePipelineBinaryInternalCacheControlKHR& safe_VkDevicePipelineBinaryInternalCacheControlKHR::operator=(
    const safe_VkDevicePipelineBinaryInternalCacheControlKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    disableInternalCache = copy_src.disableInternalCache;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDevicePipelineBinaryInternalCacheControlKHR::~safe_VkDevicePipelineBinaryInternalCacheControlKHR() { FreePnextChain(pNext); }

void safe_VkDevicePipelineBinaryInternalCacheControlKHR::initialize(const VkDevicePipelineBinaryInternalCacheControlKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    disableInternalCache = in_struct->disableInternalCache;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDevicePipelineBinaryInternalCacheControlKHR::initialize(
    const safe_VkDevicePipelineBinaryInternalCacheControlKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    disableInternalCache = copy_src->disableInternalCache;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineBinaryKeyKHR::safe_VkPipelineBinaryKeyKHR(const VkPipelineBinaryKeyKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), keySize(in_struct->keySize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR; ++i) {
        key[i] = in_struct->key[i];
    }
}

safe_VkPipelineBinaryKeyKHR::safe_VkPipelineBinaryKeyKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_BINARY_KEY_KHR), pNext(nullptr), keySize() {}

safe_VkPipelineBinaryKeyKHR::safe_VkPipelineBinaryKeyKHR(const safe_VkPipelineBinaryKeyKHR& copy_src) {
    sType = copy_src.sType;
    keySize = copy_src.keySize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR; ++i) {
        key[i] = copy_src.key[i];
    }
}

safe_VkPipelineBinaryKeyKHR& safe_VkPipelineBinaryKeyKHR::operator=(const safe_VkPipelineBinaryKeyKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    keySize = copy_src.keySize;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR; ++i) {
        key[i] = copy_src.key[i];
    }

    return *this;
}

safe_VkPipelineBinaryKeyKHR::~safe_VkPipelineBinaryKeyKHR() { FreePnextChain(pNext); }

void safe_VkPipelineBinaryKeyKHR::initialize(const VkPipelineBinaryKeyKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    keySize = in_struct->keySize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR; ++i) {
        key[i] = in_struct->key[i];
    }
}

void safe_VkPipelineBinaryKeyKHR::initialize(const safe_VkPipelineBinaryKeyKHR* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    keySize = copy_src->keySize;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_PIPELINE_BINARY_KEY_SIZE_KHR; ++i) {
        key[i] = copy_src->key[i];
    }
}

safe_VkPipelineBinaryDataKHR::safe_VkPipelineBinaryDataKHR(const VkPipelineBinaryDataKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state)
    : dataSize(in_struct->dataSize), pData(nullptr) {
    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

safe_VkPipelineBinaryDataKHR::safe_VkPipelineBinaryDataKHR() : dataSize(), pData(nullptr) {}

safe_VkPipelineBinaryDataKHR::safe_VkPipelineBinaryDataKHR(const safe_VkPipelineBinaryDataKHR& copy_src) {
    dataSize = copy_src.dataSize;

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }
}

safe_VkPipelineBinaryDataKHR& safe_VkPipelineBinaryDataKHR::operator=(const safe_VkPipelineBinaryDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }

    dataSize = copy_src.dataSize;

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }

    return *this;
}

safe_VkPipelineBinaryDataKHR::~safe_VkPipelineBinaryDataKHR() {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
}

void safe_VkPipelineBinaryDataKHR::initialize(const VkPipelineBinaryDataKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    dataSize = in_struct->dataSize;

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

void safe_VkPipelineBinaryDataKHR::initialize(const safe_VkPipelineBinaryDataKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    dataSize = copy_src->dataSize;

    if (copy_src->pData != nullptr) {
        auto temp = new std::byte[copy_src->dataSize];
        std::memcpy(temp, copy_src->pData, copy_src->dataSize);
        pData = temp;
    }
}

safe_VkPipelineBinaryKeysAndDataKHR::safe_VkPipelineBinaryKeysAndDataKHR(const VkPipelineBinaryKeysAndDataKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : binaryCount(in_struct->binaryCount), pPipelineBinaryKeys(nullptr), pPipelineBinaryData(nullptr) {
    if (binaryCount && in_struct->pPipelineBinaryKeys) {
        pPipelineBinaryKeys = new safe_VkPipelineBinaryKeyKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryKeys[i].initialize(&in_struct->pPipelineBinaryKeys[i]);
        }
    }
    if (binaryCount && in_struct->pPipelineBinaryData) {
        pPipelineBinaryData = new safe_VkPipelineBinaryDataKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryData[i].initialize(&in_struct->pPipelineBinaryData[i]);
        }
    }
}

safe_VkPipelineBinaryKeysAndDataKHR::safe_VkPipelineBinaryKeysAndDataKHR()
    : binaryCount(), pPipelineBinaryKeys(nullptr), pPipelineBinaryData(nullptr) {}

safe_VkPipelineBinaryKeysAndDataKHR::safe_VkPipelineBinaryKeysAndDataKHR(const safe_VkPipelineBinaryKeysAndDataKHR& copy_src) {
    binaryCount = copy_src.binaryCount;
    pPipelineBinaryKeys = nullptr;
    pPipelineBinaryData = nullptr;
    if (binaryCount && copy_src.pPipelineBinaryKeys) {
        pPipelineBinaryKeys = new safe_VkPipelineBinaryKeyKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryKeys[i].initialize(&copy_src.pPipelineBinaryKeys[i]);
        }
    }
    if (binaryCount && copy_src.pPipelineBinaryData) {
        pPipelineBinaryData = new safe_VkPipelineBinaryDataKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryData[i].initialize(&copy_src.pPipelineBinaryData[i]);
        }
    }
}

safe_VkPipelineBinaryKeysAndDataKHR& safe_VkPipelineBinaryKeysAndDataKHR::operator=(
    const safe_VkPipelineBinaryKeysAndDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPipelineBinaryKeys) delete[] pPipelineBinaryKeys;
    if (pPipelineBinaryData) delete[] pPipelineBinaryData;

    binaryCount = copy_src.binaryCount;
    pPipelineBinaryKeys = nullptr;
    pPipelineBinaryData = nullptr;
    if (binaryCount && copy_src.pPipelineBinaryKeys) {
        pPipelineBinaryKeys = new safe_VkPipelineBinaryKeyKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryKeys[i].initialize(&copy_src.pPipelineBinaryKeys[i]);
        }
    }
    if (binaryCount && copy_src.pPipelineBinaryData) {
        pPipelineBinaryData = new safe_VkPipelineBinaryDataKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryData[i].initialize(&copy_src.pPipelineBinaryData[i]);
        }
    }

    return *this;
}

safe_VkPipelineBinaryKeysAndDataKHR::~safe_VkPipelineBinaryKeysAndDataKHR() {
    if (pPipelineBinaryKeys) delete[] pPipelineBinaryKeys;
    if (pPipelineBinaryData) delete[] pPipelineBinaryData;
}

void safe_VkPipelineBinaryKeysAndDataKHR::initialize(const VkPipelineBinaryKeysAndDataKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPipelineBinaryKeys) delete[] pPipelineBinaryKeys;
    if (pPipelineBinaryData) delete[] pPipelineBinaryData;
    binaryCount = in_struct->binaryCount;
    pPipelineBinaryKeys = nullptr;
    pPipelineBinaryData = nullptr;
    if (binaryCount && in_struct->pPipelineBinaryKeys) {
        pPipelineBinaryKeys = new safe_VkPipelineBinaryKeyKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryKeys[i].initialize(&in_struct->pPipelineBinaryKeys[i]);
        }
    }
    if (binaryCount && in_struct->pPipelineBinaryData) {
        pPipelineBinaryData = new safe_VkPipelineBinaryDataKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryData[i].initialize(&in_struct->pPipelineBinaryData[i]);
        }
    }
}

void safe_VkPipelineBinaryKeysAndDataKHR::initialize(const safe_VkPipelineBinaryKeysAndDataKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    binaryCount = copy_src->binaryCount;
    pPipelineBinaryKeys = nullptr;
    pPipelineBinaryData = nullptr;
    if (binaryCount && copy_src->pPipelineBinaryKeys) {
        pPipelineBinaryKeys = new safe_VkPipelineBinaryKeyKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryKeys[i].initialize(&copy_src->pPipelineBinaryKeys[i]);
        }
    }
    if (binaryCount && copy_src->pPipelineBinaryData) {
        pPipelineBinaryData = new safe_VkPipelineBinaryDataKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaryData[i].initialize(&copy_src->pPipelineBinaryData[i]);
        }
    }
}

safe_VkPipelineCreateInfoKHR::safe_VkPipelineCreateInfoKHR(const VkPipelineCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineCreateInfoKHR::safe_VkPipelineCreateInfoKHR() : sType(VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR), pNext(nullptr) {}

safe_VkPipelineCreateInfoKHR::safe_VkPipelineCreateInfoKHR(const safe_VkPipelineCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineCreateInfoKHR& safe_VkPipelineCreateInfoKHR::operator=(const safe_VkPipelineCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineCreateInfoKHR::~safe_VkPipelineCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineCreateInfoKHR::initialize(const VkPipelineCreateInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineCreateInfoKHR::initialize(const safe_VkPipelineCreateInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineBinaryCreateInfoKHR::safe_VkPipelineBinaryCreateInfoKHR(const VkPipelineBinaryCreateInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pKeysAndDataInfo(nullptr), pipeline(in_struct->pipeline), pPipelineCreateInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pKeysAndDataInfo) pKeysAndDataInfo = new safe_VkPipelineBinaryKeysAndDataKHR(in_struct->pKeysAndDataInfo);
    if (in_struct->pPipelineCreateInfo) pPipelineCreateInfo = new safe_VkPipelineCreateInfoKHR(in_struct->pPipelineCreateInfo);
}

safe_VkPipelineBinaryCreateInfoKHR::safe_VkPipelineBinaryCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_BINARY_CREATE_INFO_KHR),
      pNext(nullptr),
      pKeysAndDataInfo(nullptr),
      pipeline(),
      pPipelineCreateInfo(nullptr) {}

safe_VkPipelineBinaryCreateInfoKHR::safe_VkPipelineBinaryCreateInfoKHR(const safe_VkPipelineBinaryCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    pKeysAndDataInfo = nullptr;
    pipeline = copy_src.pipeline;
    pPipelineCreateInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pKeysAndDataInfo) pKeysAndDataInfo = new safe_VkPipelineBinaryKeysAndDataKHR(*copy_src.pKeysAndDataInfo);
    if (copy_src.pPipelineCreateInfo) pPipelineCreateInfo = new safe_VkPipelineCreateInfoKHR(*copy_src.pPipelineCreateInfo);
}

safe_VkPipelineBinaryCreateInfoKHR& safe_VkPipelineBinaryCreateInfoKHR::operator=(
    const safe_VkPipelineBinaryCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pKeysAndDataInfo) delete pKeysAndDataInfo;
    if (pPipelineCreateInfo) delete pPipelineCreateInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pKeysAndDataInfo = nullptr;
    pipeline = copy_src.pipeline;
    pPipelineCreateInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pKeysAndDataInfo) pKeysAndDataInfo = new safe_VkPipelineBinaryKeysAndDataKHR(*copy_src.pKeysAndDataInfo);
    if (copy_src.pPipelineCreateInfo) pPipelineCreateInfo = new safe_VkPipelineCreateInfoKHR(*copy_src.pPipelineCreateInfo);

    return *this;
}

safe_VkPipelineBinaryCreateInfoKHR::~safe_VkPipelineBinaryCreateInfoKHR() {
    if (pKeysAndDataInfo) delete pKeysAndDataInfo;
    if (pPipelineCreateInfo) delete pPipelineCreateInfo;
    FreePnextChain(pNext);
}

void safe_VkPipelineBinaryCreateInfoKHR::initialize(const VkPipelineBinaryCreateInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pKeysAndDataInfo) delete pKeysAndDataInfo;
    if (pPipelineCreateInfo) delete pPipelineCreateInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pKeysAndDataInfo = nullptr;
    pipeline = in_struct->pipeline;
    pPipelineCreateInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pKeysAndDataInfo) pKeysAndDataInfo = new safe_VkPipelineBinaryKeysAndDataKHR(in_struct->pKeysAndDataInfo);
    if (in_struct->pPipelineCreateInfo) pPipelineCreateInfo = new safe_VkPipelineCreateInfoKHR(in_struct->pPipelineCreateInfo);
}

void safe_VkPipelineBinaryCreateInfoKHR::initialize(const safe_VkPipelineBinaryCreateInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pKeysAndDataInfo = nullptr;
    pipeline = copy_src->pipeline;
    pPipelineCreateInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pKeysAndDataInfo) pKeysAndDataInfo = new safe_VkPipelineBinaryKeysAndDataKHR(*copy_src->pKeysAndDataInfo);
    if (copy_src->pPipelineCreateInfo) pPipelineCreateInfo = new safe_VkPipelineCreateInfoKHR(*copy_src->pPipelineCreateInfo);
}

safe_VkPipelineBinaryInfoKHR::safe_VkPipelineBinaryInfoKHR(const VkPipelineBinaryInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), binaryCount(in_struct->binaryCount), pPipelineBinaries(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (binaryCount && in_struct->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaries[i] = in_struct->pPipelineBinaries[i];
        }
    }
}

safe_VkPipelineBinaryInfoKHR::safe_VkPipelineBinaryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_BINARY_INFO_KHR), pNext(nullptr), binaryCount(), pPipelineBinaries(nullptr) {}

safe_VkPipelineBinaryInfoKHR::safe_VkPipelineBinaryInfoKHR(const safe_VkPipelineBinaryInfoKHR& copy_src) {
    sType = copy_src.sType;
    binaryCount = copy_src.binaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (binaryCount && copy_src.pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaries[i] = copy_src.pPipelineBinaries[i];
        }
    }
}

safe_VkPipelineBinaryInfoKHR& safe_VkPipelineBinaryInfoKHR::operator=(const safe_VkPipelineBinaryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    binaryCount = copy_src.binaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (binaryCount && copy_src.pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaries[i] = copy_src.pPipelineBinaries[i];
        }
    }

    return *this;
}

safe_VkPipelineBinaryInfoKHR::~safe_VkPipelineBinaryInfoKHR() {
    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);
}

void safe_VkPipelineBinaryInfoKHR::initialize(const VkPipelineBinaryInfoKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    binaryCount = in_struct->binaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (binaryCount && in_struct->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaries[i] = in_struct->pPipelineBinaries[i];
        }
    }
}

void safe_VkPipelineBinaryInfoKHR::initialize(const safe_VkPipelineBinaryInfoKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    binaryCount = copy_src->binaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (binaryCount && copy_src->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[binaryCount];
        for (uint32_t i = 0; i < binaryCount; ++i) {
            pPipelineBinaries[i] = copy_src->pPipelineBinaries[i];
        }
    }
}

safe_VkReleaseCapturedPipelineDataInfoKHR::safe_VkReleaseCapturedPipelineDataInfoKHR(
    const VkReleaseCapturedPipelineDataInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkReleaseCapturedPipelineDataInfoKHR::safe_VkReleaseCapturedPipelineDataInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RELEASE_CAPTURED_PIPELINE_DATA_INFO_KHR), pNext(nullptr), pipeline() {}

safe_VkReleaseCapturedPipelineDataInfoKHR::safe_VkReleaseCapturedPipelineDataInfoKHR(
    const safe_VkReleaseCapturedPipelineDataInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkReleaseCapturedPipelineDataInfoKHR& safe_VkReleaseCapturedPipelineDataInfoKHR::operator=(
    const safe_VkReleaseCapturedPipelineDataInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkReleaseCapturedPipelineDataInfoKHR::~safe_VkReleaseCapturedPipelineDataInfoKHR() { FreePnextChain(pNext); }

void safe_VkReleaseCapturedPipelineDataInfoKHR::initialize(const VkReleaseCapturedPipelineDataInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkReleaseCapturedPipelineDataInfoKHR::initialize(const safe_VkReleaseCapturedPipelineDataInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineBinaryDataInfoKHR::safe_VkPipelineBinaryDataInfoKHR(const VkPipelineBinaryDataInfoKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipelineBinary(in_struct->pipelineBinary) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineBinaryDataInfoKHR::safe_VkPipelineBinaryDataInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_BINARY_DATA_INFO_KHR), pNext(nullptr), pipelineBinary() {}

safe_VkPipelineBinaryDataInfoKHR::safe_VkPipelineBinaryDataInfoKHR(const safe_VkPipelineBinaryDataInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipelineBinary = copy_src.pipelineBinary;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineBinaryDataInfoKHR& safe_VkPipelineBinaryDataInfoKHR::operator=(const safe_VkPipelineBinaryDataInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBinary = copy_src.pipelineBinary;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineBinaryDataInfoKHR::~safe_VkPipelineBinaryDataInfoKHR() { FreePnextChain(pNext); }

void safe_VkPipelineBinaryDataInfoKHR::initialize(const VkPipelineBinaryDataInfoKHR* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBinary = in_struct->pipelineBinary;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineBinaryDataInfoKHR::initialize(const safe_VkPipelineBinaryDataInfoKHR* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBinary = copy_src->pipelineBinary;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineBinaryHandlesInfoKHR::safe_VkPipelineBinaryHandlesInfoKHR(const VkPipelineBinaryHandlesInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), pipelineBinaryCount(in_struct->pipelineBinaryCount), pPipelineBinaries(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (pipelineBinaryCount && in_struct->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[pipelineBinaryCount];
        for (uint32_t i = 0; i < pipelineBinaryCount; ++i) {
            pPipelineBinaries[i] = in_struct->pPipelineBinaries[i];
        }
    }
}

safe_VkPipelineBinaryHandlesInfoKHR::safe_VkPipelineBinaryHandlesInfoKHR()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_BINARY_HANDLES_INFO_KHR),
      pNext(nullptr),
      pipelineBinaryCount(),
      pPipelineBinaries(nullptr) {}

safe_VkPipelineBinaryHandlesInfoKHR::safe_VkPipelineBinaryHandlesInfoKHR(const safe_VkPipelineBinaryHandlesInfoKHR& copy_src) {
    sType = copy_src.sType;
    pipelineBinaryCount = copy_src.pipelineBinaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (pipelineBinaryCount && copy_src.pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[pipelineBinaryCount];
        for (uint32_t i = 0; i < pipelineBinaryCount; ++i) {
            pPipelineBinaries[i] = copy_src.pPipelineBinaries[i];
        }
    }
}

safe_VkPipelineBinaryHandlesInfoKHR& safe_VkPipelineBinaryHandlesInfoKHR::operator=(
    const safe_VkPipelineBinaryHandlesInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBinaryCount = copy_src.pipelineBinaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (pipelineBinaryCount && copy_src.pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[pipelineBinaryCount];
        for (uint32_t i = 0; i < pipelineBinaryCount; ++i) {
            pPipelineBinaries[i] = copy_src.pPipelineBinaries[i];
        }
    }

    return *this;
}

safe_VkPipelineBinaryHandlesInfoKHR::~safe_VkPipelineBinaryHandlesInfoKHR() {
    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);
}

void safe_VkPipelineBinaryHandlesInfoKHR::initialize(const VkPipelineBinaryHandlesInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPipelineBinaries) delete[] pPipelineBinaries;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBinaryCount = in_struct->pipelineBinaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (pipelineBinaryCount && in_struct->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[pipelineBinaryCount];
        for (uint32_t i = 0; i < pipelineBinaryCount; ++i) {
            pPipelineBinaries[i] = in_struct->pPipelineBinaries[i];
        }
    }
}

void safe_VkPipelineBinaryHandlesInfoKHR::initialize(const safe_VkPipelineBinaryHandlesInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBinaryCount = copy_src->pipelineBinaryCount;
    pPipelineBinaries = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (pipelineBinaryCount && copy_src->pPipelineBinaries) {
        pPipelineBinaries = new VkPipelineBinaryKHR[pipelineBinaryCount];
        for (uint32_t i = 0; i < pipelineBinaryCount; ++i) {
            pPipelineBinaries[i] = copy_src->pPipelineBinaries[i];
        }
    }
}

safe_VkSurfacePresentModeKHR::safe_VkSurfacePresentModeKHR(const VkSurfacePresentModeKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentMode(in_struct->presentMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfacePresentModeKHR::safe_VkSurfacePresentModeKHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_KHR), pNext(nullptr), presentMode() {}

safe_VkSurfacePresentModeKHR::safe_VkSurfacePresentModeKHR(const safe_VkSurfacePresentModeKHR& copy_src) {
    sType = copy_src.sType;
    presentMode = copy_src.presentMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfacePresentModeKHR& safe_VkSurfacePresentModeKHR::operator=(const safe_VkSurfacePresentModeKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentMode = copy_src.presentMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfacePresentModeKHR::~safe_VkSurfacePresentModeKHR() { FreePnextChain(pNext); }

void safe_VkSurfacePresentModeKHR::initialize(const VkSurfacePresentModeKHR* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentMode = in_struct->presentMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfacePresentModeKHR::initialize(const safe_VkSurfacePresentModeKHR* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentMode = copy_src->presentMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfacePresentScalingCapabilitiesKHR::safe_VkSurfacePresentScalingCapabilitiesKHR(
    const VkSurfacePresentScalingCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      supportedPresentScaling(in_struct->supportedPresentScaling),
      supportedPresentGravityX(in_struct->supportedPresentGravityX),
      supportedPresentGravityY(in_struct->supportedPresentGravityY),
      minScaledImageExtent(in_struct->minScaledImageExtent),
      maxScaledImageExtent(in_struct->maxScaledImageExtent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfacePresentScalingCapabilitiesKHR::safe_VkSurfacePresentScalingCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_KHR),
      pNext(nullptr),
      supportedPresentScaling(),
      supportedPresentGravityX(),
      supportedPresentGravityY(),
      minScaledImageExtent(),
      maxScaledImageExtent() {}

safe_VkSurfacePresentScalingCapabilitiesKHR::safe_VkSurfacePresentScalingCapabilitiesKHR(
    const safe_VkSurfacePresentScalingCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    supportedPresentScaling = copy_src.supportedPresentScaling;
    supportedPresentGravityX = copy_src.supportedPresentGravityX;
    supportedPresentGravityY = copy_src.supportedPresentGravityY;
    minScaledImageExtent = copy_src.minScaledImageExtent;
    maxScaledImageExtent = copy_src.maxScaledImageExtent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfacePresentScalingCapabilitiesKHR& safe_VkSurfacePresentScalingCapabilitiesKHR::operator=(
    const safe_VkSurfacePresentScalingCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportedPresentScaling = copy_src.supportedPresentScaling;
    supportedPresentGravityX = copy_src.supportedPresentGravityX;
    supportedPresentGravityY = copy_src.supportedPresentGravityY;
    minScaledImageExtent = copy_src.minScaledImageExtent;
    maxScaledImageExtent = copy_src.maxScaledImageExtent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfacePresentScalingCapabilitiesKHR::~safe_VkSurfacePresentScalingCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkSurfacePresentScalingCapabilitiesKHR::initialize(const VkSurfacePresentScalingCapabilitiesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportedPresentScaling = in_struct->supportedPresentScaling;
    supportedPresentGravityX = in_struct->supportedPresentGravityX;
    supportedPresentGravityY = in_struct->supportedPresentGravityY;
    minScaledImageExtent = in_struct->minScaledImageExtent;
    maxScaledImageExtent = in_struct->maxScaledImageExtent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfacePresentScalingCapabilitiesKHR::initialize(const safe_VkSurfacePresentScalingCapabilitiesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportedPresentScaling = copy_src->supportedPresentScaling;
    supportedPresentGravityX = copy_src->supportedPresentGravityX;
    supportedPresentGravityY = copy_src->supportedPresentGravityY;
    minScaledImageExtent = copy_src->minScaledImageExtent;
    maxScaledImageExtent = copy_src->maxScaledImageExtent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfacePresentModeCompatibilityKHR::safe_VkSurfacePresentModeCompatibilityKHR(
    const VkSurfacePresentModeCompatibilityKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentModeCount(in_struct->presentModeCount), pPresentModes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

safe_VkSurfacePresentModeCompatibilityKHR::safe_VkSurfacePresentModeCompatibilityKHR()
    : sType(VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_KHR), pNext(nullptr), presentModeCount(), pPresentModes(nullptr) {}

safe_VkSurfacePresentModeCompatibilityKHR::safe_VkSurfacePresentModeCompatibilityKHR(
    const safe_VkSurfacePresentModeCompatibilityKHR& copy_src) {
    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }
}

safe_VkSurfacePresentModeCompatibilityKHR& safe_VkSurfacePresentModeCompatibilityKHR::operator=(
    const safe_VkSurfacePresentModeCompatibilityKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }

    return *this;
}

safe_VkSurfacePresentModeCompatibilityKHR::~safe_VkSurfacePresentModeCompatibilityKHR() {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
}

void safe_VkSurfacePresentModeCompatibilityKHR::initialize(const VkSurfacePresentModeCompatibilityKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentModeCount = in_struct->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

void safe_VkSurfacePresentModeCompatibilityKHR::initialize(const safe_VkSurfacePresentModeCompatibilityKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentModeCount = copy_src->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src->presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src->pPresentModes, sizeof(VkPresentModeKHR) * copy_src->presentModeCount);
    }
}

safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR(
    const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainMaintenance1(in_struct->swapchainMaintenance1) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_KHR), pNext(nullptr), swapchainMaintenance1() {}

safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR(
    const safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    swapchainMaintenance1 = copy_src.swapchainMaintenance1;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR& safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainMaintenance1 = copy_src.swapchainMaintenance1;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::~safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::initialize(
    const VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainMaintenance1 = in_struct->swapchainMaintenance1;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR::initialize(
    const safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainMaintenance1 = copy_src->swapchainMaintenance1;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSwapchainPresentFenceInfoKHR::safe_VkSwapchainPresentFenceInfoKHR(const VkSwapchainPresentFenceInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pFences(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (swapchainCount && in_struct->pFences) {
        pFences = new VkFence[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pFences[i] = in_struct->pFences[i];
        }
    }
}

safe_VkSwapchainPresentFenceInfoKHR::safe_VkSwapchainPresentFenceInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_KHR), pNext(nullptr), swapchainCount(), pFences(nullptr) {}

safe_VkSwapchainPresentFenceInfoKHR::safe_VkSwapchainPresentFenceInfoKHR(const safe_VkSwapchainPresentFenceInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pFences = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pFences) {
        pFences = new VkFence[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pFences[i] = copy_src.pFences[i];
        }
    }
}

safe_VkSwapchainPresentFenceInfoKHR& safe_VkSwapchainPresentFenceInfoKHR::operator=(
    const safe_VkSwapchainPresentFenceInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pFences) delete[] pFences;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pFences = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (swapchainCount && copy_src.pFences) {
        pFences = new VkFence[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pFences[i] = copy_src.pFences[i];
        }
    }

    return *this;
}

safe_VkSwapchainPresentFenceInfoKHR::~safe_VkSwapchainPresentFenceInfoKHR() {
    if (pFences) delete[] pFences;
    FreePnextChain(pNext);
}

void safe_VkSwapchainPresentFenceInfoKHR::initialize(const VkSwapchainPresentFenceInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFences) delete[] pFences;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pFences = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (swapchainCount && in_struct->pFences) {
        pFences = new VkFence[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pFences[i] = in_struct->pFences[i];
        }
    }
}

void safe_VkSwapchainPresentFenceInfoKHR::initialize(const safe_VkSwapchainPresentFenceInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pFences = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (swapchainCount && copy_src->pFences) {
        pFences = new VkFence[swapchainCount];
        for (uint32_t i = 0; i < swapchainCount; ++i) {
            pFences[i] = copy_src->pFences[i];
        }
    }
}

safe_VkSwapchainPresentModesCreateInfoKHR::safe_VkSwapchainPresentModesCreateInfoKHR(
    const VkSwapchainPresentModesCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentModeCount(in_struct->presentModeCount), pPresentModes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

safe_VkSwapchainPresentModesCreateInfoKHR::safe_VkSwapchainPresentModesCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_KHR),
      pNext(nullptr),
      presentModeCount(),
      pPresentModes(nullptr) {}

safe_VkSwapchainPresentModesCreateInfoKHR::safe_VkSwapchainPresentModesCreateInfoKHR(
    const safe_VkSwapchainPresentModesCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }
}

safe_VkSwapchainPresentModesCreateInfoKHR& safe_VkSwapchainPresentModesCreateInfoKHR::operator=(
    const safe_VkSwapchainPresentModesCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }

    return *this;
}

safe_VkSwapchainPresentModesCreateInfoKHR::~safe_VkSwapchainPresentModesCreateInfoKHR() {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
}

void safe_VkSwapchainPresentModesCreateInfoKHR::initialize(const VkSwapchainPresentModesCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentModeCount = in_struct->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

void safe_VkSwapchainPresentModesCreateInfoKHR::initialize(const safe_VkSwapchainPresentModesCreateInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentModeCount = copy_src->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src->presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src->pPresentModes, sizeof(VkPresentModeKHR) * copy_src->presentModeCount);
    }
}

safe_VkSwapchainPresentModeInfoKHR::safe_VkSwapchainPresentModeInfoKHR(const VkSwapchainPresentModeInfoKHR* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pPresentModes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->swapchainCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->swapchainCount);
    }
}

safe_VkSwapchainPresentModeInfoKHR::safe_VkSwapchainPresentModeInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_KHR), pNext(nullptr), swapchainCount(), pPresentModes(nullptr) {}

safe_VkSwapchainPresentModeInfoKHR::safe_VkSwapchainPresentModeInfoKHR(const safe_VkSwapchainPresentModeInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.swapchainCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.swapchainCount);
    }
}

safe_VkSwapchainPresentModeInfoKHR& safe_VkSwapchainPresentModeInfoKHR::operator=(
    const safe_VkSwapchainPresentModeInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.swapchainCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkSwapchainPresentModeInfoKHR::~safe_VkSwapchainPresentModeInfoKHR() {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
}

void safe_VkSwapchainPresentModeInfoKHR::initialize(const VkSwapchainPresentModeInfoKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->swapchainCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->swapchainCount);
    }
}

void safe_VkSwapchainPresentModeInfoKHR::initialize(const safe_VkSwapchainPresentModeInfoKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src->swapchainCount];
        memcpy((void*)pPresentModes, (void*)copy_src->pPresentModes, sizeof(VkPresentModeKHR) * copy_src->swapchainCount);
    }
}

safe_VkSwapchainPresentScalingCreateInfoKHR::safe_VkSwapchainPresentScalingCreateInfoKHR(
    const VkSwapchainPresentScalingCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      scalingBehavior(in_struct->scalingBehavior),
      presentGravityX(in_struct->presentGravityX),
      presentGravityY(in_struct->presentGravityY) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSwapchainPresentScalingCreateInfoKHR::safe_VkSwapchainPresentScalingCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_KHR),
      pNext(nullptr),
      scalingBehavior(),
      presentGravityX(),
      presentGravityY() {}

safe_VkSwapchainPresentScalingCreateInfoKHR::safe_VkSwapchainPresentScalingCreateInfoKHR(
    const safe_VkSwapchainPresentScalingCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    scalingBehavior = copy_src.scalingBehavior;
    presentGravityX = copy_src.presentGravityX;
    presentGravityY = copy_src.presentGravityY;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSwapchainPresentScalingCreateInfoKHR& safe_VkSwapchainPresentScalingCreateInfoKHR::operator=(
    const safe_VkSwapchainPresentScalingCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    scalingBehavior = copy_src.scalingBehavior;
    presentGravityX = copy_src.presentGravityX;
    presentGravityY = copy_src.presentGravityY;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSwapchainPresentScalingCreateInfoKHR::~safe_VkSwapchainPresentScalingCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkSwapchainPresentScalingCreateInfoKHR::initialize(const VkSwapchainPresentScalingCreateInfoKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    scalingBehavior = in_struct->scalingBehavior;
    presentGravityX = in_struct->presentGravityX;
    presentGravityY = in_struct->presentGravityY;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSwapchainPresentScalingCreateInfoKHR::initialize(const safe_VkSwapchainPresentScalingCreateInfoKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    scalingBehavior = copy_src->scalingBehavior;
    presentGravityX = copy_src->presentGravityX;
    presentGravityY = copy_src->presentGravityY;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkReleaseSwapchainImagesInfoKHR::safe_VkReleaseSwapchainImagesInfoKHR(const VkReleaseSwapchainImagesInfoKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      swapchain(in_struct->swapchain),
      imageIndexCount(in_struct->imageIndexCount),
      pImageIndices(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->imageIndexCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->imageIndexCount);
    }
}

safe_VkReleaseSwapchainImagesInfoKHR::safe_VkReleaseSwapchainImagesInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RELEASE_SWAPCHAIN_IMAGES_INFO_KHR),
      pNext(nullptr),
      swapchain(),
      imageIndexCount(),
      pImageIndices(nullptr) {}

safe_VkReleaseSwapchainImagesInfoKHR::safe_VkReleaseSwapchainImagesInfoKHR(const safe_VkReleaseSwapchainImagesInfoKHR& copy_src) {
    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndexCount = copy_src.imageIndexCount;
    pImageIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.imageIndexCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.imageIndexCount);
    }
}

safe_VkReleaseSwapchainImagesInfoKHR& safe_VkReleaseSwapchainImagesInfoKHR::operator=(
    const safe_VkReleaseSwapchainImagesInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pImageIndices) delete[] pImageIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchain = copy_src.swapchain;
    imageIndexCount = copy_src.imageIndexCount;
    pImageIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pImageIndices) {
        pImageIndices = new uint32_t[copy_src.imageIndexCount];
        memcpy((void*)pImageIndices, (void*)copy_src.pImageIndices, sizeof(uint32_t) * copy_src.imageIndexCount);
    }

    return *this;
}

safe_VkReleaseSwapchainImagesInfoKHR::~safe_VkReleaseSwapchainImagesInfoKHR() {
    if (pImageIndices) delete[] pImageIndices;
    FreePnextChain(pNext);
}

void safe_VkReleaseSwapchainImagesInfoKHR::initialize(const VkReleaseSwapchainImagesInfoKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pImageIndices) delete[] pImageIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchain = in_struct->swapchain;
    imageIndexCount = in_struct->imageIndexCount;
    pImageIndices = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pImageIndices) {
        pImageIndices = new uint32_t[in_struct->imageIndexCount];
        memcpy((void*)pImageIndices, (void*)in_struct->pImageIndices, sizeof(uint32_t) * in_struct->imageIndexCount);
    }
}

void safe_VkReleaseSwapchainImagesInfoKHR::initialize(const safe_VkReleaseSwapchainImagesInfoKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchain = copy_src->swapchain;
    imageIndexCount = copy_src->imageIndexCount;
    pImageIndices = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pImageIndices) {
        pImageIndices = new uint32_t[copy_src->imageIndexCount];
        memcpy((void*)pImageIndices, (void*)copy_src->pImageIndices, sizeof(uint32_t) * copy_src->imageIndexCount);
    }
}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR(const VkCooperativeMatrixPropertiesKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      MSize(in_struct->MSize),
      NSize(in_struct->NSize),
      KSize(in_struct->KSize),
      AType(in_struct->AType),
      BType(in_struct->BType),
      CType(in_struct->CType),
      ResultType(in_struct->ResultType),
      saturatingAccumulation(in_struct->saturatingAccumulation),
      scope(in_struct->scope) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_KHR),
      pNext(nullptr),
      MSize(),
      NSize(),
      KSize(),
      AType(),
      BType(),
      CType(),
      ResultType(),
      saturatingAccumulation(),
      scope() {}

safe_VkCooperativeMatrixPropertiesKHR::safe_VkCooperativeMatrixPropertiesKHR(
    const safe_VkCooperativeMatrixPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCooperativeMatrixPropertiesKHR& safe_VkCooperativeMatrixPropertiesKHR::operator=(
    const safe_VkCooperativeMatrixPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCooperativeMatrixPropertiesKHR::~safe_VkCooperativeMatrixPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkCooperativeMatrixPropertiesKHR::initialize(const VkCooperativeMatrixPropertiesKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    MSize = in_struct->MSize;
    NSize = in_struct->NSize;
    KSize = in_struct->KSize;
    AType = in_struct->AType;
    BType = in_struct->BType;
    CType = in_struct->CType;
    ResultType = in_struct->ResultType;
    saturatingAccumulation = in_struct->saturatingAccumulation;
    scope = in_struct->scope;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCooperativeMatrixPropertiesKHR::initialize(const safe_VkCooperativeMatrixPropertiesKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    MSize = copy_src->MSize;
    NSize = copy_src->NSize;
    KSize = copy_src->KSize;
    AType = copy_src->AType;
    BType = copy_src->BType;
    CType = copy_src->CType;
    ResultType = copy_src->ResultType;
    saturatingAccumulation = copy_src->saturatingAccumulation;
    scope = copy_src->scope;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(
    const VkPhysicalDeviceCooperativeMatrixFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeMatrix(in_struct->cooperativeMatrix),
      cooperativeMatrixRobustBufferAccess(in_struct->cooperativeMatrixRobustBufferAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR),
      pNext(nullptr),
      cooperativeMatrix(),
      cooperativeMatrixRobustBufferAccess() {}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::~safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::initialize(const VkPhysicalDeviceCooperativeMatrixFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrix = in_struct->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = in_struct->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrix = copy_src->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(
    const VkPhysicalDeviceCooperativeMatrixPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cooperativeMatrixSupportedStages(in_struct->cooperativeMatrixSupportedStages) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR),
      pNext(nullptr),
      cooperativeMatrixSupportedStages() {}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::~safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::initialize(
    const VkPhysicalDeviceCooperativeMatrixPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrixSupportedStages = in_struct->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrixSupportedStages = copy_src->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(
    const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      computeDerivativeGroupQuads(in_struct->computeDerivativeGroupQuads),
      computeDerivativeGroupLinear(in_struct->computeDerivativeGroupLinear) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR),
      pNext(nullptr),
      computeDerivativeGroupQuads(),
      computeDerivativeGroupLinear() {}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR(
    const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    computeDerivativeGroupQuads = copy_src.computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = copy_src.computeDerivativeGroupLinear;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR& safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    computeDerivativeGroupQuads = copy_src.computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = copy_src.computeDerivativeGroupLinear;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::~safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::initialize(
    const VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    computeDerivativeGroupQuads = in_struct->computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = in_struct->computeDerivativeGroupLinear;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    computeDerivativeGroupQuads = copy_src->computeDerivativeGroupQuads;
    computeDerivativeGroupLinear = copy_src->computeDerivativeGroupLinear;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(
    const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), meshAndTaskShaderDerivatives(in_struct->meshAndTaskShaderDerivatives) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR),
      pNext(nullptr),
      meshAndTaskShaderDerivatives() {}

safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR(
    const safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    meshAndTaskShaderDerivatives = copy_src.meshAndTaskShaderDerivatives;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR& safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    meshAndTaskShaderDerivatives = copy_src.meshAndTaskShaderDerivatives;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::~safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::initialize(
    const VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    meshAndTaskShaderDerivatives = in_struct->meshAndTaskShaderDerivatives;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    meshAndTaskShaderDerivatives = copy_src->meshAndTaskShaderDerivatives;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeAV1ProfileInfoKHR::safe_VkVideoDecodeAV1ProfileInfoKHR(const VkVideoDecodeAV1ProfileInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), stdProfile(in_struct->stdProfile), filmGrainSupport(in_struct->filmGrainSupport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeAV1ProfileInfoKHR::safe_VkVideoDecodeAV1ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PROFILE_INFO_KHR), pNext(nullptr), stdProfile(), filmGrainSupport() {}

safe_VkVideoDecodeAV1ProfileInfoKHR::safe_VkVideoDecodeAV1ProfileInfoKHR(const safe_VkVideoDecodeAV1ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    filmGrainSupport = copy_src.filmGrainSupport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeAV1ProfileInfoKHR& safe_VkVideoDecodeAV1ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeAV1ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    filmGrainSupport = copy_src.filmGrainSupport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeAV1ProfileInfoKHR::~safe_VkVideoDecodeAV1ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeAV1ProfileInfoKHR::initialize(const VkVideoDecodeAV1ProfileInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfile = in_struct->stdProfile;
    filmGrainSupport = in_struct->filmGrainSupport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeAV1ProfileInfoKHR::initialize(const safe_VkVideoDecodeAV1ProfileInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfile = copy_src->stdProfile;
    filmGrainSupport = copy_src->filmGrainSupport;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeAV1CapabilitiesKHR::safe_VkVideoDecodeAV1CapabilitiesKHR(const VkVideoDecodeAV1CapabilitiesKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), maxLevel(in_struct->maxLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeAV1CapabilitiesKHR::safe_VkVideoDecodeAV1CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_CAPABILITIES_KHR), pNext(nullptr), maxLevel() {}

safe_VkVideoDecodeAV1CapabilitiesKHR::safe_VkVideoDecodeAV1CapabilitiesKHR(const safe_VkVideoDecodeAV1CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeAV1CapabilitiesKHR& safe_VkVideoDecodeAV1CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeAV1CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeAV1CapabilitiesKHR::~safe_VkVideoDecodeAV1CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeAV1CapabilitiesKHR::initialize(const VkVideoDecodeAV1CapabilitiesKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevel = in_struct->maxLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeAV1CapabilitiesKHR::initialize(const safe_VkVideoDecodeAV1CapabilitiesKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevel = copy_src->maxLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR(
    const VkVideoDecodeAV1SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pStdSequenceHeader(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }
}

safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR), pNext(nullptr), pStdSequenceHeader(nullptr) {}

safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR(
    const safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }
}

safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR& safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }

    return *this;
}

safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::~safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR() {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::initialize(
    const VkVideoDecodeAV1SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }
}

void safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src->pStdSequenceHeader);
    }
}

safe_VkVideoDecodeAV1PictureInfoKHR::safe_VkVideoDecodeAV1PictureInfoKHR(const VkVideoDecodeAV1PictureInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      pStdPictureInfo(nullptr),
      frameHeaderOffset(in_struct->frameHeaderOffset),
      tileCount(in_struct->tileCount),
      pTileOffsets(nullptr),
      pTileSizes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeAV1PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }

    if (in_struct->pTileOffsets) {
        pTileOffsets = new uint32_t[in_struct->tileCount];
        memcpy((void*)pTileOffsets, (void*)in_struct->pTileOffsets, sizeof(uint32_t) * in_struct->tileCount);
    }

    if (in_struct->pTileSizes) {
        pTileSizes = new uint32_t[in_struct->tileCount];
        memcpy((void*)pTileSizes, (void*)in_struct->pTileSizes, sizeof(uint32_t) * in_struct->tileCount);
    }
}

safe_VkVideoDecodeAV1PictureInfoKHR::safe_VkVideoDecodeAV1PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      frameHeaderOffset(),
      tileCount(),
      pTileOffsets(nullptr),
      pTileSizes(nullptr) {}

safe_VkVideoDecodeAV1PictureInfoKHR::safe_VkVideoDecodeAV1PictureInfoKHR(const safe_VkVideoDecodeAV1PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    frameHeaderOffset = copy_src.frameHeaderOffset;
    tileCount = copy_src.tileCount;
    pTileOffsets = nullptr;
    pTileSizes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeAV1PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }

    if (copy_src.pTileOffsets) {
        pTileOffsets = new uint32_t[copy_src.tileCount];
        memcpy((void*)pTileOffsets, (void*)copy_src.pTileOffsets, sizeof(uint32_t) * copy_src.tileCount);
    }

    if (copy_src.pTileSizes) {
        pTileSizes = new uint32_t[copy_src.tileCount];
        memcpy((void*)pTileSizes, (void*)copy_src.pTileSizes, sizeof(uint32_t) * copy_src.tileCount);
    }
}

safe_VkVideoDecodeAV1PictureInfoKHR& safe_VkVideoDecodeAV1PictureInfoKHR::operator=(
    const safe_VkVideoDecodeAV1PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pTileOffsets) delete[] pTileOffsets;
    if (pTileSizes) delete[] pTileSizes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    frameHeaderOffset = copy_src.frameHeaderOffset;
    tileCount = copy_src.tileCount;
    pTileOffsets = nullptr;
    pTileSizes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeAV1PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }

    if (copy_src.pTileOffsets) {
        pTileOffsets = new uint32_t[copy_src.tileCount];
        memcpy((void*)pTileOffsets, (void*)copy_src.pTileOffsets, sizeof(uint32_t) * copy_src.tileCount);
    }

    if (copy_src.pTileSizes) {
        pTileSizes = new uint32_t[copy_src.tileCount];
        memcpy((void*)pTileSizes, (void*)copy_src.pTileSizes, sizeof(uint32_t) * copy_src.tileCount);
    }

    return *this;
}

safe_VkVideoDecodeAV1PictureInfoKHR::~safe_VkVideoDecodeAV1PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pTileOffsets) delete[] pTileOffsets;
    if (pTileSizes) delete[] pTileSizes;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeAV1PictureInfoKHR::initialize(const VkVideoDecodeAV1PictureInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    if (pTileOffsets) delete[] pTileOffsets;
    if (pTileSizes) delete[] pTileSizes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    frameHeaderOffset = in_struct->frameHeaderOffset;
    tileCount = in_struct->tileCount;
    pTileOffsets = nullptr;
    pTileSizes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeAV1PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }

    if (in_struct->pTileOffsets) {
        pTileOffsets = new uint32_t[in_struct->tileCount];
        memcpy((void*)pTileOffsets, (void*)in_struct->pTileOffsets, sizeof(uint32_t) * in_struct->tileCount);
    }

    if (in_struct->pTileSizes) {
        pTileSizes = new uint32_t[in_struct->tileCount];
        memcpy((void*)pTileSizes, (void*)in_struct->pTileSizes, sizeof(uint32_t) * in_struct->tileCount);
    }
}

void safe_VkVideoDecodeAV1PictureInfoKHR::initialize(const safe_VkVideoDecodeAV1PictureInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    frameHeaderOffset = copy_src->frameHeaderOffset;
    tileCount = copy_src->tileCount;
    pTileOffsets = nullptr;
    pTileSizes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeAV1PictureInfo(*copy_src->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src->referenceNameSlotIndices[i];
    }

    if (copy_src->pTileOffsets) {
        pTileOffsets = new uint32_t[copy_src->tileCount];
        memcpy((void*)pTileOffsets, (void*)copy_src->pTileOffsets, sizeof(uint32_t) * copy_src->tileCount);
    }

    if (copy_src->pTileSizes) {
        pTileSizes = new uint32_t[copy_src->tileCount];
        memcpy((void*)pTileSizes, (void*)copy_src->pTileSizes, sizeof(uint32_t) * copy_src->tileCount);
    }
}

safe_VkVideoDecodeAV1DpbSlotInfoKHR::safe_VkVideoDecodeAV1DpbSlotInfoKHR(const VkVideoDecodeAV1DpbSlotInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeAV1ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoDecodeAV1DpbSlotInfoKHR::safe_VkVideoDecodeAV1DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoDecodeAV1DpbSlotInfoKHR::safe_VkVideoDecodeAV1DpbSlotInfoKHR(const safe_VkVideoDecodeAV1DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeAV1ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoDecodeAV1DpbSlotInfoKHR& safe_VkVideoDecodeAV1DpbSlotInfoKHR::operator=(
    const safe_VkVideoDecodeAV1DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeAV1ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoDecodeAV1DpbSlotInfoKHR::~safe_VkVideoDecodeAV1DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeAV1DpbSlotInfoKHR::initialize(const VkVideoDecodeAV1DpbSlotInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeAV1ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoDecodeAV1DpbSlotInfoKHR::initialize(const safe_VkVideoDecodeAV1DpbSlotInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoDecodeAV1ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR(
    const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoEncodeAV1(in_struct->videoEncodeAV1) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_AV1_FEATURES_KHR), pNext(nullptr), videoEncodeAV1() {}

safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR(
    const safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoEncodeAV1 = copy_src.videoEncodeAV1;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR& safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoEncodeAV1 = copy_src.videoEncodeAV1;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::~safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::initialize(const VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoEncodeAV1 = in_struct->videoEncodeAV1;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::initialize(const safe_VkPhysicalDeviceVideoEncodeAV1FeaturesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoEncodeAV1 = copy_src->videoEncodeAV1;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1CapabilitiesKHR::safe_VkVideoEncodeAV1CapabilitiesKHR(const VkVideoEncodeAV1CapabilitiesKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      maxLevel(in_struct->maxLevel),
      codedPictureAlignment(in_struct->codedPictureAlignment),
      maxTiles(in_struct->maxTiles),
      minTileSize(in_struct->minTileSize),
      maxTileSize(in_struct->maxTileSize),
      superblockSizes(in_struct->superblockSizes),
      maxSingleReferenceCount(in_struct->maxSingleReferenceCount),
      singleReferenceNameMask(in_struct->singleReferenceNameMask),
      maxUnidirectionalCompoundReferenceCount(in_struct->maxUnidirectionalCompoundReferenceCount),
      maxUnidirectionalCompoundGroup1ReferenceCount(in_struct->maxUnidirectionalCompoundGroup1ReferenceCount),
      unidirectionalCompoundReferenceNameMask(in_struct->unidirectionalCompoundReferenceNameMask),
      maxBidirectionalCompoundReferenceCount(in_struct->maxBidirectionalCompoundReferenceCount),
      maxBidirectionalCompoundGroup1ReferenceCount(in_struct->maxBidirectionalCompoundGroup1ReferenceCount),
      maxBidirectionalCompoundGroup2ReferenceCount(in_struct->maxBidirectionalCompoundGroup2ReferenceCount),
      bidirectionalCompoundReferenceNameMask(in_struct->bidirectionalCompoundReferenceNameMask),
      maxTemporalLayerCount(in_struct->maxTemporalLayerCount),
      maxSpatialLayerCount(in_struct->maxSpatialLayerCount),
      maxOperatingPoints(in_struct->maxOperatingPoints),
      minQIndex(in_struct->minQIndex),
      maxQIndex(in_struct->maxQIndex),
      prefersGopRemainingFrames(in_struct->prefersGopRemainingFrames),
      requiresGopRemainingFrames(in_struct->requiresGopRemainingFrames),
      stdSyntaxFlags(in_struct->stdSyntaxFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1CapabilitiesKHR::safe_VkVideoEncodeAV1CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR),
      pNext(nullptr),
      flags(),
      maxLevel(),
      codedPictureAlignment(),
      maxTiles(),
      minTileSize(),
      maxTileSize(),
      superblockSizes(),
      maxSingleReferenceCount(),
      singleReferenceNameMask(),
      maxUnidirectionalCompoundReferenceCount(),
      maxUnidirectionalCompoundGroup1ReferenceCount(),
      unidirectionalCompoundReferenceNameMask(),
      maxBidirectionalCompoundReferenceCount(),
      maxBidirectionalCompoundGroup1ReferenceCount(),
      maxBidirectionalCompoundGroup2ReferenceCount(),
      bidirectionalCompoundReferenceNameMask(),
      maxTemporalLayerCount(),
      maxSpatialLayerCount(),
      maxOperatingPoints(),
      minQIndex(),
      maxQIndex(),
      prefersGopRemainingFrames(),
      requiresGopRemainingFrames(),
      stdSyntaxFlags() {}

safe_VkVideoEncodeAV1CapabilitiesKHR::safe_VkVideoEncodeAV1CapabilitiesKHR(const safe_VkVideoEncodeAV1CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevel = copy_src.maxLevel;
    codedPictureAlignment = copy_src.codedPictureAlignment;
    maxTiles = copy_src.maxTiles;
    minTileSize = copy_src.minTileSize;
    maxTileSize = copy_src.maxTileSize;
    superblockSizes = copy_src.superblockSizes;
    maxSingleReferenceCount = copy_src.maxSingleReferenceCount;
    singleReferenceNameMask = copy_src.singleReferenceNameMask;
    maxUnidirectionalCompoundReferenceCount = copy_src.maxUnidirectionalCompoundReferenceCount;
    maxUnidirectionalCompoundGroup1ReferenceCount = copy_src.maxUnidirectionalCompoundGroup1ReferenceCount;
    unidirectionalCompoundReferenceNameMask = copy_src.unidirectionalCompoundReferenceNameMask;
    maxBidirectionalCompoundReferenceCount = copy_src.maxBidirectionalCompoundReferenceCount;
    maxBidirectionalCompoundGroup1ReferenceCount = copy_src.maxBidirectionalCompoundGroup1ReferenceCount;
    maxBidirectionalCompoundGroup2ReferenceCount = copy_src.maxBidirectionalCompoundGroup2ReferenceCount;
    bidirectionalCompoundReferenceNameMask = copy_src.bidirectionalCompoundReferenceNameMask;
    maxTemporalLayerCount = copy_src.maxTemporalLayerCount;
    maxSpatialLayerCount = copy_src.maxSpatialLayerCount;
    maxOperatingPoints = copy_src.maxOperatingPoints;
    minQIndex = copy_src.minQIndex;
    maxQIndex = copy_src.maxQIndex;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1CapabilitiesKHR& safe_VkVideoEncodeAV1CapabilitiesKHR::operator=(
    const safe_VkVideoEncodeAV1CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    maxLevel = copy_src.maxLevel;
    codedPictureAlignment = copy_src.codedPictureAlignment;
    maxTiles = copy_src.maxTiles;
    minTileSize = copy_src.minTileSize;
    maxTileSize = copy_src.maxTileSize;
    superblockSizes = copy_src.superblockSizes;
    maxSingleReferenceCount = copy_src.maxSingleReferenceCount;
    singleReferenceNameMask = copy_src.singleReferenceNameMask;
    maxUnidirectionalCompoundReferenceCount = copy_src.maxUnidirectionalCompoundReferenceCount;
    maxUnidirectionalCompoundGroup1ReferenceCount = copy_src.maxUnidirectionalCompoundGroup1ReferenceCount;
    unidirectionalCompoundReferenceNameMask = copy_src.unidirectionalCompoundReferenceNameMask;
    maxBidirectionalCompoundReferenceCount = copy_src.maxBidirectionalCompoundReferenceCount;
    maxBidirectionalCompoundGroup1ReferenceCount = copy_src.maxBidirectionalCompoundGroup1ReferenceCount;
    maxBidirectionalCompoundGroup2ReferenceCount = copy_src.maxBidirectionalCompoundGroup2ReferenceCount;
    bidirectionalCompoundReferenceNameMask = copy_src.bidirectionalCompoundReferenceNameMask;
    maxTemporalLayerCount = copy_src.maxTemporalLayerCount;
    maxSpatialLayerCount = copy_src.maxSpatialLayerCount;
    maxOperatingPoints = copy_src.maxOperatingPoints;
    minQIndex = copy_src.minQIndex;
    maxQIndex = copy_src.maxQIndex;
    prefersGopRemainingFrames = copy_src.prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src.requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src.stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1CapabilitiesKHR::~safe_VkVideoEncodeAV1CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1CapabilitiesKHR::initialize(const VkVideoEncodeAV1CapabilitiesKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    maxLevel = in_struct->maxLevel;
    codedPictureAlignment = in_struct->codedPictureAlignment;
    maxTiles = in_struct->maxTiles;
    minTileSize = in_struct->minTileSize;
    maxTileSize = in_struct->maxTileSize;
    superblockSizes = in_struct->superblockSizes;
    maxSingleReferenceCount = in_struct->maxSingleReferenceCount;
    singleReferenceNameMask = in_struct->singleReferenceNameMask;
    maxUnidirectionalCompoundReferenceCount = in_struct->maxUnidirectionalCompoundReferenceCount;
    maxUnidirectionalCompoundGroup1ReferenceCount = in_struct->maxUnidirectionalCompoundGroup1ReferenceCount;
    unidirectionalCompoundReferenceNameMask = in_struct->unidirectionalCompoundReferenceNameMask;
    maxBidirectionalCompoundReferenceCount = in_struct->maxBidirectionalCompoundReferenceCount;
    maxBidirectionalCompoundGroup1ReferenceCount = in_struct->maxBidirectionalCompoundGroup1ReferenceCount;
    maxBidirectionalCompoundGroup2ReferenceCount = in_struct->maxBidirectionalCompoundGroup2ReferenceCount;
    bidirectionalCompoundReferenceNameMask = in_struct->bidirectionalCompoundReferenceNameMask;
    maxTemporalLayerCount = in_struct->maxTemporalLayerCount;
    maxSpatialLayerCount = in_struct->maxSpatialLayerCount;
    maxOperatingPoints = in_struct->maxOperatingPoints;
    minQIndex = in_struct->minQIndex;
    maxQIndex = in_struct->maxQIndex;
    prefersGopRemainingFrames = in_struct->prefersGopRemainingFrames;
    requiresGopRemainingFrames = in_struct->requiresGopRemainingFrames;
    stdSyntaxFlags = in_struct->stdSyntaxFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1CapabilitiesKHR::initialize(const safe_VkVideoEncodeAV1CapabilitiesKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    maxLevel = copy_src->maxLevel;
    codedPictureAlignment = copy_src->codedPictureAlignment;
    maxTiles = copy_src->maxTiles;
    minTileSize = copy_src->minTileSize;
    maxTileSize = copy_src->maxTileSize;
    superblockSizes = copy_src->superblockSizes;
    maxSingleReferenceCount = copy_src->maxSingleReferenceCount;
    singleReferenceNameMask = copy_src->singleReferenceNameMask;
    maxUnidirectionalCompoundReferenceCount = copy_src->maxUnidirectionalCompoundReferenceCount;
    maxUnidirectionalCompoundGroup1ReferenceCount = copy_src->maxUnidirectionalCompoundGroup1ReferenceCount;
    unidirectionalCompoundReferenceNameMask = copy_src->unidirectionalCompoundReferenceNameMask;
    maxBidirectionalCompoundReferenceCount = copy_src->maxBidirectionalCompoundReferenceCount;
    maxBidirectionalCompoundGroup1ReferenceCount = copy_src->maxBidirectionalCompoundGroup1ReferenceCount;
    maxBidirectionalCompoundGroup2ReferenceCount = copy_src->maxBidirectionalCompoundGroup2ReferenceCount;
    bidirectionalCompoundReferenceNameMask = copy_src->bidirectionalCompoundReferenceNameMask;
    maxTemporalLayerCount = copy_src->maxTemporalLayerCount;
    maxSpatialLayerCount = copy_src->maxSpatialLayerCount;
    maxOperatingPoints = copy_src->maxOperatingPoints;
    minQIndex = copy_src->minQIndex;
    maxQIndex = copy_src->maxQIndex;
    prefersGopRemainingFrames = copy_src->prefersGopRemainingFrames;
    requiresGopRemainingFrames = copy_src->requiresGopRemainingFrames;
    stdSyntaxFlags = copy_src->stdSyntaxFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::safe_VkVideoEncodeAV1QualityLevelPropertiesKHR(
    const VkVideoEncodeAV1QualityLevelPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      preferredRateControlFlags(in_struct->preferredRateControlFlags),
      preferredGopFrameCount(in_struct->preferredGopFrameCount),
      preferredKeyFramePeriod(in_struct->preferredKeyFramePeriod),
      preferredConsecutiveBipredictiveFrameCount(in_struct->preferredConsecutiveBipredictiveFrameCount),
      preferredTemporalLayerCount(in_struct->preferredTemporalLayerCount),
      preferredConstantQIndex(in_struct->preferredConstantQIndex),
      preferredMaxSingleReferenceCount(in_struct->preferredMaxSingleReferenceCount),
      preferredSingleReferenceNameMask(in_struct->preferredSingleReferenceNameMask),
      preferredMaxUnidirectionalCompoundReferenceCount(in_struct->preferredMaxUnidirectionalCompoundReferenceCount),
      preferredMaxUnidirectionalCompoundGroup1ReferenceCount(in_struct->preferredMaxUnidirectionalCompoundGroup1ReferenceCount),
      preferredUnidirectionalCompoundReferenceNameMask(in_struct->preferredUnidirectionalCompoundReferenceNameMask),
      preferredMaxBidirectionalCompoundReferenceCount(in_struct->preferredMaxBidirectionalCompoundReferenceCount),
      preferredMaxBidirectionalCompoundGroup1ReferenceCount(in_struct->preferredMaxBidirectionalCompoundGroup1ReferenceCount),
      preferredMaxBidirectionalCompoundGroup2ReferenceCount(in_struct->preferredMaxBidirectionalCompoundGroup2ReferenceCount),
      preferredBidirectionalCompoundReferenceNameMask(in_struct->preferredBidirectionalCompoundReferenceNameMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::safe_VkVideoEncodeAV1QualityLevelPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUALITY_LEVEL_PROPERTIES_KHR),
      pNext(nullptr),
      preferredRateControlFlags(),
      preferredGopFrameCount(),
      preferredKeyFramePeriod(),
      preferredConsecutiveBipredictiveFrameCount(),
      preferredTemporalLayerCount(),
      preferredConstantQIndex(),
      preferredMaxSingleReferenceCount(),
      preferredSingleReferenceNameMask(),
      preferredMaxUnidirectionalCompoundReferenceCount(),
      preferredMaxUnidirectionalCompoundGroup1ReferenceCount(),
      preferredUnidirectionalCompoundReferenceNameMask(),
      preferredMaxBidirectionalCompoundReferenceCount(),
      preferredMaxBidirectionalCompoundGroup1ReferenceCount(),
      preferredMaxBidirectionalCompoundGroup2ReferenceCount(),
      preferredBidirectionalCompoundReferenceNameMask() {}

safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::safe_VkVideoEncodeAV1QualityLevelPropertiesKHR(
    const safe_VkVideoEncodeAV1QualityLevelPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredKeyFramePeriod = copy_src.preferredKeyFramePeriod;
    preferredConsecutiveBipredictiveFrameCount = copy_src.preferredConsecutiveBipredictiveFrameCount;
    preferredTemporalLayerCount = copy_src.preferredTemporalLayerCount;
    preferredConstantQIndex = copy_src.preferredConstantQIndex;
    preferredMaxSingleReferenceCount = copy_src.preferredMaxSingleReferenceCount;
    preferredSingleReferenceNameMask = copy_src.preferredSingleReferenceNameMask;
    preferredMaxUnidirectionalCompoundReferenceCount = copy_src.preferredMaxUnidirectionalCompoundReferenceCount;
    preferredMaxUnidirectionalCompoundGroup1ReferenceCount = copy_src.preferredMaxUnidirectionalCompoundGroup1ReferenceCount;
    preferredUnidirectionalCompoundReferenceNameMask = copy_src.preferredUnidirectionalCompoundReferenceNameMask;
    preferredMaxBidirectionalCompoundReferenceCount = copy_src.preferredMaxBidirectionalCompoundReferenceCount;
    preferredMaxBidirectionalCompoundGroup1ReferenceCount = copy_src.preferredMaxBidirectionalCompoundGroup1ReferenceCount;
    preferredMaxBidirectionalCompoundGroup2ReferenceCount = copy_src.preferredMaxBidirectionalCompoundGroup2ReferenceCount;
    preferredBidirectionalCompoundReferenceNameMask = copy_src.preferredBidirectionalCompoundReferenceNameMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1QualityLevelPropertiesKHR& safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::operator=(
    const safe_VkVideoEncodeAV1QualityLevelPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    preferredRateControlFlags = copy_src.preferredRateControlFlags;
    preferredGopFrameCount = copy_src.preferredGopFrameCount;
    preferredKeyFramePeriod = copy_src.preferredKeyFramePeriod;
    preferredConsecutiveBipredictiveFrameCount = copy_src.preferredConsecutiveBipredictiveFrameCount;
    preferredTemporalLayerCount = copy_src.preferredTemporalLayerCount;
    preferredConstantQIndex = copy_src.preferredConstantQIndex;
    preferredMaxSingleReferenceCount = copy_src.preferredMaxSingleReferenceCount;
    preferredSingleReferenceNameMask = copy_src.preferredSingleReferenceNameMask;
    preferredMaxUnidirectionalCompoundReferenceCount = copy_src.preferredMaxUnidirectionalCompoundReferenceCount;
    preferredMaxUnidirectionalCompoundGroup1ReferenceCount = copy_src.preferredMaxUnidirectionalCompoundGroup1ReferenceCount;
    preferredUnidirectionalCompoundReferenceNameMask = copy_src.preferredUnidirectionalCompoundReferenceNameMask;
    preferredMaxBidirectionalCompoundReferenceCount = copy_src.preferredMaxBidirectionalCompoundReferenceCount;
    preferredMaxBidirectionalCompoundGroup1ReferenceCount = copy_src.preferredMaxBidirectionalCompoundGroup1ReferenceCount;
    preferredMaxBidirectionalCompoundGroup2ReferenceCount = copy_src.preferredMaxBidirectionalCompoundGroup2ReferenceCount;
    preferredBidirectionalCompoundReferenceNameMask = copy_src.preferredBidirectionalCompoundReferenceNameMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::~safe_VkVideoEncodeAV1QualityLevelPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::initialize(const VkVideoEncodeAV1QualityLevelPropertiesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    preferredRateControlFlags = in_struct->preferredRateControlFlags;
    preferredGopFrameCount = in_struct->preferredGopFrameCount;
    preferredKeyFramePeriod = in_struct->preferredKeyFramePeriod;
    preferredConsecutiveBipredictiveFrameCount = in_struct->preferredConsecutiveBipredictiveFrameCount;
    preferredTemporalLayerCount = in_struct->preferredTemporalLayerCount;
    preferredConstantQIndex = in_struct->preferredConstantQIndex;
    preferredMaxSingleReferenceCount = in_struct->preferredMaxSingleReferenceCount;
    preferredSingleReferenceNameMask = in_struct->preferredSingleReferenceNameMask;
    preferredMaxUnidirectionalCompoundReferenceCount = in_struct->preferredMaxUnidirectionalCompoundReferenceCount;
    preferredMaxUnidirectionalCompoundGroup1ReferenceCount = in_struct->preferredMaxUnidirectionalCompoundGroup1ReferenceCount;
    preferredUnidirectionalCompoundReferenceNameMask = in_struct->preferredUnidirectionalCompoundReferenceNameMask;
    preferredMaxBidirectionalCompoundReferenceCount = in_struct->preferredMaxBidirectionalCompoundReferenceCount;
    preferredMaxBidirectionalCompoundGroup1ReferenceCount = in_struct->preferredMaxBidirectionalCompoundGroup1ReferenceCount;
    preferredMaxBidirectionalCompoundGroup2ReferenceCount = in_struct->preferredMaxBidirectionalCompoundGroup2ReferenceCount;
    preferredBidirectionalCompoundReferenceNameMask = in_struct->preferredBidirectionalCompoundReferenceNameMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1QualityLevelPropertiesKHR::initialize(const safe_VkVideoEncodeAV1QualityLevelPropertiesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    preferredRateControlFlags = copy_src->preferredRateControlFlags;
    preferredGopFrameCount = copy_src->preferredGopFrameCount;
    preferredKeyFramePeriod = copy_src->preferredKeyFramePeriod;
    preferredConsecutiveBipredictiveFrameCount = copy_src->preferredConsecutiveBipredictiveFrameCount;
    preferredTemporalLayerCount = copy_src->preferredTemporalLayerCount;
    preferredConstantQIndex = copy_src->preferredConstantQIndex;
    preferredMaxSingleReferenceCount = copy_src->preferredMaxSingleReferenceCount;
    preferredSingleReferenceNameMask = copy_src->preferredSingleReferenceNameMask;
    preferredMaxUnidirectionalCompoundReferenceCount = copy_src->preferredMaxUnidirectionalCompoundReferenceCount;
    preferredMaxUnidirectionalCompoundGroup1ReferenceCount = copy_src->preferredMaxUnidirectionalCompoundGroup1ReferenceCount;
    preferredUnidirectionalCompoundReferenceNameMask = copy_src->preferredUnidirectionalCompoundReferenceNameMask;
    preferredMaxBidirectionalCompoundReferenceCount = copy_src->preferredMaxBidirectionalCompoundReferenceCount;
    preferredMaxBidirectionalCompoundGroup1ReferenceCount = copy_src->preferredMaxBidirectionalCompoundGroup1ReferenceCount;
    preferredMaxBidirectionalCompoundGroup2ReferenceCount = copy_src->preferredMaxBidirectionalCompoundGroup2ReferenceCount;
    preferredBidirectionalCompoundReferenceNameMask = copy_src->preferredBidirectionalCompoundReferenceNameMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1SessionCreateInfoKHR::safe_VkVideoEncodeAV1SessionCreateInfoKHR(
    const VkVideoEncodeAV1SessionCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), useMaxLevel(in_struct->useMaxLevel), maxLevel(in_struct->maxLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1SessionCreateInfoKHR::safe_VkVideoEncodeAV1SessionCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_CREATE_INFO_KHR), pNext(nullptr), useMaxLevel(), maxLevel() {}

safe_VkVideoEncodeAV1SessionCreateInfoKHR::safe_VkVideoEncodeAV1SessionCreateInfoKHR(
    const safe_VkVideoEncodeAV1SessionCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMaxLevel = copy_src.useMaxLevel;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1SessionCreateInfoKHR& safe_VkVideoEncodeAV1SessionCreateInfoKHR::operator=(
    const safe_VkVideoEncodeAV1SessionCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMaxLevel = copy_src.useMaxLevel;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1SessionCreateInfoKHR::~safe_VkVideoEncodeAV1SessionCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1SessionCreateInfoKHR::initialize(const VkVideoEncodeAV1SessionCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMaxLevel = in_struct->useMaxLevel;
    maxLevel = in_struct->maxLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1SessionCreateInfoKHR::initialize(const safe_VkVideoEncodeAV1SessionCreateInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMaxLevel = copy_src->useMaxLevel;
    maxLevel = copy_src->maxLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR(
    const VkVideoEncodeAV1SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pStdSequenceHeader(nullptr),
      pStdDecoderModelInfo(nullptr),
      stdOperatingPointCount(in_struct->stdOperatingPointCount),
      pStdOperatingPoints(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }

    if (in_struct->pStdDecoderModelInfo) {
        pStdDecoderModelInfo = new StdVideoEncodeAV1DecoderModelInfo(*in_struct->pStdDecoderModelInfo);
    }

    if (in_struct->pStdOperatingPoints) {
        pStdOperatingPoints = new StdVideoEncodeAV1OperatingPointInfo[in_struct->stdOperatingPointCount];
        memcpy((void*)pStdOperatingPoints, (void*)in_struct->pStdOperatingPoints,
               sizeof(StdVideoEncodeAV1OperatingPointInfo) * in_struct->stdOperatingPointCount);
    }
}

safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      pStdSequenceHeader(nullptr),
      pStdDecoderModelInfo(nullptr),
      stdOperatingPointCount(),
      pStdOperatingPoints(nullptr) {}

safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR(
    const safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pStdDecoderModelInfo = nullptr;
    stdOperatingPointCount = copy_src.stdOperatingPointCount;
    pStdOperatingPoints = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }

    if (copy_src.pStdDecoderModelInfo) {
        pStdDecoderModelInfo = new StdVideoEncodeAV1DecoderModelInfo(*copy_src.pStdDecoderModelInfo);
    }

    if (copy_src.pStdOperatingPoints) {
        pStdOperatingPoints = new StdVideoEncodeAV1OperatingPointInfo[copy_src.stdOperatingPointCount];
        memcpy((void*)pStdOperatingPoints, (void*)copy_src.pStdOperatingPoints,
               sizeof(StdVideoEncodeAV1OperatingPointInfo) * copy_src.stdOperatingPointCount);
    }
}

safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR& safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSequenceHeader) delete pStdSequenceHeader;
    if (pStdDecoderModelInfo) delete pStdDecoderModelInfo;
    if (pStdOperatingPoints) delete[] pStdOperatingPoints;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pStdDecoderModelInfo = nullptr;
    stdOperatingPointCount = copy_src.stdOperatingPointCount;
    pStdOperatingPoints = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }

    if (copy_src.pStdDecoderModelInfo) {
        pStdDecoderModelInfo = new StdVideoEncodeAV1DecoderModelInfo(*copy_src.pStdDecoderModelInfo);
    }

    if (copy_src.pStdOperatingPoints) {
        pStdOperatingPoints = new StdVideoEncodeAV1OperatingPointInfo[copy_src.stdOperatingPointCount];
        memcpy((void*)pStdOperatingPoints, (void*)copy_src.pStdOperatingPoints,
               sizeof(StdVideoEncodeAV1OperatingPointInfo) * copy_src.stdOperatingPointCount);
    }

    return *this;
}

safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::~safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR() {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    if (pStdDecoderModelInfo) delete pStdDecoderModelInfo;
    if (pStdOperatingPoints) delete[] pStdOperatingPoints;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::initialize(
    const VkVideoEncodeAV1SessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    if (pStdDecoderModelInfo) delete pStdDecoderModelInfo;
    if (pStdOperatingPoints) delete[] pStdOperatingPoints;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdSequenceHeader = nullptr;
    pStdDecoderModelInfo = nullptr;
    stdOperatingPointCount = in_struct->stdOperatingPointCount;
    pStdOperatingPoints = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }

    if (in_struct->pStdDecoderModelInfo) {
        pStdDecoderModelInfo = new StdVideoEncodeAV1DecoderModelInfo(*in_struct->pStdDecoderModelInfo);
    }

    if (in_struct->pStdOperatingPoints) {
        pStdOperatingPoints = new StdVideoEncodeAV1OperatingPointInfo[in_struct->stdOperatingPointCount];
        memcpy((void*)pStdOperatingPoints, (void*)in_struct->pStdOperatingPoints,
               sizeof(StdVideoEncodeAV1OperatingPointInfo) * in_struct->stdOperatingPointCount);
    }
}

void safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoEncodeAV1SessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdSequenceHeader = nullptr;
    pStdDecoderModelInfo = nullptr;
    stdOperatingPointCount = copy_src->stdOperatingPointCount;
    pStdOperatingPoints = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src->pStdSequenceHeader);
    }

    if (copy_src->pStdDecoderModelInfo) {
        pStdDecoderModelInfo = new StdVideoEncodeAV1DecoderModelInfo(*copy_src->pStdDecoderModelInfo);
    }

    if (copy_src->pStdOperatingPoints) {
        pStdOperatingPoints = new StdVideoEncodeAV1OperatingPointInfo[copy_src->stdOperatingPointCount];
        memcpy((void*)pStdOperatingPoints, (void*)copy_src->pStdOperatingPoints,
               sizeof(StdVideoEncodeAV1OperatingPointInfo) * copy_src->stdOperatingPointCount);
    }
}

safe_VkVideoEncodeAV1PictureInfoKHR::safe_VkVideoEncodeAV1PictureInfoKHR(const VkVideoEncodeAV1PictureInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      predictionMode(in_struct->predictionMode),
      rateControlGroup(in_struct->rateControlGroup),
      constantQIndex(in_struct->constantQIndex),
      pStdPictureInfo(nullptr),
      primaryReferenceCdfOnly(in_struct->primaryReferenceCdfOnly),
      generateObuExtensionHeader(in_struct->generateObuExtensionHeader) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeAV1PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }
}

safe_VkVideoEncodeAV1PictureInfoKHR::safe_VkVideoEncodeAV1PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PICTURE_INFO_KHR),
      pNext(nullptr),
      predictionMode(),
      rateControlGroup(),
      constantQIndex(),
      pStdPictureInfo(nullptr),
      primaryReferenceCdfOnly(),
      generateObuExtensionHeader() {}

safe_VkVideoEncodeAV1PictureInfoKHR::safe_VkVideoEncodeAV1PictureInfoKHR(const safe_VkVideoEncodeAV1PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    predictionMode = copy_src.predictionMode;
    rateControlGroup = copy_src.rateControlGroup;
    constantQIndex = copy_src.constantQIndex;
    pStdPictureInfo = nullptr;
    primaryReferenceCdfOnly = copy_src.primaryReferenceCdfOnly;
    generateObuExtensionHeader = copy_src.generateObuExtensionHeader;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeAV1PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }
}

safe_VkVideoEncodeAV1PictureInfoKHR& safe_VkVideoEncodeAV1PictureInfoKHR::operator=(
    const safe_VkVideoEncodeAV1PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    predictionMode = copy_src.predictionMode;
    rateControlGroup = copy_src.rateControlGroup;
    constantQIndex = copy_src.constantQIndex;
    pStdPictureInfo = nullptr;
    primaryReferenceCdfOnly = copy_src.primaryReferenceCdfOnly;
    generateObuExtensionHeader = copy_src.generateObuExtensionHeader;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeAV1PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }

    return *this;
}

safe_VkVideoEncodeAV1PictureInfoKHR::~safe_VkVideoEncodeAV1PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeAV1PictureInfoKHR::initialize(const VkVideoEncodeAV1PictureInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    predictionMode = in_struct->predictionMode;
    rateControlGroup = in_struct->rateControlGroup;
    constantQIndex = in_struct->constantQIndex;
    pStdPictureInfo = nullptr;
    primaryReferenceCdfOnly = in_struct->primaryReferenceCdfOnly;
    generateObuExtensionHeader = in_struct->generateObuExtensionHeader;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeAV1PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }
}

void safe_VkVideoEncodeAV1PictureInfoKHR::initialize(const safe_VkVideoEncodeAV1PictureInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    predictionMode = copy_src->predictionMode;
    rateControlGroup = copy_src->rateControlGroup;
    constantQIndex = copy_src->constantQIndex;
    pStdPictureInfo = nullptr;
    primaryReferenceCdfOnly = copy_src->primaryReferenceCdfOnly;
    generateObuExtensionHeader = copy_src->generateObuExtensionHeader;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoEncodeAV1PictureInfo(*copy_src->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src->referenceNameSlotIndices[i];
    }
}

safe_VkVideoEncodeAV1DpbSlotInfoKHR::safe_VkVideoEncodeAV1DpbSlotInfoKHR(const VkVideoEncodeAV1DpbSlotInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), pStdReferenceInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeAV1ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeAV1DpbSlotInfoKHR::safe_VkVideoEncodeAV1DpbSlotInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_DPB_SLOT_INFO_KHR), pNext(nullptr), pStdReferenceInfo(nullptr) {}

safe_VkVideoEncodeAV1DpbSlotInfoKHR::safe_VkVideoEncodeAV1DpbSlotInfoKHR(const safe_VkVideoEncodeAV1DpbSlotInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeAV1ReferenceInfo(*copy_src.pStdReferenceInfo);
    }
}

safe_VkVideoEncodeAV1DpbSlotInfoKHR& safe_VkVideoEncodeAV1DpbSlotInfoKHR::operator=(
    const safe_VkVideoEncodeAV1DpbSlotInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeAV1ReferenceInfo(*copy_src.pStdReferenceInfo);
    }

    return *this;
}

safe_VkVideoEncodeAV1DpbSlotInfoKHR::~safe_VkVideoEncodeAV1DpbSlotInfoKHR() {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeAV1DpbSlotInfoKHR::initialize(const VkVideoEncodeAV1DpbSlotInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdReferenceInfo) delete pStdReferenceInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeAV1ReferenceInfo(*in_struct->pStdReferenceInfo);
    }
}

void safe_VkVideoEncodeAV1DpbSlotInfoKHR::initialize(const safe_VkVideoEncodeAV1DpbSlotInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdReferenceInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdReferenceInfo) {
        pStdReferenceInfo = new StdVideoEncodeAV1ReferenceInfo(*copy_src->pStdReferenceInfo);
    }
}

safe_VkVideoEncodeAV1ProfileInfoKHR::safe_VkVideoEncodeAV1ProfileInfoKHR(const VkVideoEncodeAV1ProfileInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), stdProfile(in_struct->stdProfile) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1ProfileInfoKHR::safe_VkVideoEncodeAV1ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR), pNext(nullptr), stdProfile() {}

safe_VkVideoEncodeAV1ProfileInfoKHR::safe_VkVideoEncodeAV1ProfileInfoKHR(const safe_VkVideoEncodeAV1ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1ProfileInfoKHR& safe_VkVideoEncodeAV1ProfileInfoKHR::operator=(
    const safe_VkVideoEncodeAV1ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1ProfileInfoKHR::~safe_VkVideoEncodeAV1ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1ProfileInfoKHR::initialize(const VkVideoEncodeAV1ProfileInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfile = in_struct->stdProfile;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1ProfileInfoKHR::initialize(const safe_VkVideoEncodeAV1ProfileInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfile = copy_src->stdProfile;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR(
    const VkVideoEncodeAV1GopRemainingFrameInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useGopRemainingFrames(in_struct->useGopRemainingFrames),
      gopRemainingIntra(in_struct->gopRemainingIntra),
      gopRemainingPredictive(in_struct->gopRemainingPredictive),
      gopRemainingBipredictive(in_struct->gopRemainingBipredictive) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_GOP_REMAINING_FRAME_INFO_KHR),
      pNext(nullptr),
      useGopRemainingFrames(),
      gopRemainingIntra(),
      gopRemainingPredictive(),
      gopRemainingBipredictive() {}

safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR(
    const safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR& copy_src) {
    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingIntra = copy_src.gopRemainingIntra;
    gopRemainingPredictive = copy_src.gopRemainingPredictive;
    gopRemainingBipredictive = copy_src.gopRemainingBipredictive;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR& safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::operator=(
    const safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useGopRemainingFrames = copy_src.useGopRemainingFrames;
    gopRemainingIntra = copy_src.gopRemainingIntra;
    gopRemainingPredictive = copy_src.gopRemainingPredictive;
    gopRemainingBipredictive = copy_src.gopRemainingBipredictive;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::~safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::initialize(const VkVideoEncodeAV1GopRemainingFrameInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useGopRemainingFrames = in_struct->useGopRemainingFrames;
    gopRemainingIntra = in_struct->gopRemainingIntra;
    gopRemainingPredictive = in_struct->gopRemainingPredictive;
    gopRemainingBipredictive = in_struct->gopRemainingBipredictive;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR::initialize(const safe_VkVideoEncodeAV1GopRemainingFrameInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useGopRemainingFrames = copy_src->useGopRemainingFrames;
    gopRemainingIntra = copy_src->gopRemainingIntra;
    gopRemainingPredictive = copy_src->gopRemainingPredictive;
    gopRemainingBipredictive = copy_src->gopRemainingBipredictive;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1RateControlInfoKHR::safe_VkVideoEncodeAV1RateControlInfoKHR(
    const VkVideoEncodeAV1RateControlInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      gopFrameCount(in_struct->gopFrameCount),
      keyFramePeriod(in_struct->keyFramePeriod),
      consecutiveBipredictiveFrameCount(in_struct->consecutiveBipredictiveFrameCount),
      temporalLayerCount(in_struct->temporalLayerCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1RateControlInfoKHR::safe_VkVideoEncodeAV1RateControlInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_INFO_KHR),
      pNext(nullptr),
      flags(),
      gopFrameCount(),
      keyFramePeriod(),
      consecutiveBipredictiveFrameCount(),
      temporalLayerCount() {}

safe_VkVideoEncodeAV1RateControlInfoKHR::safe_VkVideoEncodeAV1RateControlInfoKHR(
    const safe_VkVideoEncodeAV1RateControlInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    keyFramePeriod = copy_src.keyFramePeriod;
    consecutiveBipredictiveFrameCount = copy_src.consecutiveBipredictiveFrameCount;
    temporalLayerCount = copy_src.temporalLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1RateControlInfoKHR& safe_VkVideoEncodeAV1RateControlInfoKHR::operator=(
    const safe_VkVideoEncodeAV1RateControlInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    gopFrameCount = copy_src.gopFrameCount;
    keyFramePeriod = copy_src.keyFramePeriod;
    consecutiveBipredictiveFrameCount = copy_src.consecutiveBipredictiveFrameCount;
    temporalLayerCount = copy_src.temporalLayerCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1RateControlInfoKHR::~safe_VkVideoEncodeAV1RateControlInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1RateControlInfoKHR::initialize(const VkVideoEncodeAV1RateControlInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    gopFrameCount = in_struct->gopFrameCount;
    keyFramePeriod = in_struct->keyFramePeriod;
    consecutiveBipredictiveFrameCount = in_struct->consecutiveBipredictiveFrameCount;
    temporalLayerCount = in_struct->temporalLayerCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1RateControlInfoKHR::initialize(const safe_VkVideoEncodeAV1RateControlInfoKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    gopFrameCount = copy_src->gopFrameCount;
    keyFramePeriod = copy_src->keyFramePeriod;
    consecutiveBipredictiveFrameCount = copy_src->consecutiveBipredictiveFrameCount;
    temporalLayerCount = copy_src->temporalLayerCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1RateControlLayerInfoKHR::safe_VkVideoEncodeAV1RateControlLayerInfoKHR(
    const VkVideoEncodeAV1RateControlLayerInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      useMinQIndex(in_struct->useMinQIndex),
      minQIndex(in_struct->minQIndex),
      useMaxQIndex(in_struct->useMaxQIndex),
      maxQIndex(in_struct->maxQIndex),
      useMaxFrameSize(in_struct->useMaxFrameSize),
      maxFrameSize(in_struct->maxFrameSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1RateControlLayerInfoKHR::safe_VkVideoEncodeAV1RateControlLayerInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_RATE_CONTROL_LAYER_INFO_KHR),
      pNext(nullptr),
      useMinQIndex(),
      minQIndex(),
      useMaxQIndex(),
      maxQIndex(),
      useMaxFrameSize(),
      maxFrameSize() {}

safe_VkVideoEncodeAV1RateControlLayerInfoKHR::safe_VkVideoEncodeAV1RateControlLayerInfoKHR(
    const safe_VkVideoEncodeAV1RateControlLayerInfoKHR& copy_src) {
    sType = copy_src.sType;
    useMinQIndex = copy_src.useMinQIndex;
    minQIndex = copy_src.minQIndex;
    useMaxQIndex = copy_src.useMaxQIndex;
    maxQIndex = copy_src.maxQIndex;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1RateControlLayerInfoKHR& safe_VkVideoEncodeAV1RateControlLayerInfoKHR::operator=(
    const safe_VkVideoEncodeAV1RateControlLayerInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    useMinQIndex = copy_src.useMinQIndex;
    minQIndex = copy_src.minQIndex;
    useMaxQIndex = copy_src.useMaxQIndex;
    maxQIndex = copy_src.maxQIndex;
    useMaxFrameSize = copy_src.useMaxFrameSize;
    maxFrameSize = copy_src.maxFrameSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1RateControlLayerInfoKHR::~safe_VkVideoEncodeAV1RateControlLayerInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeAV1RateControlLayerInfoKHR::initialize(const VkVideoEncodeAV1RateControlLayerInfoKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    useMinQIndex = in_struct->useMinQIndex;
    minQIndex = in_struct->minQIndex;
    useMaxQIndex = in_struct->useMaxQIndex;
    maxQIndex = in_struct->maxQIndex;
    useMaxFrameSize = in_struct->useMaxFrameSize;
    maxFrameSize = in_struct->maxFrameSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1RateControlLayerInfoKHR::initialize(const safe_VkVideoEncodeAV1RateControlLayerInfoKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    useMinQIndex = copy_src->useMinQIndex;
    minQIndex = copy_src->minQIndex;
    useMaxQIndex = copy_src->useMaxQIndex;
    maxQIndex = copy_src->maxQIndex;
    useMaxFrameSize = copy_src->useMaxFrameSize;
    maxFrameSize = copy_src->maxFrameSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR(
    const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoDecodeVP9(in_struct->videoDecodeVP9) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_DECODE_VP9_FEATURES_KHR), pNext(nullptr), videoDecodeVP9() {}

safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR(
    const safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoDecodeVP9 = copy_src.videoDecodeVP9;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR& safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoDecodeVP9 = copy_src.videoDecodeVP9;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::~safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::initialize(const VkPhysicalDeviceVideoDecodeVP9FeaturesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoDecodeVP9 = in_struct->videoDecodeVP9;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR::initialize(const safe_VkPhysicalDeviceVideoDecodeVP9FeaturesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoDecodeVP9 = copy_src->videoDecodeVP9;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeVP9ProfileInfoKHR::safe_VkVideoDecodeVP9ProfileInfoKHR(const VkVideoDecodeVP9ProfileInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), stdProfile(in_struct->stdProfile) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeVP9ProfileInfoKHR::safe_VkVideoDecodeVP9ProfileInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PROFILE_INFO_KHR), pNext(nullptr), stdProfile() {}

safe_VkVideoDecodeVP9ProfileInfoKHR::safe_VkVideoDecodeVP9ProfileInfoKHR(const safe_VkVideoDecodeVP9ProfileInfoKHR& copy_src) {
    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeVP9ProfileInfoKHR& safe_VkVideoDecodeVP9ProfileInfoKHR::operator=(
    const safe_VkVideoDecodeVP9ProfileInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stdProfile = copy_src.stdProfile;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeVP9ProfileInfoKHR::~safe_VkVideoDecodeVP9ProfileInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeVP9ProfileInfoKHR::initialize(const VkVideoDecodeVP9ProfileInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stdProfile = in_struct->stdProfile;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeVP9ProfileInfoKHR::initialize(const safe_VkVideoDecodeVP9ProfileInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stdProfile = copy_src->stdProfile;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeVP9CapabilitiesKHR::safe_VkVideoDecodeVP9CapabilitiesKHR(const VkVideoDecodeVP9CapabilitiesKHR* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), maxLevel(in_struct->maxLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoDecodeVP9CapabilitiesKHR::safe_VkVideoDecodeVP9CapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_CAPABILITIES_KHR), pNext(nullptr), maxLevel() {}

safe_VkVideoDecodeVP9CapabilitiesKHR::safe_VkVideoDecodeVP9CapabilitiesKHR(const safe_VkVideoDecodeVP9CapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoDecodeVP9CapabilitiesKHR& safe_VkVideoDecodeVP9CapabilitiesKHR::operator=(
    const safe_VkVideoDecodeVP9CapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxLevel = copy_src.maxLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoDecodeVP9CapabilitiesKHR::~safe_VkVideoDecodeVP9CapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoDecodeVP9CapabilitiesKHR::initialize(const VkVideoDecodeVP9CapabilitiesKHR* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxLevel = in_struct->maxLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoDecodeVP9CapabilitiesKHR::initialize(const safe_VkVideoDecodeVP9CapabilitiesKHR* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxLevel = copy_src->maxLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeVP9PictureInfoKHR::safe_VkVideoDecodeVP9PictureInfoKHR(const VkVideoDecodeVP9PictureInfoKHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      pStdPictureInfo(nullptr),
      uncompressedHeaderOffset(in_struct->uncompressedHeaderOffset),
      compressedHeaderOffset(in_struct->compressedHeaderOffset),
      tilesOffset(in_struct->tilesOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeVP9PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }
}

safe_VkVideoDecodeVP9PictureInfoKHR::safe_VkVideoDecodeVP9PictureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_VP9_PICTURE_INFO_KHR),
      pNext(nullptr),
      pStdPictureInfo(nullptr),
      uncompressedHeaderOffset(),
      compressedHeaderOffset(),
      tilesOffset() {}

safe_VkVideoDecodeVP9PictureInfoKHR::safe_VkVideoDecodeVP9PictureInfoKHR(const safe_VkVideoDecodeVP9PictureInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    uncompressedHeaderOffset = copy_src.uncompressedHeaderOffset;
    compressedHeaderOffset = copy_src.compressedHeaderOffset;
    tilesOffset = copy_src.tilesOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeVP9PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }
}

safe_VkVideoDecodeVP9PictureInfoKHR& safe_VkVideoDecodeVP9PictureInfoKHR::operator=(
    const safe_VkVideoDecodeVP9PictureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdPictureInfo = nullptr;
    uncompressedHeaderOffset = copy_src.uncompressedHeaderOffset;
    compressedHeaderOffset = copy_src.compressedHeaderOffset;
    tilesOffset = copy_src.tilesOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeVP9PictureInfo(*copy_src.pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src.referenceNameSlotIndices[i];
    }

    return *this;
}

safe_VkVideoDecodeVP9PictureInfoKHR::~safe_VkVideoDecodeVP9PictureInfoKHR() {
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeVP9PictureInfoKHR::initialize(const VkVideoDecodeVP9PictureInfoKHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdPictureInfo) delete pStdPictureInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdPictureInfo = nullptr;
    uncompressedHeaderOffset = in_struct->uncompressedHeaderOffset;
    compressedHeaderOffset = in_struct->compressedHeaderOffset;
    tilesOffset = in_struct->tilesOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeVP9PictureInfo(*in_struct->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = in_struct->referenceNameSlotIndices[i];
    }
}

void safe_VkVideoDecodeVP9PictureInfoKHR::initialize(const safe_VkVideoDecodeVP9PictureInfoKHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdPictureInfo = nullptr;
    uncompressedHeaderOffset = copy_src->uncompressedHeaderOffset;
    compressedHeaderOffset = copy_src->compressedHeaderOffset;
    tilesOffset = copy_src->tilesOffset;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdPictureInfo) {
        pStdPictureInfo = new StdVideoDecodeVP9PictureInfo(*copy_src->pStdPictureInfo);
    }

    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        referenceNameSlotIndices[i] = copy_src->referenceNameSlotIndices[i];
    }
}

safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR(
    const VkPhysicalDeviceVideoMaintenance1FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoMaintenance1(in_struct->videoMaintenance1) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_1_FEATURES_KHR), pNext(nullptr), videoMaintenance1() {}

safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR(
    const safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoMaintenance1 = copy_src.videoMaintenance1;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR& safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoMaintenance1 = copy_src.videoMaintenance1;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::~safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::initialize(const VkPhysicalDeviceVideoMaintenance1FeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoMaintenance1 = in_struct->videoMaintenance1;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR::initialize(
    const safe_VkPhysicalDeviceVideoMaintenance1FeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoMaintenance1 = copy_src->videoMaintenance1;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoInlineQueryInfoKHR::safe_VkVideoInlineQueryInfoKHR(const VkVideoInlineQueryInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      queryPool(in_struct->queryPool),
      firstQuery(in_struct->firstQuery),
      queryCount(in_struct->queryCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoInlineQueryInfoKHR::safe_VkVideoInlineQueryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR), pNext(nullptr), queryPool(), firstQuery(), queryCount() {}

safe_VkVideoInlineQueryInfoKHR::safe_VkVideoInlineQueryInfoKHR(const safe_VkVideoInlineQueryInfoKHR& copy_src) {
    sType = copy_src.sType;
    queryPool = copy_src.queryPool;
    firstQuery = copy_src.firstQuery;
    queryCount = copy_src.queryCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoInlineQueryInfoKHR& safe_VkVideoInlineQueryInfoKHR::operator=(const safe_VkVideoInlineQueryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    queryPool = copy_src.queryPool;
    firstQuery = copy_src.firstQuery;
    queryCount = copy_src.queryCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoInlineQueryInfoKHR::~safe_VkVideoInlineQueryInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoInlineQueryInfoKHR::initialize(const VkVideoInlineQueryInfoKHR* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queryPool = in_struct->queryPool;
    firstQuery = in_struct->firstQuery;
    queryCount = in_struct->queryCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoInlineQueryInfoKHR::initialize(const safe_VkVideoInlineQueryInfoKHR* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queryPool = copy_src->queryPool;
    firstQuery = copy_src->firstQuery;
    queryCount = copy_src->queryCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR(
    const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      unifiedImageLayouts(in_struct->unifiedImageLayouts),
      unifiedImageLayoutsVideo(in_struct->unifiedImageLayoutsVideo) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR),
      pNext(nullptr),
      unifiedImageLayouts(),
      unifiedImageLayoutsVideo() {}

safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR(
    const safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    unifiedImageLayouts = copy_src.unifiedImageLayouts;
    unifiedImageLayoutsVideo = copy_src.unifiedImageLayoutsVideo;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR& safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    unifiedImageLayouts = copy_src.unifiedImageLayouts;
    unifiedImageLayoutsVideo = copy_src.unifiedImageLayoutsVideo;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::~safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::initialize(
    const VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    unifiedImageLayouts = in_struct->unifiedImageLayouts;
    unifiedImageLayoutsVideo = in_struct->unifiedImageLayoutsVideo;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    unifiedImageLayouts = copy_src->unifiedImageLayouts;
    unifiedImageLayoutsVideo = copy_src->unifiedImageLayoutsVideo;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCalibratedTimestampInfoKHR::safe_VkCalibratedTimestampInfoKHR(const VkCalibratedTimestampInfoKHR* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), timeDomain(in_struct->timeDomain) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCalibratedTimestampInfoKHR::safe_VkCalibratedTimestampInfoKHR()
    : sType(VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR), pNext(nullptr), timeDomain() {}

safe_VkCalibratedTimestampInfoKHR::safe_VkCalibratedTimestampInfoKHR(const safe_VkCalibratedTimestampInfoKHR& copy_src) {
    sType = copy_src.sType;
    timeDomain = copy_src.timeDomain;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCalibratedTimestampInfoKHR& safe_VkCalibratedTimestampInfoKHR::operator=(const safe_VkCalibratedTimestampInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    timeDomain = copy_src.timeDomain;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCalibratedTimestampInfoKHR::~safe_VkCalibratedTimestampInfoKHR() { FreePnextChain(pNext); }

void safe_VkCalibratedTimestampInfoKHR::initialize(const VkCalibratedTimestampInfoKHR* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    timeDomain = in_struct->timeDomain;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCalibratedTimestampInfoKHR::initialize(const safe_VkCalibratedTimestampInfoKHR* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    timeDomain = copy_src->timeDomain;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::safe_VkVideoEncodeIntraRefreshCapabilitiesKHR(
    const VkVideoEncodeIntraRefreshCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      intraRefreshModes(in_struct->intraRefreshModes),
      maxIntraRefreshCycleDuration(in_struct->maxIntraRefreshCycleDuration),
      maxIntraRefreshActiveReferencePictures(in_struct->maxIntraRefreshActiveReferencePictures),
      partitionIndependentIntraRefreshRegions(in_struct->partitionIndependentIntraRefreshRegions),
      nonRectangularIntraRefreshRegions(in_struct->nonRectangularIntraRefreshRegions) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::safe_VkVideoEncodeIntraRefreshCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_CAPABILITIES_KHR),
      pNext(nullptr),
      intraRefreshModes(),
      maxIntraRefreshCycleDuration(),
      maxIntraRefreshActiveReferencePictures(),
      partitionIndependentIntraRefreshRegions(),
      nonRectangularIntraRefreshRegions() {}

safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::safe_VkVideoEncodeIntraRefreshCapabilitiesKHR(
    const safe_VkVideoEncodeIntraRefreshCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    intraRefreshModes = copy_src.intraRefreshModes;
    maxIntraRefreshCycleDuration = copy_src.maxIntraRefreshCycleDuration;
    maxIntraRefreshActiveReferencePictures = copy_src.maxIntraRefreshActiveReferencePictures;
    partitionIndependentIntraRefreshRegions = copy_src.partitionIndependentIntraRefreshRegions;
    nonRectangularIntraRefreshRegions = copy_src.nonRectangularIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeIntraRefreshCapabilitiesKHR& safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::operator=(
    const safe_VkVideoEncodeIntraRefreshCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    intraRefreshModes = copy_src.intraRefreshModes;
    maxIntraRefreshCycleDuration = copy_src.maxIntraRefreshCycleDuration;
    maxIntraRefreshActiveReferencePictures = copy_src.maxIntraRefreshActiveReferencePictures;
    partitionIndependentIntraRefreshRegions = copy_src.partitionIndependentIntraRefreshRegions;
    nonRectangularIntraRefreshRegions = copy_src.nonRectangularIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::~safe_VkVideoEncodeIntraRefreshCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::initialize(const VkVideoEncodeIntraRefreshCapabilitiesKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    intraRefreshModes = in_struct->intraRefreshModes;
    maxIntraRefreshCycleDuration = in_struct->maxIntraRefreshCycleDuration;
    maxIntraRefreshActiveReferencePictures = in_struct->maxIntraRefreshActiveReferencePictures;
    partitionIndependentIntraRefreshRegions = in_struct->partitionIndependentIntraRefreshRegions;
    nonRectangularIntraRefreshRegions = in_struct->nonRectangularIntraRefreshRegions;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeIntraRefreshCapabilitiesKHR::initialize(const safe_VkVideoEncodeIntraRefreshCapabilitiesKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    intraRefreshModes = copy_src->intraRefreshModes;
    maxIntraRefreshCycleDuration = copy_src->maxIntraRefreshCycleDuration;
    maxIntraRefreshActiveReferencePictures = copy_src->maxIntraRefreshActiveReferencePictures;
    partitionIndependentIntraRefreshRegions = copy_src->partitionIndependentIntraRefreshRegions;
    nonRectangularIntraRefreshRegions = copy_src->nonRectangularIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR(
    const VkVideoEncodeSessionIntraRefreshCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), intraRefreshMode(in_struct->intraRefreshMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_SESSION_INTRA_REFRESH_CREATE_INFO_KHR), pNext(nullptr), intraRefreshMode() {}

safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR(
    const safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    intraRefreshMode = copy_src.intraRefreshMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR& safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::operator=(
    const safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    intraRefreshMode = copy_src.intraRefreshMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::~safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::initialize(const VkVideoEncodeSessionIntraRefreshCreateInfoKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    intraRefreshMode = in_struct->intraRefreshMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR::initialize(
    const safe_VkVideoEncodeSessionIntraRefreshCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    intraRefreshMode = copy_src->intraRefreshMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeIntraRefreshInfoKHR::safe_VkVideoEncodeIntraRefreshInfoKHR(const VkVideoEncodeIntraRefreshInfoKHR* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      intraRefreshCycleDuration(in_struct->intraRefreshCycleDuration),
      intraRefreshIndex(in_struct->intraRefreshIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeIntraRefreshInfoKHR::safe_VkVideoEncodeIntraRefreshInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_INTRA_REFRESH_INFO_KHR),
      pNext(nullptr),
      intraRefreshCycleDuration(),
      intraRefreshIndex() {}

safe_VkVideoEncodeIntraRefreshInfoKHR::safe_VkVideoEncodeIntraRefreshInfoKHR(
    const safe_VkVideoEncodeIntraRefreshInfoKHR& copy_src) {
    sType = copy_src.sType;
    intraRefreshCycleDuration = copy_src.intraRefreshCycleDuration;
    intraRefreshIndex = copy_src.intraRefreshIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeIntraRefreshInfoKHR& safe_VkVideoEncodeIntraRefreshInfoKHR::operator=(
    const safe_VkVideoEncodeIntraRefreshInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    intraRefreshCycleDuration = copy_src.intraRefreshCycleDuration;
    intraRefreshIndex = copy_src.intraRefreshIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeIntraRefreshInfoKHR::~safe_VkVideoEncodeIntraRefreshInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeIntraRefreshInfoKHR::initialize(const VkVideoEncodeIntraRefreshInfoKHR* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    intraRefreshCycleDuration = in_struct->intraRefreshCycleDuration;
    intraRefreshIndex = in_struct->intraRefreshIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeIntraRefreshInfoKHR::initialize(const safe_VkVideoEncodeIntraRefreshInfoKHR* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    intraRefreshCycleDuration = copy_src->intraRefreshCycleDuration;
    intraRefreshIndex = copy_src->intraRefreshIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoReferenceIntraRefreshInfoKHR::safe_VkVideoReferenceIntraRefreshInfoKHR(
    const VkVideoReferenceIntraRefreshInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), dirtyIntraRefreshRegions(in_struct->dirtyIntraRefreshRegions) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoReferenceIntraRefreshInfoKHR::safe_VkVideoReferenceIntraRefreshInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_REFERENCE_INTRA_REFRESH_INFO_KHR), pNext(nullptr), dirtyIntraRefreshRegions() {}

safe_VkVideoReferenceIntraRefreshInfoKHR::safe_VkVideoReferenceIntraRefreshInfoKHR(
    const safe_VkVideoReferenceIntraRefreshInfoKHR& copy_src) {
    sType = copy_src.sType;
    dirtyIntraRefreshRegions = copy_src.dirtyIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoReferenceIntraRefreshInfoKHR& safe_VkVideoReferenceIntraRefreshInfoKHR::operator=(
    const safe_VkVideoReferenceIntraRefreshInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dirtyIntraRefreshRegions = copy_src.dirtyIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoReferenceIntraRefreshInfoKHR::~safe_VkVideoReferenceIntraRefreshInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoReferenceIntraRefreshInfoKHR::initialize(const VkVideoReferenceIntraRefreshInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dirtyIntraRefreshRegions = in_struct->dirtyIntraRefreshRegions;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoReferenceIntraRefreshInfoKHR::initialize(const safe_VkVideoReferenceIntraRefreshInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dirtyIntraRefreshRegions = copy_src->dirtyIntraRefreshRegions;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR(
    const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), videoEncodeIntraRefresh(in_struct->videoEncodeIntraRefresh) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_INTRA_REFRESH_FEATURES_KHR), pNext(nullptr), videoEncodeIntraRefresh() {}

safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR(
    const safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoEncodeIntraRefresh = copy_src.videoEncodeIntraRefresh;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR& safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoEncodeIntraRefresh = copy_src.videoEncodeIntraRefresh;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::~safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::initialize(
    const VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoEncodeIntraRefresh = in_struct->videoEncodeIntraRefresh;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceVideoEncodeIntraRefreshFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoEncodeIntraRefresh = copy_src->videoEncodeIntraRefresh;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::safe_VkVideoEncodeQuantizationMapCapabilitiesKHR(
    const VkVideoEncodeQuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxQuantizationMapExtent(in_struct->maxQuantizationMapExtent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::safe_VkVideoEncodeQuantizationMapCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_CAPABILITIES_KHR), pNext(nullptr), maxQuantizationMapExtent() {}

safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::safe_VkVideoEncodeQuantizationMapCapabilitiesKHR(
    const safe_VkVideoEncodeQuantizationMapCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    maxQuantizationMapExtent = copy_src.maxQuantizationMapExtent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQuantizationMapCapabilitiesKHR& safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::operator=(
    const safe_VkVideoEncodeQuantizationMapCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxQuantizationMapExtent = copy_src.maxQuantizationMapExtent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::~safe_VkVideoEncodeQuantizationMapCapabilitiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::initialize(const VkVideoEncodeQuantizationMapCapabilitiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxQuantizationMapExtent = in_struct->maxQuantizationMapExtent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQuantizationMapCapabilitiesKHR::initialize(const safe_VkVideoEncodeQuantizationMapCapabilitiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxQuantizationMapExtent = copy_src->maxQuantizationMapExtent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoFormatQuantizationMapPropertiesKHR::safe_VkVideoFormatQuantizationMapPropertiesKHR(
    const VkVideoFormatQuantizationMapPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), quantizationMapTexelSize(in_struct->quantizationMapTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoFormatQuantizationMapPropertiesKHR::safe_VkVideoFormatQuantizationMapPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_FORMAT_QUANTIZATION_MAP_PROPERTIES_KHR), pNext(nullptr), quantizationMapTexelSize() {}

safe_VkVideoFormatQuantizationMapPropertiesKHR::safe_VkVideoFormatQuantizationMapPropertiesKHR(
    const safe_VkVideoFormatQuantizationMapPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    quantizationMapTexelSize = copy_src.quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoFormatQuantizationMapPropertiesKHR& safe_VkVideoFormatQuantizationMapPropertiesKHR::operator=(
    const safe_VkVideoFormatQuantizationMapPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    quantizationMapTexelSize = copy_src.quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoFormatQuantizationMapPropertiesKHR::~safe_VkVideoFormatQuantizationMapPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoFormatQuantizationMapPropertiesKHR::initialize(const VkVideoFormatQuantizationMapPropertiesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    quantizationMapTexelSize = in_struct->quantizationMapTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoFormatQuantizationMapPropertiesKHR::initialize(const safe_VkVideoFormatQuantizationMapPropertiesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    quantizationMapTexelSize = copy_src->quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeQuantizationMapInfoKHR::safe_VkVideoEncodeQuantizationMapInfoKHR(
    const VkVideoEncodeQuantizationMapInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      quantizationMap(in_struct->quantizationMap),
      quantizationMapExtent(in_struct->quantizationMapExtent) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQuantizationMapInfoKHR::safe_VkVideoEncodeQuantizationMapInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_INFO_KHR), pNext(nullptr), quantizationMap(), quantizationMapExtent() {}

safe_VkVideoEncodeQuantizationMapInfoKHR::safe_VkVideoEncodeQuantizationMapInfoKHR(
    const safe_VkVideoEncodeQuantizationMapInfoKHR& copy_src) {
    sType = copy_src.sType;
    quantizationMap = copy_src.quantizationMap;
    quantizationMapExtent = copy_src.quantizationMapExtent;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQuantizationMapInfoKHR& safe_VkVideoEncodeQuantizationMapInfoKHR::operator=(
    const safe_VkVideoEncodeQuantizationMapInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    quantizationMap = copy_src.quantizationMap;
    quantizationMapExtent = copy_src.quantizationMapExtent;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQuantizationMapInfoKHR::~safe_VkVideoEncodeQuantizationMapInfoKHR() { FreePnextChain(pNext); }

void safe_VkVideoEncodeQuantizationMapInfoKHR::initialize(const VkVideoEncodeQuantizationMapInfoKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    quantizationMap = in_struct->quantizationMap;
    quantizationMapExtent = in_struct->quantizationMapExtent;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQuantizationMapInfoKHR::initialize(const safe_VkVideoEncodeQuantizationMapInfoKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    quantizationMap = copy_src->quantizationMap;
    quantizationMapExtent = copy_src->quantizationMapExtent;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR(
    const VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), quantizationMapTexelSize(in_struct->quantizationMapTexelSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUANTIZATION_MAP_SESSION_PARAMETERS_CREATE_INFO_KHR),
      pNext(nullptr),
      quantizationMapTexelSize() {}

safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR(
    const safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    quantizationMapTexelSize = copy_src.quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR&
safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::operator=(
    const safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    quantizationMapTexelSize = copy_src.quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::
    ~safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::initialize(
    const VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    quantizationMapTexelSize = in_struct->quantizationMapTexelSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR::initialize(
    const safe_VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    quantizationMapTexelSize = copy_src->quantizationMapTexelSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(
    const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), videoEncodeQuantizationMap(in_struct->videoEncodeQuantizationMap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_ENCODE_QUANTIZATION_MAP_FEATURES_KHR),
      pNext(nullptr),
      videoEncodeQuantizationMap() {}

safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR(
    const safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoEncodeQuantizationMap = copy_src.videoEncodeQuantizationMap;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR& safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoEncodeQuantizationMap = copy_src.videoEncodeQuantizationMap;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::~safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::initialize(
    const VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoEncodeQuantizationMap = in_struct->videoEncodeQuantizationMap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoEncodeQuantizationMap = copy_src->videoEncodeQuantizationMap;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR(
    const VkVideoEncodeH264QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minQpDelta(in_struct->minQpDelta), maxQpDelta(in_struct->maxQpDelta) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUANTIZATION_MAP_CAPABILITIES_KHR), pNext(nullptr), minQpDelta(), maxQpDelta() {}

safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR(
    const safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    minQpDelta = copy_src.minQpDelta;
    maxQpDelta = copy_src.maxQpDelta;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR& safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::operator=(
    const safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minQpDelta = copy_src.minQpDelta;
    maxQpDelta = copy_src.maxQpDelta;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::~safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::initialize(
    const VkVideoEncodeH264QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minQpDelta = in_struct->minQpDelta;
    maxQpDelta = in_struct->maxQpDelta;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR::initialize(
    const safe_VkVideoEncodeH264QuantizationMapCapabilitiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minQpDelta = copy_src->minQpDelta;
    maxQpDelta = copy_src->maxQpDelta;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR(
    const VkVideoEncodeH265QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minQpDelta(in_struct->minQpDelta), maxQpDelta(in_struct->maxQpDelta) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUANTIZATION_MAP_CAPABILITIES_KHR), pNext(nullptr), minQpDelta(), maxQpDelta() {}

safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR(
    const safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    minQpDelta = copy_src.minQpDelta;
    maxQpDelta = copy_src.maxQpDelta;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR& safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::operator=(
    const safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minQpDelta = copy_src.minQpDelta;
    maxQpDelta = copy_src.maxQpDelta;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::~safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::initialize(
    const VkVideoEncodeH265QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minQpDelta = in_struct->minQpDelta;
    maxQpDelta = in_struct->maxQpDelta;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR::initialize(
    const safe_VkVideoEncodeH265QuantizationMapCapabilitiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minQpDelta = copy_src->minQpDelta;
    maxQpDelta = copy_src->maxQpDelta;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoFormatH265QuantizationMapPropertiesKHR::safe_VkVideoFormatH265QuantizationMapPropertiesKHR(
    const VkVideoFormatH265QuantizationMapPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), compatibleCtbSizes(in_struct->compatibleCtbSizes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoFormatH265QuantizationMapPropertiesKHR::safe_VkVideoFormatH265QuantizationMapPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_FORMAT_H265_QUANTIZATION_MAP_PROPERTIES_KHR), pNext(nullptr), compatibleCtbSizes() {}

safe_VkVideoFormatH265QuantizationMapPropertiesKHR::safe_VkVideoFormatH265QuantizationMapPropertiesKHR(
    const safe_VkVideoFormatH265QuantizationMapPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    compatibleCtbSizes = copy_src.compatibleCtbSizes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoFormatH265QuantizationMapPropertiesKHR& safe_VkVideoFormatH265QuantizationMapPropertiesKHR::operator=(
    const safe_VkVideoFormatH265QuantizationMapPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    compatibleCtbSizes = copy_src.compatibleCtbSizes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoFormatH265QuantizationMapPropertiesKHR::~safe_VkVideoFormatH265QuantizationMapPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoFormatH265QuantizationMapPropertiesKHR::initialize(const VkVideoFormatH265QuantizationMapPropertiesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    compatibleCtbSizes = in_struct->compatibleCtbSizes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoFormatH265QuantizationMapPropertiesKHR::initialize(
    const safe_VkVideoFormatH265QuantizationMapPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    compatibleCtbSizes = copy_src->compatibleCtbSizes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR(
    const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), minQIndexDelta(in_struct->minQIndexDelta), maxQIndexDelta(in_struct->maxQIndexDelta) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_QUANTIZATION_MAP_CAPABILITIES_KHR),
      pNext(nullptr),
      minQIndexDelta(),
      maxQIndexDelta() {}

safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR(
    const safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR& copy_src) {
    sType = copy_src.sType;
    minQIndexDelta = copy_src.minQIndexDelta;
    maxQIndexDelta = copy_src.maxQIndexDelta;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR& safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::operator=(
    const safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minQIndexDelta = copy_src.minQIndexDelta;
    maxQIndexDelta = copy_src.maxQIndexDelta;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::~safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::initialize(
    const VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minQIndexDelta = in_struct->minQIndexDelta;
    maxQIndexDelta = in_struct->maxQIndexDelta;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR::initialize(
    const safe_VkVideoEncodeAV1QuantizationMapCapabilitiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minQIndexDelta = copy_src->minQIndexDelta;
    maxQIndexDelta = copy_src->maxQIndexDelta;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::safe_VkVideoFormatAV1QuantizationMapPropertiesKHR(
    const VkVideoFormatAV1QuantizationMapPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), compatibleSuperblockSizes(in_struct->compatibleSuperblockSizes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::safe_VkVideoFormatAV1QuantizationMapPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_FORMAT_AV1_QUANTIZATION_MAP_PROPERTIES_KHR), pNext(nullptr), compatibleSuperblockSizes() {}

safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::safe_VkVideoFormatAV1QuantizationMapPropertiesKHR(
    const safe_VkVideoFormatAV1QuantizationMapPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    compatibleSuperblockSizes = copy_src.compatibleSuperblockSizes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkVideoFormatAV1QuantizationMapPropertiesKHR& safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::operator=(
    const safe_VkVideoFormatAV1QuantizationMapPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    compatibleSuperblockSizes = copy_src.compatibleSuperblockSizes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::~safe_VkVideoFormatAV1QuantizationMapPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::initialize(const VkVideoFormatAV1QuantizationMapPropertiesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    compatibleSuperblockSizes = in_struct->compatibleSuperblockSizes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkVideoFormatAV1QuantizationMapPropertiesKHR::initialize(
    const safe_VkVideoFormatAV1QuantizationMapPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    compatibleSuperblockSizes = copy_src->compatibleSuperblockSizes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(
    const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderRelaxedExtendedInstruction(in_struct->shaderRelaxedExtendedInstruction) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR),
      pNext(nullptr),
      shaderRelaxedExtendedInstruction() {}

safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR(
    const safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    shaderRelaxedExtendedInstruction = copy_src.shaderRelaxedExtendedInstruction;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR&
safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderRelaxedExtendedInstruction = copy_src.shaderRelaxedExtendedInstruction;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::
    ~safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::initialize(
    const VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderRelaxedExtendedInstruction = in_struct->shaderRelaxedExtendedInstruction;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderRelaxedExtendedInstruction = copy_src->shaderRelaxedExtendedInstruction;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance7FeaturesKHR::safe_VkPhysicalDeviceMaintenance7FeaturesKHR(
    const VkPhysicalDeviceMaintenance7FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maintenance7(in_struct->maintenance7) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance7FeaturesKHR::safe_VkPhysicalDeviceMaintenance7FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR), pNext(nullptr), maintenance7() {}

safe_VkPhysicalDeviceMaintenance7FeaturesKHR::safe_VkPhysicalDeviceMaintenance7FeaturesKHR(
    const safe_VkPhysicalDeviceMaintenance7FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    maintenance7 = copy_src.maintenance7;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance7FeaturesKHR& safe_VkPhysicalDeviceMaintenance7FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance7FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maintenance7 = copy_src.maintenance7;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance7FeaturesKHR::~safe_VkPhysicalDeviceMaintenance7FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance7FeaturesKHR::initialize(const VkPhysicalDeviceMaintenance7FeaturesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maintenance7 = in_struct->maintenance7;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance7FeaturesKHR::initialize(const safe_VkPhysicalDeviceMaintenance7FeaturesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maintenance7 = copy_src->maintenance7;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance7PropertiesKHR::safe_VkPhysicalDeviceMaintenance7PropertiesKHR(
    const VkPhysicalDeviceMaintenance7PropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      robustFragmentShadingRateAttachmentAccess(in_struct->robustFragmentShadingRateAttachmentAccess),
      separateDepthStencilAttachmentAccess(in_struct->separateDepthStencilAttachmentAccess),
      maxDescriptorSetTotalUniformBuffersDynamic(in_struct->maxDescriptorSetTotalUniformBuffersDynamic),
      maxDescriptorSetTotalStorageBuffersDynamic(in_struct->maxDescriptorSetTotalStorageBuffersDynamic),
      maxDescriptorSetTotalBuffersDynamic(in_struct->maxDescriptorSetTotalBuffersDynamic),
      maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic(
          in_struct->maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic),
      maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic(
          in_struct->maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic),
      maxDescriptorSetUpdateAfterBindTotalBuffersDynamic(in_struct->maxDescriptorSetUpdateAfterBindTotalBuffersDynamic) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance7PropertiesKHR::safe_VkPhysicalDeviceMaintenance7PropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR),
      pNext(nullptr),
      robustFragmentShadingRateAttachmentAccess(),
      separateDepthStencilAttachmentAccess(),
      maxDescriptorSetTotalUniformBuffersDynamic(),
      maxDescriptorSetTotalStorageBuffersDynamic(),
      maxDescriptorSetTotalBuffersDynamic(),
      maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic(),
      maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic(),
      maxDescriptorSetUpdateAfterBindTotalBuffersDynamic() {}

safe_VkPhysicalDeviceMaintenance7PropertiesKHR::safe_VkPhysicalDeviceMaintenance7PropertiesKHR(
    const safe_VkPhysicalDeviceMaintenance7PropertiesKHR& copy_src) {
    sType = copy_src.sType;
    robustFragmentShadingRateAttachmentAccess = copy_src.robustFragmentShadingRateAttachmentAccess;
    separateDepthStencilAttachmentAccess = copy_src.separateDepthStencilAttachmentAccess;
    maxDescriptorSetTotalUniformBuffersDynamic = copy_src.maxDescriptorSetTotalUniformBuffersDynamic;
    maxDescriptorSetTotalStorageBuffersDynamic = copy_src.maxDescriptorSetTotalStorageBuffersDynamic;
    maxDescriptorSetTotalBuffersDynamic = copy_src.maxDescriptorSetTotalBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalBuffersDynamic;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance7PropertiesKHR& safe_VkPhysicalDeviceMaintenance7PropertiesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance7PropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    robustFragmentShadingRateAttachmentAccess = copy_src.robustFragmentShadingRateAttachmentAccess;
    separateDepthStencilAttachmentAccess = copy_src.separateDepthStencilAttachmentAccess;
    maxDescriptorSetTotalUniformBuffersDynamic = copy_src.maxDescriptorSetTotalUniformBuffersDynamic;
    maxDescriptorSetTotalStorageBuffersDynamic = copy_src.maxDescriptorSetTotalStorageBuffersDynamic;
    maxDescriptorSetTotalBuffersDynamic = copy_src.maxDescriptorSetTotalBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalBuffersDynamic = copy_src.maxDescriptorSetUpdateAfterBindTotalBuffersDynamic;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance7PropertiesKHR::~safe_VkPhysicalDeviceMaintenance7PropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance7PropertiesKHR::initialize(const VkPhysicalDeviceMaintenance7PropertiesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    robustFragmentShadingRateAttachmentAccess = in_struct->robustFragmentShadingRateAttachmentAccess;
    separateDepthStencilAttachmentAccess = in_struct->separateDepthStencilAttachmentAccess;
    maxDescriptorSetTotalUniformBuffersDynamic = in_struct->maxDescriptorSetTotalUniformBuffersDynamic;
    maxDescriptorSetTotalStorageBuffersDynamic = in_struct->maxDescriptorSetTotalStorageBuffersDynamic;
    maxDescriptorSetTotalBuffersDynamic = in_struct->maxDescriptorSetTotalBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic =
        in_struct->maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic =
        in_struct->maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalBuffersDynamic = in_struct->maxDescriptorSetUpdateAfterBindTotalBuffersDynamic;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance7PropertiesKHR::initialize(const safe_VkPhysicalDeviceMaintenance7PropertiesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    robustFragmentShadingRateAttachmentAccess = copy_src->robustFragmentShadingRateAttachmentAccess;
    separateDepthStencilAttachmentAccess = copy_src->separateDepthStencilAttachmentAccess;
    maxDescriptorSetTotalUniformBuffersDynamic = copy_src->maxDescriptorSetTotalUniformBuffersDynamic;
    maxDescriptorSetTotalStorageBuffersDynamic = copy_src->maxDescriptorSetTotalStorageBuffersDynamic;
    maxDescriptorSetTotalBuffersDynamic = copy_src->maxDescriptorSetTotalBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic = copy_src->maxDescriptorSetUpdateAfterBindTotalUniformBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic = copy_src->maxDescriptorSetUpdateAfterBindTotalStorageBuffersDynamic;
    maxDescriptorSetUpdateAfterBindTotalBuffersDynamic = copy_src->maxDescriptorSetUpdateAfterBindTotalBuffersDynamic;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceLayeredApiPropertiesKHR::safe_VkPhysicalDeviceLayeredApiPropertiesKHR(
    const VkPhysicalDeviceLayeredApiPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), vendorID(in_struct->vendorID), deviceID(in_struct->deviceID), layeredAPI(in_struct->layeredAPI) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < VK_MAX_PHYSICAL_DEVICE_NAME_SIZE; ++i) {
        deviceName[i] = in_struct->deviceName[i];
    }
}

safe_VkPhysicalDeviceLayeredApiPropertiesKHR::safe_VkPhysicalDeviceLayeredApiPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_KHR), pNext(nullptr), vendorID(), deviceID(), layeredAPI() {}

safe_VkPhysicalDeviceLayeredApiPropertiesKHR::safe_VkPhysicalDeviceLayeredApiPropertiesKHR(
    const safe_VkPhysicalDeviceLayeredApiPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    vendorID = copy_src.vendorID;
    deviceID = copy_src.deviceID;
    layeredAPI = copy_src.layeredAPI;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_PHYSICAL_DEVICE_NAME_SIZE; ++i) {
        deviceName[i] = copy_src.deviceName[i];
    }
}

safe_VkPhysicalDeviceLayeredApiPropertiesKHR& safe_VkPhysicalDeviceLayeredApiPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceLayeredApiPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vendorID = copy_src.vendorID;
    deviceID = copy_src.deviceID;
    layeredAPI = copy_src.layeredAPI;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < VK_MAX_PHYSICAL_DEVICE_NAME_SIZE; ++i) {
        deviceName[i] = copy_src.deviceName[i];
    }

    return *this;
}

safe_VkPhysicalDeviceLayeredApiPropertiesKHR::~safe_VkPhysicalDeviceLayeredApiPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceLayeredApiPropertiesKHR::initialize(const VkPhysicalDeviceLayeredApiPropertiesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vendorID = in_struct->vendorID;
    deviceID = in_struct->deviceID;
    layeredAPI = in_struct->layeredAPI;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < VK_MAX_PHYSICAL_DEVICE_NAME_SIZE; ++i) {
        deviceName[i] = in_struct->deviceName[i];
    }
}

void safe_VkPhysicalDeviceLayeredApiPropertiesKHR::initialize(const safe_VkPhysicalDeviceLayeredApiPropertiesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vendorID = copy_src->vendorID;
    deviceID = copy_src->deviceID;
    layeredAPI = copy_src->layeredAPI;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < VK_MAX_PHYSICAL_DEVICE_NAME_SIZE; ++i) {
        deviceName[i] = copy_src->deviceName[i];
    }
}

safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::safe_VkPhysicalDeviceLayeredApiPropertiesListKHR(
    const VkPhysicalDeviceLayeredApiPropertiesListKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), layeredApiCount(in_struct->layeredApiCount), pLayeredApis(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (layeredApiCount && in_struct->pLayeredApis) {
        pLayeredApis = new safe_VkPhysicalDeviceLayeredApiPropertiesKHR[layeredApiCount];
        for (uint32_t i = 0; i < layeredApiCount; ++i) {
            pLayeredApis[i].initialize(&in_struct->pLayeredApis[i]);
        }
    }
}

safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::safe_VkPhysicalDeviceLayeredApiPropertiesListKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_PROPERTIES_LIST_KHR),
      pNext(nullptr),
      layeredApiCount(),
      pLayeredApis(nullptr) {}

safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::safe_VkPhysicalDeviceLayeredApiPropertiesListKHR(
    const safe_VkPhysicalDeviceLayeredApiPropertiesListKHR& copy_src) {
    sType = copy_src.sType;
    layeredApiCount = copy_src.layeredApiCount;
    pLayeredApis = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layeredApiCount && copy_src.pLayeredApis) {
        pLayeredApis = new safe_VkPhysicalDeviceLayeredApiPropertiesKHR[layeredApiCount];
        for (uint32_t i = 0; i < layeredApiCount; ++i) {
            pLayeredApis[i].initialize(&copy_src.pLayeredApis[i]);
        }
    }
}

safe_VkPhysicalDeviceLayeredApiPropertiesListKHR& safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::operator=(
    const safe_VkPhysicalDeviceLayeredApiPropertiesListKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pLayeredApis) delete[] pLayeredApis;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    layeredApiCount = copy_src.layeredApiCount;
    pLayeredApis = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (layeredApiCount && copy_src.pLayeredApis) {
        pLayeredApis = new safe_VkPhysicalDeviceLayeredApiPropertiesKHR[layeredApiCount];
        for (uint32_t i = 0; i < layeredApiCount; ++i) {
            pLayeredApis[i].initialize(&copy_src.pLayeredApis[i]);
        }
    }

    return *this;
}

safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::~safe_VkPhysicalDeviceLayeredApiPropertiesListKHR() {
    if (pLayeredApis) delete[] pLayeredApis;
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::initialize(const VkPhysicalDeviceLayeredApiPropertiesListKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pLayeredApis) delete[] pLayeredApis;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    layeredApiCount = in_struct->layeredApiCount;
    pLayeredApis = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (layeredApiCount && in_struct->pLayeredApis) {
        pLayeredApis = new safe_VkPhysicalDeviceLayeredApiPropertiesKHR[layeredApiCount];
        for (uint32_t i = 0; i < layeredApiCount; ++i) {
            pLayeredApis[i].initialize(&in_struct->pLayeredApis[i]);
        }
    }
}

void safe_VkPhysicalDeviceLayeredApiPropertiesListKHR::initialize(const safe_VkPhysicalDeviceLayeredApiPropertiesListKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    layeredApiCount = copy_src->layeredApiCount;
    pLayeredApis = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (layeredApiCount && copy_src->pLayeredApis) {
        pLayeredApis = new safe_VkPhysicalDeviceLayeredApiPropertiesKHR[layeredApiCount];
        for (uint32_t i = 0; i < layeredApiCount; ++i) {
            pLayeredApis[i].initialize(&copy_src->pLayeredApis[i]);
        }
    }
}

safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR(
    const VkPhysicalDeviceLayeredApiVulkanPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), properties(&in_struct->properties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_API_VULKAN_PROPERTIES_KHR), pNext(nullptr) {}

safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR(
    const safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    properties.initialize(&copy_src.properties);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR& safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::operator=(
    const safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    properties.initialize(&copy_src.properties);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::~safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::initialize(const VkPhysicalDeviceLayeredApiVulkanPropertiesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    properties.initialize(&in_struct->properties);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR::initialize(
    const safe_VkPhysicalDeviceLayeredApiVulkanPropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    properties.initialize(&copy_src->properties);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance8FeaturesKHR::safe_VkPhysicalDeviceMaintenance8FeaturesKHR(
    const VkPhysicalDeviceMaintenance8FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maintenance8(in_struct->maintenance8) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance8FeaturesKHR::safe_VkPhysicalDeviceMaintenance8FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR), pNext(nullptr), maintenance8() {}

safe_VkPhysicalDeviceMaintenance8FeaturesKHR::safe_VkPhysicalDeviceMaintenance8FeaturesKHR(
    const safe_VkPhysicalDeviceMaintenance8FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    maintenance8 = copy_src.maintenance8;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance8FeaturesKHR& safe_VkPhysicalDeviceMaintenance8FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance8FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maintenance8 = copy_src.maintenance8;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance8FeaturesKHR::~safe_VkPhysicalDeviceMaintenance8FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance8FeaturesKHR::initialize(const VkPhysicalDeviceMaintenance8FeaturesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maintenance8 = in_struct->maintenance8;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance8FeaturesKHR::initialize(const safe_VkPhysicalDeviceMaintenance8FeaturesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maintenance8 = copy_src->maintenance8;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryBarrierAccessFlags3KHR::safe_VkMemoryBarrierAccessFlags3KHR(const VkMemoryBarrierAccessFlags3KHR* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), srcAccessMask3(in_struct->srcAccessMask3), dstAccessMask3(in_struct->dstAccessMask3) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryBarrierAccessFlags3KHR::safe_VkMemoryBarrierAccessFlags3KHR()
    : sType(VK_STRUCTURE_TYPE_MEMORY_BARRIER_ACCESS_FLAGS_3_KHR), pNext(nullptr), srcAccessMask3(), dstAccessMask3() {}

safe_VkMemoryBarrierAccessFlags3KHR::safe_VkMemoryBarrierAccessFlags3KHR(const safe_VkMemoryBarrierAccessFlags3KHR& copy_src) {
    sType = copy_src.sType;
    srcAccessMask3 = copy_src.srcAccessMask3;
    dstAccessMask3 = copy_src.dstAccessMask3;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryBarrierAccessFlags3KHR& safe_VkMemoryBarrierAccessFlags3KHR::operator=(
    const safe_VkMemoryBarrierAccessFlags3KHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    srcAccessMask3 = copy_src.srcAccessMask3;
    dstAccessMask3 = copy_src.dstAccessMask3;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryBarrierAccessFlags3KHR::~safe_VkMemoryBarrierAccessFlags3KHR() { FreePnextChain(pNext); }

void safe_VkMemoryBarrierAccessFlags3KHR::initialize(const VkMemoryBarrierAccessFlags3KHR* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    srcAccessMask3 = in_struct->srcAccessMask3;
    dstAccessMask3 = in_struct->dstAccessMask3;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryBarrierAccessFlags3KHR::initialize(const safe_VkMemoryBarrierAccessFlags3KHR* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    srcAccessMask3 = copy_src->srcAccessMask3;
    dstAccessMask3 = copy_src->dstAccessMask3;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance9FeaturesKHR::safe_VkPhysicalDeviceMaintenance9FeaturesKHR(
    const VkPhysicalDeviceMaintenance9FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maintenance9(in_struct->maintenance9) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance9FeaturesKHR::safe_VkPhysicalDeviceMaintenance9FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR), pNext(nullptr), maintenance9() {}

safe_VkPhysicalDeviceMaintenance9FeaturesKHR::safe_VkPhysicalDeviceMaintenance9FeaturesKHR(
    const safe_VkPhysicalDeviceMaintenance9FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    maintenance9 = copy_src.maintenance9;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance9FeaturesKHR& safe_VkPhysicalDeviceMaintenance9FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance9FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maintenance9 = copy_src.maintenance9;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance9FeaturesKHR::~safe_VkPhysicalDeviceMaintenance9FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance9FeaturesKHR::initialize(const VkPhysicalDeviceMaintenance9FeaturesKHR* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maintenance9 = in_struct->maintenance9;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance9FeaturesKHR::initialize(const safe_VkPhysicalDeviceMaintenance9FeaturesKHR* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maintenance9 = copy_src->maintenance9;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMaintenance9PropertiesKHR::safe_VkPhysicalDeviceMaintenance9PropertiesKHR(
    const VkPhysicalDeviceMaintenance9PropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      image2DViewOf3DSparse(in_struct->image2DViewOf3DSparse),
      defaultVertexAttributeValue(in_struct->defaultVertexAttributeValue) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMaintenance9PropertiesKHR::safe_VkPhysicalDeviceMaintenance9PropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR),
      pNext(nullptr),
      image2DViewOf3DSparse(),
      defaultVertexAttributeValue() {}

safe_VkPhysicalDeviceMaintenance9PropertiesKHR::safe_VkPhysicalDeviceMaintenance9PropertiesKHR(
    const safe_VkPhysicalDeviceMaintenance9PropertiesKHR& copy_src) {
    sType = copy_src.sType;
    image2DViewOf3DSparse = copy_src.image2DViewOf3DSparse;
    defaultVertexAttributeValue = copy_src.defaultVertexAttributeValue;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMaintenance9PropertiesKHR& safe_VkPhysicalDeviceMaintenance9PropertiesKHR::operator=(
    const safe_VkPhysicalDeviceMaintenance9PropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image2DViewOf3DSparse = copy_src.image2DViewOf3DSparse;
    defaultVertexAttributeValue = copy_src.defaultVertexAttributeValue;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMaintenance9PropertiesKHR::~safe_VkPhysicalDeviceMaintenance9PropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMaintenance9PropertiesKHR::initialize(const VkPhysicalDeviceMaintenance9PropertiesKHR* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image2DViewOf3DSparse = in_struct->image2DViewOf3DSparse;
    defaultVertexAttributeValue = in_struct->defaultVertexAttributeValue;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMaintenance9PropertiesKHR::initialize(const safe_VkPhysicalDeviceMaintenance9PropertiesKHR* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image2DViewOf3DSparse = copy_src->image2DViewOf3DSparse;
    defaultVertexAttributeValue = copy_src->defaultVertexAttributeValue;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyOwnershipTransferPropertiesKHR::safe_VkQueueFamilyOwnershipTransferPropertiesKHR(
    const VkQueueFamilyOwnershipTransferPropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), optimalImageTransferToQueueFamilies(in_struct->optimalImageTransferToQueueFamilies) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyOwnershipTransferPropertiesKHR::safe_VkQueueFamilyOwnershipTransferPropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_OWNERSHIP_TRANSFER_PROPERTIES_KHR),
      pNext(nullptr),
      optimalImageTransferToQueueFamilies() {}

safe_VkQueueFamilyOwnershipTransferPropertiesKHR::safe_VkQueueFamilyOwnershipTransferPropertiesKHR(
    const safe_VkQueueFamilyOwnershipTransferPropertiesKHR& copy_src) {
    sType = copy_src.sType;
    optimalImageTransferToQueueFamilies = copy_src.optimalImageTransferToQueueFamilies;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyOwnershipTransferPropertiesKHR& safe_VkQueueFamilyOwnershipTransferPropertiesKHR::operator=(
    const safe_VkQueueFamilyOwnershipTransferPropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    optimalImageTransferToQueueFamilies = copy_src.optimalImageTransferToQueueFamilies;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyOwnershipTransferPropertiesKHR::~safe_VkQueueFamilyOwnershipTransferPropertiesKHR() { FreePnextChain(pNext); }

void safe_VkQueueFamilyOwnershipTransferPropertiesKHR::initialize(const VkQueueFamilyOwnershipTransferPropertiesKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    optimalImageTransferToQueueFamilies = in_struct->optimalImageTransferToQueueFamilies;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyOwnershipTransferPropertiesKHR::initialize(const safe_VkQueueFamilyOwnershipTransferPropertiesKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    optimalImageTransferToQueueFamilies = copy_src->optimalImageTransferToQueueFamilies;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR(
    const VkPhysicalDeviceVideoMaintenance2FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), videoMaintenance2(in_struct->videoMaintenance2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VIDEO_MAINTENANCE_2_FEATURES_KHR), pNext(nullptr), videoMaintenance2() {}

safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR(
    const safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    videoMaintenance2 = copy_src.videoMaintenance2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR& safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    videoMaintenance2 = copy_src.videoMaintenance2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::~safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::initialize(const VkPhysicalDeviceVideoMaintenance2FeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    videoMaintenance2 = in_struct->videoMaintenance2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR::initialize(
    const safe_VkPhysicalDeviceVideoMaintenance2FeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    videoMaintenance2 = copy_src->videoMaintenance2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::safe_VkVideoDecodeH264InlineSessionParametersInfoKHR(
    const VkVideoDecodeH264InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pStdSPS(nullptr), pStdPPS(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSPS) {
        pStdSPS = new StdVideoH264SequenceParameterSet(*in_struct->pStdSPS);
    }

    if (in_struct->pStdPPS) {
        pStdPPS = new StdVideoH264PictureParameterSet(*in_struct->pStdPPS);
    }
}

safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::safe_VkVideoDecodeH264InlineSessionParametersInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_INLINE_SESSION_PARAMETERS_INFO_KHR),
      pNext(nullptr),
      pStdSPS(nullptr),
      pStdPPS(nullptr) {}

safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::safe_VkVideoDecodeH264InlineSessionParametersInfoKHR(
    const safe_VkVideoDecodeH264InlineSessionParametersInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPS) {
        pStdSPS = new StdVideoH264SequenceParameterSet(*copy_src.pStdSPS);
    }

    if (copy_src.pStdPPS) {
        pStdPPS = new StdVideoH264PictureParameterSet(*copy_src.pStdPPS);
    }
}

safe_VkVideoDecodeH264InlineSessionParametersInfoKHR& safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::operator=(
    const safe_VkVideoDecodeH264InlineSessionParametersInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSPS) {
        pStdSPS = new StdVideoH264SequenceParameterSet(*copy_src.pStdSPS);
    }

    if (copy_src.pStdPPS) {
        pStdPPS = new StdVideoH264PictureParameterSet(*copy_src.pStdPPS);
    }

    return *this;
}

safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::~safe_VkVideoDecodeH264InlineSessionParametersInfoKHR() {
    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::initialize(
    const VkVideoDecodeH264InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSPS) {
        pStdSPS = new StdVideoH264SequenceParameterSet(*in_struct->pStdSPS);
    }

    if (in_struct->pStdPPS) {
        pStdPPS = new StdVideoH264PictureParameterSet(*in_struct->pStdPPS);
    }
}

void safe_VkVideoDecodeH264InlineSessionParametersInfoKHR::initialize(
    const safe_VkVideoDecodeH264InlineSessionParametersInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSPS) {
        pStdSPS = new StdVideoH264SequenceParameterSet(*copy_src->pStdSPS);
    }

    if (copy_src->pStdPPS) {
        pStdPPS = new StdVideoH264PictureParameterSet(*copy_src->pStdPPS);
    }
}

safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::safe_VkVideoDecodeH265InlineSessionParametersInfoKHR(
    const VkVideoDecodeH265InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pStdVPS(nullptr), pStdSPS(nullptr), pStdPPS(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdVPS) {
        pStdVPS = new StdVideoH265VideoParameterSet(*in_struct->pStdVPS);
    }

    if (in_struct->pStdSPS) {
        pStdSPS = new StdVideoH265SequenceParameterSet(*in_struct->pStdSPS);
    }

    if (in_struct->pStdPPS) {
        pStdPPS = new StdVideoH265PictureParameterSet(*in_struct->pStdPPS);
    }
}

safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::safe_VkVideoDecodeH265InlineSessionParametersInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_INLINE_SESSION_PARAMETERS_INFO_KHR),
      pNext(nullptr),
      pStdVPS(nullptr),
      pStdSPS(nullptr),
      pStdPPS(nullptr) {}

safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::safe_VkVideoDecodeH265InlineSessionParametersInfoKHR(
    const safe_VkVideoDecodeH265InlineSessionParametersInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdVPS = nullptr;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPS) {
        pStdVPS = new StdVideoH265VideoParameterSet(*copy_src.pStdVPS);
    }

    if (copy_src.pStdSPS) {
        pStdSPS = new StdVideoH265SequenceParameterSet(*copy_src.pStdSPS);
    }

    if (copy_src.pStdPPS) {
        pStdPPS = new StdVideoH265PictureParameterSet(*copy_src.pStdPPS);
    }
}

safe_VkVideoDecodeH265InlineSessionParametersInfoKHR& safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::operator=(
    const safe_VkVideoDecodeH265InlineSessionParametersInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdVPS) delete pStdVPS;
    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdVPS = nullptr;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdVPS) {
        pStdVPS = new StdVideoH265VideoParameterSet(*copy_src.pStdVPS);
    }

    if (copy_src.pStdSPS) {
        pStdSPS = new StdVideoH265SequenceParameterSet(*copy_src.pStdSPS);
    }

    if (copy_src.pStdPPS) {
        pStdPPS = new StdVideoH265PictureParameterSet(*copy_src.pStdPPS);
    }

    return *this;
}

safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::~safe_VkVideoDecodeH265InlineSessionParametersInfoKHR() {
    if (pStdVPS) delete pStdVPS;
    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::initialize(
    const VkVideoDecodeH265InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdVPS) delete pStdVPS;
    if (pStdSPS) delete pStdSPS;
    if (pStdPPS) delete pStdPPS;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdVPS = nullptr;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdVPS) {
        pStdVPS = new StdVideoH265VideoParameterSet(*in_struct->pStdVPS);
    }

    if (in_struct->pStdSPS) {
        pStdSPS = new StdVideoH265SequenceParameterSet(*in_struct->pStdSPS);
    }

    if (in_struct->pStdPPS) {
        pStdPPS = new StdVideoH265PictureParameterSet(*in_struct->pStdPPS);
    }
}

void safe_VkVideoDecodeH265InlineSessionParametersInfoKHR::initialize(
    const safe_VkVideoDecodeH265InlineSessionParametersInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdVPS = nullptr;
    pStdSPS = nullptr;
    pStdPPS = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdVPS) {
        pStdVPS = new StdVideoH265VideoParameterSet(*copy_src->pStdVPS);
    }

    if (copy_src->pStdSPS) {
        pStdSPS = new StdVideoH265SequenceParameterSet(*copy_src->pStdSPS);
    }

    if (copy_src->pStdPPS) {
        pStdPPS = new StdVideoH265PictureParameterSet(*copy_src->pStdPPS);
    }
}

safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR(
    const VkVideoDecodeAV1InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pStdSequenceHeader(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }
}

safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR()
    : sType(VK_STRUCTURE_TYPE_VIDEO_DECODE_AV1_INLINE_SESSION_PARAMETERS_INFO_KHR), pNext(nullptr), pStdSequenceHeader(nullptr) {}

safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR(
    const safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR& copy_src) {
    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }
}

safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR& safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::operator=(
    const safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src.pStdSequenceHeader);
    }

    return *this;
}

safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::~safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR() {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);
}

void safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::initialize(
    const VkVideoDecodeAV1InlineSessionParametersInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStdSequenceHeader) delete pStdSequenceHeader;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*in_struct->pStdSequenceHeader);
    }
}

void safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR::initialize(
    const safe_VkVideoDecodeAV1InlineSessionParametersInfoKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pStdSequenceHeader = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pStdSequenceHeader) {
        pStdSequenceHeader = new StdVideoAV1SequenceHeader(*copy_src->pStdSequenceHeader);
    }
}

safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR(
    const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), depthClampZeroOne(in_struct->depthClampZeroOne) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_KHR), pNext(nullptr), depthClampZeroOne() {}

safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR(
    const safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    depthClampZeroOne = copy_src.depthClampZeroOne;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR& safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    depthClampZeroOne = copy_src.depthClampZeroOne;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::~safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::initialize(const VkPhysicalDeviceDepthClampZeroOneFeaturesKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    depthClampZeroOne = in_struct->depthClampZeroOne;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceDepthClampZeroOneFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    depthClampZeroOne = copy_src->depthClampZeroOne;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRobustness2FeaturesKHR::safe_VkPhysicalDeviceRobustness2FeaturesKHR(
    const VkPhysicalDeviceRobustness2FeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      robustBufferAccess2(in_struct->robustBufferAccess2),
      robustImageAccess2(in_struct->robustImageAccess2),
      nullDescriptor(in_struct->nullDescriptor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRobustness2FeaturesKHR::safe_VkPhysicalDeviceRobustness2FeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_KHR),
      pNext(nullptr),
      robustBufferAccess2(),
      robustImageAccess2(),
      nullDescriptor() {}

safe_VkPhysicalDeviceRobustness2FeaturesKHR::safe_VkPhysicalDeviceRobustness2FeaturesKHR(
    const safe_VkPhysicalDeviceRobustness2FeaturesKHR& copy_src) {
    sType = copy_src.sType;
    robustBufferAccess2 = copy_src.robustBufferAccess2;
    robustImageAccess2 = copy_src.robustImageAccess2;
    nullDescriptor = copy_src.nullDescriptor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRobustness2FeaturesKHR& safe_VkPhysicalDeviceRobustness2FeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRobustness2FeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    robustBufferAccess2 = copy_src.robustBufferAccess2;
    robustImageAccess2 = copy_src.robustImageAccess2;
    nullDescriptor = copy_src.nullDescriptor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRobustness2FeaturesKHR::~safe_VkPhysicalDeviceRobustness2FeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRobustness2FeaturesKHR::initialize(const VkPhysicalDeviceRobustness2FeaturesKHR* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    robustBufferAccess2 = in_struct->robustBufferAccess2;
    robustImageAccess2 = in_struct->robustImageAccess2;
    nullDescriptor = in_struct->nullDescriptor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRobustness2FeaturesKHR::initialize(const safe_VkPhysicalDeviceRobustness2FeaturesKHR* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    robustBufferAccess2 = copy_src->robustBufferAccess2;
    robustImageAccess2 = copy_src->robustImageAccess2;
    nullDescriptor = copy_src->nullDescriptor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRobustness2PropertiesKHR::safe_VkPhysicalDeviceRobustness2PropertiesKHR(
    const VkPhysicalDeviceRobustness2PropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      robustStorageBufferAccessSizeAlignment(in_struct->robustStorageBufferAccessSizeAlignment),
      robustUniformBufferAccessSizeAlignment(in_struct->robustUniformBufferAccessSizeAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRobustness2PropertiesKHR::safe_VkPhysicalDeviceRobustness2PropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_KHR),
      pNext(nullptr),
      robustStorageBufferAccessSizeAlignment(),
      robustUniformBufferAccessSizeAlignment() {}

safe_VkPhysicalDeviceRobustness2PropertiesKHR::safe_VkPhysicalDeviceRobustness2PropertiesKHR(
    const safe_VkPhysicalDeviceRobustness2PropertiesKHR& copy_src) {
    sType = copy_src.sType;
    robustStorageBufferAccessSizeAlignment = copy_src.robustStorageBufferAccessSizeAlignment;
    robustUniformBufferAccessSizeAlignment = copy_src.robustUniformBufferAccessSizeAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRobustness2PropertiesKHR& safe_VkPhysicalDeviceRobustness2PropertiesKHR::operator=(
    const safe_VkPhysicalDeviceRobustness2PropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    robustStorageBufferAccessSizeAlignment = copy_src.robustStorageBufferAccessSizeAlignment;
    robustUniformBufferAccessSizeAlignment = copy_src.robustUniformBufferAccessSizeAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRobustness2PropertiesKHR::~safe_VkPhysicalDeviceRobustness2PropertiesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRobustness2PropertiesKHR::initialize(const VkPhysicalDeviceRobustness2PropertiesKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    robustStorageBufferAccessSizeAlignment = in_struct->robustStorageBufferAccessSizeAlignment;
    robustUniformBufferAccessSizeAlignment = in_struct->robustUniformBufferAccessSizeAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRobustness2PropertiesKHR::initialize(const safe_VkPhysicalDeviceRobustness2PropertiesKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    robustStorageBufferAccessSizeAlignment = copy_src->robustStorageBufferAccessSizeAlignment;
    robustUniformBufferAccessSizeAlignment = copy_src->robustUniformBufferAccessSizeAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR(
    const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), presentModeFifoLatestReady(in_struct->presentModeFifoLatestReady) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_KHR),
      pNext(nullptr),
      presentModeFifoLatestReady() {}

safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR(
    const safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    presentModeFifoLatestReady = copy_src.presentModeFifoLatestReady;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR& safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::operator=(
    const safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentModeFifoLatestReady = copy_src.presentModeFifoLatestReady;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::~safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::initialize(
    const VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentModeFifoLatestReady = in_struct->presentModeFifoLatestReady;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR::initialize(
    const safe_VkPhysicalDevicePresentModeFifoLatestReadyFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentModeFifoLatestReady = copy_src->presentModeFifoLatestReady;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR(const VkDeviceOrHostAddressConstKHR* in_struct,
                                                                       PNextCopyState*) {
    initialize(in_struct);
}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR() : hostAddress(nullptr) {}

safe_VkDeviceOrHostAddressConstKHR::safe_VkDeviceOrHostAddressConstKHR(const safe_VkDeviceOrHostAddressConstKHR& copy_src) {
    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;
}

safe_VkDeviceOrHostAddressConstKHR& safe_VkDeviceOrHostAddressConstKHR::operator=(
    const safe_VkDeviceOrHostAddressConstKHR& copy_src) {
    if (&copy_src == this) return *this;

    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;

    return *this;
}

safe_VkDeviceOrHostAddressConstKHR::~safe_VkDeviceOrHostAddressConstKHR() {}

void safe_VkDeviceOrHostAddressConstKHR::initialize(const VkDeviceOrHostAddressConstKHR* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = in_struct->deviceAddress;
    hostAddress = in_struct->hostAddress;
}

void safe_VkDeviceOrHostAddressConstKHR::initialize(const safe_VkDeviceOrHostAddressConstKHR* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = copy_src->deviceAddress;
    hostAddress = copy_src->hostAddress;
}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR(const VkDeviceOrHostAddressKHR* in_struct, PNextCopyState*) {
    initialize(in_struct);
}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR() : hostAddress(nullptr) {}

safe_VkDeviceOrHostAddressKHR::safe_VkDeviceOrHostAddressKHR(const safe_VkDeviceOrHostAddressKHR& copy_src) {
    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;
}

safe_VkDeviceOrHostAddressKHR& safe_VkDeviceOrHostAddressKHR::operator=(const safe_VkDeviceOrHostAddressKHR& copy_src) {
    if (&copy_src == this) return *this;

    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;

    return *this;
}

safe_VkDeviceOrHostAddressKHR::~safe_VkDeviceOrHostAddressKHR() {}

void safe_VkDeviceOrHostAddressKHR::initialize(const VkDeviceOrHostAddressKHR* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = in_struct->deviceAddress;
    hostAddress = in_struct->hostAddress;
}

void safe_VkDeviceOrHostAddressKHR::initialize(const safe_VkDeviceOrHostAddressKHR* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = copy_src->deviceAddress;
    hostAddress = copy_src->hostAddress;
}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR(
    const VkAccelerationStructureBuildSizesInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructureSize(in_struct->accelerationStructureSize),
      updateScratchSize(in_struct->updateScratchSize),
      buildScratchSize(in_struct->buildScratchSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR),
      pNext(nullptr),
      accelerationStructureSize(),
      updateScratchSize(),
      buildScratchSize() {}

safe_VkAccelerationStructureBuildSizesInfoKHR::safe_VkAccelerationStructureBuildSizesInfoKHR(
    const safe_VkAccelerationStructureBuildSizesInfoKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructureSize = copy_src.accelerationStructureSize;
    updateScratchSize = copy_src.updateScratchSize;
    buildScratchSize = copy_src.buildScratchSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureBuildSizesInfoKHR& safe_VkAccelerationStructureBuildSizesInfoKHR::operator=(
    const safe_VkAccelerationStructureBuildSizesInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureSize = copy_src.accelerationStructureSize;
    updateScratchSize = copy_src.updateScratchSize;
    buildScratchSize = copy_src.buildScratchSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureBuildSizesInfoKHR::~safe_VkAccelerationStructureBuildSizesInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureBuildSizesInfoKHR::initialize(const VkAccelerationStructureBuildSizesInfoKHR* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureSize = in_struct->accelerationStructureSize;
    updateScratchSize = in_struct->updateScratchSize;
    buildScratchSize = in_struct->buildScratchSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureBuildSizesInfoKHR::initialize(const safe_VkAccelerationStructureBuildSizesInfoKHR* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureSize = copy_src->accelerationStructureSize;
    updateScratchSize = copy_src->updateScratchSize;
    buildScratchSize = copy_src->buildScratchSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR(
    const VkAccelerationStructureGeometryTrianglesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      vertexFormat(in_struct->vertexFormat),
      vertexData(&in_struct->vertexData),
      vertexStride(in_struct->vertexStride),
      maxVertex(in_struct->maxVertex),
      indexType(in_struct->indexType),
      indexData(&in_struct->indexData),
      transformData(&in_struct->transformData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR),
      pNext(nullptr),
      vertexFormat(),
      vertexStride(),
      maxVertex(),
      indexType() {}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::safe_VkAccelerationStructureGeometryTrianglesDataKHR(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR& copy_src) {
    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    maxVertex = copy_src.maxVertex;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    transformData.initialize(&copy_src.transformData);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR& safe_VkAccelerationStructureGeometryTrianglesDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    maxVertex = copy_src.maxVertex;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    transformData.initialize(&copy_src.transformData);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryTrianglesDataKHR::~safe_VkAccelerationStructureGeometryTrianglesDataKHR() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryTrianglesDataKHR::initialize(
    const VkAccelerationStructureGeometryTrianglesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexFormat = in_struct->vertexFormat;
    vertexData.initialize(&in_struct->vertexData);
    vertexStride = in_struct->vertexStride;
    maxVertex = in_struct->maxVertex;
    indexType = in_struct->indexType;
    indexData.initialize(&in_struct->indexData);
    transformData.initialize(&in_struct->transformData);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryTrianglesDataKHR::initialize(
    const safe_VkAccelerationStructureGeometryTrianglesDataKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexFormat = copy_src->vertexFormat;
    vertexData.initialize(&copy_src->vertexData);
    vertexStride = copy_src->vertexStride;
    maxVertex = copy_src->maxVertex;
    indexType = copy_src->indexType;
    indexData.initialize(&copy_src->indexData);
    transformData.initialize(&copy_src->transformData);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR(
    const VkAccelerationStructureGeometryAabbsDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), data(&in_struct->data), stride(in_struct->stride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR), pNext(nullptr), stride() {}

safe_VkAccelerationStructureGeometryAabbsDataKHR::safe_VkAccelerationStructureGeometryAabbsDataKHR(
    const safe_VkAccelerationStructureGeometryAabbsDataKHR& copy_src) {
    sType = copy_src.sType;
    data.initialize(&copy_src.data);
    stride = copy_src.stride;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryAabbsDataKHR& safe_VkAccelerationStructureGeometryAabbsDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryAabbsDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    data.initialize(&copy_src.data);
    stride = copy_src.stride;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryAabbsDataKHR::~safe_VkAccelerationStructureGeometryAabbsDataKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureGeometryAabbsDataKHR::initialize(const VkAccelerationStructureGeometryAabbsDataKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    data.initialize(&in_struct->data);
    stride = in_struct->stride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryAabbsDataKHR::initialize(const safe_VkAccelerationStructureGeometryAabbsDataKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    data.initialize(&copy_src->data);
    stride = copy_src->stride;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR(
    const VkAccelerationStructureGeometryInstancesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), arrayOfPointers(in_struct->arrayOfPointers), data(&in_struct->data) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR), pNext(nullptr), arrayOfPointers() {}

safe_VkAccelerationStructureGeometryInstancesDataKHR::safe_VkAccelerationStructureGeometryInstancesDataKHR(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR& copy_src) {
    sType = copy_src.sType;
    arrayOfPointers = copy_src.arrayOfPointers;
    data.initialize(&copy_src.data);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryInstancesDataKHR& safe_VkAccelerationStructureGeometryInstancesDataKHR::operator=(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    arrayOfPointers = copy_src.arrayOfPointers;
    data.initialize(&copy_src.data);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryInstancesDataKHR::~safe_VkAccelerationStructureGeometryInstancesDataKHR() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryInstancesDataKHR::initialize(
    const VkAccelerationStructureGeometryInstancesDataKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    arrayOfPointers = in_struct->arrayOfPointers;
    data.initialize(&in_struct->data);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryInstancesDataKHR::initialize(
    const safe_VkAccelerationStructureGeometryInstancesDataKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    arrayOfPointers = copy_src->arrayOfPointers;
    data.initialize(&copy_src->data);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR(
    const VkAccelerationStructureCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
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

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR),
      pNext(nullptr),
      createFlags(),
      buffer(),
      offset(),
      size(),
      type(),
      deviceAddress() {}

safe_VkAccelerationStructureCreateInfoKHR::safe_VkAccelerationStructureCreateInfoKHR(
    const safe_VkAccelerationStructureCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    createFlags = copy_src.createFlags;
    buffer = copy_src.buffer;
    offset = copy_src.offset;
    size = copy_src.size;
    type = copy_src.type;
    deviceAddress = copy_src.deviceAddress;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureCreateInfoKHR& safe_VkAccelerationStructureCreateInfoKHR::operator=(
    const safe_VkAccelerationStructureCreateInfoKHR& copy_src) {
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

safe_VkAccelerationStructureCreateInfoKHR::~safe_VkAccelerationStructureCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureCreateInfoKHR::initialize(const VkAccelerationStructureCreateInfoKHR* in_struct,
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

void safe_VkAccelerationStructureCreateInfoKHR::initialize(const safe_VkAccelerationStructureCreateInfoKHR* copy_src,
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

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR(
    const VkWriteDescriptorSetAccelerationStructureKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), accelerationStructureCount(in_struct->accelerationStructureCount), pAccelerationStructures(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR()
    : sType(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR),
      pNext(nullptr),
      accelerationStructureCount(),
      pAccelerationStructures(nullptr) {}

safe_VkWriteDescriptorSetAccelerationStructureKHR::safe_VkWriteDescriptorSetAccelerationStructureKHR(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureKHR& safe_VkWriteDescriptorSetAccelerationStructureKHR::operator=(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }

    return *this;
}

safe_VkWriteDescriptorSetAccelerationStructureKHR::~safe_VkWriteDescriptorSetAccelerationStructureKHR() {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
}

void safe_VkWriteDescriptorSetAccelerationStructureKHR::initialize(const VkWriteDescriptorSetAccelerationStructureKHR* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureCount = in_struct->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

void safe_VkWriteDescriptorSetAccelerationStructureKHR::initialize(
    const safe_VkWriteDescriptorSetAccelerationStructureKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureCount = copy_src->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (accelerationStructureCount && copy_src->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureKHR[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src->pAccelerationStructures[i];
        }
    }
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(
    const VkPhysicalDeviceAccelerationStructureFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructure(in_struct->accelerationStructure),
      accelerationStructureCaptureReplay(in_struct->accelerationStructureCaptureReplay),
      accelerationStructureIndirectBuild(in_struct->accelerationStructureIndirectBuild),
      accelerationStructureHostCommands(in_struct->accelerationStructureHostCommands),
      descriptorBindingAccelerationStructureUpdateAfterBind(in_struct->descriptorBindingAccelerationStructureUpdateAfterBind) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR),
      pNext(nullptr),
      accelerationStructure(),
      accelerationStructureCaptureReplay(),
      accelerationStructureIndirectBuild(),
      accelerationStructureHostCommands(),
      descriptorBindingAccelerationStructureUpdateAfterBind() {}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureCaptureReplay = copy_src.accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src.accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src.accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src.descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    accelerationStructureCaptureReplay = copy_src.accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src.accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src.accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src.descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::~safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::initialize(
    const VkPhysicalDeviceAccelerationStructureFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    accelerationStructureCaptureReplay = in_struct->accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = in_struct->accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = in_struct->accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = in_struct->descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    accelerationStructureCaptureReplay = copy_src->accelerationStructureCaptureReplay;
    accelerationStructureIndirectBuild = copy_src->accelerationStructureIndirectBuild;
    accelerationStructureHostCommands = copy_src->accelerationStructureHostCommands;
    descriptorBindingAccelerationStructureUpdateAfterBind = copy_src->descriptorBindingAccelerationStructureUpdateAfterBind;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(
    const VkPhysicalDeviceAccelerationStructurePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxGeometryCount(in_struct->maxGeometryCount),
      maxInstanceCount(in_struct->maxInstanceCount),
      maxPrimitiveCount(in_struct->maxPrimitiveCount),
      maxPerStageDescriptorAccelerationStructures(in_struct->maxPerStageDescriptorAccelerationStructures),
      maxPerStageDescriptorUpdateAfterBindAccelerationStructures(
          in_struct->maxPerStageDescriptorUpdateAfterBindAccelerationStructures),
      maxDescriptorSetAccelerationStructures(in_struct->maxDescriptorSetAccelerationStructures),
      maxDescriptorSetUpdateAfterBindAccelerationStructures(in_struct->maxDescriptorSetUpdateAfterBindAccelerationStructures),
      minAccelerationStructureScratchOffsetAlignment(in_struct->minAccelerationStructureScratchOffsetAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR),
      pNext(nullptr),
      maxGeometryCount(),
      maxInstanceCount(),
      maxPrimitiveCount(),
      maxPerStageDescriptorAccelerationStructures(),
      maxPerStageDescriptorUpdateAfterBindAccelerationStructures(),
      maxDescriptorSetAccelerationStructures(),
      maxDescriptorSetUpdateAfterBindAccelerationStructures(),
      minAccelerationStructureScratchOffsetAlignment() {}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxPrimitiveCount = copy_src.maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src.maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src.maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src.minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxPrimitiveCount = copy_src.maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src.maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src.maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src.minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::~safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::initialize(
    const VkPhysicalDeviceAccelerationStructurePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxGeometryCount = in_struct->maxGeometryCount;
    maxInstanceCount = in_struct->maxInstanceCount;
    maxPrimitiveCount = in_struct->maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = in_struct->maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        in_struct->maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = in_struct->maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = in_struct->maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = in_struct->minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxGeometryCount = copy_src->maxGeometryCount;
    maxInstanceCount = copy_src->maxInstanceCount;
    maxPrimitiveCount = copy_src->maxPrimitiveCount;
    maxPerStageDescriptorAccelerationStructures = copy_src->maxPerStageDescriptorAccelerationStructures;
    maxPerStageDescriptorUpdateAfterBindAccelerationStructures =
        copy_src->maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    maxDescriptorSetAccelerationStructures = copy_src->maxDescriptorSetAccelerationStructures;
    maxDescriptorSetUpdateAfterBindAccelerationStructures = copy_src->maxDescriptorSetUpdateAfterBindAccelerationStructures;
    minAccelerationStructureScratchOffsetAlignment = copy_src->minAccelerationStructureScratchOffsetAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR(
    const VkAccelerationStructureDeviceAddressInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), accelerationStructure(in_struct->accelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR), pNext(nullptr), accelerationStructure() {}

safe_VkAccelerationStructureDeviceAddressInfoKHR::safe_VkAccelerationStructureDeviceAddressInfoKHR(
    const safe_VkAccelerationStructureDeviceAddressInfoKHR& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureDeviceAddressInfoKHR& safe_VkAccelerationStructureDeviceAddressInfoKHR::operator=(
    const safe_VkAccelerationStructureDeviceAddressInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureDeviceAddressInfoKHR::~safe_VkAccelerationStructureDeviceAddressInfoKHR() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureDeviceAddressInfoKHR::initialize(const VkAccelerationStructureDeviceAddressInfoKHR* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureDeviceAddressInfoKHR::initialize(const safe_VkAccelerationStructureDeviceAddressInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR(
    const VkAccelerationStructureVersionInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pVersionData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)in_struct->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_VERSION_INFO_KHR), pNext(nullptr), pVersionData(nullptr) {}

safe_VkAccelerationStructureVersionInfoKHR::safe_VkAccelerationStructureVersionInfoKHR(
    const safe_VkAccelerationStructureVersionInfoKHR& copy_src) {
    sType = copy_src.sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src.pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkAccelerationStructureVersionInfoKHR& safe_VkAccelerationStructureVersionInfoKHR::operator=(
    const safe_VkAccelerationStructureVersionInfoKHR& copy_src) {
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

safe_VkAccelerationStructureVersionInfoKHR::~safe_VkAccelerationStructureVersionInfoKHR() {
    if (pVersionData) delete[] pVersionData;
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureVersionInfoKHR::initialize(const VkAccelerationStructureVersionInfoKHR* in_struct,
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

void safe_VkAccelerationStructureVersionInfoKHR::initialize(const safe_VkAccelerationStructureVersionInfoKHR* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pVersionData = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pVersionData) {
        pVersionData = new uint8_t[2 * VK_UUID_SIZE];
        memcpy((void*)pVersionData, (void*)copy_src->pVersionData, sizeof(uint8_t) * 2 * VK_UUID_SIZE);
    }
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR(
    const VkCopyAccelerationStructureToMemoryInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(&in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR), pNext(nullptr), src(), mode() {}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::safe_VkCopyAccelerationStructureToMemoryInfoKHR(
    const safe_VkCopyAccelerationStructureToMemoryInfoKHR& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR& safe_VkCopyAccelerationStructureToMemoryInfoKHR::operator=(
    const safe_VkCopyAccelerationStructureToMemoryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst.initialize(&copy_src.dst);
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyAccelerationStructureToMemoryInfoKHR::~safe_VkCopyAccelerationStructureToMemoryInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyAccelerationStructureToMemoryInfoKHR::initialize(const VkCopyAccelerationStructureToMemoryInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst.initialize(&in_struct->dst);
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyAccelerationStructureToMemoryInfoKHR::initialize(const safe_VkCopyAccelerationStructureToMemoryInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst.initialize(&copy_src->dst);
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR(
    const VkCopyMemoryToAccelerationStructureInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(&in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR), pNext(nullptr), dst(), mode() {}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::safe_VkCopyMemoryToAccelerationStructureInfoKHR(
    const safe_VkCopyMemoryToAccelerationStructureInfoKHR& copy_src) {
    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR& safe_VkCopyMemoryToAccelerationStructureInfoKHR::operator=(
    const safe_VkCopyMemoryToAccelerationStructureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src.initialize(&copy_src.src);
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyMemoryToAccelerationStructureInfoKHR::~safe_VkCopyMemoryToAccelerationStructureInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyMemoryToAccelerationStructureInfoKHR::initialize(const VkCopyMemoryToAccelerationStructureInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src.initialize(&in_struct->src);
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyMemoryToAccelerationStructureInfoKHR::initialize(const safe_VkCopyMemoryToAccelerationStructureInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src.initialize(&copy_src->src);
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR(
    const VkCopyAccelerationStructureInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), src(in_struct->src), dst(in_struct->dst), mode(in_struct->mode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR()
    : sType(VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR), pNext(nullptr), src(), dst(), mode() {}

safe_VkCopyAccelerationStructureInfoKHR::safe_VkCopyAccelerationStructureInfoKHR(
    const safe_VkCopyAccelerationStructureInfoKHR& copy_src) {
    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyAccelerationStructureInfoKHR& safe_VkCopyAccelerationStructureInfoKHR::operator=(
    const safe_VkCopyAccelerationStructureInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    src = copy_src.src;
    dst = copy_src.dst;
    mode = copy_src.mode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyAccelerationStructureInfoKHR::~safe_VkCopyAccelerationStructureInfoKHR() { FreePnextChain(pNext); }

void safe_VkCopyAccelerationStructureInfoKHR::initialize(const VkCopyAccelerationStructureInfoKHR* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    src = in_struct->src;
    dst = in_struct->dst;
    mode = in_struct->mode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyAccelerationStructureInfoKHR::initialize(const safe_VkCopyAccelerationStructureInfoKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    src = copy_src->src;
    dst = copy_src->dst;
    mode = copy_src->mode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR(
    const VkRayTracingShaderGroupCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      generalShader(in_struct->generalShader),
      closestHitShader(in_struct->closestHitShader),
      anyHitShader(in_struct->anyHitShader),
      intersectionShader(in_struct->intersectionShader),
      pShaderGroupCaptureReplayHandle(in_struct->pShaderGroupCaptureReplayHandle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR),
      pNext(nullptr),
      type(),
      generalShader(),
      closestHitShader(),
      anyHitShader(),
      intersectionShader(),
      pShaderGroupCaptureReplayHandle(nullptr) {}

safe_VkRayTracingShaderGroupCreateInfoKHR::safe_VkRayTracingShaderGroupCreateInfoKHR(
    const safe_VkRayTracingShaderGroupCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src.pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingShaderGroupCreateInfoKHR& safe_VkRayTracingShaderGroupCreateInfoKHR::operator=(
    const safe_VkRayTracingShaderGroupCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src.pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingShaderGroupCreateInfoKHR::~safe_VkRayTracingShaderGroupCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkRayTracingShaderGroupCreateInfoKHR::initialize(const VkRayTracingShaderGroupCreateInfoKHR* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    generalShader = in_struct->generalShader;
    closestHitShader = in_struct->closestHitShader;
    anyHitShader = in_struct->anyHitShader;
    intersectionShader = in_struct->intersectionShader;
    pShaderGroupCaptureReplayHandle = in_struct->pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingShaderGroupCreateInfoKHR::initialize(const safe_VkRayTracingShaderGroupCreateInfoKHR* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    generalShader = copy_src->generalShader;
    closestHitShader = copy_src->closestHitShader;
    anyHitShader = copy_src->anyHitShader;
    intersectionShader = copy_src->intersectionShader;
    pShaderGroupCaptureReplayHandle = copy_src->pShaderGroupCaptureReplayHandle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR(
    const VkRayTracingPipelineInterfaceCreateInfoKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxPipelineRayPayloadSize(in_struct->maxPipelineRayPayloadSize),
      maxPipelineRayHitAttributeSize(in_struct->maxPipelineRayHitAttributeSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR),
      pNext(nullptr),
      maxPipelineRayPayloadSize(),
      maxPipelineRayHitAttributeSize() {}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::safe_VkRayTracingPipelineInterfaceCreateInfoKHR(
    const safe_VkRayTracingPipelineInterfaceCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    maxPipelineRayPayloadSize = copy_src.maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src.maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR& safe_VkRayTracingPipelineInterfaceCreateInfoKHR::operator=(
    const safe_VkRayTracingPipelineInterfaceCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxPipelineRayPayloadSize = copy_src.maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src.maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingPipelineInterfaceCreateInfoKHR::~safe_VkRayTracingPipelineInterfaceCreateInfoKHR() { FreePnextChain(pNext); }

void safe_VkRayTracingPipelineInterfaceCreateInfoKHR::initialize(const VkRayTracingPipelineInterfaceCreateInfoKHR* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxPipelineRayPayloadSize = in_struct->maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = in_struct->maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingPipelineInterfaceCreateInfoKHR::initialize(const safe_VkRayTracingPipelineInterfaceCreateInfoKHR* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxPipelineRayPayloadSize = copy_src->maxPipelineRayPayloadSize;
    maxPipelineRayHitAttributeSize = copy_src->maxPipelineRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR(const VkRayTracingPipelineCreateInfoKHR* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      groupCount(in_struct->groupCount),
      pGroups(nullptr),
      maxPipelineRayRecursionDepth(in_struct->maxPipelineRayRecursionDepth),
      pLibraryInfo(nullptr),
      pLibraryInterface(nullptr),
      pDynamicState(nullptr),
      layout(in_struct->layout),
      basePipelineHandle(in_struct->basePipelineHandle),
      basePipelineIndex(in_struct->basePipelineIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
    if (in_struct->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(in_struct->pLibraryInterface);
    if (in_struct->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR),
      pNext(nullptr),
      flags(),
      stageCount(),
      pStages(nullptr),
      groupCount(),
      pGroups(nullptr),
      maxPipelineRayRecursionDepth(),
      pLibraryInfo(nullptr),
      pLibraryInterface(nullptr),
      pDynamicState(nullptr),
      layout(),
      basePipelineHandle(),
      basePipelineIndex() {}

safe_VkRayTracingPipelineCreateInfoKHR::safe_VkRayTracingPipelineCreateInfoKHR(
    const safe_VkRayTracingPipelineCreateInfoKHR& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src.maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);
    if (copy_src.pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src.pLibraryInterface);
    if (copy_src.pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);
}

safe_VkRayTracingPipelineCreateInfoKHR& safe_VkRayTracingPipelineCreateInfoKHR::operator=(
    const safe_VkRayTracingPipelineCreateInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src.maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);
    if (copy_src.pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src.pLibraryInterface);
    if (copy_src.pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);

    return *this;
}

safe_VkRayTracingPipelineCreateInfoKHR::~safe_VkRayTracingPipelineCreateInfoKHR() {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
}

void safe_VkRayTracingPipelineCreateInfoKHR::initialize(const VkRayTracingPipelineCreateInfoKHR* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    if (pLibraryInfo) delete pLibraryInfo;
    if (pLibraryInterface) delete pLibraryInterface;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    groupCount = in_struct->groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = in_struct->maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
    if (in_struct->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(in_struct->pLibraryInterface);
    if (in_struct->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
}

void safe_VkRayTracingPipelineCreateInfoKHR::initialize(const safe_VkRayTracingPipelineCreateInfoKHR* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    groupCount = copy_src->groupCount;
    pGroups = nullptr;
    maxPipelineRayRecursionDepth = copy_src->maxPipelineRayRecursionDepth;
    pLibraryInfo = nullptr;
    pLibraryInterface = nullptr;
    pDynamicState = nullptr;
    layout = copy_src->layout;
    basePipelineHandle = copy_src->basePipelineHandle;
    basePipelineIndex = copy_src->basePipelineIndex;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (groupCount && copy_src->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src->pGroups[i]);
        }
    }
    if (copy_src->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src->pLibraryInfo);
    if (copy_src->pLibraryInterface)
        pLibraryInterface = new safe_VkRayTracingPipelineInterfaceCreateInfoKHR(*copy_src->pLibraryInterface);
    if (copy_src->pDynamicState) pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src->pDynamicState);
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(
    const VkPhysicalDeviceRayTracingPipelineFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      rayTracingPipeline(in_struct->rayTracingPipeline),
      rayTracingPipelineShaderGroupHandleCaptureReplay(in_struct->rayTracingPipelineShaderGroupHandleCaptureReplay),
      rayTracingPipelineShaderGroupHandleCaptureReplayMixed(in_struct->rayTracingPipelineShaderGroupHandleCaptureReplayMixed),
      rayTracingPipelineTraceRaysIndirect(in_struct->rayTracingPipelineTraceRaysIndirect),
      rayTraversalPrimitiveCulling(in_struct->rayTraversalPrimitiveCulling) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR),
      pNext(nullptr),
      rayTracingPipeline(),
      rayTracingPipelineShaderGroupHandleCaptureReplay(),
      rayTracingPipelineShaderGroupHandleCaptureReplayMixed(),
      rayTracingPipelineTraceRaysIndirect(),
      rayTraversalPrimitiveCulling() {}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayTracingPipeline = copy_src.rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src.rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src.rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingPipeline = copy_src.rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src.rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src.rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src.rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::~safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::initialize(const VkPhysicalDeviceRayTracingPipelineFeaturesKHR* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingPipeline = in_struct->rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = in_struct->rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = in_struct->rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = in_struct->rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = in_struct->rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingPipeline = copy_src->rayTracingPipeline;
    rayTracingPipelineShaderGroupHandleCaptureReplay = copy_src->rayTracingPipelineShaderGroupHandleCaptureReplay;
    rayTracingPipelineShaderGroupHandleCaptureReplayMixed = copy_src->rayTracingPipelineShaderGroupHandleCaptureReplayMixed;
    rayTracingPipelineTraceRaysIndirect = copy_src->rayTracingPipelineTraceRaysIndirect;
    rayTraversalPrimitiveCulling = copy_src->rayTraversalPrimitiveCulling;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderGroupHandleSize(in_struct->shaderGroupHandleSize),
      maxRayRecursionDepth(in_struct->maxRayRecursionDepth),
      maxShaderGroupStride(in_struct->maxShaderGroupStride),
      shaderGroupBaseAlignment(in_struct->shaderGroupBaseAlignment),
      shaderGroupHandleCaptureReplaySize(in_struct->shaderGroupHandleCaptureReplaySize),
      maxRayDispatchInvocationCount(in_struct->maxRayDispatchInvocationCount),
      shaderGroupHandleAlignment(in_struct->shaderGroupHandleAlignment),
      maxRayHitAttributeSize(in_struct->maxRayHitAttributeSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR),
      pNext(nullptr),
      shaderGroupHandleSize(),
      maxRayRecursionDepth(),
      maxShaderGroupStride(),
      shaderGroupBaseAlignment(),
      shaderGroupHandleCaptureReplaySize(),
      maxRayDispatchInvocationCount(),
      shaderGroupHandleAlignment(),
      maxRayHitAttributeSize() {}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& copy_src) {
    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src.maxRayRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src.shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src.maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src.shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src.maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::operator=(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src.maxRayRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src.shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src.maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src.shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src.maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::~safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::initialize(
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderGroupHandleSize = in_struct->shaderGroupHandleSize;
    maxRayRecursionDepth = in_struct->maxRayRecursionDepth;
    maxShaderGroupStride = in_struct->maxShaderGroupStride;
    shaderGroupBaseAlignment = in_struct->shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = in_struct->shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = in_struct->maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = in_struct->shaderGroupHandleAlignment;
    maxRayHitAttributeSize = in_struct->maxRayHitAttributeSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR::initialize(
    const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderGroupHandleSize = copy_src->shaderGroupHandleSize;
    maxRayRecursionDepth = copy_src->maxRayRecursionDepth;
    maxShaderGroupStride = copy_src->maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src->shaderGroupBaseAlignment;
    shaderGroupHandleCaptureReplaySize = copy_src->shaderGroupHandleCaptureReplaySize;
    maxRayDispatchInvocationCount = copy_src->maxRayDispatchInvocationCount;
    shaderGroupHandleAlignment = copy_src->shaderGroupHandleAlignment;
    maxRayHitAttributeSize = copy_src->maxRayHitAttributeSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR(
    const VkPhysicalDeviceRayQueryFeaturesKHR* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), rayQuery(in_struct->rayQuery) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR), pNext(nullptr), rayQuery() {}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::safe_VkPhysicalDeviceRayQueryFeaturesKHR(
    const safe_VkPhysicalDeviceRayQueryFeaturesKHR& copy_src) {
    sType = copy_src.sType;
    rayQuery = copy_src.rayQuery;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR& safe_VkPhysicalDeviceRayQueryFeaturesKHR::operator=(
    const safe_VkPhysicalDeviceRayQueryFeaturesKHR& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayQuery = copy_src.rayQuery;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayQueryFeaturesKHR::~safe_VkPhysicalDeviceRayQueryFeaturesKHR() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRayQueryFeaturesKHR::initialize(const VkPhysicalDeviceRayQueryFeaturesKHR* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayQuery = in_struct->rayQuery;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayQueryFeaturesKHR::initialize(const safe_VkPhysicalDeviceRayQueryFeaturesKHR* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayQuery = copy_src->rayQuery;
    pNext = SafePnextCopy(copy_src->pNext);
}

}  // namespace vku

// NOLINTEND
