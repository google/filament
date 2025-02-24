#version 320 es

layout(location = 0) in highp vec3 inVertex;

layout(push_constant) uniform PushConsts
{
	highp mat4 MVPMatrix;
	mediump float WindowWidth;
}pushConstant;

layout(location = 0) out highp vec4 pos;

void main()
{
	// Transform position
	gl_Position = pushConstant.MVPMatrix * vec4(inVertex, 1.0);
	pos = gl_Position;
}