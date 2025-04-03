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

#include "dawn/native/opengl/DeviceGL.h"

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/opengl/BindGroupGL.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/CommandBufferGL.h"
#include "dawn/native/opengl/ComputePipelineGL.h"
#include "dawn/native/opengl/ContextEGL.h"
#include "dawn/native/opengl/DisplayEGL.h"
#include "dawn/native/opengl/PhysicalDeviceGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/QuerySetGL.h"
#include "dawn/native/opengl/QueueGL.h"
#include "dawn/native/opengl/RenderPipelineGL.h"
#include "dawn/native/opengl/SamplerGL.h"
#include "dawn/native/opengl/ShaderModuleGL.h"
#include "dawn/native/opengl/SharedFenceEGL.h"
#include "dawn/native/opengl/SharedTextureMemoryEGL.h"
#include "dawn/native/opengl/SwapChainEGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "dawn/native/opengl/opengl_platform.h"

#if DAWN_PLATFORM_IS(ANDROID)
#include "dawn/native/AHBFunctions.h"
#endif  // DAWN_PLATFORM_IS(ANDROID)

namespace {

void KHRONOS_APIENTRY OnGLDebugMessage(GLenum source,
                                       GLenum type,
                                       GLuint id,
                                       GLenum severity,
                                       GLsizei length,
                                       const GLchar* message,
                                       const void* userParam) {
    const char* sourceText;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceText = "OpenGL";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceText = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceText = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceText = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceText = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceText = "Other";
            break;
        default:
            sourceText = "UNKNOWN";
            break;
    }

    const char* severityText;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severityText = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityText = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityText = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityText = "Notification";
            break;
        default:
            severityText = "UNKNOWN";
            break;
    }

    if (type == GL_DEBUG_TYPE_ERROR) {
        dawn::WarningLog() << "OpenGL error:" << "\n    Source: " << sourceText  //
                           << "\n    ID: " << id                                 //
                           << "\n    Severity: " << severityText                 //
                           << "\n    Message: " << message;

        // Abort on an error when in Debug mode.
        DAWN_UNREACHABLE();
    }
}

}  // anonymous namespace

namespace dawn::native::opengl {

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const UnpackedPtr<DeviceDescriptor>& descriptor,
                                          const OpenGLFunctions& functions,
                                          std::unique_ptr<ContextEGL> context,
                                          const TogglesState& deviceToggles,
                                          Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    Ref<Device> device = AcquireRef(new Device(adapter, descriptor, functions, std::move(context),
                                               deviceToggles, std::move(lostEvent)));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::Device(AdapterBase* adapter,
               const UnpackedPtr<DeviceDescriptor>& descriptor,
               const OpenGLFunctions& functions,
               std::unique_ptr<ContextEGL> context,
               const TogglesState& deviceToggles,
               Ref<DeviceBase::DeviceLostEvent>&& lostEvent)
    : DeviceBase(adapter, descriptor, deviceToggles, std::move(lostEvent)),
      mGL(functions),
      mContext(std::move(context)) {}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    // Directly set the context current and use mGL instead of calling GetGL as GetGL will notify
    // the (yet inexistent) queue that GL was used.
    mContext->MakeCurrent();
    mContext->RequestRequiredExtensionsExplicitly();

    const OpenGLFunctions& gl = mGL;

    mFormatTable = BuildGLFormatTable(gl);

    // Use the debug output functionality to get notified about GL errors
    // TODO(crbug.com/dawn/1475): add support for the KHR_debug and ARB_debug_output
    // extensions
    bool hasDebugOutput = gl.IsAtLeastGL(4, 3) || gl.IsAtLeastGLES(3, 2);

    if (GetAdapter()->GetInstance()->IsBackendValidationEnabled() && hasDebugOutput) {
        DAWN_GL_TRY(gl, Enable(GL_DEBUG_OUTPUT));
        DAWN_GL_TRY(gl, Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS));

        // Any GL error; dangerous undefined behavior; any shader compiler and linker errors
        DAWN_GL_TRY(gl, DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0,
                                            nullptr, GL_TRUE));

        // Severe performance warnings; GLSL or other shader compiler and linker warnings;
        // use of currently deprecated behavior
        DAWN_GL_TRY(gl, DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0,
                                            nullptr, GL_TRUE));

        // Performance warnings from redundant state changes; trivial undefined behavior
        // This is disabled because we do an incredible amount of redundant state changes.
        DAWN_GL_TRY(gl, DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0,
                                            nullptr, GL_FALSE));

        // Any message which is not an error or performance concern
        DAWN_GL_TRY(gl, DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                                            GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE));
        DAWN_GL_TRY(gl, DebugMessageCallback(&OnGLDebugMessage, nullptr));
    }

    // Set initial state.
    DAWN_GL_TRY(gl, Enable(GL_DEPTH_TEST));
    DAWN_GL_TRY(gl, Enable(GL_SCISSOR_TEST));
    if (gl.GetVersion().IsDesktop()) {
        // These are not necessary on GLES. The functionality is enabled by default, and
        // works by specifying sample counts and SRGB textures, respectively.
        DAWN_GL_TRY(gl, Enable(GL_MULTISAMPLE));
        DAWN_GL_TRY(gl, Enable(GL_FRAMEBUFFER_SRGB));
    }
    DAWN_GL_TRY(gl, Enable(GL_SAMPLE_MASK));

    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue));
    if (HasAnisotropicFiltering(gl)) {
        DAWN_GL_TRY(gl, GetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &mMaxTextureMaxAnisotropy));
    }
    return DeviceBase::Initialize(std::move(queue));
}

