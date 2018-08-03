#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 1) in VertexOut
{
	vec4 a;
	vec4 b;
};

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = a + b;
}
