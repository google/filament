#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;

uniform highp mat4 uModelViewMatrix;
uniform highp mat3 uModelViewITMatrix;
uniform highp mat4 uModelViewProjectionMatrix;

uniform highp vec3 uLightPosition;

out mediump vec3 vNormal;
out mediump vec3 vLightDirection;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(inVertex, 1.0);
	vNormal = uModelViewITMatrix * inNormal;

	highp vec3 position = (uModelViewMatrix * vec4(inVertex, 1.0)).xyz;
	vLightDirection = uLightPosition - position;
}
