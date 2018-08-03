#version 310 es
#extension GL_EXT_geometry_shader : require

layout(points) in;
layout(points, max_vertices = 3) out;

layout(location = 0) in VertexData {
    vec3 normal;
} vin[];

layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal;
    EmitVertex();

    EndPrimitive();
}
