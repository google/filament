#version 310 es

layout(binding = 0, std140) uniform UBO
{
   mat4 mvp; 
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = mvp * aVertex;
    vNormal = aNormal;
}
