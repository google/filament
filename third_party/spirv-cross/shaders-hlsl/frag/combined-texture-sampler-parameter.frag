#version 310 es
precision mediump float;

layout(set = 0, binding = 0) uniform mediump sampler2D uSampler;
layout(set = 0, binding = 1) uniform mediump sampler2DShadow uSamplerShadow;
layout(location = 0) out float FragColor;

vec4 samp2(sampler2D s)
{
   return texture(s, vec2(1.0)) + texelFetch(s, ivec2(10), 0);
}

vec4 samp3(sampler2D s)
{
   return samp2(s);
}

float samp4(mediump sampler2DShadow s)
{
   return texture(s, vec3(1.0));
}

float samp(sampler2D s0, mediump sampler2DShadow s1)
{
   return samp3(s0).x + samp4(s1);
}

void main()
{
   FragColor = samp(uSampler, uSamplerShadow);
}
