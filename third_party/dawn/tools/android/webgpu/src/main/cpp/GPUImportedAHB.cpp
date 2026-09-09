// Copyright 2026 The Dawn & Tint Authors
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

#include <android/hardware_buffer_jni.h>
#include <android/sync.h>
#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <webgpu/webgpu_cpp.h>

#include <vector>

#include "JNIClasses.h"
#include "JNIContext.h"
#include "src/dawn/common/ColorSpace.h"
#include "src/dawn/common/ExternalTextureParams.h"
#include "structures.h"

#ifndef VK_IMAGE_LAYOUT_GENERAL
#define VK_IMAGE_LAYOUT_GENERAL 1
#endif

#ifndef VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 5
#endif

namespace dawn::kotlin_api {

struct AhbTextureWrapper {
    wgpu::SharedTextureMemory stm;
    wgpu::Texture texture;
    wgpu::TextureView view;
    wgpu::ExternalTexture externalTexture;
    AHardwareBuffer* ahb = nullptr;
    wgpu::Device device;
};

struct UniqueFd {
    int fd = -1;
    UniqueFd(int f) : fd(f) {}
    ~UniqueFd() {
        if (fd >= 0) {
            close(fd);
        }
    }
    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;
    UniqueFd(UniqueFd&& other) : fd(other.fd) { other.fd = -1; }
    UniqueFd& operator=(UniqueFd&& other) {
        if (this != &other) {
            if (fd >= 0) {
                close(fd);
            }
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    int release() {
        int f = fd;
        fd = -1;
        return f;
    }
    operator int() const { return fd; }
};

extern "C" {

static wgpu::SharedTextureMemory createSharedTextureMemoryFromAhb(JNIEnv* env,
                                                                  jobject deviceObj,
                                                                  jobject hardwareBuffer,
                                                                  AHardwareBuffer** outAhb,
                                                                  wgpu::Device* outDevice) {
    JNIClasses* classes = JNIClasses::getInstance(env);
    jmethodID getHandle = env->GetMethodID(classes->device, "getHandle", "()J");

    WGPUDevice rawDevice = reinterpret_cast<WGPUDevice>(env->CallLongMethod(deviceObj, getHandle));
    if (!rawDevice) {
        env->ThrowNew(classes->javaIllegalStateException, "WGPUDevice handle is null.");
        return nullptr;
    }
    *outDevice = wgpu::Device(rawDevice);

    *outAhb = AHardwareBuffer_fromHardwareBuffer(env, hardwareBuffer);
    if (!*outAhb) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Failed to extract native AHardwareBuffer.");
        return nullptr;
    }
    // AHardwareBuffer_fromHardwareBuffer does not acquire a reference; the returned pointer
    // is only guaranteed to be valid while the Java HardwareBuffer remains alive. Since the
    // wrapper outlives the JNI call, take our own reference to match the release in
    // nativeDestroy.
    AHardwareBuffer_acquire(*outAhb);

    wgpu::SharedTextureMemoryAHardwareBufferDescriptor ahbDesc = {};
    ahbDesc.handle = *outAhb;

    wgpu::SharedTextureMemoryDescriptor stmDesc = {};
    stmDesc.nextInChain = &ahbDesc;

    wgpu::SharedTextureMemory stm = outDevice->ImportSharedTextureMemory(&stmDesc);
    if (!stm && !env->ExceptionCheck()) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Dawn failed to import the AHardwareBuffer.");
    }
    return stm;
}

JNIEXPORT jobject JNICALL
Java_androidx_webgpu_helper_GPUAndroidHardwareBufferUtil_createTextureNative(JNIEnv* env,
                                                                             jclass clazz,
                                                                             jobject deviceObj,
                                                                             jobject hardwareBuffer,
                                                                             jint usage) {
    JNIContext c(env);
    JNIClasses* classes = JNIClasses::getInstance(env);
    wgpu::Device device = nullptr;
    AHardwareBuffer* ahb = nullptr;

    wgpu::SharedTextureMemory stm =
        createSharedTextureMemoryFromAhb(env, deviceObj, hardwareBuffer, &ahb, &device);
    if (!stm) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        return nullptr;
    }

