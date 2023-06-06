#version 310 es
precision mediump float;

layout(set = 0, binding = 0) uniform mediump sampler uSampler;
layout(set = 0, binding = 1) uniform mediump texture2D uDepth;
layout(location = 0) out vec4 FragColor;

vec4 samp(texture2D t, mediump sampler s)
{
   return texture(sampler2D(t, s), vec2(0.5));
}

void main()
{
   FragColor = samp(uDepth, uSampler);
}
