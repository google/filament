attribute highp vec4 inVertex;
attribute mediump vec3 inNormal;
attribute highp vec2 inTexCoord;

uniform highp mat4 mVPMatrix;
uniform highp mat3 mVITMatrix;

varying mediump vec3 viewNormal;
varying mediump vec2 texCoord;

void main()
{
	gl_Position = mVPMatrix * inVertex;

	//View space coordinates to calculate the light.
	viewNormal = mVITMatrix * inNormal;
	texCoord = inTexCoord;
}