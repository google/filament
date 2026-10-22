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
    {%- if arg == None -%}
        void
    {%- elif arg.length and (arg.length != 'constant' or arg.constant_length != 1) -%}
        {%- if arg.type.category in ['callback function', 'function pointer', 'kotlin type', 'object', 'structure'] -%}
            jobjectArray
        {%- elif arg.type.name.get() == 'void' -%}
            jobject
        {%- elif arg.type.category in ['bitmask', 'enum'] or arg.type.name.get() in ['int', 'int32_t', 'uint32_t'] -%}
            jintArray
        {%- elif arg.type.name.get() == 'float' -%}
            jfloatArray
        {%- elif arg.type.name.get() == 'char' -%}
            jobjectArray
        {%- else -%}
            {{ unreachable_code('Unsupported array type in arg_to_jni_type: ' ~ arg.type.name.get()) }}
        {%- endif -%}
    {%- else -%}
        {{ to_jni_type(arg.type) }}
    {%- endif -%}
{% endmacro %}

{% macro to_jni_type(type) %}
    {%- if type.name.get() == "string view" -%}
        jstring
    {%- elif type.category in ['callback function', 'function pointer', 'kotlin type', 'object', 'structure'] -%}
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
    {%- elif member.length and (member.length != 'constant' or member.constant_length != 1) -%}
        [{{ jni_signature_single_value(member.type) }}
    {%- else -%}
        {{ jni_signature_single_value(member.type) }}
    {%- endif %}
{% endmacro %}

{% macro jni_signature_single_value(type) %}
    {%- if type.category in ['callback function', 'function pointer', 'kotlin type', 'object', 'structure'] -%}
        L{{ jni_name(type, type.category) }};
    {%- elif type.category in ['bitmask', 'enum'] -%}
        {{ jni_signatures['int32_t'] }}
    {%- elif type.category == 'native' -%}
        {%- if type.name.get() == 'char' -%}
            Ljava/lang/String;
        {%- else -%}
            {{ jni_signatures[type.name.get()] }}
        {%- endif -%}
    {%- else -%}
        {{ unreachable_code('Unsupported type: ' + type.name.get()) }}
    {%- endif -%}
{% endmacro %}

{% macro convert_to_kotlin(input, output, size, member) %}
    {% if size %}
        {% if member.type.name.get() in ['void const *', 'void *'] %}
            jobject {{ output }} = toByteBuffer(env, {{ input }}, {{ size }});
        {% elif member.type.category in ['object', 'structure'] %}
            //* Native container converted to a Kotlin container.
            jobjectArray {{ output }} = env->NewObjectArray(
                    {{ size }}, classes->{{ member.type.name.camelCase() }}, 0);
            for (int idx = 0; idx != {{ size }}; idx++) {
                {{ convert_to_kotlin(input + '[idx]', 'element', None, {'type': member.type})  | indent(4) }}
                env->SetObjectArrayElement({{ output }}, idx, element);
            }
        {% elif member.type.category in ['bitmask', 'enum'] or member.type.name.get() in ['int', 'int32_t', 'uint32_t'] %}
            jintArray {{ output }} = env->NewIntArray({{ size }});
            {{'    '}}env->SetIntArrayRegion({{ output }}, 0, {{ size }}, reinterpret_cast<const jint *>({{ input }}));
        {% elif member.type.name.get() == 'float' %}
            jfloatArray {{ output }} = env->NewFloatArray({{ size }});
            {{'    '}}env->SetFloatArrayRegion({{ output }}, 0, {{ size }}, reinterpret_cast<const jfloat *>({{ input }}));
        {% elif member.type.name.get() == 'char' and member.annotation == 'const*const*' %}
            jobjectArray {{ output }} = env->NewObjectArray({{ size }}, classes->stringClass, nullptr);
            for (int idx = 0; idx != {{ size }}; idx++) {
                jstring element = env->NewStringUTF({{ input }}[idx]);
                env->SetObjectArrayElement({{ output }}, idx, element);
                env->DeleteLocalRef(element);
            }
        {% else %}
            {{ unreachable_code('Unsupported array type in convert_to_kotlin: ' ~ member.type.name.get()) }}
        {% endif %}
    {% elif member.type.category == 'object' %}
        jobject {{ output }};
        {
            jclass clz = classes->{{ member.type.name.camelCase() }};
            jmethodID init = env->GetMethodID(clz, "<init>", "(J)V");
            {{ output }} = env->NewObject(clz, init, reinterpret_cast<jlong>({{ input }}));
        }
    {% elif member.type.category == 'structure' %}
        jobject {{ output }} = ToKotlin(env, {{ '&' if member.annotation not in ['*', 'const*'] }}{{ input }});
    {% elif member.type.category in ['bitmask', 'enum', 'native'] %}
        //* We use Kotlin value classes for bitmask and enum, and they get inlined as lone values.
        {{ to_jni_type(member.type) }} {{ output }} = static_cast<{{ to_jni_type(member.type) }}>({{ input }});
    {% elif member.type.category != 'kotlin type' %}
        {{ unreachable_code('Unsupported type: ' + member.type.name.get()) }}
    {% endif %}
{% endmacro %}
