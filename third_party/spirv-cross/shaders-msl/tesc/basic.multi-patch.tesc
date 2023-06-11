#version 310 es
#extension GL_EXT_tessellation_shader : require

layout(location = 0) patch out vec3 vFoo;

layout(vertices = 1) out;

void main()
{
    gl_TessLevelInner[0] = 8.9;
    gl_TessLevelInner[1] = 6.9;
    gl_TessLevelOuter[0] = 8.9;
    gl_TessLevelOuter[1] = 6.9;
    gl_TessLevelOuter[2] = 3.9;
    gl_TessLevelOuter[3] = 4.9;
    vFoo = vec3(1.0);
}
