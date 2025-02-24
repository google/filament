#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in highp vec2	inTexCoord;

uniform highp mat4 MVPMatrix;
uniform highp mat3 MVITMatrix;

out mediump vec3 ViewNormal;
out mediump vec2 TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;

	//View space coordinates to calculate the light.
	ViewNormal = MVITMatrix * inNormal;
	TexCoord = inTexCoord;
}