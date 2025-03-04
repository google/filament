#version 450
layout(quads) in;

vec4 read_patch_vertices()
{
	return vec4(gl_PatchVerticesIn, 0, 0, 1);
}

void main()
{
	gl_Position = read_patch_vertices();
}
