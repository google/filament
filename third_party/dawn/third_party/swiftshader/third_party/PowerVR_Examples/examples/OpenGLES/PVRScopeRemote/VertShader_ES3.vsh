#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;

uniform highp mat4 mVPMatrix;
uniform highp mat3 mVITMatrix;

out mediump vec3 viewNormal;
out mediump vec2 texCoord;

void main()
{
	gl_Position = mVPMatrix * inVertex;

	//View space coordinates to calculate the light.
	viewNormal = mVITMatrix * inNormal;
	texCoord = inTexCoord;
}