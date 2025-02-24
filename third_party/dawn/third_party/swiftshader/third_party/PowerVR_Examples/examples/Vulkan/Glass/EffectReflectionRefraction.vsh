#version 320 es

const mediump float Eta = 0.87;
const mediump float FresnelBias = 0.3;
const mediump float FresnelScale = 0.7;
const mediump float FresnelPower = 1.5;

layout(std140, set = 0, binding = 0) uniform Buffer
{
	highp mat4 MVPMatrix;
	mediump mat3 MMatrix;
	highp vec4 EyePos;
};

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;

layout(location = 0) out highp vec3 ReflectDir;
layout(location = 1) out highp vec3 RefractDir;
layout(location = 2) out highp float ReflectFactor;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Calculate view direction in model space
	mediump vec3 ViewDir = normalize(inVertex - vec3(EyePos));

	// Reflect view direction and transform to world space
	ReflectDir = MMatrix * reflect(ViewDir, inNormal);

	RefractDir = MMatrix * refract(ViewDir, inNormal, Eta);

	// Calculate the reflection factor
	ReflectFactor = FresnelBias + FresnelScale * pow(1.0 + dot(ViewDir, inNormal), FresnelPower);
	ReflectFactor = clamp(ReflectFactor, 0.0, 1.0);
}
