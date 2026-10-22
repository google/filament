#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2DMS uSampler;

layout(location = 0) out vec4 FragColor;

void main()
{
    ivec2 _17 = ivec2(gl_FragCoord.xy);
    FragColor = ((texelFetch(uSampler, _17, 0) + texelFetch(uSampler, _17, 1)) + texelFetch(uSampler, _17, 2)) + texelFetch(uSampler, _17, 3);
}

