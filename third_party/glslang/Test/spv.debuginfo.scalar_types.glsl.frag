/*
The MIT License (MIT)

Copyright (c) 2023 NVIDIA CORPORATION.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#version 460

#extension GL_EXT_shader_explicit_arithmetic_types : require

bool VAR_bool;
int VAR_int;
uint VAR_uint;
float VAR_float;
double VAR_double;
int8_t VAR_int8_t;
uint8_t VAR_uint8_t;
int16_t VAR_int16_t;
uint16_t VAR_uint16_t;
int64_t VAR_int64_t;
uint64_t VAR_uint64_t;
float16_t VAR_float16_t;

void main() {
    VAR_bool = bool(0);
    VAR_int = int(0);
    VAR_uint = uint(0);
    VAR_float = float(0);
    VAR_double = double(0);
    VAR_int8_t = int8_t(0);
    VAR_uint8_t = uint8_t(0);
    VAR_int16_t = int16_t(0);
    VAR_uint16_t = uint16_t(0);
    VAR_int64_t = int64_t(0);
    VAR_uint64_t = uint64_t(0);
    VAR_float16_t = float16_t(0);
}