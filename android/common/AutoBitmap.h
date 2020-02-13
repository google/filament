/*
 * Copyright (C) 2020 The Android Open Source Project
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

#pragma once

#include <jni.h>

#include <algorithm>
#include <functional>

#include <android/bitmap.h>

#include <backend/BufferDescriptor.h>
#include <filament/Engine.h>
#include <filament/Stream.h>
#include <filament/Texture.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

#define BITMAP_CONFIG_ALPHA_8   0
#define BITMAP_CONFIG_RGB_565   1
#define BITMAP_CONFIG_RGBA_4444 2
#define BITMAP_CONFIG_RGBA_8888 3
#define BITMAP_CONFIG_RGBA_F16  4
#define BITMAP_CONFIG_HARDWARE  5

class AutoBitmap {
public:
    AutoBitmap(JNIEnv* env, jobject bitmap) noexcept
            : mEnv(env)
            , mBitmap(env->NewGlobalRef(bitmap))
    {
        if (mBitmap) {
            AndroidBitmap_getInfo(mEnv, mBitmap, &mInfo);
            AndroidBitmap_lockPixels(mEnv, mBitmap, &mData);
        }
    }

    ~AutoBitmap() noexcept {
        if (mBitmap) {
            AndroidBitmap_unlockPixels(mEnv, mBitmap);
            mEnv->DeleteGlobalRef(mBitmap);
        }
    }

    AutoBitmap(AutoBitmap &&rhs) noexcept {
        mEnv = rhs.mEnv;
        std::swap(mData, rhs.mData);
        std::swap(mBitmap, rhs.mBitmap);
        std::swap(mInfo, rhs.mInfo);
    }

    void* getData() const noexcept {
        return mData;
    }

    size_t getSizeInBytes() const noexcept {
        return mInfo.height * mInfo.stride;
    }

    filament::backend::PixelDataFormat getFormat(int format) const noexcept {
        // AndroidBitmapInfo does not capture the HARDWARE and RGBA_F16 formats
        // so we switch on the Bitmap.Config values directly
        switch (format) {
            case BITMAP_CONFIG_ALPHA_8: return filament::backend::PixelDataFormat::ALPHA;
            case BITMAP_CONFIG_RGB_565: return filament::backend::PixelDataFormat::RGB;
            default:                    return filament::backend::PixelDataFormat::RGBA;
        }
    }

    filament::backend::PixelDataType getType(int format) const noexcept {
        switch (format) {
            case BITMAP_CONFIG_RGBA_F16: return filament::backend::PixelDataType::HALF;
            default:                     return filament::backend::PixelDataType::UBYTE;
        }
    }

    static void invoke(void* buffer, size_t n, void* user) {
        AutoBitmap* data = reinterpret_cast<AutoBitmap*>(user);
        data->~AutoBitmap();
    }

    static AutoBitmap* make(filament::Engine* engine, JNIEnv* env, jobject bitmap) {
        void* that = engine->streamAlloc(sizeof(AutoBitmap), alignof(AutoBitmap));
        return new (that) AutoBitmap(env, bitmap);
    }

private:
    JNIEnv* mEnv;
    void* mData = nullptr;
    jobject mBitmap = nullptr;
    AndroidBitmapInfo mInfo;
};