    if (!device.HasFeature(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer)) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "Missing wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer.");
        return nullptr;
    }

    wgpu::SharedTextureMemoryProperties props = {};
    stm.GetProperties(&props);

    if (props.format == wgpu::TextureFormat::Undefined) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        env->ThrowNew(
            classes->javaUnsupportedOperationException,
            "AHB format maps to wgpu::TextureFormat::Undefined. Dawn cannot sample this.");
        return nullptr;
    }

    wgpu::TextureDescriptor texDesc = {};
    texDesc.usage = static_cast<wgpu::TextureUsage>(usage);
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = {props.size.width, props.size.height, props.size.depthOrArrayLayers};
    texDesc.format = props.format;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;

    wgpu::Texture texture = stm.CreateTexture(&texDesc);
    if (!texture || env->ExceptionCheck()) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        return nullptr;
    }

    // The Java wrapper and AhbTextureWrapper independently release references on close().
    // We explicitly add a reference here for the Java object to assume ownership of.
    wgpuTextureAddRef(texture.Get());
    auto* wrapper = new AhbTextureWrapper{stm, texture, nullptr, nullptr, ahb, device};

    jobject texture_kt =
        env->NewObject(classes->texture, env->GetMethodID(classes->texture, "<init>", "(J)V"),
                       reinterpret_cast<jlong>(texture.Get()));

    jclass wrapperClz = classes->gpuHardwareBufferTexture;
    jmethodID wrapperInit =
        env->GetMethodID(wrapperClz, "<init>", "(JLandroidx/webgpu/GPUTexture;)V");
    return env->NewObject(wrapperClz, wrapperInit, reinterpret_cast<jlong>(wrapper), texture_kt);
}

JNIEXPORT jobject JNICALL
Java_androidx_webgpu_helper_GPUAndroidHardwareBufferUtil_createExternalTextureNative(
    JNIEnv* env,
    jclass clazz,
    jobject deviceObj,
    jobject hardwareBuffer,
    jobject descriptorObj) {
    JNIContext c(env);
    JNIClasses* classes = JNIClasses::getInstance(env);
    wgpu::Device device = nullptr;
    AHardwareBuffer* ahb = nullptr;

    wgpu::SharedTextureMemory stm =
        createSharedTextureMemoryFromAhb(env, deviceObj, hardwareBuffer, &ahb, &device);
    if (!stm) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        return nullptr;
    }

    if (!device.HasFeature(wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture)) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "Missing wgpu::FeatureName::OpaqueYCbCrAndroidForExternalTexture.");
        return nullptr;
    }

    wgpu::SharedTextureMemoryProperties props = {};
    stm.GetProperties(&props);
    if (env->ExceptionCheck()) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        return nullptr;
    }

    if (props.format == wgpu::TextureFormat::Undefined) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        env->ThrowNew(classes->javaUnsupportedOperationException,
                      "YUV AHB format maps to wgpu::TextureFormat::Undefined.");
        return nullptr;
    }

    wgpu::TextureDescriptor texDesc = {};
    texDesc.usage = wgpu::TextureUsage::TextureBinding;
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = {props.size.width, props.size.height, 1};
    texDesc.format = props.format;
    texDesc.mipLevelCount = 1;
    texDesc.sampleCount = 1;

    wgpu::Texture texture = stm.CreateTexture(&texDesc);
    wgpu::TextureView view = texture.CreateView();
    if (env->ExceptionCheck()) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        return nullptr;
    }

    wgpu::ExternalTextureDescriptor extDesc = {};
    extDesc.plane0 = view;

    jclass descClass = env->GetObjectClass(descriptorObj);

    auto callObjectMethod = [&](const char* methodName, const char* sig) {
        jmethodID mid = env->GetMethodID(descClass, methodName, sig);
        return mid ? env->CallObjectMethod(descriptorObj, mid) : nullptr;
    };

    jobject colorSpaceObj =
        callObjectMethod("getSrcColorSpace", "()Landroidx/webgpu/GPUColorSpaceDawn;");
    jint dstColorSpaceInt =
        env->CallIntMethod(descriptorObj, env->GetMethodID(descClass, "getDstColorSpace", "()I"));

    wgpu::ColorSpaceDawn srcColorSpace = {};
    srcColorSpace.primaries = static_cast<wgpu::ColorSpacePrimariesDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpacePrimaries));
    srcColorSpace.transfer = static_cast<wgpu::ColorSpaceTransferDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceTransfer));
    srcColorSpace.yCbCrRange = static_cast<wgpu::ColorSpaceYCbCrRangeDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceYCbCrRange));
    srcColorSpace.yCbCrMatrix = static_cast<wgpu::ColorSpaceYCbCrMatrixDawn>(
        env->GetIntField(colorSpaceObj, classes->colorSpaceYCbCrMatrix));
    env->DeleteLocalRef(colorSpaceObj);

    wgpu::PredefinedColorSpace dstColorSpace =
        static_cast<wgpu::PredefinedColorSpace>(dstColorSpaceInt);

    dawn::ExternalTextureColorSpaceParams params;
    wgpu::Status status = dawn::ComputeExternalTextureParams(srcColorSpace, dstColorSpace, &params);

    if (status != wgpu::Status::Success) {
        if (ahb) {
            AHardwareBuffer_release(ahb);
        }
        env->ThrowNew(classes->javaIllegalStateException,
                      "Failed to compute external texture params.");
        return nullptr;
    }

    extDesc.yuvToRgbConversionMatrix = params.yuvToRgbConversionMatrix.data();
    extDesc.srcTransferFunctionParameters = params.srcTransferFunction.data();
    extDesc.dstTransferFunctionParameters = params.dstTransferFunction.data();
    extDesc.gamutConversionMatrix = params.gamutConversionMatrix.data();

    jobject originObj = callObjectMethod("getCropOrigin", "()Landroidx/webgpu/GPUOrigin2D;");
    if (originObj) {
        extDesc.cropOrigin.x = env->GetIntField(originObj, classes->originX);
        extDesc.cropOrigin.y = env->GetIntField(originObj, classes->originY);
        env->DeleteLocalRef(originObj);
    }

    jobject cropSizeObj = callObjectMethod("getCropSize", "()Landroidx/webgpu/GPUExtent2D;");
    if (cropSizeObj) {
        extDesc.cropSize.width = env->GetIntField(cropSizeObj, classes->extentWidth);
        extDesc.cropSize.height = env->GetIntField(cropSizeObj, classes->extentHeight);
        env->DeleteLocalRef(cropSizeObj);
    }
    if (extDesc.cropSize.width == 0) {
        extDesc.cropSize.width = props.size.width;
    }
    if (extDesc.cropSize.height == 0) {
        extDesc.cropSize.height = props.size.height;
    }

    extDesc.apparentSize = extDesc.cropSize;
    extDesc.mirrored = static_cast<bool>(
        env->CallBooleanMethod(descriptorObj, env->GetMethodID(descClass, "isMirrored", "()Z")));

    jint rotation =
        env->CallIntMethod(descriptorObj, env->GetMethodID(descClass, "getRotation", "()I"));
    switch (rotation) {
        case 2:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate90Degrees;
            break;
        case 3:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate180Degrees;
            break;
        case 4:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate270Degrees;
            break;
        default:
            extDesc.rotation = wgpu::ExternalTextureRotation::Rotate0Degrees;
            break;
    }

    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&extDesc);
    env->DeleteLocalRef(descClass);

    // The Java wrapper and AhbTextureWrapper independently release references on close().
    // We explicitly add a reference here for the Java object to assume ownership of.
    wgpuExternalTextureAddRef(externalTexture.Get());
    auto* wrapper = new AhbTextureWrapper{stm, texture, view, externalTexture, ahb, device};

    jobject extTex_kt = env->NewObject(classes->externalTexture,
                                       env->GetMethodID(classes->externalTexture, "<init>", "(J)V"),
                                       reinterpret_cast<jlong>(externalTexture.Get()));

    jclass wrapperClz = classes->gpuHardwareBufferExternalTexture;
    jmethodID wrapperInit =
        env->GetMethodID(wrapperClz, "<init>", "(JLandroidx/webgpu/GPUExternalTexture;)V");

    return env->NewObject(wrapperClz, wrapperInit, reinterpret_cast<jlong>(wrapper), extTex_kt);
}

