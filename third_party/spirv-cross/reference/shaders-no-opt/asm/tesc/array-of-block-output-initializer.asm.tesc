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
} _5;

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

const vec4 _3_0_init[4] = vec4[](vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
const float _3_1_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _3_2_init[4][1] = float[][](float[](0.0), float[](0.0), float[](0.0), float[](0.0));
const float _3_3_init[4][1] = float[][](float[](0.0), float[](0.0), float[](0.0), float[](0.0));
const float _6_0_init[2] = float[](0.0, 0.0);
const float _6_1_init[2] = float[](0.0, 0.0);
const float _7_init = 0.0;
const float _8_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _9_0_init[4] = float[](0.0, 0.0, 0.0, 0.0);
const float _9_1_init[4] = float[](0.0, 0.0, 0.0, 0.0);

void main()
{
    gl_out[gl_InvocationID].gl_Position = _3_0_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_PointSize = _3_1_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_ClipDistance = _3_2_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_CullDistance = _3_3_init[gl_InvocationID];
    if (gl_InvocationID == 0)
    {
        _5.v0 = 0.0;
    }
    if (gl_InvocationID == 0)
    {
        _5.v1 = 0.0;
    }
    if (gl_InvocationID == 0)
    {
        patches[0].v2 = _6_0_init[0];
    }
    if (gl_InvocationID == 0)
    {
        patches[1].v2 = _6_0_init[1];
    }
    if (gl_InvocationID == 0)
    {
        patches[0].v3 = _6_1_init[0];
    }
    if (gl_InvocationID == 0)
    {
        patches[1].v3 = _6_1_init[1];
    }
    if (gl_InvocationID == 0)
    {
        v2 = _7_init;
    }
    v3[gl_InvocationID] = _8_init[gl_InvocationID];
    verts[gl_InvocationID].v4 = _9_0_init[gl_InvocationID];
    verts[gl_InvocationID].v5 = _9_1_init[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = vec4(1.0);
}

