#version 450
#if defined(GL_AMD_gpu_shader_half_float)
#extension GL_AMD_gpu_shader_half_float : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for FP16.
#endif

layout(binding = 0) uniform sampler2D uTexture;

layout(location = 0) out f16vec4 FragColor;
layout(location = 0) in f16vec2 UV;

void main()
{
    FragColor = f16vec4(texture(uTexture, UV));
}

