#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
precision mediump float;
precision highp int;

mediump vec4 uSubpass0;
mediump vec4 uSubpass1;

layout(location = 0) inout vec3 FragColor;
layout(location = 1) inout vec4 FragColor2;

void main()
{
    uSubpass0.xyz = FragColor;
    uSubpass1 = FragColor2;
    FragColor = uSubpass0.xyz + uSubpass1.xyz;
}

