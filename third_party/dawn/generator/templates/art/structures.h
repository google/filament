//* Copyright 2024 The Dawn & Tint Authors
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
#include <jni.h>
#include <webgpu/webgpu.h>

namespace dawn::kotlin_api {

class JNIContext;

struct UserData {
    jobject callback;
    jobject executor;
    JavaVM *jvm;

    ~UserData() {
        if (!jvm) return;
        JNIEnv *env = nullptr;
        jint envStat = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        bool didAttach = false;

        if (envStat == JNI_EDETACHED) {
            //* Deal with difference in signatures between Oracle's jni.h and Android's.
            #ifdef _JAVASOFT_JNI_H_  //* Oracle's jni.h violates the JNI spec.
                if (jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL) == JNI_OK) {
            #else
                if (jvm->AttachCurrentThread(&env, NULL) == JNI_OK) {
            #endif
                didAttach = true;
            }
        }

        if (env) {
            if (callback) env->DeleteGlobalRef(callback);
            if (executor) env->DeleteGlobalRef(executor);
        }

        if (didAttach) {
            jvm->DetachCurrentThread();
        }
    }
};

// Converts Kotlin objects representing Dawn structures into native structures that can be passed
// into the native Dawn API.
jobject ToKotlin(JNIEnv* env, const WGPUStringView* s);
void ToNative(JNIContext* c, JNIEnv* env, jstring obj, WGPUStringView* s);

{% for structure in by_category['structure'] if include_structure(structure) %}
    {% if structure.needs_n2k %}
        jobject ToKotlin(JNIEnv *env, const {{ as_cType(structure.name) }}* input);
    {% endif %}
    {% if structure.needs_k2n %}
        void ToNative(JNIContext* c, JNIEnv* env, jobject obj, {{ as_cType(structure.name) }}* converted);
    {% endif %}
{% endfor %}

}  // namespace dawn::kotlin_api
