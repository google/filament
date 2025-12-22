/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filament/TextureSampler.h>

#include <utils/algorithm.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;

namespace filament::JniUtils {

jlong to_long(TextureSampler const& sampler) noexcept {
    return jlong(utils::bit_cast<uint32_t>(sampler.getSamplerParams()));
}

TextureSampler from_long(jlong params) noexcept {
    return TextureSampler{
            utils::bit_cast<backend::SamplerParams>(
                    static_cast<uint32_t>(params))};
}

} // namespace filament::JniUtils

using namespace JniUtils;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nCreateSampler(JNIEnv * env, jclass, jint min,
        jint max, jint s, jint t, jint r) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nCreateSampler", 0, [&]() -> jlong {
            TextureSampler sampler(static_cast<TextureSampler::MinFilter>(min),
                                   static_cast<TextureSampler::MagFilter>(max),
                                   static_cast<TextureSampler::WrapMode>(s),
                                   static_cast<TextureSampler::WrapMode>(t),
                                   static_cast<TextureSampler::WrapMode>(r));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nCreateCompareSampler(JNIEnv * env, jclass,
        jint mode, jint function) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nCreateCompareSampler", 0, [&]() -> jlong {
            TextureSampler sampler(static_cast<TextureSampler::CompareMode>(mode),
                                   static_cast<TextureSampler::CompareFunc>(function));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetMinFilter(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetMinFilter", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getMinFilter());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetMinFilter(JNIEnv * env, jclass, jlong sampler_, jint filter) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetMinFilter", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setMinFilter(static_cast<TextureSampler::MinFilter>(filter));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetMagFilter(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetMagFilter", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getMagFilter());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetMagFilter(JNIEnv * env, jclass, jlong sampler_, jint filter) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetMagFilter", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setMagFilter(static_cast<TextureSampler::MagFilter>(filter));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeS(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetWrapModeS", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getWrapModeS());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeS(JNIEnv * env, jclass, jlong sampler_, jint mode) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetWrapModeS", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setWrapModeS(static_cast<TextureSampler::WrapMode>(mode));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeT(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetWrapModeT", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getWrapModeT());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeT(JNIEnv * env, jclass, jlong sampler_, jint mode) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetWrapModeT", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setWrapModeT(static_cast<TextureSampler::WrapMode>(mode));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeR(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetWrapModeR", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getWrapModeR());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeR(JNIEnv * env, jclass, jlong sampler_, jint mode) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetWrapModeR", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setWrapModeR(static_cast<TextureSampler::WrapMode>(mode));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetCompareMode(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetCompareMode", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getCompareMode());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetCompareMode(JNIEnv * env, jclass, jlong sampler_, jint mode) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetCompareMode", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setCompareMode(static_cast<TextureSampler::CompareMode>(mode),
                    sampler.getCompareFunc());
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetCompareFunction(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_TextureSampler_nGetCompareFunction", 0, [&]() -> jint {
            return static_cast<jint>(from_long(sampler).getCompareFunc());
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetCompareFunction(JNIEnv * env, jclass, jlong sampler_, jint function) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetCompareFunction", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setCompareMode(sampler.getCompareMode(),
                    static_cast<TextureSampler::CompareFunc>(function));
            return to_long(sampler);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_TextureSampler_nGetAnisotropy(JNIEnv * env, jclass, jlong sampler) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_TextureSampler_nGetAnisotropy", 0.0f, [&]() -> jfloat {
            return from_long(sampler).getAnisotropy();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_TextureSampler_nSetAnisotropy(JNIEnv * env, jclass, jlong sampler_, jfloat anisotropy) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_TextureSampler_nSetAnisotropy", 0, [&]() -> jlong {
            TextureSampler sampler{from_long(sampler_)};
            sampler.setAnisotropy(anisotropy);
            return to_long(sampler);
    });
}
