#version 320 es

#define VERTEX_ARRAY 0

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;

layout(set = 1, binding = 1) uniform DynamicsPerPointLight
{
	highp mat4 mWorldViewProjectionMatrix;
	highp vec4 vViewPosition;
	highp mat4 mProxyWorldViewProjectionMatrix;
	highp mat4 mProxyWorldViewMatrix;
};

layout(location = 0) out highp vec3 vPositionVS;
layout(location = 1) out mediump vec3 vViewDirVS;

void main()
{
	gl_Position = mProxyWorldViewProjectionMatrix * vec4(inVertex, 1.0);
	gl_Position.xyz = gl_Position.xyz / gl_Position.w;
	gl_Position.w = 1.0;

	// Calculate the view-space position for lighting calculations
	vPositionVS = (mProxyWorldViewMatrix * vec4(inVertex, 1.0)).xyz;

	// Pass the view direction
	vViewDirVS = vPositionVS;
}