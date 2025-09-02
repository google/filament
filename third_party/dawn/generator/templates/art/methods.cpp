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
{% from 'art/kotlin_record_conversion.cpp' import define_kotlin_record_structure, define_kotlin_to_struct_conversion with context %}
{% from 'art/api_jni_types.cpp' import arg_to_jni_type, convert_to_kotlin, jni_signature, to_jni_type with context %}
#include <jni.h>
#include <stdlib.h>
#include <webgpu/webgpu.h>

#include "JNIClasses.h"
#include "JNIContext.h"
#include "structures.h"

// Converts every method call from the Kotlin-hosted version to the native version, performing
// the necessary JNI calls to fetch data, and converting the data to the correct form.

#define DEFAULT __attribute__((visibility("default")))

namespace dawn::kotlin_api {

jobject toByteBuffer(JNIEnv *env, const void* address, jlong size) {
    if (!address) {
      //* TODO(b/344805524): custom exception for Dawn.
      env->ThrowNew(env->FindClass("java/lang/Error"), "Invalid byte buffer.");
      return nullptr;
    }
    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");

    //* Dawn always uses little endian format, so we pre-convert for the client's convenience.
    jclass byteOrderClass = env->FindClass("java/nio/ByteOrder");
    jobject littleEndian = env->NewGlobalRef(env->GetStaticObjectField(
            byteOrderClass, env->GetStaticFieldID(byteOrderClass, "LITTLE_ENDIAN",
                                                  "Ljava/nio/ByteOrder;")));

    jobject byteBuffer = env->NewDirectByteBuffer(const_cast<void *>(address), size);

    env->CallObjectMethod(
            byteBuffer, env->GetMethodID(byteBufferClass, "order",
                                         "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;"),
            littleEndian);
    return byteBuffer;
}

{% macro render_method(method, object) %}
    {% set ObjectName = object.name.CamelCase() if object else "FunctionsKt" %}
    {% set FunctionSuffix = ObjectName + "_" +  method.name.camelCase() %}
    {% set KotlinRecord = FunctionSuffix + "KotlinRecord" %}
    {% set ArgsStruct = FunctionSuffix + "ArgsStruct" %}

    //* Define the helper structs to perform most of the conversion.
    struct {{KotlinRecord}} {
        {% for arg in kotlin_record_members(method.arguments) %}
            {{ arg_to_jni_type(arg) }} {{ as_varName(arg.name) }};
        {% endfor %}
    };
    struct {{ArgsStruct}} {
        {% for arg in method.arguments %}
            {{ as_annotated_cType(arg) }};
        {% endfor %}
    };
    {{ define_kotlin_to_struct_conversion("ConvertInternal", KotlinRecord, ArgsStruct, method.arguments)}}

