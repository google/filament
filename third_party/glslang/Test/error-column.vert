#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (binding = 0) uniform block {
    const mat4 model;
    const mat4 view;
    const mat4 projection;
};

layout (location = 0) out Vertex
{
    vec3 color;
    vec3 worldSpacePos;
    vec3 worldSpaceNorm;
    const vec2 texCoord1;
    flat int cameraIndex;
    float ii;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(Pos, 1.0);  
    vs_out.texCoord1 = aTexCoords;
}

