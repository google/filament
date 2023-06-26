#version 450

#extension GL_AMD_gpu_shader_int16 : require

layout(location = 0) flat in int16_t a;
layout(location = 1) flat in ivec2 b;
layout(location = 2) flat in uint16_t c[2];
layout(location = 4) flat in uvec4 e[2];
layout(location = 6) in vec2 d;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(float(int(a)), float(b.x), vec2(uint(c[1]), float(e[0].w)) + d);
}

