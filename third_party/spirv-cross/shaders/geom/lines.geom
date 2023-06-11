#version 310 es
#extension GL_EXT_geometry_shader : require

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

layout(location = 0) in VertexData {
    vec3 normal;
} vin[];

layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    vNormal = vin[1].normal;
    EmitVertex();

    EndPrimitive();
}
