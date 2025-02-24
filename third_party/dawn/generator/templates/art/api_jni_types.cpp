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

{% macro arg_to_jni_type(arg) %}
    {%- if arg.length and arg.length != 'constant' -%}
        {%- if arg.type.category in ['bitmask', 'callback function', 'enum', 'function pointer', 'object', 'callback info', 'structure'] -%}
            jobjectArray
        {%- elif arg.type.name.get() == 'void' -%}
            jobject
        {%- elif arg.type.name.get() == 'uint32_t' -%}
            jintArray
        {%- else -%}
            {{ unreachable_code() }}
        {%- endif -%}
    {%- else -%}
        {{ to_jni_type(arg.type) }}
    {%- endif -%}
{% endmacro %}

{% macro to_jni_type(type) %}
    {%- if type.name.get() == "string view" -%}
        jstring
    {%- elif type.category in ['callback function', 'function pointer', 'object', 'callback info', 'structure'] -%}
        jobject
    {%- elif type.category in ['bitmask', 'enum'] -%}
        jint
    {%- else -%}
        {{ jni_primitives[type.name.get()] }}
    {%- endif -%}
{% endmacro %}

{% macro jni_signature(member) %}
    {%- if member.type.name.get() == 'string view' -%}
        Ljava/lang/String;
    {%- elif member.length and member.length != 'constant' -%}
        {%- if member.type.category in ['bitmask', 'enum'] -%}
            //*  JvmInline does not inline bitmask/enums in arrays.
            [L{{ jni_name(member.type) }};
        {%- else -%}
            [{{ jni_signature_single_value(member.type) }}
        {%- endif -%}
    {%- else -%}
        {{ jni_signature_single_value(member.type) }}
    {%- endif %}
{% endmacro %}

{% macro jni_signature_single_value(type) %}
    {%- if type.category in ['function pointer', 'object', 'callback function', 'callback info', 'structure'] -%}
        L{{ jni_name(type) }};
    {%- elif type.category in ['bitmask', 'enum'] -%}
        {{ jni_signatures['int32_t'] }}//*  JvmInline makes lone bitmask/enums appear as integer to JNI.
    {%- elif type.category == 'native' -%}
        {{ jni_signatures[type.name.get()] }}
    {%- else -%}
        {{ unreachable_code('Unsupported type: ' + type.name.get()) }}
    {%- endif -%}
{% endmacro %}

{% macro convert_array_element_to_kotlin(input, output, size, member) %}
    {%- if member.type.category in ['bitmask', 'enum'] -%}
        //* Kotlin value classes do not get inlined in arrays, so the creation method is different.
        jclass clz = env->FindClass("{{ jni_name(member.type) }}");
        jmethodID init = env->GetMethodID(clz, "<init>", "(I)V");
        jobject {{ output }} = env->NewObject(clz, init, static_cast<jint>({{ input }}));
    {%- else -%}
        //* Declares an instance of 'output'
        {{ convert_to_kotlin(input, output, size, member) }}
    {%- endif -%}
{% endmacro %}

{% macro convert_to_kotlin(input, output, size, member) %}
    {% if size is string %}
        {% if member.type.name.get() in ['void const *', 'void *'] %}
            jobject {{ output }} = toByteBuffer(env, {{ input }}, {{ size }});
        {% elif member.type.category in ['bitmask', 'enum', 'object', 'callback info', 'structure'] %}
            //* Native container converted to a Kotlin container.
            jobjectArray {{ output }} = env->NewObjectArray(
                    {{ size }},
                    env->FindClass("{{ jni_name(member.type) }}"), 0);
            for (int idx = 0; idx != {{ size }}; idx++) {
                {{ convert_array_element_to_kotlin(input + '[idx]', 'element', None, {'type': member.type})  | indent(4) }}
                env->SetObjectArrayElement({{ output }}, idx, element);
            }
        {% elif member.type.name.get() in ['int', 'int32_t', 'uint32_t'] %}
            jintArray {{ output }} = env->NewIntArray({{ size }});
            {{'    '}}env->SetIntArrayRegion({{ output }}, 0, {{ size }}, reinterpret_cast<const jint *>({{ input }}));
        {% else %}
            {{ unreachable_code() }}
        {% endif %}
    {% elif member.type.category == 'object' %}
        jobject {{ output }};
        {
            jclass clz = env->FindClass("{{ jni_name(member.type) }}");
            jmethodID init = env->GetMethodID(clz, "<init>", "(J)V");
            {{ output }} = env->NewObject(clz, init, reinterpret_cast<jlong>({{ input }}));
        }
    {% elif member.type.category in ['callback info', 'structure'] %}
        jobject {{ output }} = ToKotlin(env, {{ '&' if member.annotation not in ['*', 'const*'] }}{{ input }});
    {% elif member.type.name.get() == 'void *' %}
        jlong {{ output }} = reinterpret_cast<jlong>({{ input }});
    {% elif member.type.category in ['bitmask', 'enum', 'native'] %}
        //* We use Kotlin value classes for bitmask and enum, and they get inlined as lone values.
        {{ to_jni_type(member.type) }} {{ output }} = static_cast<{{ to_jni_type(member.type) }}>({{ input }});
    {% elif member.type.category in ['callback function', 'function pointer'] %}
        jobject {{ output }} = nullptr;
        dawn::WarningLog() << "while converting {{ as_cType(member.type.name) }}: Native callbacks cannot be converted to Kotlin";
    {% else %}
        {{ unreachable_code() }}
    {% endif %}
{% endmacro %}
