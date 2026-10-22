#version 310 es
#extension GL_EXT_geometry_shader : require
layout(triangles) in;
layout(max_vertices = 3, triangle_strip) out;

layout(location = 0) out vec3 vNormal;
layout(location = 0) in VertexData
{
    vec3 normal;
} vin[3];


void main()
{
    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    vNormal = vin[1].normal;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    vNormal = vin[2].normal;
    EmitVertex();
    EndPrimitive();
}

