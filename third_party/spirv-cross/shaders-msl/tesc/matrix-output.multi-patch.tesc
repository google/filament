#version 310 es
#extension GL_EXT_tessellation_shader : require

layout(vertices = 3) out;

layout(location = 0) in  highp float in_tc_attr[];
layout(location = 0) out highp float in_te_attr[];

layout(location = 1) out mediump mat4x3 in_te_data0[];
layout(location = 5) out mediump mat4x3 in_te_data1[];

void main (void)
{
    mat4x3 d = mat4x3(gl_InvocationID);
    in_te_data0[gl_InvocationID] = d;
    barrier();
    in_te_data1[gl_InvocationID] = d + in_te_data0[(gl_InvocationID + 1) % 3];

    in_te_attr[gl_InvocationID] = in_tc_attr[gl_InvocationID];

    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;

    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;
}
