#version 450
layout(vertices = 1) out;

in gl_PerVertex
{
   vec4 gl_Position;
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex
{
   vec4 gl_Position;
} gl_out[1];

layout(location = 0) patch out vec3 vFoo;


void main()
{
    gl_TessLevelInner[0] = 8.9;
    gl_TessLevelInner[1] = 6.9;
    gl_TessLevelOuter[0] = 8.9;
    gl_TessLevelOuter[1] = 6.9;
    gl_TessLevelOuter[2] = 3.9;
    gl_TessLevelOuter[3] = 4.9;
    vFoo = vec3(1.0);

    gl_out[gl_InvocationID].gl_Position = gl_in[0].gl_Position + gl_in[1].gl_Position;
}
