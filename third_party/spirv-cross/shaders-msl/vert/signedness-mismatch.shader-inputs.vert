#version 450

#extension GL_AMD_gpu_shader_int16 : require

layout(location = 0) in int16_t a;
layout(location = 1) in ivec2 b;
layout(location = 2) in uint16_t c[2];
layout(location = 4) in uvec4 d[2];

void main()
{
    gl_Position = vec4(float(int(a)), float(b.x), float(uint(c[1])), float(d[0].w));
}

