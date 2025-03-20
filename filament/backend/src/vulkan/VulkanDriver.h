/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANDRIVER_H
#define TNT_FILAMENT_BACKEND_VULKANDRIVER_H

#include "VulkanBlitter.h"
#include "VulkanConstants.h"
#include "VulkanContext.h"
#include "VulkanFboCache.h"
#include "VulkanHandles.h"
#include "VulkanPipelineCache.h"
#include "VulkanReadPixels.h"
#include "VulkanSamplerCache.h"
#include "VulkanStagePool.h"
#include "VulkanQueryManager.h"
#include "vulkan/VulkanDescriptorSetCache.h"
#include "vulkan/VulkanPipelineLayoutCache.h"
#include "vulkan/memory/ResourceManager.h"
#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/Definitions.h"

#include "backend/DriverEnums.h"

#include "DriverBase.h"
#include "private/backend/Driver.h"

#include <utils/Allocator.h>
#include <utils/compiler.h>

namespace filament::backend {

class VulkanPlatform;

// The maximum number of attachments for any renderpass (color + resolve + depth)
constexpr uint8_t MAX_RENDERTARGET_ATTACHMENT_TEXTURES =
        MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT * 2 + 1;

class VulkanDriver final : public DriverBase {
public:
    static Driver* create(VulkanPlatform* platform, VulkanContext const& context,
            Platform::DriverConfig const& driverConfig) noexcept;

#if FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)
    // Encapsulates the VK_EXT_debug_utils extension.  In particular, we use
    // vkSetDebugUtilsObjectNameEXT and vkCreateDebugUtilsMessengerEXT
    class DebugUtils {
    public:
        static void setName(VkObjectType type, uint64_t handle, char const* name);

    private:
        static DebugUtils* get();

        DebugUtils(VkInstance instance, VkDevice device, VulkanContext const* context);
        ~DebugUtils();

        VkInstance const mInstance;
        VkDevice const mDevice;
        bool const mEnabled;
        VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;

        static DebugUtils* mSingleton;

        friend class VulkanDriver;
    };
#endif // FVK_ENABLED(FVK_DEBUG_DEBUG_UTILS)

private:
    template<typename D>
    using resource_ptr = fvkmemory::resource_ptr<D>;

    static constexpr uint8_t MAX_SAMPLER_BINDING_COUNT = Program::SAMPLER_BINDING_COUNT;

    void debugCommandBegin(CommandStream* cmds, bool synchronous,
            const char* methodName) noexcept override;

    inline VulkanDriver(VulkanPlatform* platform, VulkanContext const& context,
            Platform::DriverConfig const& driverConfig) noexcept;

    ~VulkanDriver() noexcept override;

    Dispatcher getDispatcher() const noexcept final;

    ShaderModel getShaderModel() const noexcept final;
    ShaderLanguage getShaderLanguage() const noexcept final;

    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params)                                            \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)                       \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                            \
    RetType methodName##S() noexcept override;                                                     \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"

    VulkanDriver(VulkanDriver const&) = delete;
    VulkanDriver& operator=(VulkanDriver const&) = delete;

private:
    void collectGarbage();

    VulkanPlatform* mPlatform = nullptr;
    fvkmemory::ResourceManager mResourceManager;

    resource_ptr<VulkanSwapChain> mCurrentSwapChain;
    resource_ptr<VulkanRenderTarget> mDefaultRenderTarget;
    VulkanRenderPass mCurrentRenderPass = {};
    VmaAllocator mAllocator = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT mDebugCallback = VK_NULL_HANDLE;

    VulkanContext mContext = {};

    VulkanCommands mCommands;
    VulkanPipelineLayoutCache mPipelineLayoutCache;
    VulkanPipelineCache mPipelineCache;
    VulkanStagePool mStagePool;
    VulkanFboCache mFramebufferCache;
    VulkanSamplerCache mSamplerCache;
    VulkanBlitter mBlitter;
    VulkanReadPixels mReadPixels;
    VulkanDescriptorSetCache mDescriptorSetCache;
    VulkanQueryManager mQueryManager;

    // This is necessary for us to write to push constants after binding a pipeline.
    struct {
        resource_ptr<VulkanProgram> program;
        VkPipelineLayout pipelineLayout;
        fvkutils::DescriptorSetMask descriptorSetMask;
    } mBoundPipeline = {};

    // We need to store information about a render pass to enable better barriers at the end of a
    // renderpass.
    struct {
        using AttachmentArray =
                fvkutils::StaticVector<VulkanAttachment, MAX_RENDERTARGET_ATTACHMENT_TEXTURES>;
        AttachmentArray attachments;
    } mRenderPassFboInfo = {};

    bool const mIsSRGBSwapChainSupported;
    backend::StereoscopicType const mStereoscopicType;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANDRIVER_H
