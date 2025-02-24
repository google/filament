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
//*
//*
//* This generator is used to produce the number-to-string mappings for
//* Emscripten's library_webgpu.js.
//* https://github.com/emscripten-core/emscripten/blob/main/src/library_webgpu.js
//*
{{'{{{'}}
    globalThis.__HAVE_EMDAWNWEBGPU_ENUM_TABLES = true;

    // Constant values used at code-generation time in an Emscripten build.
    // These will not appear in the final build result, so we can just dump
    // every enum here without affecting binary size.
    globalThis.WEBGPU_ENUM_CONSTANT_TABLES = {
        {% for type in by_category["enum"] + by_category["bitmask"] %}
            {{type.name.CamelCase()}}: {
                {% for value in type.values %}
                    '{{value.name.CamelCase()}}': {{value.value}},
                {% endfor %}
            },
        {% endfor %}
    };

    // Maps from enum string back to enum number, for callbacks.
    // These appear in the final build result so should be kept minimal.
    globalThis.WEBGPU_STRING_TO_INT_TABLES = `
        {% for type in by_category["enum"] if type.json_data.get("emscripten_string_to_int", False) %}
            Int_{{type.name.CamelCase()}}: {
                {% for value in type.values if value.json_data.get("emscripten_string_to_int", True) %}
                    {% if type.name.name == 'device lost reason' and value.name.name == 'unknown' %}
                        'undefined': {{value.value}},  // For older browsers
                    {% endif %}
                    {{as_jsEnumValue(value)}}: {{value.value}},
                {% endfor %}
            },
        {% endfor %}
        Int_PreferredFormat: {
            {% for value in types['texture format'].values if value.name.name in ['RGBA8 unorm', 'BGRA8 unorm'] %}
                {{as_jsEnumValue(value)}}: {{value.value}},
            {% endfor %}
        },
`;

    // Maps from enum number to enum string.
    // These appear in the final build result so should be kept minimal.
    globalThis.WEBGPU_INT_TO_STRING_TABLES = `
        {% for type in by_category["enum"] if not type.json_data.get("emscripten_no_enum_table") %}
            {{type.name.CamelCase()}}: {% if type.contiguousFromZero -%}
                [
                    {% for value in type.values %}
                        {{as_jsEnumValue(value)}},
                    {% endfor %}
                ]
            {%- else -%}
                {
                    {% for value in type.values %}
                        {{value.value}}: {{as_jsEnumValue(value)}},
                    {% endfor %}
                }
            {%- endif -%}
            ,
        {% endfor %}
`;

    null;
{{'}}}'}}
