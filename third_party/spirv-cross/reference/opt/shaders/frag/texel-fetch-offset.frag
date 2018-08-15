#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
    mediump ivec2 _22 = ivec2(gl_FragCoord.xy);
    FragColor = texelFetchOffset(uTexture, _22, 0, ivec2(1));
    FragColor += texelFetchOffset(uTexture, _22, 0, ivec2(-1, 1));
}

