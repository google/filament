#version 310 es

layout(binding = 0, std140) uniform Block
{
    layout(row_major) mat2x3 var[3][4];
} _106;

layout(location = 0) in vec4 a_position;
layout(location = 0) out mediump float v_vtxResult;

void main()
{
    gl_Position = a_position;
    v_vtxResult = ((float(abs(_106.var[0][0][0].x - 2.0) < 0.0500000007450580596923828125) * float(abs(_106.var[0][0][0].y - 6.0) < 0.0500000007450580596923828125)) * float(abs(_106.var[0][0][0].z - (-6.0)) < 0.0500000007450580596923828125)) * ((float(abs(_106.var[0][0][1].x) < 0.0500000007450580596923828125) * float(abs(_106.var[0][0][1].y - 5.0) < 0.0500000007450580596923828125)) * float(abs(_106.var[0][0][1].z - 5.0) < 0.0500000007450580596923828125));
}

