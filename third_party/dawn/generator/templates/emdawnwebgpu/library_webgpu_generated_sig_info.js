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
//* This generator is used to produce the Wasm signatures for any C functions implemented in JS.
//* It is equivalent to the emscripten_webgpu_* and wgpu* signatures in:
//* https://github.com/emscripten-core/emscripten/blob/main/src/library_sigs.js
//*
{{'{{{'}}
    globalThis.__HAVE_EMDAWNWEBGPU_SIG_INFO = true;
    null;
{{'}}}'}}
{%- macro render_function_or_method_sig(object_or_none, function) -%}
    {%- if not has_wasmType(function.returns, function.arguments) -%}
        //
    {%- endif -%}
    {{as_cMethod(object_or_none.name if object_or_none else None, function.name)}}__sig: '
        {{- as_wasmType(function.returns) -}}
        {%- if object_or_none -%}
            {{- as_wasmType(object_or_none) -}}
        {%- endif -%}
        {%- for arg in function.arguments -%}
            {{- as_wasmType(arg) -}}
        {%- endfor -%}
    ',
{%- endmacro -%}
const webgpuSigs = {
    // emscripten_webgpu_*
    emscripten_webgpu_get_device__sig: 'p',
    emscripten_webgpu_release_js_handle__sig: 'vi',
    {% for type in by_category["object"] %}
        emscripten_webgpu_export_{{type.name.snake_case()}}__sig: 'ip',
        emscripten_webgpu_import_{{type.name.snake_case()}}__sig: 'pi',
    {% endfor %}

    // wgpu*
    {% for function in by_category["function"] %}
        {{render_function_or_method_sig(None, function)}}
    {% endfor %}
    {% for type in by_category["object"] %}
        {% for method in c_methods(type) %}
            {{render_function_or_method_sig(type, method)}}
        {% endfor %}
    {% endfor %}
};

// Delete all of the WebGPU sig info from Emscripten's builtins first.
for (const k of Object.keys(LibraryManager.library)) {
    if (k.endsWith('__sig') && (k.startsWith('emscripten_webgpu_') || k.startsWith('wgpu'))) {
        delete LibraryManager.library[k];
    }
}
mergeInto(LibraryManager.library, webgpuSigs, {allowMissing: true});
