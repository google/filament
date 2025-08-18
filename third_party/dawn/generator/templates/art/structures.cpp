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
{% from 'art/api_jni_types.cpp' import convert_to_kotlin, jni_signature with context %}
{% from 'art/kotlin_record_conversion.cpp' import define_kotlin_record_structure, define_kotlin_to_struct_conversion with context %}
#include "structures.h"

#include <cassert>
#include <string>

#include <jni.h>
#include <webgpu/webgpu.h>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "JNIClasses.h"
#include "JNIContext.h"

// Converts Kotlin objects representing Dawn structures into native structures that can be passed
// into the native Dawn API.

namespace dawn::kotlin_api {

// Helper functions to call the correct JNIEnv::Call*Method depending on what return type we expect.
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jboolean* result) {
    *result = env->CallBooleanMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jbyte* result) {
    *result = env->CallByteMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jchar* result) {
    *result = env->CallCharMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jshort* result) {
    *result = env->CallShortMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jint* result) {
    *result = env->CallIntMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jlong* result) {
    *result = env->CallLongMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jfloat* result) {
    *result = env->CallFloatMethod(obj, getter);
}
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, jdouble* result) {
    *result = env->CallDoubleMethod(obj, getter);
}
template <typename T>
void CallGetter(JNIEnv* env, jmethodID getter, jobject obj, T** result) {
    *result = reinterpret_cast<T*>(env->CallObjectMethod(obj, getter));
}

// Special-case [Nullable]StringView
void ToNative(JNIContext* c, JNIEnv* env, jstring obj, WGPUStringView* s) {
    if (obj == nullptr) {
        *s = {nullptr, WGPU_STRLEN};
        return;
    }
    *s = {c->GetStringUTFChars(obj), static_cast<size_t>(env->GetStringUTFLength(obj))};
}

jobject ToKotlin(JNIEnv* env, const WGPUStringView* s) {
    if (s->length == WGPU_STRLEN) {
        if (s->data == nullptr) {
            return nullptr;
        }
        return env->NewStringUTF(s->data);
    }
    std::string nullTerminated(s->data, s->length);
    return env->NewStringUTF(nullTerminated.c_str());
}

{%- for structure in by_category['structure'] + by_category['callback info'] if include_structure(structure) %}

    //* Native -> Kotlin converter.
    //* TODO(b/354411474): Filter the structures for which to add a ToKotlin conversion.
    jobject ToKotlin(JNIEnv *env, const {{ as_cType(structure.name) }}* input) {
        if (!input) {
            return nullptr;
        }
        JNIClasses* classes = JNIClasses::getInstance(env);
        //* Make a new Kotlin object to receive a copy of the structure.
        jclass clz = classes->{{ structure.name.camelCase() }};
        //* JNI signature needs to be built using the same logic used in the Kotlin structure spec.
        jmethodID ctor = env->GetMethodID(clz, "<init>", "(
        {%- for member in kotlin_record_members(structure.members) %}
            {{- jni_signature(member) -}}
        {%- endfor -%}
        {%- for structure in chain_children[structure.name.get()] -%}
            {{- jni_signature({'type': structure}) -}}
        {%- endfor %})V");
        //* Each field converted using the individual value converter.
        {% for member in kotlin_record_members(structure.members) %}
            {{ convert_to_kotlin('input->' + member.name.camelCase(), member.name.camelCase(),
                                 'input->' + member.length.name.camelCase() if member.length.name,
                                 member) | indent(4) -}}
        {% endfor %}
        //* Allow conversion of every child structure.
        {%- for structure in chain_children[structure.name.get()] %}
            jobject {{ structure.name.camelCase() }} = nullptr;
        {% endfor %}
        //* Walk the chain to find and convert (recursively) all child structures.
        {%- if chain_children[structure.name.get()] %}
            for (const WGPUChainedStruct* child = input->nextInChain;
                    child != nullptr; child = child->next) {
                switch (child->sType) {
                    {% for structure in chain_children[structure.name.get()] %}
                        case WGPUSType_{{ structure.name.CamelCase() }}:
                            {{ structure.name.camelCase() }} = ToKotlin(env,
                                    reinterpret_cast<const {{ as_cType(structure.name) }}*>(child));
                            break;
                    {% endfor %}
                        default:
                            DAWN_UNREACHABLE();
                }
            }
        {% endif %}

        //* Now all the fields are converted, invoke the constructor.
        jobject converted = env->NewObject(
            clz,
            ctor
        {%- for member in kotlin_record_members(structure.members) %},
                {{ member.name.camelCase() -}}
        {%- endfor -%}
        {%- for structure in chain_children[structure.name.get()] -%},
                {{ structure.name.camelCase() -}}
        {%- endfor -%}
        );
        return converted;
    }

    {% set Struct = as_cType(structure.name) %}
    {% set KotlinRecord = "KotlinRecord" + structure.name.CamelCase() %}
    {{ define_kotlin_record_structure(KotlinRecord, structure.members)}}
    {{ define_kotlin_to_struct_conversion("ConvertInternal", KotlinRecord, Struct, structure.members)}}
    void ToNative(JNIContext* c, JNIEnv* env, jobject obj, {{ as_cType(structure.name) }}* converted) {
        JNIClasses* classes = JNIClasses::getInstance(env);
        jclass clz = classes->{{ structure.name.camelCase() }};

        //* Use getters to fill in the Kotlin record that will get converted to our struct.
        {{KotlinRecord}} kotlinRecord;
        {% for member in kotlin_record_members(structure.members) %}
            {
                jmethodID getter = env->GetMethodID(clz, "get{{member.name.CamelCase()}}", "(){{jni_signature(member)}}");
                CallGetter(env, getter, obj, &kotlinRecord.{{as_varName(member.name)}});
            }
        {% endfor %}

        //* Fill all struct members from the Kotlin record.
        ConvertInternal(c, kotlinRecord, converted);
        //* Set up the chain type and links for child objects.
        {% if structure.chained %}
            converted->chain = {.sType = WGPUSType_{{ structure.name.CamelCase() }}};
        {% endif %}
        {% for child in chain_children[structure.name.get()] %}
            {
                jobject child = env->CallObjectMethod(obj,
                        env->GetMethodID(clz, "get{{ child.name.CamelCase() }}", "()L{{ jni_name(child) }};"));
                if (child) {
                    auto out = c->Alloc<{{ as_cType(child.name) }}>();
                    ToNative(c, env, child, out);
                    out->chain.next = converted->nextInChain;
                    converted->nextInChain = &out->chain;
                }
            }
        {% endfor %}
    }
{% endfor %}

}  // namespace dawn::kotlin_api
