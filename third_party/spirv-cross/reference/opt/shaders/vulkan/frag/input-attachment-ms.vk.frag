#version 450

layout(binding = 0) uniform sampler2DMS uSubpass0;
layout(binding = 1) uniform sampler2DMS uSubpass1;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = (texelFetch(uSubpass0, ivec2(gl_FragCoord.xy), 1) + texelFetch(uSubpass1, ivec2(gl_FragCoord.xy), 2)) + texelFetch(uSubpass0, ivec2(gl_FragCoord.xy), gl_SampleID);
}