    {% set _kotlin_return = kotlin_return(method) %}
    //*  A JNI-external method is built with the JNI signature expected to match the host Kotlin.
    DEFAULT extern "C"
    {{ arg_to_jni_type(_kotlin_return) }}
    Java_{{ kotlin_package.replace('.', '_') }}_{{ FunctionSuffix }}
            (JNIEnv *env{{ ', jobject obj' if object else ', jclass clazz' -}}

    //* Make the signature for each argument in turn.
    {% for arg in kotlin_record_members(method.arguments) %},
        {{ arg_to_jni_type(arg) }} _{{ as_varName(arg.name) }}
    {% endfor %}) {

    // * Helper context for the duration of this method call.
    JNIClasses* classes = JNIClasses::getInstance(env);
    JNIContext c(env);

    //* Perform the conversion of arguments.
    {{KotlinRecord}} kotlinRecord;
    {% for arg in kotlin_record_members(method.arguments) %}
        kotlinRecord.{{ as_varName(arg.name) }} = _{{ as_varName(arg.name) }};
    {% endfor %}
    {{ArgsStruct}} args;
    ConvertInternal(&c, kotlinRecord, &args);

    {% if object %}
        jclass memberClass = classes->{{ object.name.camelCase() }};
        jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
        auto handle =
                reinterpret_cast<{{ as_cType(object.name) }}>(env->CallLongMethod(obj, getHandle));
    {% endif %}

    //* Actually invoke the native version of the method.
    {% if _kotlin_return and _kotlin_return.length == 'size_t' %}
        //* Methods that return containers are converted from two-call to single call, and the
        //* return type is switched to a Kotlin container.
        size_t size = wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%},
                //* The replaced output parameter is set to nullptr on the first call.
                {{ 'nullptr' if arg.annotation == '*' else "args." + as_varName(arg.name) -}}
            {% endfor %}
        );
        //* Allocate the native container
        args.{{ as_varName(_kotlin_return.name) }} = c.AllocArray<{{ as_cType(_kotlin_return.type.name) }}>(size);
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
        //* Second call completes the native container
        wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
            {% for arg in method.arguments -%}
                , {{- "args." + as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return nullptr;
        }
    {% else %}
        {% if _kotlin_return and _kotlin_return.annotation == '*' %}
            //* Make a native container to accept the data output via parameter.
            {{ as_cType(_kotlin_return.type.name) }} out = {};
            args.{{ as_varName(_kotlin_return.name) }} = &out;
        {% endif %}
        {{ 'auto result =' if method.returns }}
        {% if object %}
            wgpu{{ object.name.CamelCase() }}{{ method.name.CamelCase() }}(handle
        {% else %}
            wgpu{{ method.name.CamelCase() }}(
        {% endif %}
            {% for arg in method.arguments -%}
                {{- ',' if object or not loop.first }}args.{{ as_varName(arg.name) -}}
            {% endfor %}
        );
        if (env->ExceptionCheck()) {  //* Early out if client (Kotlin) callback threw an exception.
            return{{ ' 0' if _kotlin_return }};
        }
        {% if method.returns and method.returns.type.name.canonical_case() == 'status' %}
            if (result != WGPUStatus_Success) {
                //* TODO(b/344805524): custom exception for Dawn.
                env->ThrowNew(env->FindClass("java/lang/Error"), "Method failed");
                return{{ ' 0' if method.returns }};
            }
        {% endif %}
    {% endif %}
    {% if _kotlin_return %}
        {% if _kotlin_return.type.name.get() in ['void const *', 'void *'] %}
            size_t size = args.size;
        {% endif %}
        {{ convert_to_kotlin("args." + as_varName(_kotlin_return.name) if _kotlin_return.annotation == '*' else 'result',
                             'result_kt',
                             'size' if _kotlin_return.type.name.get() in ['void const *', 'void *'] or _kotlin_return.length == 'size_t',
                             _kotlin_return) }}
        return result_kt;
    {% endif %}
} {% endmacro %}

{% for obj in by_category['object'] %}
    {% for method in obj.methods if include_method(method) %}
        {{ render_method(method, obj) }}
    {% endfor %}

    //* Every object gets a Release method, to supply a Kotlin AutoCloseable.
    extern "C"
    JNIEXPORT void JNICALL
    Java_{{ kotlin_package.replace('.', '_') }}_{{ obj.name.CamelCase() }}_close(
            JNIEnv *env, jobject obj) {
        JNIClasses* classes = JNIClasses::getInstance(env);
        jclass clz = classes->{{ obj.name.camelCase() }};
        const {{ as_cType(obj.name) }} handle = reinterpret_cast<{{ as_cType(obj.name) }}>(
                env->CallLongMethod(obj, env->GetMethodID(clz, "getHandle", "()J")));
        wgpu{{ obj.name.CamelCase() }}Release(handle);
    }
{% endfor %}

//* Global functions don't have an associated class.
{% for function in by_category['function'] if include_method(function) %}
    {{ render_method(function, None) }}
{% endfor %}

}  // namespace dawn::kotlin_api
