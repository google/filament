#version 320 es
layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;

layout(std140, set = 1, binding = 0) uniform Dynamics
{
	highp mat4 WVPMatrix;
	highp mat3 WorldViewIT;
};

layout(std140, set = 2, binding = 0) uniform Statics
{
	mediump vec4 LightDirection;
};

layout(location = 0) out mediump float LightIntensity;
layout(location = 1) out mediump vec2 TexCoord;

void main()
{
	gl_Position = WVPMatrix * vec4(inVertex, 1.0);
	mediump vec3 Normals = normalize(WorldViewIT * inNormal);
	LightIntensity = max(dot(Normals, -LightDirection.xyz), 0.);
	TexCoord = inTexCoord;
}