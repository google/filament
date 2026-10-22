#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texelFetchOffset(uTexture, ivec2(gl_FragCoord.xy), 0, ivec2(1));
    FragColor += texelFetchOffset(uTexture, ivec2(gl_FragCoord.xy), 0, ivec2(-1, 1));
}

