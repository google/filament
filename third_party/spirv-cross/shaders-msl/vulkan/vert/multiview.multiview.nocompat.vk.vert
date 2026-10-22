#version 310 es
#extension GL_EXT_multiview : require

layout(std140, binding = 0) uniform MVPs
{
	mat4 MVP[2];
};

layout(location = 0) in vec4 Position;

void main()
{
	gl_Position = MVP[gl_ViewIndex] * Position;
}
