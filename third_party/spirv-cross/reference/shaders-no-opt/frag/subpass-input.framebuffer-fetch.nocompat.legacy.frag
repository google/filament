#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
precision mediump float;
precision highp int;

mediump vec4 uSubpass0;
mediump vec4 uSubpass1;

void main()
{
    uSubpass0 = gl_LastFragData[0];
    uSubpass1 = gl_LastFragData[1];
    gl_FragData[0] = uSubpass0.xyz + uSubpass1.xyz;
}

