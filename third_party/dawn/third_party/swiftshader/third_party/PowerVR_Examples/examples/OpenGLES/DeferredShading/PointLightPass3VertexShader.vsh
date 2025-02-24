#version 310 es

#define VERTEX_ARRAY 0

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;

layout(std140, binding = 0) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

void main()
{
	// pass-through position and texture coordinates
	gl_Position = mWorldViewProjectionMatrix * vec4(inVertex, 1.0);
}