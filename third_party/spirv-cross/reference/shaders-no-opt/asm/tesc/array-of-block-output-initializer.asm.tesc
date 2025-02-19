#version 450
layout(vertices = 4) out;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
} gl_out[4];

layout(location = 0) patch out vert
{
    float v0;
    float v1;
} _7;

layout(location = 2) patch out vert_patch
{
    float v2;
    float v3;
} patches[2];

layout(location = 6) patch out float v2;
layout(location = 7) out float v3[4];
layout(location = 8) out vert2
{
    float v4;
    float v5;
} verts[4];

const vec4 _4_0_init[4] = vec4[](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
const float _4_1_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _4_2_init[4][1] = float[][](float[](0.0), float[](0.0), float[](0.0), float[](0.0));
const float _4_3_init[4][1] = float[][](float[](0.0), float[](0.0), float[](0.0), float[](0.0));
const float _8_0_init[2] = float[](0.0, 0.0);
const float _8_1_init[2] = float[](0.0, 0.0);
const float _9_init = 0.0;
const float _10_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _11_0_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _11_1_init[4] = float[](0.0, 0.0, 0.0, 0.0);

void main()
{
    gl_out[gl_InvocationID].gl_Position = _4_0_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_PointSize = _4_1_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_ClipDistance = _4_2_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_CullDistance = _4_3_init[gl_InvocationID];
    if (gl_InvocationID == 0)
    {
        _7.v0 = 0.0;
    }
    if (gl_InvocationID == 0)
    {
        _7.v1 = 0.0;
    }
    if (gl_InvocationID == 0)
    {
        patches[0].v2 = _8_0_init[0];
    }
    if (gl_InvocationID == 0)
    {
        patches[1].v2 = _8_0_init[1];
    }
    if (gl_InvocationID == 0)
    {
        patches[0].v3 = _8_1_init[0];
    }
    if (gl_InvocationID == 0)
    {
        patches[1].v3 = _8_1_init[1];
    }
    if (gl_InvocationID == 0)
    {
        v2 = _9_init;
    }
    v3[gl_InvocationID] = _10_init[gl_InvocationID];
    verts[gl_InvocationID].v4 = _11_0_init[gl_InvocationID];
    verts[gl_InvocationID].v5 = _11_1_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = vec4(1.0);
}

