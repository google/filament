#version 450
layout(quads) in;

in gl_PerVertex
{
	float gl_ClipDistance[2];
	float gl_CullDistance[3];
	vec4 gl_Position;
} gl_in[];

void main()
{
	gl_Position.x = gl_in[0].gl_ClipDistance[0];
	gl_Position.y = gl_in[1].gl_CullDistance[0];
	gl_Position.z = gl_in[0].gl_ClipDistance[1];
	gl_Position.w = gl_in[1].gl_CullDistance[1];
	gl_Position += gl_in[0].gl_Position;
	gl_Position += gl_in[1].gl_Position;
}
