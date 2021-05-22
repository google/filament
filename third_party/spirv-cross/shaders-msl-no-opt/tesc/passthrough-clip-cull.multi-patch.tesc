#version 450

layout(vertices = 4) out;

in gl_PerVertex
{
	float gl_ClipDistance[2];
	float gl_CullDistance[1];
} gl_in[];

out gl_PerVertex
{
	float gl_ClipDistance[2];
	float gl_CullDistance[1];
} gl_out[];

void main()
{
	gl_out[gl_InvocationID].gl_ClipDistance[0] = gl_in[gl_InvocationID].gl_ClipDistance[0];
	gl_out[gl_InvocationID].gl_ClipDistance[1] = gl_in[gl_InvocationID].gl_ClipDistance[1];
	gl_out[gl_InvocationID].gl_CullDistance[0] = gl_in[gl_InvocationID].gl_CullDistance[0];
}
