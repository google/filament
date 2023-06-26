#version 450

layout(std430, push_constant) uniform Push
{
   vec4 push;
} registers;

layout(std140, set = 0, binding = 5) uniform UBO
{
   vec4 ubo;
};

layout(std430, set = 1, binding = 7) buffer SSBO
{
   vec4 ssbo;
};

layout(std430, set = 1, binding = 8) readonly buffer SSBOs
{
   vec4 ssbo;
} ssbos[2];

layout(std140, set = 2, binding = 4) uniform UBOs
{
   vec4 ubo;
} ubos[4];

layout(set = 0, binding = 2) uniform sampler2D uTexture;
layout(set = 0, binding = 6) uniform sampler2D uTextures[2];
layout(set = 1, binding = 3) uniform texture2D uTexture2[4];
layout(set = 1, binding = 10) uniform sampler uSampler[2];
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

vec4 sample_in_function2()
{
   vec4 ret = texture(uTexture, vUV);
   ret += texture(sampler2D(uTexture2[2], uSampler[1]), vUV);
   ret += texture(uTextures[1], vUV);
   ret += ssbo;
   ret += ssbos[0].ssbo;
   ret += registers.push;
   return ret;
}

vec4 sample_in_function()
{
   vec4 ret = sample_in_function2();
   ret += ubo;
   ret += ubos[0].ubo;
   return ret;
}

void main()
{
   FragColor = sample_in_function();
   FragColor += ubo;
   FragColor += ssbo;
   FragColor += ubos[1].ubo;
   FragColor += registers.push;
}
