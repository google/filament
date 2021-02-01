#version 310 es
#extension GL_EXT_tessellation_shader : require

layout(vertices = 3) out;

layout(location = 0) in  highp float in_tc_attr[];
layout(location = 0) out highp float in_te_attr[];

struct te_data
{
    mediump float a;
    mediump float b;
    mediump uint c;
};

layout(location = 1) out te_data in_te_data0[];
layout(location = 4) out te_data in_te_data1[];

void main (void)
{
    te_data d = te_data(float(gl_InvocationID), float(gl_InvocationID + 1), uint(gl_InvocationID));
    in_te_data0[gl_InvocationID] = d;
    barrier();
	te_data e = in_te_data0[(gl_InvocationID + 1) % 3];
    in_te_data1[gl_InvocationID] = te_data(d.a + e.a, d.b + e.b, d.c + e.c);

    in_te_attr[gl_InvocationID] = in_tc_attr[gl_InvocationID];

    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;

    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;
}
