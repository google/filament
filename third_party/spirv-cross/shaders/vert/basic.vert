#version 310 es

layout(std140) uniform UBO
{
    uniform mat4 uMVP;
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = uMVP * aVertex;
    vNormal = aNormal;
}
