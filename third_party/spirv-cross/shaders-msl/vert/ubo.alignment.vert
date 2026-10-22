#version 310 es

layout(binding = 0, std140) uniform UBO
{
   mat4 mvp;
   vec2 targSize;
   vec3 color;      // vec3 following vec2 should cause MSL to add pad if float3 is packed
   float opacity;   // Single float following vec3 should cause MSL float3 to pack
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec2 vSize;

void main()
{
    gl_Position = mvp * aVertex;
    vNormal = aNormal;
    vColor = color * opacity;
    vSize = targSize * opacity;
}
