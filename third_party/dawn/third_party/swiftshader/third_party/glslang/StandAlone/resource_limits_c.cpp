/**
BSD 2-Clause License

Copyright (c) 2020, Travis Fort
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include "glslang/Public/resource_limits_c.h"
#include "glslang/Public/ResourceLimits.h"
#include <stdlib.h>
#include <string.h>
#include <string>

glslang_resource_t* glslang_resource(void)
{
    return reinterpret_cast<glslang_resource_t*>(GetResources());
}

const glslang_resource_t* glslang_default_resource(void)
{
    return reinterpret_cast<const glslang_resource_t*>(GetDefaultResources());
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

const char* glslang_default_resource_string()
{
    std::string cpp_str = GetDefaultTBuiltInResourceString();
    char* c_str = (char*)malloc(cpp_str.length() + 1);
    strcpy(c_str, cpp_str.c_str());
    return c_str;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

void glslang_decode_resource_limits(glslang_resource_t* resources, char* config)
{
    DecodeResourceLimits(reinterpret_cast<TBuiltInResource*>(resources), config);
}
