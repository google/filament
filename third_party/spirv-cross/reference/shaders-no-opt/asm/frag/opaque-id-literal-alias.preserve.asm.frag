#version 450

layout(binding = 0) uniform sampler2DMS uSampled;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;

void main()
{
    FragColor = vec4(0.0);
    if (gl_FragCoord.x < 10.0)
    {
        FragColor += texelFetch(uSampled, ivec2(vUV), 0);
    }
    else
    {
        FragColor += texelFetch(uSampled, ivec2(vUV), 1);
    }
}

