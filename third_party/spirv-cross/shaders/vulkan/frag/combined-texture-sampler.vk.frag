#version 310 es
precision mediump float;

layout(set = 0, binding = 0) uniform mediump sampler uSampler0;
layout(set = 0, binding = 1) uniform mediump sampler uSampler1;
layout(set = 0, binding = 2) uniform mediump texture2D uTexture0;
layout(set = 0, binding = 3) uniform mediump texture2D uTexture1;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vTex;

vec4 sample_dual(mediump sampler samp, mediump texture2D tex)
{
   return texture(sampler2D(tex, samp), vTex);
}

vec4 sample_global_tex(mediump sampler samp)
{
   vec4 a = texture(sampler2D(uTexture0, samp), vTex);
   vec4 b = sample_dual(samp, uTexture1);
   return a + b;
}

vec4 sample_global_sampler(mediump texture2D tex)
{
   vec4 a = texture(sampler2D(tex, uSampler0), vTex);
   vec4 b = sample_dual(uSampler1, tex);
   return a + b;
}

vec4 sample_duals()
{
   vec4 a = sample_dual(uSampler0, uTexture0);
   vec4 b = sample_dual(uSampler1, uTexture1);
   return a + b;
}

void main()
{
    vec4 c0 = sample_duals();
    vec4 c1 = sample_global_tex(uSampler0);
    vec4 c2 = sample_global_tex(uSampler1);
    vec4 c3 = sample_global_sampler(uTexture0);
    vec4 c4 = sample_global_sampler(uTexture1);

    FragColor = c0 + c1 + c2 + c3 + c4;
}
