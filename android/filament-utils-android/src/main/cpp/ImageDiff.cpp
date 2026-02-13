/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include <android/bitmap.h>

#include <imagediff/ImageDiff.h>
#include <utils/Log.h>

#include <vector>

using namespace imagediff;
using namespace utils;

namespace {

struct BitmapLock {
    JNIEnv* env;
    jobject bitmap;
    void* pixels;
    AndroidBitmapInfo info;

    BitmapLock(JNIEnv* env, jobject bitmap) : env(env), bitmap(bitmap), pixels(nullptr) {
        if (!bitmap) return;
        if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
            return;
        }
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
            pixels = nullptr;
        }
    }

    ~BitmapLock() {
        if (pixels) {
            AndroidBitmap_unlockPixels(env, bitmap);
        }
    }

    bool isValid() const { return pixels != nullptr; }

    imagediff::Bitmap toBitmap() const {
        return {
            .width = (uint32_t) info.width,
            .height = (uint32_t) info.height,
            .stride = (size_t) info.stride,
            .data = pixels
        };
    }
};

} // namespace



// Helper to convert C++ ImageDiffResult to Java Result
jobject createResult(JNIEnv* env, ImageDiffResult const& result, bool generateDiff) {
    // Create Result class/objects
    jclass resultClass = env->FindClass("com/google/android/filament/utils/ImageDiff$Result");
    jmethodID resultCtor = env->GetMethodID(resultClass, "<init>", "()V");
    jobject resultObj = env->NewObject(resultClass, resultCtor);
    jfieldID statusField = env->GetFieldID(resultClass, "status", "Lcom/google/android/filament/utils/ImageDiff$Result$Status;");
    jfieldID failingCountField = env->GetFieldID(resultClass, "failingPixelCount", "J");
    jfieldID maxDiffField = env->GetFieldID(resultClass, "maxDiffFound", "[F");
    jfieldID diffImageField = env->GetFieldID(resultClass, "diffImage", "Landroid/graphics/Bitmap;");

    // Map Status enum
    jclass statusEnum = env->FindClass("com/google/android/filament/utils/ImageDiff$Result$Status");
    jobject statusObj = nullptr;
    jfieldID enumField = nullptr;
    switch (result.status) {
        case ImageDiffResult::Status::PASSED:
            enumField = env->GetStaticFieldID(statusEnum, "PASSED", "Lcom/google/android/filament/utils/ImageDiff$Result$Status;");
            break;
        case ImageDiffResult::Status::SIZE_MISMATCH:
             enumField = env->GetStaticFieldID(statusEnum, "SIZE_MISMATCH", "Lcom/google/android/filament/utils/ImageDiff$Result$Status;");
            break;
        case ImageDiffResult::Status::PIXEL_DIFFERENCE:
             enumField = env->GetStaticFieldID(statusEnum, "PIXEL_DIFFERENCE", "Lcom/google/android/filament/utils/ImageDiff$Result$Status;");
            break;
    }
    statusObj = env->GetStaticObjectField(statusEnum, enumField);
    env->SetObjectField(resultObj, statusField, statusObj);

    env->SetLongField(resultObj, failingCountField, (jlong) result.failingPixelCount);

    jfloatArray maxDiffArray = env->NewFloatArray(4);
    env->SetFloatArrayRegion(maxDiffArray, 0, 4, result.maxDiffFound);
    env->SetObjectField(resultObj, maxDiffField, maxDiffArray);

    if (generateDiff && result.diffImage.getWidth() > 0) {
        jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmap = env->GetStaticMethodID(bitmapClass, "createBitmap", 
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        
        jclass configClass = env->FindClass("android/graphics/Bitmap$Config");
        jfieldID argb8888 = env->GetStaticFieldID(configClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
        jobject configObj = env->GetStaticObjectField(configClass, argb8888);

        uint32_t width = result.diffImage.getWidth();
        uint32_t height = result.diffImage.getHeight();
        jobject diffBitmap = env->CallStaticObjectMethod(bitmapClass, createBitmap, (jint)width, (jint)height, configObj);
        
        if (diffBitmap) {
            void* diffPixels;
            if (AndroidBitmap_lockPixels(env, diffBitmap, &diffPixels) == 0) {
                float const* src = result.diffImage.getPixelRef();
                uint8_t* dst = (uint8_t*) diffPixels;
                uint32_t channels = result.diffImage.getChannels(); // usually 4
                
                for (size_t i = 0; i < width * height; ++i) {
                    for (int c = 0; c < 4; ++c) {
                        float v = 0.0f;
                        if (c < channels) v = src[i * channels + c];
                        if (c == 3 && channels < 4) v = 1.0f; // Alpha 1.0 if missing
                        dst[i * 4 + c] = (uint8_t) std::min(255.0f, std::max(0.0f, v * 255.0f));
                    }
                }
                AndroidBitmap_unlockPixels(env, diffBitmap);
                env->SetObjectField(resultObj, diffImageField, diffBitmap);
            }
        }
    }
    
    return resultObj;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_google_android_filament_utils_ImageDiff_nCompareBasic(JNIEnv* env, jclass,
        jobject refBitmap, jobject candBitmap, jint mode, jint swizzle, jint channelMask,
        jfloat maxAbsDiff, jfloat maxFailingPixelsFraction, jobject maskBitmap) {

    BitmapLock refArg(env, refBitmap);
    BitmapLock candArg(env, candBitmap);
    BitmapLock maskArg(env, maskBitmap);

    if (!refArg.isValid() || !candArg.isValid()) {
         ImageDiffResult emptyResult; 
         emptyResult.status = ImageDiffResult::Status::SIZE_MISMATCH; // or ERROR
         return createResult(env, emptyResult, false);
    }

    ImageDiffConfig config;
    config.mode = (ImageDiffConfig::Mode) mode;
    config.swizzle = (ImageDiffConfig::Swizzle) swizzle;
    config.channelMask = (uint8_t) channelMask;
    config.maxAbsDiff = maxAbsDiff;
    config.maxFailingPixelsFraction = maxFailingPixelsFraction;

    imagediff::Bitmap const* maskPtr = nullptr;
    imagediff::Bitmap maskVal;
    if (maskBitmap && maskArg.isValid()) {
         maskVal = maskArg.toBitmap();
         maskPtr = &maskVal;
    }

    bool generateDiff = true;
    ImageDiffResult result = compare(refArg.toBitmap(), candArg.toBitmap(), config, maskPtr, generateDiff);

    return createResult(env, result, generateDiff);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_google_android_filament_utils_ImageDiff_nCompareJson(JNIEnv* env, jclass,
        jobject refBitmap, jobject candBitmap, jstring jsonConfig, jobject maskBitmap) {
        
    BitmapLock refArg(env, refBitmap);
    BitmapLock candArg(env, candBitmap);
    BitmapLock maskArg(env, maskBitmap);

    if (!refArg.isValid() || !candArg.isValid()) {
         ImageDiffResult emptyResult; 
         emptyResult.status = ImageDiffResult::Status::SIZE_MISMATCH; // or ERROR
         return createResult(env, emptyResult, false);
    }

    ImageDiffConfig config;
    const char* nativeJson = env->GetStringUTFChars(jsonConfig, 0);
    size_t length = env->GetStringUTFLength(jsonConfig);
    
    bool parsed = parseConfig(nativeJson, length, &config);
    env->ReleaseStringUTFChars(jsonConfig, nativeJson);

    if (!parsed) {
        // Fallback to default or error?
        // We could log error.
        utils::slog.e << "ImageDiff JNI: Failed to parse JSON config" << utils::io::endl;
        ImageDiffResult errResult;
        errResult.status = ImageDiffResult::Status::PIXEL_DIFFERENCE; // assume fail
        return createResult(env, errResult, false);
    }

    imagediff::Bitmap const* maskPtr = nullptr;
    imagediff::Bitmap maskVal;
    if (maskBitmap && maskArg.isValid()) {
         maskVal = maskArg.toBitmap();
         maskPtr = &maskVal;
    }

    bool generateDiff = true;
    ImageDiffResult result = compare(refArg.toBitmap(), candArg.toBitmap(), config, maskPtr, generateDiff);

    return createResult(env, result, generateDiff);
}

