#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uSampler;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

vec4 foo()
{
    vec4 color;
    if (!gl_HelperInvocation)
    {
        color = textureLod(uSampler, vUV, 0.0);
    }
    else
    {
        color = vec4(1.0);
    }
    return color;
}

void main()
{
    FragColor = foo();
}

