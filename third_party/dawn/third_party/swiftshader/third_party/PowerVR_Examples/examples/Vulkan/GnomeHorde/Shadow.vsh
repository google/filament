#version 320 es

layout(set = 1, binding = 0) uniform _per_scene
{
	mediump vec4 directionToLight;
} perScene;
layout(set = 2, binding = 0) uniform _per_object
{
	highp mat4 modelMatrix;
	highp mat4 modelMatrixIT;
} perObject;
layout(set = 3, binding = 0) uniform _per_frame
{
	highp mat4 viewProjectionMatrix;
} perFrame;

layout(location = 0) in highp vec3 inVertexPosition;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoords;

layout(location = 0) out mediump vec2 uvs;

void main()
{
	uvs = inTexCoords;

	gl_Position = perFrame.viewProjectionMatrix * perObject.modelMatrix * vec4(inVertexPosition, 1.0);
}

