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

#include <jni.h>

#ifdef __ANDROID__
#include <android/bitmap.h>
#include <android/hardware_buffer_jni.h>
#include <backend/platforms/PlatformEGLAndroid.h>
#if FILAMENT_SUPPORTS_VULKAN
#include <backend/platforms/VulkanPlatformAndroid.h>
#endif

#include <filament/Engine.h>
#include <filament/Texture.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"
#include <common/JniUtils.h>

#include "private/backend/VirtualMachineEnv.h"

using namespace filament;
using namespace backend;
using namespace filament::android;

#define BITMAP_CONFIG_ALPHA_8   0
#define BITMAP_CONFIG_RGB_565   1
#define BITMAP_CONFIG_RGBA_4444 2
#define BITMAP_CONFIG_RGBA_8888 3
#define BITMAP_CONFIG_RGBA_F16  4
#define BITMAP_CONFIG_HARDWARE  5

class AutoBitmap : public JniCallback {
private:

    AutoBitmap(JNIEnv* env, jobject bitmap) noexcept
            : JniCallback(),
              mBitmap(env->NewGlobalRef(bitmap)) {
        if (mBitmap) {
            AndroidBitmap_getInfo(env, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(env, mBitmap, &mData);
        }
    }

    AutoBitmap(JNIEnv* env, jobject bitmap, jobject handler, jobject runnable) noexcept
            : JniCallback(env, handler, runnable),
              mBitmap(env->NewGlobalRef(bitmap)) {
        if (mBitmap) {
            AndroidBitmap_getInfo(env, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(env, mBitmap, &mData);
        }
    }

    void release(JNIEnv* env) {
        if (mBitmap) {
            AndroidBitmap_unlockPixels(env, mBitmap);
            env->DeleteGlobalRef(mBitmap);
        }
    }

    ~AutoBitmap() override = default;

public:
    void* getData() const noexcept {
        return mData;
    }

    size_t getSizeInBytes() const noexcept {
        return mInfo.height * mInfo.stride;
    }

    PixelDataFormat getFormat(int format) const noexcept {
        // AndroidBitmapInfo does not capture the HARDWARE and RGBA_F16 formats
        // so we switch on the Bitmap.Config values directly
        switch (format) {
            case BITMAP_CONFIG_ALPHA_8: return PixelDataFormat::ALPHA;
            case BITMAP_CONFIG_RGB_565: return PixelDataFormat::RGB;
            default:                    return PixelDataFormat::RGBA;
        }
    }

    PixelDataType getType(int format) const noexcept {
        switch (format) {
            case BITMAP_CONFIG_RGB_565:  return PixelDataType::USHORT_565;
            case BITMAP_CONFIG_RGBA_F16: return PixelDataType::HALF;
            default:                     return PixelDataType::UBYTE;
        }
    }

    // create a AutoBitmap
    static AutoBitmap* make(JNIEnv* env, jobject bitmap, jobject handler, jobject runnable) {
        return new AutoBitmap(env, bitmap, handler, runnable);
    }

    // execute the callback on the java thread and destroy ourselves
    static void invoke(void*, size_t, void* user) {
        auto* autoBitmap = reinterpret_cast<AutoBitmap*>(user);
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        releaseCallbackJni(env, autoBitmap->mCallbackUtils, autoBitmap->mHandler, autoBitmap->mCallback);
        autoBitmap->release(env);
        delete autoBitmap;
    }

    // create a AutoBitmap without a handler
    static AutoBitmap* make(JNIEnv* env, jobject bitmap) {
        return new AutoBitmap(env, bitmap);
    }

    // just destroy ourselves
    static void invokeNoCallback(void*, size_t, void* user) {
        auto* autoBitmap = reinterpret_cast<AutoBitmap*>(user);
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        autoBitmap->release(env);
        delete autoBitmap;
    }

private:
    void* mData = nullptr;
    jobject mBitmap{};
    AndroidBitmapInfo mInfo{};
};

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetBitmap(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject bitmap, jint format) {
    Texture* texture = (Texture*) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    auto* autoBitmap = AutoBitmap::make(env, bitmap);

    Texture::PixelBufferDescriptor desc(
            autoBitmap->getData(),
            autoBitmap->getSizeInBytes(),
            autoBitmap->getFormat(format),
            autoBitmap->getType(format),
            &AutoBitmap::invokeNoCallback, autoBitmap);

    filament::android::wrapJni(env, [&]() {
        texture->setImage(*engine, (size_t) level,
                (uint32_t) xoffset, (uint32_t) yoffset,
                (uint32_t) width, (uint32_t) height,
                std::move(desc));
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetBitmapWithCallback(JNIEnv* env, jclass,
        jlong nativeTexture, jlong nativeEngine, jint level, jint xoffset, jint yoffset,
        jint width, jint height, jobject bitmap, jint format, jobject handler, jobject runnable) {
    Texture* texture = (Texture*) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    auto* autoBitmap = AutoBitmap::make(env, bitmap, handler, runnable);

    Texture::PixelBufferDescriptor desc(
            autoBitmap->getData(),
            autoBitmap->getSizeInBytes(),
            autoBitmap->getFormat(format),
            autoBitmap->getType(format),
            autoBitmap->getHandler(), &AutoBitmap::invoke, autoBitmap);

    filament::android::wrapJni(env, [&]() {
        texture->setImage(*engine, (size_t) level,
                (uint32_t) xoffset, (uint32_t) yoffset,
                (uint32_t) width, (uint32_t) height,
                std::move(desc));
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_android_TextureHelper_nSetExternalImageByAHB(JNIEnv *env, jclass clazz,
        jlong nativeTexture, jlong nativeEngine, jobject ahb) {
    Texture *texture = (Texture *) nativeTexture;
    Engine *engine = (Engine *) nativeEngine;

    Platform* platform = engine->getPlatform();
    AHardwareBuffer* nativeBuffer = nullptr;
    if (__builtin_available(android 26, *)) {
        nativeBuffer = AHardwareBuffer_fromHardwareBuffer(env, ahb);
    }
    if (!nativeBuffer) {
        return JNI_FALSE;
    }

    return wrapJni<jboolean>(env, [=]() {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
        if (engine->getBackend() == Backend::OPENGL) {
#if UTILS_HAS_RTTI
            if (!dynamic_cast<PlatformEGLAndroid*>(platform)) {
                return JNI_FALSE;
            }
#endif
            auto* eglPlatform = (PlatformEGLAndroid*) platform;
            auto ref = eglPlatform->createExternalImage(nativeBuffer, false);
            texture->setExternalImage(*engine, ref);
        }
#if FILAMENT_SUPPORTS_VULKAN
        else if (engine->getBackend() == Backend::VULKAN) {
#if UTILS_HAS_RTTI
            if (!dynamic_cast<VulkanPlatformAndroid*>(platform)) {
                return JNI_FALSE;
            }
#endif
            auto* vkPlatform = (VulkanPlatformAndroid*) platform;
            auto ref = vkPlatform->createExternalImage(nativeBuffer, false);
            texture->setExternalImage(*engine, ref);
        }
#endif
        else {
            return JNI_FALSE;
        }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
        return JNI_TRUE;
    });
}

#endif
