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
vec4 _48;
vec4 _49;
vec4 _50;
vec4 _56;

void fork0_epilogue(vec4 _87, vec4 _88, vec4 _89)
{
    gl_TessLevelOuter[0u] = _87.x;
    gl_TessLevelOuter[1u] = _88.x;
    gl_TessLevelOuter[2u] = _89.x;
}

void fork0(uint vForkInstanceId)
{
    vec4 r0;
    r0.x = uintBitsToFloat(vForkInstanceId);
    opc[floatBitsToInt(r0.x)].x = cb0_0._m0[0u].x;
    _48 = opc[0u];
    _49 = opc[1u];
    _50 = opc[2u];
    fork0_epilogue(_48, _49, _50);
}

void fork1_epilogue(vec4 _109)
{
    gl_TessLevelInner[0u] = _109.x;
}

void fork1()
{
    opc[3u].x = cb0_0._m0[0u].x;
    _56 = opc[3u];
    fork1_epilogue(_56);
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

