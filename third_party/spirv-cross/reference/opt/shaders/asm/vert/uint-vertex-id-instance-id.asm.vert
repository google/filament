#version 450

uniform int SPIRV_Cross_BaseInstance;

void main()
{
    gl_Position = vec4(float(uint(gl_VertexID) + uint((gl_InstanceID + SPIRV_Cross_BaseInstance))));
}

