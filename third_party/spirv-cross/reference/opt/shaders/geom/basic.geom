#version 310 es
#extension GL_EXT_geometry_shader : require
layout(invocations = 4, triangles) in;
layout(max_vertices = 3, triangle_strip) out;

layout(location = 0) out vec3 vNormal;
layout(location = 0) in VertexData
{
    vec3 normal;
} vin[3];


void main()
{
    gl_Position = gl_in[0].gl_Position;
    float _37 = float(gl_InvocationID);
    vNormal = vin[0].normal + vec3(_37);
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    vNormal = vin[1].normal + vec3(4.0 * _37);
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    vNormal = vin[2].normal + vec3(2.0 * _37);
    EmitVertex();
    EndPrimitive();
}

