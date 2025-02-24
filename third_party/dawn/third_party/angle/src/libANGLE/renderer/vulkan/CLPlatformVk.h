//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformVk.h: Defines the class interface for CLPlatformVk, implementing CLPlatformImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_

#include "common/MemoryBuffer.h"
#include "common/SimpleMutex.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/CLPlatformImpl.h"

#include "libANGLE/renderer/vulkan/vk_utils.h"

#include "libANGLE/Display.h"
#include "libANGLE/SizedMRUCache.h"

namespace rx
{

class CLPlatformVk : public CLPlatformImpl, public vk::ErrorContext, public vk::GlobalOps
{
  public:
    using Ptr = std::unique_ptr<CLPlatformVk>;

    ~CLPlatformVk() override;

    Info createInfo() const override;
    CLDeviceImpl::CreateDatas createDevices() const override;

    angle::Result createContext(cl::Context &context,
                                const cl::DevicePtrs &devices,
                                bool userSync,
                                CLContextImpl::Ptr *contextOut) override;

    angle::Result createContextFromType(cl::Context &context,
                                        cl::DeviceType deviceType,
                                        bool userSync,
                                        CLContextImpl::Ptr *contextOut) override;

    angle::Result unloadCompiler() override;

    static void Initialize(CreateFuncs &createFuncs);

    static constexpr cl_version GetVersion();
    static const std::string &GetVersionString();

    angle::Result initBackendRenderer();

    // vk::ErrorContext
    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    // vk::GlobalOps
    void putBlob(const angle::BlobCacheKey &key, const angle::MemoryBuffer &value) override;
    bool getBlob(const angle::BlobCacheKey &key, angle::BlobCacheValue *valueOut) override;
    std::shared_ptr<angle::WaitableEvent> postMultiThreadWorkerTask(
        const std::shared_ptr<angle::Closure> &task) override;
    void notifyDeviceLost() override;

  private:
    explicit CLPlatformVk(const cl::Platform &platform);

    angle::NativeWindowSystem getWindowSystem();
    const char *getWSIExtension();
    const char *getWSILayer() { return nullptr; }

    mutable angle::SimpleMutex mBlobCacheMutex;
    angle::SizedMRUCache<angle::BlobCacheKey, angle::MemoryBuffer> mBlobCache;
};

constexpr cl_version CLPlatformVk::GetVersion()
{
    return CL_MAKE_VERSION(3, 0, 0);
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLPLATFORMVK_H_
