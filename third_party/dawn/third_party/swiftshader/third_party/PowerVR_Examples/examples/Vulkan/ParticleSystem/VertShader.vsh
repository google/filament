#version 320 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;

layout(location = 0) out mediump vec3 vNormal;
layout(location = 1) out mediump vec3 vLightDirection;

layout (std140,set = 0, binding = 0) uniform DynamicModel
{
	highp mat4 uModelViewMatrix;
	highp mat4 uModelViewProjectionMatrix;
	highp mat3 uModelViewITMatrix;
};

layout (std140, set = 1, binding = 0) uniform Static
{
	highp vec3 uLightPosition;
};

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);
	vNormal = mat3(uModelViewITMatrix) * inNormal;

	highp vec3 position = (uModelViewMatrix * vec4(inVertex, 1.0)).xyz;
	vLightDirection = uLightPosition - position;
}