JNIEXPORT void JNICALL
Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeBeginAccess(JNIEnv* env,
                                                                jobject thiz,
                                                                jlong handle,
                                                                jintArray fences) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);
    JNIClasses* classes = JNIClasses::getInstance(env);

    std::vector<wgpu::SharedFence> importedFences;
    if (fences != nullptr) {
        jsize len = env->GetArrayLength(fences);
        if (len > 0) {
            jint* fds = env->GetIntArrayElements(fences, nullptr);
            for (int i = 0; i < len; ++i) {
                int fd = fds[i];
                if (fd >= 0) {
                    // We only borrow the integer from Java's ParcelFileDescriptor.getFd().
                    // Dawn's ImportSharedFence internally duplicates (dup) the file descriptor.
                    // This allows Java's ParcelFileDescriptor to retain ownership of the original
                    // FD and close it safely when it gets garbage collected, preventing leaks.
                    wgpu::SharedFenceSyncFDDescriptor syncFdDesc = {};
                    syncFdDesc.handle = fd;

                    wgpu::SharedFenceDescriptor fenceDesc = {};
                    fenceDesc.nextInChain = &syncFdDesc;

                    wgpu::SharedFence fence = wrapper->device.ImportSharedFence(&fenceDesc);
                    if (fence) {
                        importedFences.push_back(fence);
                    }
                }
            }
            env->ReleaseIntArrayElements(fences, fds, JNI_ABORT);
        }
    }

    std::vector<uint64_t> signaledValues(importedFences.size(), 1);
    wgpu::SharedTextureMemoryBeginAccessDescriptor beginDesc = {};
    beginDesc.fenceCount = importedFences.size();
    beginDesc.fences = importedFences.data();
    beginDesc.signaledValueCount = signaledValues.size();
    beginDesc.signaledValues = signaledValues.data();
    beginDesc.concurrentRead = false;
    beginDesc.initialized = true;

    wgpu::SharedTextureMemoryVkImageLayoutBeginState beginLayout = {};
    wgpu::AdapterInfo adapterInfo;
    wrapper->device.GetAdapterInfo(&adapterInfo);
    if (adapterInfo.backendType == wgpu::BackendType::Vulkan) {
        beginLayout.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        beginLayout.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        beginDesc.nextInChain = &beginLayout;
    }

    wrapper->stm.BeginAccess(wrapper->texture, &beginDesc);
}

