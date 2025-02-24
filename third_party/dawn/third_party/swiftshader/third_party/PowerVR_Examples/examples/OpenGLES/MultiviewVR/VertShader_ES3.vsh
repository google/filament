#version 300 es
#define NUM_VIEWS 4
#extension GL_OVR_multiview : enable
layout(num_views = NUM_VIEWS) in;

in highp vec3 inVertex;
in mediump vec3 inNormal;
in mediump vec2 inTexCoord;

uniform highp mat4 MVPMatrix[NUM_VIEWS];
uniform highp mat4 WorldMatrix;

out mediump vec2 TexCoord;
out mediump vec3 worldNormal;

void main()
{
	// Transform position
	gl_Position = MVPMatrix[gl_ViewID_OVR] * vec4(inVertex, 1.0);
	worldNormal = normalize(mat3(WorldMatrix) * inNormal);
	// Pass through texcoords
	TexCoord = inTexCoord;
}