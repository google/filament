// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/DawnNative.h"

#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Texture.h"
#include "dawn/platform/DawnPlatform.h"
#include "tint/tint.h"

// Contains the entry-points into dawn_native

namespace dawn::native {

const char MemoryDump::kNameSize[] = "size";
const char MemoryDump::kNameObjectCount[] = "object_count";
const char MemoryDump::kUnitsBytes[] = "bytes";
const char MemoryDump::kUnitsObjects[] = "objects";

const DawnProcTable& GetProcsAutogen();

const DawnProcTable& GetProcs() {
    return GetProcsAutogen();
}

std::vector<const char*> GetTogglesUsed(WGPUDevice device) {
    return FromAPI(device)->GetTogglesUsed();
}

// Adapter

Adapter::Adapter() = default;

Adapter::Adapter(AdapterBase* impl) : mImpl(impl) {
    if (mImpl != nullptr) {
        mImpl->AddRef();
    }
}

Adapter::~Adapter() {
    if (mImpl != nullptr) {
        mImpl->Release();
    }
    mImpl = nullptr;
}

Adapter::Adapter(const Adapter& other) : Adapter(other.mImpl) {}

Adapter& Adapter::operator=(const Adapter& other) {
    if (this != &other) {
        if (mImpl) {
            mImpl->Release();
        }
        mImpl = other.mImpl;
        if (mImpl) {
            mImpl->AddRef();
        }
    }
    return *this;
}

WGPUAdapter Adapter::Get() const {
    return ToAPI(mImpl);
}

void Adapter::SetUseTieredLimits(bool useTieredLimits) {
    mImpl->SetUseTieredLimits(useTieredLimits);
}

bool Adapter::SupportsExternalImages() const {
    return mImpl->GetPhysicalDevice()->SupportsExternalImages();
}

Adapter::operator bool() const {
    return mImpl != nullptr;
}

WGPUDevice Adapter::CreateDevice(const wgpu::DeviceDescriptor* deviceDescriptor) {
    return CreateDevice(reinterpret_cast<const WGPUDeviceDescriptor*>(deviceDescriptor));
}

WGPUDevice Adapter::CreateDevice(const WGPUDeviceDescriptor* deviceDescriptor) {
    return ToAPI(mImpl->APICreateDevice(FromAPI(deviceDescriptor)));
}

void Adapter::ResetInternalDeviceForTesting() {
    [[maybe_unused]] bool hadError = mImpl->GetInstance()->ConsumedError(
        mImpl->GetPhysicalDevice()->ResetInternalDeviceForTesting());
}

// DawnInstanceDescriptor

DawnInstanceDescriptor::DawnInstanceDescriptor() {
    sType = wgpu::SType::DawnInstanceDescriptor;
}

bool DawnInstanceDescriptor::operator==(const DawnInstanceDescriptor& rhs) const {
    return (nextInChain == rhs.nextInChain) &&
           std::tie(additionalRuntimeSearchPathsCount, additionalRuntimeSearchPaths, platform,
                    backendValidationLevel, beginCaptureOnStartup) ==
               std::tie(rhs.additionalRuntimeSearchPathsCount, rhs.additionalRuntimeSearchPaths,
                        rhs.platform, rhs.backendValidationLevel, rhs.beginCaptureOnStartup);
}

// Instance

Instance::Instance(const WGPUInstanceDescriptor* desc)
    : mImpl(APICreateInstance(reinterpret_cast<const InstanceDescriptor*>(desc))) {
    tint::Initialize();
}

Instance::Instance(const wgpu::InstanceDescriptor* desc)
    : mImpl(APICreateInstance(reinterpret_cast<const InstanceDescriptor*>(desc))) {
    tint::Initialize();
}

Instance::Instance(InstanceBase* impl) : mImpl(impl) {
    if (mImpl != nullptr) {
        mImpl->APIAddRef();
    }
    tint::Initialize();
}

Instance::~Instance() {
    if (mImpl != nullptr) {
        mImpl->APIRelease();
        mImpl = nullptr;
    }
}

std::vector<Adapter> Instance::EnumerateAdapters(const WGPURequestAdapterOptions* options) const {
    // Adapters are owned by mImpl so it is safe to return non RAII pointers to them
    std::vector<Adapter> adapters;
    for (const Ref<AdapterBase>& adapter : mImpl->EnumerateAdapters(FromAPI(options))) {
        adapters.push_back(Adapter(adapter.Get()));
    }
    return adapters;
}

std::vector<Adapter> Instance::EnumerateAdapters(const wgpu::RequestAdapterOptions* options) const {
    return EnumerateAdapters(reinterpret_cast<const WGPURequestAdapterOptions*>(options));
}

const ToggleInfo* Instance::GetToggleInfo(const char* toggleName) {
    return mImpl->GetToggleInfo(toggleName);
}

void Instance::SetBackendValidationLevel(BackendValidationLevel level) {
    mImpl->SetBackendValidationLevel(level);
}

uint64_t Instance::GetDeviceCountForTesting() const {
    return mImpl->GetDeviceCountForTesting();
}

uint64_t Instance::GetDeprecationWarningCountForTesting() const {
    return mImpl->GetDeprecationWarningCountForTesting();
}

WGPUInstance Instance::Get() const {
    return ToAPI(mImpl);
}

void Instance::DisconnectDawnPlatform() {
    mImpl->DisconnectDawnPlatform();
}

size_t GetLazyClearCountForTesting(WGPUDevice device) {
    return FromAPI(device)->GetLazyClearCountForTesting();
}

bool IsTextureSubresourceInitialized(WGPUTexture texture,
                                     uint32_t baseMipLevel,
                                     uint32_t levelCount,
                                     uint32_t baseArrayLayer,
                                     uint32_t layerCount,
                                     WGPUTextureAspect cAspect) {
    TextureBase* textureBase = FromAPI(texture);
    if (textureBase->IsError()) {
        return false;
    }

    Aspect aspect =
        ConvertAspect(textureBase->GetFormat(), static_cast<wgpu::TextureAspect>(cAspect));
    SubresourceRange range(aspect, {baseArrayLayer, layerCount}, {baseMipLevel, levelCount});
    return textureBase->IsSubresourceContentInitialized(range);
}

std::vector<std::string_view> GetProcMapNamesForTestingInternal();

std::vector<std::string_view> GetProcMapNamesForTesting() {
    return GetProcMapNamesForTestingInternal();
}

DAWN_NATIVE_EXPORT bool DeviceTick(WGPUDevice device) {
    return FromAPI(device)->APITick();
}

DAWN_NATIVE_EXPORT bool InstanceProcessEvents(WGPUInstance instance) {
    return FromAPI(instance)->ProcessEvents();
}

// ExternalImageDescriptor

ExternalImageDescriptor::ExternalImageDescriptor(ExternalImageType type) : mType(type) {}

ExternalImageType ExternalImageDescriptor::GetType() const {
    return mType;
}

// ExternalImageExportInfo

ExternalImageExportInfo::ExternalImageExportInfo(ExternalImageType type) : mType(type) {}

ExternalImageType ExternalImageExportInfo::GetType() const {
    return mType;
}

bool CheckIsErrorForTesting(void* objectHandle) {
    return reinterpret_cast<ErrorMonad*>(objectHandle)->IsError();
}

const char* GetObjectLabelForTesting(void* objectHandle) {
    ApiObjectBase* object = reinterpret_cast<ApiObjectBase*>(objectHandle);
    return object->GetLabel().c_str();
}

uint64_t GetAllocatedSizeForTesting(WGPUBuffer buffer) {
    return FromAPI(buffer)->GetAllocatedSize();
}

std::vector<const ToggleInfo*> AllToggleInfos() {
    return TogglesInfo::AllToggleInfos();
}

const FeatureInfo* GetFeatureInfo(wgpu::FeatureName feature) {
    Feature f = FromAPI(feature);
    if (f == Feature::InvalidEnum) {
        return nullptr;
    }
    return &kFeatureNameAndInfoList[FromAPI(feature)];
}

void DumpMemoryStatistics(WGPUDevice device, MemoryDump* dump) {
    auto deviceGuard = FromAPI(device)->GetGuard();
    FromAPI(device)->DumpMemoryStatistics(dump);
}

MemoryUsageInfo ComputeEstimatedMemoryUsageInfo(WGPUDevice device) {
    auto deviceGuard = FromAPI(device)->GetGuard();
    return FromAPI(device)->ComputeEstimatedMemoryUsage();
}

AllocatorMemoryInfo GetAllocatorMemoryInfo(WGPUDevice device) {
    auto deviceGuard = FromAPI(device)->GetGuard();
    return FromAPI(device)->GetAllocatorMemoryInfo();
}

bool ReduceMemoryUsage(WGPUDevice device) {
    auto deviceGuard = FromAPI(device)->GetGuard();
    return FromAPI(device)->ReduceMemoryUsage();
}

void PerformIdleTasks(const wgpu::Device& device) {
    auto* deviceBase = FromAPI(device.Get());
    auto deviceGuard = deviceBase->GetGuard();
    deviceBase->PerformIdleTasks();
}

bool IsDeviceLost(WGPUDevice device) {
    return FromAPI(device)->IsLost();
}

}  // namespace dawn::native
