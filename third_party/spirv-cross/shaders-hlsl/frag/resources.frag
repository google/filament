#version 310 es
precision mediump float;

layout(binding = 3, std140) uniform CBuffer
{
   vec4 a;
} cbuf;

layout(binding = 4) uniform sampler2D uSampledImage;
layout(binding = 5) uniform mediump texture2D uTexture;
layout(binding = 6) uniform mediump sampler uSampler;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vTex;

layout(std430, push_constant) uniform PushMe
{
   vec4 d;
} registers;

void main()
{
   vec4 c0 = texture(uSampledImage, vTex);
   vec4 c1 = texture(sampler2D(uTexture, uSampler), vTex);
   vec4 c2 = cbuf.a + registers.d;
   FragColor = c0 + c1 + c2;
}
