#version 320 es

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;

layout(location = 0) out mediump vec3 vNormal;
layout(location = 1) out mediump vec3 vLightDirection;

layout (std140, set = 0, binding = 0) uniform DynamicModel
{
	highp mat4 uModelViewMatrix;
	highp mat4 uModelViewProjectionMatrix;
	highp mat3 uModelViewITMatrix;
	highp vec3 uLightPosition;
};

void main()
{
	highp vec4 position = (uModelViewMatrix * vec4(inVertex, 1.0));
	gl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);
	vNormal = uModelViewITMatrix * inNormal;
	vLightDirection = uLightPosition - position.xyz;
}
