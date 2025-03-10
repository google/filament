#version 450
#extension GL_EXT_mesh_shader : require

layout(location = 0) perprimitiveEXT flat in uvec4 v;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(v);
}
