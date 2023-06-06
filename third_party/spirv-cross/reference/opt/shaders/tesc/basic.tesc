#version 310 es
#extension GL_EXT_tessellation_shader : require
layout(vertices = 1) out;

layout(location = 0) patch out vec3 vFoo;

void main()
{
    gl_TessLevelInner[0] = 8.8999996185302734375;
    gl_TessLevelInner[1] = 6.900000095367431640625;
    gl_TessLevelOuter[0] = 8.8999996185302734375;
    gl_TessLevelOuter[1] = 6.900000095367431640625;
    gl_TessLevelOuter[2] = 3.900000095367431640625;
    gl_TessLevelOuter[3] = 4.900000095367431640625;
    vFoo = vec3(1.0);
}

