#version 450

out gl_PerVertex
{
   vec4 gl_Position;
};

layout(std140) uniform UBO
{
    mat4 uMVP;
};
layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = uMVP * aVertex;
    vNormal = aNormal;
}
