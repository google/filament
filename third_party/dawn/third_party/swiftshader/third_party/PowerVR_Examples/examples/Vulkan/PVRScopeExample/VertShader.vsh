#version 320 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;


layout(std140, set = 0, binding = 0) uniform Dynamic
{
   highp mat4 MVPMatrix;
   highp mat3 MVITMatrix;
};

layout(location = 0)out mediump vec3 ViewNormal;
layout(location = 1)out mediump vec2 TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;

	//View space coordinates to calculate the light.
	ViewNormal = MVITMatrix * inNormal;
	TexCoord = inTexCoord;
}
