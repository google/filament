#version 310 es

layout(binding = 0, std140) uniform Block
{
    layout(row_major) mat2x3 var[3][4];
} _104;

layout(location = 0) in vec4 a_position;
layout(location = 0) out mediump float v_vtxResult;

mat2x3 spvWorkaroundRowMajor(mat2x3 wrap) { return wrap; }

mediump float compare_float(float a, float b)
{
    return float(abs(a - b) < 0.0500000007450580596923828125);
}

mediump float compare_vec3(vec3 a, vec3 b)
{
    float param = a.x;
    float param_1 = b.x;
    float param_2 = a.y;
    float param_3 = b.y;
    float param_4 = a.z;
    float param_5 = b.z;
    return (compare_float(param, param_1) * compare_float(param_2, param_3)) * compare_float(param_4, param_5);
}

mediump float compare_mat2x3(mat2x3 a, mat2x3 b)
{
    vec3 param = a[0];
    vec3 param_1 = b[0];
    vec3 param_2 = a[1];
    vec3 param_3 = b[1];
    return compare_vec3(param, param_1) * compare_vec3(param_2, param_3);
}

void main()
{
    gl_Position = a_position;
    mediump float result = 1.0;
    mat2x3 param = spvWorkaroundRowMajor(_104.var[0][0]);
    mat2x3 param_1 = mat2x3(vec3(2.0, 6.0, -6.0), vec3(0.0, 5.0, 5.0));
    result *= compare_mat2x3(param, param_1);
    v_vtxResult = result;
}

