#version 450
layout(vertices = 3) out;

layout(binding = 0, std140) uniform cb1_struct
{
    vec4 _m0[1];
} cb0_0;

layout(location = 0) in vec4 v0[];
layout(location = 1) in vec4 v1[];
layout(location = 2) in vec3 vicp0[];
layout(location = 3) out vec3 vocp0[3];
layout(location = 4) in vec4 vicp1[];
layout(location = 5) out vec4 vocp1[3];
vec4 opc[4];
vec4 vicp[2][3];
vec4 _52;
vec4 _55;
vec4 _58;
vec4 _89;

void fork0_epilogue(vec4 _61, vec4 _62, vec4 _63)
{
    gl_TessLevelOuter[0u] = _61.x;
    gl_TessLevelOuter[1u] = _62.x;
    gl_TessLevelOuter[2u] = _63.x;
}

void fork0(uint vForkInstanceId)
{
    vec4 r0;
    r0.x = uintBitsToFloat(vForkInstanceId);
    opc[floatBitsToInt(r0.x)].x = cb0_0._m0[0u].x;
    _52 = opc[0u];
    _55 = opc[1u];
    _58 = opc[2u];
    fork0_epilogue(_52, _55, _58);
}

void fork1_epilogue(vec4 _92)
{
    gl_TessLevelInner[0u] = _92.x;
}

void fork1()
{
    opc[3u].x = cb0_0._m0[0u].x;
    _89 = opc[3u];
    fork1_epilogue(_89);
}

void main()
{
    vec4 _126_unrolled[3];
    for (int i = 0; i < int(3); i++)
    {
        _126_unrolled[i] = v0[i];
    }
    vicp[0u] = _126_unrolled;
    vec4 _127_unrolled[3];
    for (int i = 0; i < int(3); i++)
    {
        _127_unrolled[i] = v1[i];
    }
    vicp[1u] = _127_unrolled;
    vocp0[gl_InvocationID] = vicp0[gl_InvocationID];
    vocp1[gl_InvocationID] = vicp1[gl_InvocationID];
    fork0(0u);
    fork0(1u);
    fork0(2u);
    fork1();
}

