#version 450 core

// GL_EXT_shader_16bit_storage doesn't support input/output.
#extension GL_EXT_shader_8bit_storage : require
#extension GL_AMD_gpu_shader_int16 : require
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 0, component = 0) in int16_t foo;
layout(location = 0, component = 1) in uint16_t bar;
layout(location = 1) in float16_t baz;

layout(binding = 0) uniform block {
    i16vec2 a;
    u16vec2 b;
    i8vec2 c;
    u8vec2 d;
    f16vec2 e;
};

layout(binding = 1) readonly buffer storage {
    i16vec3 f;
    u16vec3 g;
    i8vec3 h;
    u8vec3 i;
    f16vec3 j;
};

layout(push_constant) uniform pushconst {
    i16vec4 k;
    u16vec4 l;
    i8vec4 m;
    u8vec4 n;
    f16vec4 o;
};

layout(location = 0) out i16vec4 p;
layout(location = 1) out u16vec4 q;
layout(location = 2) out f16vec4 r;

void main() {
    p = i16vec4(int(foo) + ivec4(ivec2(a), ivec2(c)) - ivec4(ivec3(f) / ivec3(h), 1) + ivec4(k) + ivec4(m));
    q = u16vec4(uint(bar) + uvec4(uvec2(b), uvec2(d)) - uvec4(uvec3(g) / uvec3(i), 1) + uvec4(l) + uvec4(n));
    r = f16vec4(float(baz) + vec4(vec2(e), 0, 1) - vec4(vec3(j), 1) + vec4(o));
    gl_Position = vec4(0, 0, 0, 1);
}

