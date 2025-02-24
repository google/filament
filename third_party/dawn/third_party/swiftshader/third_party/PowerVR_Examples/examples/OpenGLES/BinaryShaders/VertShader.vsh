attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec2 inTexCoord;

uniform highp mat4 WVPMatrix;
uniform mediump vec3 LightDirection;
uniform highp mat4 WorldViewIT;

varying mediump float LightIntensity;
varying mediump vec2 TexCoord;

void main()
{
	gl_Position = WVPMatrix * vec4(inVertex, 1.0);
	mediump vec3 Normals = normalize(mat3(WorldViewIT) * inNormal);
	LightIntensity = max(dot(Normals, -LightDirection), 0.0);
	TexCoord = inTexCoord;
}