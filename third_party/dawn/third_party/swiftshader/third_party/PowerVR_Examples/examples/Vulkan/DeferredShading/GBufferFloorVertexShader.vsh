#version 320 es

#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoords;

layout(set = 1, binding = 1) uniform DynamicsPerModel
{
	highp mat4 mWorldViewProjectionMatrix;
	highp mat4 mWorldViewMatrix;
	highp mat4 mWorldViewITMatrix;
};

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out mediump vec3 vNormal;
layout(location = 2) out highp vec3 vViewPosition;

void main() 
{
	gl_Position = mWorldViewProjectionMatrix * vec4(inVertex, 1.0);

	// Transform normal from model space to eye space
	vNormal = mat3(mWorldViewITMatrix) * inNormal;

	// Pass the vertex position in view space for depth calculations
	vViewPosition = (mWorldViewMatrix * vec4(inVertex, 1.0)).xyz;

	// Pass the texture coordinates to the fragment shader
	vTexCoord = inTexCoords;
}