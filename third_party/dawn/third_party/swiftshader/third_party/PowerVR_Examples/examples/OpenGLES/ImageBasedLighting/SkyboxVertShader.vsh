#version 310 es

layout(location = 0) in highp vec3 inVertex;

layout(location = 0) out mediump vec3 outRayDir;

layout(std140, binding = 0) uniform Dynamic
{
	highp mat4 InvVPMatrix;
	mediump vec3 EyePos;
}dynamicUbo;

void main()
{
	const mediump vec3 positions[6]= vec3[]
	(
		vec3(-1.0f, 1.0f, 1.0f),// top left
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(1.0f, 1.0f, 1.0f),// top right
		vec3(-1.0f, -1.0f, 1.0f),// bottom left
		vec3(1.0f, -1.0f, 1.0f)// bottom right
	);

	// Set position
	gl_Position = vec4(positions[gl_VertexID], 1.0);

	mediump vec4 WorldPos = dynamicUbo.InvVPMatrix * gl_Position;
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	outRayDir = normalize(WorldPos.xyz - vec3(dynamicUbo.EyePos));
}
