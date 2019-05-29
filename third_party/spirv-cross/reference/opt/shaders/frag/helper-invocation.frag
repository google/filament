#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uSampler;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 _51;
    if (!gl_HelperInvocation)
    {
        _51 = textureLod(uSampler, vUV, 0.0);
    }
    else
    {
        _51 = vec4(1.0);
    }
    FragColor = _51;
}

