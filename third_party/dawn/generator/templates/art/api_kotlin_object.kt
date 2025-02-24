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
package {{ kotlin_package }}

import dalvik.annotation.optimization.FastNative
import java.nio.ByteBuffer

{% from 'art/api_kotlin_types.kt' import kotlin_declaration, kotlin_definition with context %}

public class {{ obj.name.CamelCase() }}(public val handle: Long): AutoCloseable {
    {% for method in obj.methods if include_method(method) %}
        @FastNative
        @JvmName("{{ method.name.camelCase() }}")
        public external fun {{ method.name.camelCase() }}(
        //* TODO(b/341923892): rework async methods to use futures.
        {%- for arg in kotlin_record_members(method.arguments) %}
            {{- as_varName(arg.name) }}: {{ kotlin_definition(arg) }},{{ ' ' }}
        {%- endfor -%}): {{ kotlin_declaration(kotlin_return(method)) }}

        {% if method.name.chunks[0] == 'get' and not method.arguments %}
            //* For the Kotlin getter, strip word 'get' from name and convert the remainder to
            //* camelCase() (lower case first word). E.g. "get foo bar" translated to fooBar.
            {% set name = method.name.chunks[1] + method.name.chunks[2:] | map('title') | join %}
            @get:JvmName("{{ name }}")
            public val {{ name }}: {{ kotlin_declaration(kotlin_return(method)) }} get() = {{ method.name.camelCase() }}()

        {% endif %}
    {% endfor %}
    external override fun close();

    //* By default, the equals() function implements referential equality.
    //* see: https://kotlinlang.org/docs/equality.html#structural-equality
    //* A structural comparison of the wrapper object is equivalent to a referential comparison of
    //* the wrapped object.
    override fun equals(other: Any?): Boolean =
        other is {{ obj.name.CamelCase() }} && other.handle == handle
}
