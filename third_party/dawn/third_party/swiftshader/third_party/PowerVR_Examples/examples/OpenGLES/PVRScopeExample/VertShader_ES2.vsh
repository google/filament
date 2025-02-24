attribute highp vec4 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec2 inTexCoord;

uniform highp mat4 MVPMatrix;
uniform highp mat3 MVITMatrix;

varying mediump vec3 ViewNormal;
varying mediump vec2 TexCoord;

void main()
{
	gl_Position = MVPMatrix * inVertex;

	//View space coordinates to calculate the light.
	ViewNormal = MVITMatrix * inNormal;
	TexCoord = inTexCoord;
}