//* Copyright 2019 The Dawn & Tint Authors
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

#include <KHR/khrplatform.h>

using GLvoid = void;
using GLchar = char;
using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLbyte = khronos_int8_t;
using GLshort = short;
using GLint = int;
using GLsizei = int;
using GLubyte = khronos_uint8_t;
using GLushort = unsigned short;
using GLuint = unsigned int;
using GLfloat = khronos_float_t;
using GLclampf = khronos_float_t;
using GLdouble = double;
using GLclampd = double;
using GLfixed = khronos_int32_t;
using GLintptr = khronos_intptr_t;
using GLsizeiptr = khronos_ssize_t;
using GLhalf = unsigned short;
using GLint64 = khronos_int64_t;
using GLuint64 = khronos_uint64_t;
using GLsync = struct __GLsync*;
using GLeglImageOES = void*;
using GLDEBUGPROC = void(KHRONOS_APIENTRY*)(GLenum source,
                                            GLenum type,
                                            GLuint id,
                                            GLenum severity,
                                            GLsizei length,
                                            const GLchar* message,
                                            const void* userParam);
using GLDEBUGPROCARB = GLDEBUGPROC;
using GLDEBUGPROCKHR = GLDEBUGPROC;
using GLDEBUGPROCAMD = void(KHRONOS_APIENTRY*)(GLuint id,
                                               GLenum category,
                                               GLenum severity,
                                               GLsizei length,
                                               const GLchar* message,
                                               void* userParam);

using AnyGLProc = void (*)();
using GLGetProcProc = AnyGLProc (KHRONOS_APIENTRY*) (const char*);

{% for block in header_blocks %}
    // {{block.description}}
    {% for enum in block.enums %}
        #define {{enum.name}} {{enum.value}}
    {% endfor %}

    {% for proc in block.procs %}
        using {{proc.PFNGLPROCNAME()}} = {{proc.return_type}}(KHRONOS_APIENTRY *)(
            {%- for param in proc.params -%}
                {%- if not loop.first %}, {% endif -%}
                {{param.type}} {{param.name}}
            {%- endfor -%}
        );
    {% endfor %}

{% endfor%}

#undef DAWN_GL_APIENTRY