const GLFormat& Device::GetGLFormat(const Format& format) {
    DAWN_ASSERT(format.IsSupported());
    DAWN_ASSERT(format.GetIndex() < mFormatTable.size());

    const GLFormat& result = mFormatTable[format.GetIndex()];
    DAWN_ASSERT(result.isSupportedOnBackend);
    return result;
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(this, descriptor));
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::Create(this, descriptor);
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return AcquireRef(new PipelineLayout(this, descriptor));
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, internalExtensions, parseResult,
                                compilationMessages);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(Surface* surface,
                                                              SwapChainBase* previousSwapChain,
                                                              const SurfaceConfiguration* config) {
    return SwapChainEGL::Create(this, surface, previousSwapChain, config);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::Create(this, descriptor);
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    return TextureView::Create(texture, descriptor);
}

ResultOrError<Ref<SharedTextureMemoryBase>> Device::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* descriptor) {
    UnpackedPtr<SharedTextureMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (unpacked.ValidateBranches<Branch<SharedTextureMemoryAHardwareBufferDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedTextureMemoryAHardwareBufferDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryAHardwareBuffer),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer);
            return SharedTextureMemoryEGL::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryAHardwareBufferDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<Ref<SharedFenceBase>> Device::ImportSharedFenceImpl(
    const SharedFenceDescriptor* descriptor) {
    UnpackedPtr<SharedFenceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type, (unpacked.ValidateBranches<Branch<SharedFenceSyncFDDescriptor>,
                                                     Branch<SharedFenceEGLSyncDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedFenceSyncFDDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceSyncFD), "%s is not enabled.",
                            wgpu::FeatureName::SharedFenceSyncFD);
            return SharedFenceEGL::Create(this, descriptor->label,
                                          unpacked.Get<SharedFenceSyncFDDescriptor>());
        case wgpu::SType::SharedFenceEGLSyncDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceEGLSync), "%s is not enabled.",
                            wgpu::FeatureName::SharedFenceEGLSync);
            return SharedFenceEGL::Create(this, descriptor->label,
                                          unpacked.Get<SharedFenceEGLSyncDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError Device::ValidateTextureCanBeWrapped(const UnpackedPtr<TextureDescriptor>& descriptor) {
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);

    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);

    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);

    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    return {};
}

Ref<TextureBase> Device::CreateTextureWrappingEGLImage(const ExternalImageDescriptor* descriptor,
                                                       ::EGLImage image) {
    Ref<TextureBase> result;
    if (ConsumedError(CreateTextureWrappingEGLImageImpl(descriptor, image), &result,
                      "calling %s.CreateTextureWrappingEGLImage(%s).", this, descriptor)) {
        return nullptr;
    }
    return result;
}

