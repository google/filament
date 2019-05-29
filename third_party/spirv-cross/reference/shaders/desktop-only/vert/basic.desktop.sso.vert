#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(binding = 0, std140) uniform UBO
{
    mat4 uMVP;
} _16;

layout(location = 0) in vec4 aVertex;
layout(location = 0) out vec3 vNormal;
layout(location = 1) in vec3 aNormal;

void main()
{
    gl_Position = _16.uMVP * aVertex;
    vNormal = aNormal;
}

