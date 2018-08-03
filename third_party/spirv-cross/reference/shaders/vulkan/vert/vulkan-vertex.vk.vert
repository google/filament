#version 310 es

uniform int SPIRV_Cross_BaseInstance;

void main()
{
    gl_Position = vec4(1.0, 2.0, 3.0, 4.0) * float(gl_VertexID + (gl_InstanceID + SPIRV_Cross_BaseInstance));
}

