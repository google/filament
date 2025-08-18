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
{% from 'art/api_jni_types.cpp' import arg_to_jni_type, convert_to_kotlin, jni_signature with context %}

{% macro define_kotlin_record_structure(struct_name, members) %}
    struct {{struct_name}} {
        {% for member in kotlin_record_members(members) %}
            //* HACK: Hardcode that ANativeWindow is a jlong instead of an actual pointer. Instead
            //* of this, we should have manually written method that directly creates the
            //* wgpu::Surface from the Java Surface.
            {% if member.name.get() == "window" and member.type.name.get() == "void *"%}
                jlong {{ as_varName(member.name) }};
            {% else %}
                {{ arg_to_jni_type(member) }} {{ as_varName(member.name) }};
            {% endif %}
        {% endfor%}
    };
{% endmacro %}

{% macro define_kotlin_to_struct_conversion(function_name, kotlin_name, struct_name, members) %}
    inline void {{function_name}}(JNIContext* c, const {{kotlin_name}}& inStruct, {{struct_name}}* outStruct) {
        JNIEnv* env = c->env;
        JNIClasses* classes = JNIClasses::getInstance(env);
        *outStruct = {};

        {% for member in kotlin_record_members(members) %}
            {
                auto& in = inStruct.{{member.name.camelCase()}};
                auto& out = outStruct->{{member.name.camelCase()}};
                {% if member.constant_length == 1 %}
                    {% if member.type.category == 'structure' %}
                        if (in != nullptr) {
                            auto converted = c->Alloc<{{as_cType(member.type.name)}}>();
                            ToNative(c, env, in, converted);
                            out = converted;
                        }
                    {% elif member.type.name.get() == 'void' %}
                        out = reinterpret_cast<void*>(in);
                    {% else %}
                        {{ unreachable_code() }}
                    {% endif %}
                {% elif member.length %}
                    auto& outLength = outStruct->{{member.length.name.camelCase()}};
                    {% if member.constant_length %}
                        {{ unreachable_code() }}
                    {% endif %}
                    //* Convert container, including the length field.
                    {% if member.type.name.get() == 'uint32_t' %}
                        //* This container type is represented in Kotlin as a primitive array.
                        out = reinterpret_cast<const uint32_t*>(c->GetIntArrayElements(in));
                        outLength = env->GetArrayLength(in);
                    {% elif member.type.name.get() == 'void' %}
                        out = env->GetDirectBufferAddress(in);
                        outLength = env->GetDirectBufferCapacity(in);
                    {% else %}
                        //* These container types are represented in Kotlin as arrays of objects.
                        outLength = env->GetArrayLength(in);
                        auto array = c->AllocArray<{{ as_cType(member.type.name) }}>(outLength);
                        out = array;

                        {% if member.type.category in ['bitmask', 'enum'] %}
                            jclass memberClass = classes->{{ member.type.name.camelCase() }};
                            jmethodID getValue = env->GetMethodID(memberClass, "getValue", "()I");
                            for (int idx = 0; idx != outLength; idx++) {
                                jobject element = env->GetObjectArrayElement(in, idx);
                                array[idx] = static_cast<{{ as_cType(member.type.name) }}>(
                                        env->CallIntMethod(element, getValue));
                            }
                        {% elif member.type.category == 'object' %}
                            jclass memberClass = classes->{{ member.type.name.camelCase() }};
                            jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                            for (int idx = 0; idx != outLength; idx++) {
                                jobject element = env->GetObjectArrayElement(in, idx);
                                array[idx] = reinterpret_cast<{{ as_cType(member.type.name) }}>(
                                        env->CallLongMethod(element, getHandle));
                            }
                        {% elif member.type.category == 'structure' %}
                            for (int idx = 0; idx != outLength; idx++) {
                                ToNative(c, env, env->GetObjectArrayElement(in, idx), &array[idx]);
                            }
                        {% else %}
                            {{ unreachable_code() }}
                        {% endif %}
                    {% endif %}
                //* From here members are single values.
                {% elif member.type.category == 'object' %}
                    if (in != nullptr) {
                        jclass memberClass = classes->{{ member.type.name.camelCase() }};
                        jmethodID getHandle = env->GetMethodID(memberClass, "getHandle", "()J");
                        out = reinterpret_cast<{{as_cType(member.type.name)}}>(
                                env->CallLongMethod(in, getHandle));
                    } else {
                        out = nullptr;
                    }
                {% elif member.type.category in ['callback info', 'structure'] %}
                    //* Mandatory structure.
                    ToNative(c, env, in, &out);
                {% elif member.name.get() == "window" and member.type.name.get() == "void *" %}
                    //* HACK: Hardcode that ANativeWindow is a jlong instead of an actual pointer. Instead
                    //* of this, we should have manually written method that directly creates the
                    //* wgpu::Surface from the Java Surface.
                    out = reinterpret_cast<{{as_cType(member.type.name)}}>(static_cast<uintptr_t>(in));
                {% elif member.type.category in ["native", "enum", "bitmask"] %}
                    out = static_cast<{{as_cType(member.type.name)}}>(in);
                {% elif member.type.category in ['callback function', 'function pointer'] %}
                    //* Function pointers and callback functions require each argument converting.
                    //* A custom native callback is generated to wrap the Kotlin callback.
                    out = [](
                        {%- for callbackArg in member.type.arguments %}
                            {{- as_annotated_cType(callbackArg) }}{{ ', ' if not loop.last }}
                        {%- endfor -%}
                        {%- if member.type.category == 'function pointer' -%}
                            //* We rely on the function pointer definitions (dawn.json) always
                            //* including a parameter named 'userdata' as the final parameter.
                            {%- set userdata = 'userdata' -%}
                        {%- else %}
                            //* Callback functions do not specify user data params in dawn.json.
                            //* However, the C API always supplements two parameters with the names
                            //* below.
                            , void* userdata1, void* userdata2
                            {%- set userdata = 'userdata1' -%}
                        {%- endif %}) {
                        //* User data is used to carry the JNI context (env) for use by the
                        //* callback.
                        UserData* userData1 = static_cast<UserData *>({{ userdata }});
                        JNIEnv *env = NULL;
                        JavaVM* jvm = userData1->jvm;
                        //* Deal with difference in signatures between Oracle's jni.h and Android's.
                        #ifdef _JAVASOFT_JNI_H_  //* Oracle's jni.h violates the JNI spec.
                            jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), NULL);
                        #else
                            jvm->AttachCurrentThread(&env, NULL);
                        #endif

                        if (env->ExceptionCheck()) {
                            return;
                        }
                        JNIClasses* classes = JNIClasses::getInstance(env);

                        {% for callbackArg in kotlin_record_members(member.type.arguments) -%}
                            {{ convert_to_kotlin(callbackArg.name.camelCase(),
                                                 '_' + callbackArg.name.camelCase(),
                                                 'input->' + callbackArg.length.name.camelCase() if callbackArg.length.name,
                                                 callbackArg) }}
                        {% endfor %}

                        //* Get the client (Kotlin) callback so we can call it.
                        jmethodID callbackMethod = env->GetMethodID(
                                classes->{{ member.type.name.camelCase() }}, "callback", "(
                            {%- for callbackArg in kotlin_record_members(member.type.arguments) -%}
                                {{- jni_signature(callbackArg) -}}
                            {%- endfor %})V");

                        //* Call the callback with all converted parameters.
                        env->CallVoidMethod(userData1->callback, callbackMethod
                        {%- for callbackArg in kotlin_record_members(member.type.arguments) %}
                             {{- ', ' }}_{{ callbackArg.name.camelCase() }}
                        {%- endfor %});
                    };
                    //* TODO(b/330293719): free associated resources.
                    outStruct->{{ userdata }} = new UserData(
                            {.callback = env->NewGlobalRef(in), .jvm = c->jvm});

                {% else %}
                    {{ unreachable_code() }}
                {% endif %}
            }
        {% endfor -%}
    }
{% endmacro %}
