#version 310 es
#extension GL_EXT_geometry_shader : require

layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in VertexData {
    vec3 normal;
} vin[];

layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal + float(gl_InvocationID);
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    vNormal = vin[1].normal + 4.0 * float(gl_InvocationID);
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    vNormal = vin[2].normal + 2.0 * float(gl_InvocationID);
    EmitVertex();

    EndPrimitive();
}
