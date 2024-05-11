#version 450
layout(invocations = 4, triangles) in;
layout(max_vertices = 3, triangle_strip) out;

in gl_PerVertex
{
    vec4 gl_Position;
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) out vec3 vNormal;
layout(location = 0) in VertexData
{
    vec3 normal;
} vin[3];


void main()
{
    gl_Position = gl_in[0].gl_Position;
    vNormal = vin[0].normal + vec3(float(gl_InvocationID));
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    vNormal = vin[1].normal + vec3(4.0 * float(gl_InvocationID));
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    vNormal = vin[2].normal + vec3(2.0 * float(gl_InvocationID));
    EmitVertex();
    EndPrimitive();
}

