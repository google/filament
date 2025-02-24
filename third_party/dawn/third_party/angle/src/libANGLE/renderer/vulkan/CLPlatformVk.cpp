//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformVk.cpp: Implements the class methods for CLPlatformVk.

#include "libANGLE/renderer/vulkan/CLPlatformVk.h"
#include "common/vulkan/vulkan_icd.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/CLDeviceVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include "libANGLE/CLPlatform.h"
#include "libANGLE/cl_utils.h"

#include "anglebase/no_destructor.h"
#include "common/angle_version_info.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"
#include "vulkan/vulkan_core.h"

namespace rx
{

namespace
{
#if defined(ANGLE_ENABLE_VULKAN_VALIDATION_LAYERS_BY_DEFAULT)
constexpr vk::UseDebugLayers kUseDebugLayers = vk::UseDebugLayers::YesIfAvailable;
#else
constexpr vk::UseDebugLayers kUseDebugLayers = vk::UseDebugLayers::No;
#endif
}  // namespace

angle::Result CLPlatformVk::initBackendRenderer()
{
    ASSERT(mRenderer != nullptr);

    angle::FeatureOverrides featureOverrides;

    // In memory |SizedMRUCache| does not require dual slots, supports zero sized values, and evicts
    // minumum number of old items when storing a new item.
    featureOverrides.disabled.push_back("useDualPipelineBlobCacheSlots");
    featureOverrides.enabled.push_back("useEmptyBlobsToEraseOldPipelineCacheFromBlobCache");
    featureOverrides.enabled.push_back("hasBlobCacheThatEvictsOldItemsFirst");
    featureOverrides.disabled.push_back("verifyPipelineCacheInBlobCache");

    ANGLE_TRY(mRenderer->initialize(this, this, angle::vk::ICD::Default, 0, 0, nullptr, nullptr,
                                    static_cast<VkDriverId>(0), kUseDebugLayers, getWSIExtension(),
                                    getWSILayer(), getWindowSystem(), featureOverrides));

    return angle::Result::Continue;
}

CLPlatformVk::~CLPlatformVk()
{
    ASSERT(mRenderer);
    mRenderer->onDestroy(this);
    delete mRenderer;
}

CLPlatformImpl::Info CLPlatformVk::createInfo() const
{
    NameVersionVector extList = {
        cl_name_version{CL_MAKE_VERSION(1, 0, 0), "cl_khr_icd"},
        cl_name_version{CL_MAKE_VERSION(1, 0, 0), "cl_khr_extended_versioning"}};

    Info info;
    info.name.assign("ANGLE Vulkan");
    info.profile.assign("FULL_PROFILE");
    info.versionStr.assign(GetVersionString());
    info.hostTimerRes = 0u;
    info.version      = GetVersion();

    info.initializeVersionedExtensions(std::move(extList));
    return info;
}

CLDeviceImpl::CreateDatas CLPlatformVk::createDevices() const
{
    CLDeviceImpl::CreateDatas createDatas;

    // Convert Vk device type to CL equivalent
    cl_device_type type = CL_DEVICE_TYPE_DEFAULT;
    switch (mRenderer->getPhysicalDeviceProperties().deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            type |= CL_DEVICE_TYPE_CPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            type |= CL_DEVICE_TYPE_GPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            // The default OpenCL device must not be a CL_DEVICE_TYPE_CUSTOM device.
            // Thus, we override type bitfield to custom only.
            type = CL_DEVICE_TYPE_CUSTOM;
            break;
    }

    createDatas.emplace_back(type, [this](const cl::Device &device) {
        return CLDeviceVk::Ptr(new CLDeviceVk(device, mRenderer));
    });
    return createDatas;
}

angle::Result CLPlatformVk::createContext(cl::Context &context,
                                          const cl::DevicePtrs &devices,
                                          bool userSync,
                                          CLContextImpl::Ptr *contextOut)
{
    *contextOut = CLContextImpl::Ptr(new (std::nothrow) CLContextVk(context, devices));
    if (*contextOut == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    return angle::Result::Continue;
}

angle::Result CLPlatformVk::createContextFromType(cl::Context &context,
                                                  cl::DeviceType deviceType,
                                                  bool userSync,
                                                  CLContextImpl::Ptr *contextOut)
{
    const VkPhysicalDeviceType &vkPhysicalDeviceType =
        getRenderer()->getPhysicalDeviceProperties().deviceType;

    if (deviceType.intersects(CL_DEVICE_TYPE_CPU) &&
        vkPhysicalDeviceType != VK_PHYSICAL_DEVICE_TYPE_CPU)
    {
        ANGLE_CL_RETURN_ERROR(CL_DEVICE_NOT_FOUND);
    }
    else if (deviceType.intersects(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT))
    {
        switch (vkPhysicalDeviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                break;
            default:
                ANGLE_CL_RETURN_ERROR(CL_DEVICE_NOT_FOUND);
        }
    }
    else
    {
        ANGLE_CL_RETURN_ERROR(CL_DEVICE_NOT_FOUND);
    }

    cl::DevicePtrs devices;
    for (const auto &platformDevice : mPlatform.getDevices())
    {
        const auto &platformDeviceInfo = platformDevice->getInfo();
        if (platformDeviceInfo.type.intersects(deviceType))
        {
            devices.push_back(platformDevice);
        }
    }

    *contextOut = CLContextImpl::Ptr(new (std::nothrow) CLContextVk(context, devices));
    if (*contextOut == nullptr)
    {
        ANGLE_CL_RETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    return angle::Result::Continue;
}

angle::Result CLPlatformVk::unloadCompiler()
{
    return angle::Result::Continue;
}

void CLPlatformVk::Initialize(CreateFuncs &createFuncs)
{
    createFuncs.emplace_back([](const cl::Platform &platform) -> CLPlatformImpl::Ptr {
        CLPlatformVk::Ptr platformVk = CLPlatformVk::Ptr(new (std::nothrow) CLPlatformVk(platform));
        if (platformVk == nullptr || IsError(platformVk->initBackendRenderer()))
        {
            return Ptr(nullptr);
        }
        return Ptr(std::move(platformVk));
    });
}

const std::string &CLPlatformVk::GetVersionString()
{
    static const angle::base::NoDestructor<const std::string> sVersion(
        "OpenCL " + std::to_string(CL_VERSION_MAJOR(GetVersion())) + "." +
        std::to_string(CL_VERSION_MINOR(GetVersion())) + " ANGLE " +
        angle::GetANGLEVersionString());
    return *sVersion;
}

CLPlatformVk::CLPlatformVk(const cl::Platform &platform)
    : CLPlatformImpl(platform), vk::ErrorContext(new vk::Renderer()), mBlobCache(1024 * 1024)
{}

void CLPlatformVk::handleError(VkResult result,
                               const char *file,
                               const char *function,
                               unsigned int line)
{
    ASSERT(result != VK_SUCCESS);

    std::stringstream errorStream;
    errorStream << "Internal Vulkan error (" << result << "): " << VulkanResultString(result)
                << ", in " << file << ", " << function << ":" << line << ".";
    std::string errorString = errorStream.str();

    if (result == VK_ERROR_DEVICE_LOST)
    {
        WARN() << errorString;
        mRenderer->notifyDeviceLost();
    }
}

angle::NativeWindowSystem CLPlatformVk::getWindowSystem()
{
#if defined(ANGLE_ENABLE_VULKAN)
#    if defined(ANGLE_PLATFORM_LINUX)
#        if defined(ANGLE_USE_GBM)
    return angle::NativeWindowSystem::Gbm;
#        elif defined(ANGLE_USE_X11)
    return angle::NativeWindowSystem::X11;
#        elif defined(ANGLE_USE_WAYLAND)
    return angle::NativeWindowSystem::Wayland;
#        else
    handleError(VK_ERROR_INCOMPATIBLE_DRIVER, __FILE__, __func__, __LINE__);
    return angle::NativeWindowSystem::Other;
#        endif
#    elif defined(ANGLE_PLATFORM_ANDROID)
    return angle::NativeWindowSystem::Other;
#    else
    handleError(VK_ERROR_INCOMPATIBLE_DRIVER, __FILE__, __func__, __LINE__);
    return angle::NativeWindowSystem::Other;
#    endif
#elif
    UNREACHABLE();
#endif
}

const char *CLPlatformVk::getWSIExtension()
{
#if defined(ANGLE_ENABLE_VULKAN)
#    if defined(ANGLE_PLATFORM_LINUX)
#        if defined(ANGLE_USE_GBM)
    return nullptr;
#        elif defined(ANGLE_USE_X11)
    return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#        elif defined(ANGLE_USE_WAYLAND)
    return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#        else
    handleError(VK_ERROR_INCOMPATIBLE_DRIVER, __FILE__, __func__, __LINE__);
    return nullptr;
#        endif
#    elif defined(ANGLE_PLATFORM_ANDROID)
    return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#    else
    handleError(VK_ERROR_INCOMPATIBLE_DRIVER, __FILE__, __func__, __LINE__);
    return nullptr;
#    endif
#elif
    UNREACHABLE();
#endif
}

// vk::GlobalOps
void CLPlatformVk::putBlob(const angle::BlobCacheKey &key, const angle::MemoryBuffer &value)
{
    std::scoped_lock<angle::SimpleMutex> lock(mBlobCacheMutex);
    size_t valueSize = value.size();
    mBlobCache.put(key, std::move(const_cast<angle::MemoryBuffer &>(value)), valueSize);
}

bool CLPlatformVk::getBlob(const angle::BlobCacheKey &key, angle::BlobCacheValue *valueOut)
{
    std::scoped_lock<angle::SimpleMutex> lock(mBlobCacheMutex);
    const angle::MemoryBuffer *entry;
    bool result = mBlobCache.get(key, &entry);
    if (result)
    {
        *valueOut = angle::BlobCacheValue(entry->data(), entry->size());
    }
    return result;
}

std::shared_ptr<angle::WaitableEvent> CLPlatformVk::postMultiThreadWorkerTask(
    const std::shared_ptr<angle::Closure> &task)
{
    return mPlatform.getMultiThreadPool()->postWorkerTask(task);
}

void CLPlatformVk::notifyDeviceLost()
{
    return;
}

}  // namespace rx
