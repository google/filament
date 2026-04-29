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

{% macro define_kotlin_to_struct_conversion(function_name, kotlin_name, struct_name, members,is_structure_converter=False) %}
    inline void {{function_name}}(JNIContext* c, const {{kotlin_name}}& inStruct, {{struct_name}}* outStruct) {
        JNIEnv* env = c->env;
        JNIClasses* classes = JNIClasses::getInstance(env);
        *outStruct = {};

        {% for member in kotlin_record_members(members) %}
            {
                {% if member.type.category == 'callback function' %}
                    if (inStruct.{{ as_varName(member.name) }})
                    {
                        auto& callbackInfo = outStruct->{{as_varName(member.name)}}Info;
                        callbackInfo = {};

                        //* Check if the member's corresponding callback info has a callbackMode.
                        {% set callbackInfoType = find_by_name(by_category["callback info"], member.type.name.get() ~ " info") %}
                        {% do assert(callbackInfoType, "callbackInfoType must exist but was not found") %}
                        {% if find_by_name(callbackInfoType.members, 'mode') %}
                            callbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
                        {% endif %}

                        //* The callback functions require each argument converting.
                        //* A custom native callback is generated to wrap the Kotlin callback.
                        callbackInfo.callback = [](
                            {%- for callbackArg in member.type.arguments %}
                                {{- as_annotated_cType(callbackArg) }}{{ ', ' if not loop.last }}
                            {%- endfor -%}
                            //* Callback functions do not specify user data params in dawn.json.
                            //* However, the C API always supplements two parameters with the names
                            //* below.
                            , void* userdata1, void* userdata2) {
                                {% set userdata = 'userdata1' %}
                            //* User data is used to carry the JNI context (env) for use by the
                            //* callback.
                            std::unique_ptr<UserData> userData1{static_cast<UserData *>({{ userdata }})};
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

                            {# Determine the type of Runnable we are building (Structure vs Generic)     #}

                            {%- set info = namespace(mode='standard', class='gpuCallbackRunnable', sig='', has_err=false) -%}
                            {%- set vars = namespace(status='0', msg=none, load=none, type=none) -%}

                            {# Detect ErrorType #}
                            {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                {%- if arg.type.name.get() == 'error type' %}
                                    {% set info.has_err = true %}
                                {% endif -%}
                            {%- endfor -%}

                            {# Determine Initial Mode/Signature #}
                            {%- if is_structure_converter -%}
                                {%- set info.mode = 'structure' -%}
                                {%- set info.class = member.type.name.camelCase() + 'Runnable' -%}
                                {%- set sb = namespace(v='(L' + jni_name(member.type) + ';') -%}
                                {%- for arg in kotlin_record_members(member.type.arguments) %}
                                    {% set sb.v = sb.v + jni_signature(arg) %}
                                {% endfor -%}
                                {%- set info.sig = sb.v + ')V' -%}
                            {%- elif info.has_err -%}
                                {%- set info.mode = 'error' -%}
                                {%- set info.class = 'gpuCallbackErrorTypeRunnable' -%}
                                {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;IILjava/lang/String;)V' -%}
                            {%- else -%}
                                {# Default to Standard, might verify to Void later #}
                                {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;ILjava/lang/String;Ljava/lang/Object;)V' -%}
                            {%- endif -%}

                            {# Generate the C++ variable conversions (e.g. _status, _message) #}
                            {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                {{ convert_to_kotlin(arg.name.camelCase(), '_' + arg.name.camelCase(), 'input->' + arg.length.name.camelCase() if arg.length.name, arg) }}
                                {%- set vName = '_' + arg.name.camelCase() -%}
                                {%- set aName = arg.name.get() -%}
                                {%- if aName == 'status' %}
                                    {% set vars.status = vName %}
                                {%- elif aName == 'message' %}
                                    {% set vars.msg = vName %}
                                {%- elif aName == 'type' %}
                                    {% set vars.type = vName %}
                                {%- else %}
                                    {% set vars.load = vName %}
                                {%- endif -%}
                            {%- endfor -%}


                            {# Handle missing strings for Generics #}
                            {%- if info.mode != 'structure' -%}
                                {%- if vars.msg is none -%}
                                    jstring _empty_msg = env->NewStringUTF("");
                                    {%- set vars.msg = '_empty_msg' -%}
                                {%- endif -%}
                                {# If Standard mode but no payload found -> Switch to Void #}
                                {%- if info.mode == 'standard' and vars.load is none -%}
                                    {%- set info.mode = 'void' -%}
                                    {%- set info.class = 'gpuCallbackVoidRunnable' -%}
                                    {%- set info.sig = '(Landroidx/webgpu/GPURequestCallback;ILjava/lang/String;)V' -%}
                                {%- endif -%}
                            {%- endif -%}


                            jclass executorClass = env->FindClass("java/util/concurrent/Executor");
                            jmethodID executeMethodID = env->GetMethodID(executorClass, "execute", "(Ljava/lang/Runnable;)V");
                            jmethodID runCtor = env->GetMethodID(classes->{{ info.class }}, "<init>", "{{ info.sig }}");

                            jobject runnable = env->NewObject(classes->{{ info.class }}, runCtor, userData1->callback
                            {%- if info.mode == 'structure' -%}
                                {%- for arg in kotlin_record_members(member.type.arguments) -%}
                                    {%- if member.type.category != 'kotlin type' %}
                                        , _{{ arg.name.camelCase() }}
                                    {% endif -%}
                                {%- endfor -%}
                            {%- elif info.mode == 'error' -%}
                                , {{ vars.status }}, {{ vars.type }}, {{ vars.msg }}
                                {%- elif info.mode == 'void' -%}
                                , {{ vars.status }}, {{ vars.msg }}
                            {%- else -%}
                                , {{ vars.status }}, {{ vars.msg }}, {{ vars.load }}
                            {%- endif -%}
                            );

                            env->CallVoidMethod(userData1->executor, executeMethodID, runnable);
                        };
                        //* The user data is owned by the callback and freed when it is called.
                        callbackInfo.{{ userdata }} = std::unique_ptr<UserData>(new UserData({
                          .callback = env->NewGlobalRef(inStruct.{{member.name.camelCase()}}),
                          .executor = env->NewGlobalRef(inStruct.{{as_varName(member.name)}}Executor),
                          .jvm = c->jvm
                        })).release();
                    }
                {% elif member.type.category != 'kotlin type' %}
                    auto& in = inStruct.{{member.name.camelCase()}};
                    auto& out = outStruct->{{member.name.camelCase()}};
                {% endif %}
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
                    {% if member.type.name.get() == 'uint32_t' or member.type.category in ['bitmask', 'enum'] %}
                        out = reinterpret_cast<const {{ as_cType(member.type.name) }}*>(c->GetIntArrayElements(in));
                        outLength = env->GetArrayLength(in);
                    {% elif member.type.name.get() == 'void' %}
                        out = env->GetDirectBufferAddress(in);
                        outLength = env->GetDirectBufferCapacity(in);
                    {% else %}
                        //* These container types are represented in Kotlin as arrays of objects.
                        outLength = env->GetArrayLength(in);
                        auto array = c->AllocArray<{{ as_cType(member.type.name) }}>(outLength);
                        out = array;

                        {% if member.type.category == 'object' %}
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
                {% elif member.type.category == 'callback function' %}
                    //* Do nothing for this case.
                {% elif member.type.category == 'structure' %}
                    //* Mandatory structure.
                    ToNative(c, env, in, &out);
                {% elif member.name.get() == "window" and member.type.name.get() == "void *" %}
                    //* HACK: Hardcode that ANativeWindow is a jlong instead of an actual pointer. Instead
                    //* of this, we should have manually written method that directly creates the
                    //* wgpu::Surface from the Java Surface.
                    out = reinterpret_cast<{{as_cType(member.type.name)}}>(static_cast<uintptr_t>(in));
                {% elif member.type.category in ["native", "enum", "bitmask"] %}
                    out = static_cast<{{as_cType(member.type.name)}}>(in);
                {% elif member.type.category != 'kotlin type' %}
                    {{ unreachable_code() }}
                {% endif %}
            }
        {% endfor -%}
    }
{% endmacro %}