JNIEXPORT jint JNICALL Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeEndAccess(JNIEnv* env,
                                                                                     jobject thiz,
                                                                                     jlong handle) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);
    JNIClasses* classes = JNIClasses::getInstance(env);

    wgpu::SharedTextureMemoryEndAccessState endState = {};
    wgpu::SharedTextureMemoryVkImageLayoutEndState endLayout = {};
    wgpu::AdapterInfo adapterInfo;
    wrapper->device.GetAdapterInfo(&adapterInfo);
    if (adapterInfo.backendType == wgpu::BackendType::Vulkan) {
        endLayout.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        endLayout.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        endState.nextInChain = &endLayout;
    }

    wgpu::Status status = wrapper->stm.EndAccess(wrapper->texture, &endState);
    if (status != wgpu::Status::Success) {
        env->ThrowNew(classes->javaIllegalStateException,
                      "Failed to end access on SharedTextureMemory.");
        return -1;
    }

    std::vector<UniqueFd> fds;
    bool failed = false;
    if (endState.fenceCount > 0) {
        for (size_t i = 0; i < endState.fenceCount && !failed; ++i) {
            wgpu::SharedFence cFence = endState.fences[i];

            wgpu::SharedFenceExportInfo exportInfo = {};
            wgpu::SharedFenceSyncFDExportInfo syncFdExportInfo = {};
            exportInfo.nextInChain = &syncFdExportInfo;

            cFence.ExportInfo(&exportInfo);
            int fd = syncFdExportInfo.handle;
            if (fd >= 0) {
                // Dawn's ExportInfo borrows the FD; Dawn retains ownership of the underlying fence
                // and closes it when the WGPUSharedFence is released. We dup() it here to safely
                // take ownership of our own copy for JNI.
                int dupedFd = dup(fd);
                if (dupedFd >= 0) {
                    fds.emplace_back(dupedFd);
                } else {
                    failed = true;
                }
            }
        }
    }

    UniqueFd mergedFd(-1);
    for (size_t i = 0; i < fds.size() && !failed; ++i) {
        UniqueFd& fd = fds[i];
        if (mergedFd.fd == -1) {
            mergedFd = std::move(fd);
        } else {
            int newFd = sync_merge("wgpu_fence", mergedFd.fd, fd.fd);
            mergedFd = UniqueFd(newFd);
            if (newFd < 0) {
                failed = true;
            }
        }
    }

    if (failed) {
        env->ThrowNew(classes->javaIllegalStateException, "Failed to merge sync fences.");
        return -1;
    }

    if (mergedFd.fd >= 0) {
        return mergedFd.release();
    }

    return -1;
}

JNIEXPORT void JNICALL Java_androidx_webgpu_GPUHardwareBufferWrapper_nativeDestroy(JNIEnv* env,
                                                                                   jobject thiz,
                                                                                   jlong handle) {
    auto* wrapper = reinterpret_cast<AhbTextureWrapper*>(handle);

    if (wrapper->ahb) {
        AHardwareBuffer_release(wrapper->ahb);
    }

    delete wrapper;
}

JNIEXPORT jboolean JNICALL
Java_androidx_webgpu_GPUSyncFence_awaitParcelFileDescriptorNative(JNIEnv* env,
                                                                  jobject thiz,
                                                                  jobject pfd,
                                                                  jlong timeoutMillis) {
    if (!pfd) {
        return JNI_TRUE;
    }

    JNIClasses* classes = JNIClasses::getInstance(env);
    jclass pfdClass = classes->parcelFileDescriptor;
    jmethodID getFdMethod = classes->pfdGetFd;
    int fd = env->CallIntMethod(pfd, getFdMethod);

    if (fd < 0) {
        return JNI_TRUE;
    }

    struct pollfd pfd_struct = {
        .fd = fd,
        .events = POLLIN,
    };

    int result = poll(&pfd_struct, 1, timeoutMillis);
    return (result > 0 && (pfd_struct.revents & POLLIN)) ? JNI_TRUE : JNI_FALSE;
}

}  // extern "C"
}  // namespace dawn::kotlin_api