ResultOrError<Ref<TextureBase>> Device::CreateTextureWrappingEGLImageImpl(
    const ExternalImageDescriptor* descriptor,
    ::EGLImage image) {
    const OpenGLFunctions& gl = GetGL();

    TextureDescriptor reifiedDescriptor =
        FromAPI(descriptor->cTextureDescriptor)->WithTrivialFrontendDefaults();
    UnpackedPtr<TextureDescriptor> textureDescriptor;
    DAWN_TRY_ASSIGN(textureDescriptor, ValidateAndUnpack(&reifiedDescriptor));
    DAWN_TRY(ValidateTextureDescriptor(this, textureDescriptor));
    DAWN_TRY(ValidateTextureCanBeWrapped(textureDescriptor));
    // The EGLImage was created from outside of Dawn so it must be on the same display that was
    // provided to create the device. The best check we can do is that we indeed have
    // EGL_KHR_image_base.
    DAWN_ASSERT(GetEGL(false).HasExt(EGLExt::ImageBase));

    GLuint tex;
    DAWN_GL_TRY(gl, GenTextures(1, &tex));
    DAWN_GL_TRY(gl, BindTexture(GL_TEXTURE_2D, tex));
    DAWN_GL_TRY(gl, EGLImageTargetTexture2DOES(GL_TEXTURE_2D, image));
    DAWN_GL_TRY(gl, TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

    GLint width, height;
    DAWN_GL_TRY(gl, GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
    DAWN_GL_TRY(gl, GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));

    if (textureDescriptor->size.width != static_cast<uint32_t>(width) ||
        textureDescriptor->size.height != static_cast<uint32_t>(height) ||
        textureDescriptor->size.depthOrArrayLayers != 1) {
        DAWN_GL_TRY(gl, DeleteTextures(1, &tex));

        return DAWN_VALIDATION_ERROR(
            "EGLImage size (width: %u, height: %u, depth: 1) doesn't match descriptor size %s.",
            width, height, &textureDescriptor->size);
    }

    // TODO(dawn:803): Validate the OpenGL texture format from the EGLImage against the format
    // in the passed-in TextureDescriptor.
    auto result = AcquireRef(new Texture(this, textureDescriptor, tex, OwnsHandle::No));
    result->SetIsSubresourceContentInitialized(descriptor->isInitialized,
                                               result->GetAllSubresources());
    return result;
}

Ref<TextureBase> Device::CreateTextureWrappingGLTexture(const ExternalImageDescriptor* descriptor,
                                                        GLuint texture) {
    Ref<TextureBase> result;
    if (ConsumedError(CreateTextureWrappingGLTextureImpl(descriptor, texture), &result,
                      "calling %s.CreateTextureWrappingGLTexture(%s).", this, descriptor)) {
        return nullptr;
    }
    return result;
}

ResultOrError<Ref<TextureBase>> Device::CreateTextureWrappingGLTextureImpl(
    const ExternalImageDescriptor* descriptor,
    GLuint texture) {
    const OpenGLFunctions& gl = GetGL();

    TextureDescriptor reifiedDescriptor =
        FromAPI(descriptor->cTextureDescriptor)->WithTrivialFrontendDefaults();
    UnpackedPtr<TextureDescriptor> textureDescriptor;
    DAWN_TRY_ASSIGN(textureDescriptor, ValidateAndUnpack(&reifiedDescriptor));
    DAWN_TRY(ValidateTextureDescriptor(this, textureDescriptor));
    if (!HasFeature(Feature::ANGLETextureSharing)) {
        return DAWN_VALIDATION_ERROR("Device does not support ANGLE GL texture sharing.");
    }
    DAWN_TRY(ValidateTextureCanBeWrapped(textureDescriptor));

    DAWN_GL_TRY(gl, BindTexture(GL_TEXTURE_2D, texture));

    GLint width, height;
    DAWN_GL_TRY(gl, GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
    DAWN_GL_TRY(gl, GetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));

    if (textureDescriptor->size.width != static_cast<uint32_t>(width) ||
        textureDescriptor->size.height != static_cast<uint32_t>(height) ||
        textureDescriptor->size.depthOrArrayLayers != 1) {
        return DAWN_VALIDATION_ERROR(
            "GL texture size (width: %u, height: %u, depth: 1) doesn't match descriptor size %s.",
            width, height, &textureDescriptor->size);
    }

    auto result = AcquireRef(new Texture(this, textureDescriptor, texture, OwnsHandle::No));
    result->SetIsSubresourceContentInitialized(descriptor->isInitialized,
                                               result->GetAllSubresources());
    return result;
}

MaybeError Device::TickImpl() {
    DAWN_TRY(ToBackend(GetQueue())->SubmitFenceSync());
    return {};
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer.");
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TexelCopyBufferLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    // If implemented, be sure to call SynchronizeTextureBeforeUse on the destination texture.
    return DAWN_UNIMPLEMENTED_ERROR("Device unable to copy from staging buffer to texture.");
}

void Device::DestroyImpl() {
    DAWN_ASSERT(GetState() == State::Disconnected);
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 1;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

bool Device::MayRequireDuplicationOfIndirectParameters() const {
    return true;
}

bool Device::ShouldApplyIndexBufferOffsetToFirstIndex() const {
    return true;
}

const AHBFunctions* Device::GetOrLoadAHBFunctions() {
#if DAWN_PLATFORM_IS(ANDROID)
    if (mAHBFunctions == nullptr) {
        mAHBFunctions = std::make_unique<AHBFunctions>();
    }
    return mAHBFunctions.get();
#else
    DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(ANDROID)
}

const OpenGLFunctions& Device::GetGL() const {
    mContext->MakeCurrent();
    ToBackend(GetQueue())->OnGLUsed();
    return mGL;
}

int Device::GetMaxTextureMaxAnisotropy() const {
    return mMaxTextureMaxAnisotropy;
}

const EGLFunctions& Device::GetEGL(bool makeCurrent) const {
    if (makeCurrent) {
        mContext->MakeCurrent();
        ToBackend(GetQueue())->OnGLUsed();
    }
    return ToBackend(GetPhysicalDevice())->GetDisplay()->egl;
}

EGLDisplay Device::GetEGLDisplay() const {
    return ToBackend(GetPhysicalDevice())->GetDisplay()->GetDisplay();
}

ContextEGL* Device::GetContext() const {
    return mContext.get();
}

}  // namespace dawn::native::opengl
