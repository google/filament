//* Copyright 2025 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "JNIClasses.h"

namespace dawn::kotlin_api {
JNIClasses* JNIClasses::getInstance(JNIEnv* env) {
  static JNIClasses instance(env);
  return &instance;
}

JNIClasses::JNIClasses(JNIEnv* env) {
    //* Time of lookups measured on Pixel 6 at 1.7ms.
    {% for entity in has_kotlin_classes %}
        {{ entity.name.camelCase() }} = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("{{ jni_name(entity, entity.category) }}")));
        {%if entity.category == 'callback function' %}
            {{ entity.name.camelCase() }}Runnable = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("{{ jni_name(entity) }}Runnable")));
        {%endif %}
    {% endfor %}
    gpuCallbackRunnable = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("androidx/webgpu/GPURequestCallbackRunnable")));
    gpuCallbackErrorTypeRunnable = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("androidx/webgpu/GPURequestCallbackErrorTypeRunnable")));
    gpuCallbackVoidRunnable = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("androidx/webgpu/GPURequestCallbackVoidRunnable")));
    stringClass = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/String")));
    gpuHardwareBufferExternalTexture = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("androidx/webgpu/GPUHardwareBufferExternalTexture")));
    gpuHardwareBufferTexture = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("androidx/webgpu/GPUHardwareBufferTexture")));
    javaIllegalStateException = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/IllegalStateException")));
    javaUnsupportedOperationException = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/UnsupportedOperationException")));
    parcelFileDescriptor = reinterpret_cast<jclass>(env->NewGlobalRef(env->FindClass("android/os/ParcelFileDescriptor")));
    pfdGetFd = env->GetMethodID(parcelFileDescriptor, "getFd", "()I");
    pfdAdoptFd = env->GetStaticMethodID(parcelFileDescriptor, "adoptFd", "(I)Landroid/os/ParcelFileDescriptor;");
    jclass originClz = env->FindClass("androidx/webgpu/GPUOrigin2D");
    originX = env->GetFieldID(originClz, "x", "I");
    originY = env->GetFieldID(originClz, "y", "I");
    env->DeleteLocalRef(originClz);

    jclass extentClz = env->FindClass("androidx/webgpu/GPUExtent2D");
    extentWidth = env->GetFieldID(extentClz, "width", "I");
    extentHeight = env->GetFieldID(extentClz, "height", "I");
    env->DeleteLocalRef(extentClz);

    jclass colorSpaceClz = env->FindClass("androidx/webgpu/GPUColorSpaceDawn");
    colorSpacePrimaries = env->GetFieldID(colorSpaceClz, "primaries", "I");
    colorSpaceTransfer = env->GetFieldID(colorSpaceClz, "transfer", "I");
    colorSpaceYCbCrRange = env->GetFieldID(colorSpaceClz, "yCbCrRange", "I");
    colorSpaceYCbCrMatrix = env->GetFieldID(colorSpaceClz, "yCbCrMatrix", "I");
    env->DeleteLocalRef(colorSpaceClz);

}

}  // namespace dawn::kotlin_api
