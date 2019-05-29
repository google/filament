#version 450

struct V2F
{
    vec4 Position;
    vec4 Color;
};

struct InstanceData
{
    mat4 MATRIX_MVP;
    vec4 Color;
};

layout(binding = 0, std430) readonly buffer gInstanceData
{
    layout(row_major) InstanceData _data[];
} gInstanceData_1;

layout(location = 0) in vec3 PosL;
uniform int SPIRV_Cross_BaseInstance;
layout(location = 0) out vec4 _entryPointOutput_Color;

V2F _VS(vec3 PosL_1, uint instanceID)
{
    InstanceData instData;
    instData.MATRIX_MVP = gInstanceData_1._data[instanceID].MATRIX_MVP;
    instData.Color = gInstanceData_1._data[instanceID].Color;
    V2F v2f;
    v2f.Position = instData.MATRIX_MVP * vec4(PosL_1, 1.0);
    v2f.Color = instData.Color;
    return v2f;
}

void main()
{
    vec3 PosL_1 = PosL;
    uint instanceID = uint((gl_InstanceID + SPIRV_Cross_BaseInstance));
    vec3 param = PosL_1;
    uint param_1 = instanceID;
    V2F flattenTemp = _VS(param, param_1);
    gl_Position = flattenTemp.Position;
    _entryPointOutput_Color = flattenTemp.Color;
}

