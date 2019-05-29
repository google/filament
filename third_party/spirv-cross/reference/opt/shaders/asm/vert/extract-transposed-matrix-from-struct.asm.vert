#version 450

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

void main()
{
    gl_Position = gInstanceData_1._data[uint((gl_InstanceID + SPIRV_Cross_BaseInstance))].MATRIX_MVP * vec4(PosL, 1.0);
    _entryPointOutput_Color = gInstanceData_1._data[uint((gl_InstanceID + SPIRV_Cross_BaseInstance))].Color;
}

