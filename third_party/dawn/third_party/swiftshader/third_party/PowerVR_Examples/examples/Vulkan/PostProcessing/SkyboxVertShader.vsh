#version 320 es

layout(location = 0) out mediump vec3 rayDirection;

layout(std140, set = 0, binding = 1) uniform SceneBuffer
{
	highp mat4 inverseViewProjectionMatrix;
	mediump vec3 eyePosition;
};

void main()
{
	const mediump vec3 positions[6] = vec3[]
	(
		vec3(-1.0f, 1.0f, 1.0f),	// top left
		vec3(-1.0f, -1.0f, 1.0f),	// bottom left
		vec3(1.0f, 1.0f, 1.0f),		// top right
		vec3(1.0f, 1.0f, 1.0f),		// top right
		vec3(-1.0f, -1.0f, 1.0f),	// bottom left
		vec3(1.0f, -1.0f, 1.0f)		// bottom right
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	gl_Position.y = -gl_Position.y;

	highp vec4 worldPosition = inverseViewProjectionMatrix * gl_Position;
	worldPosition.xyz /= worldPosition.w;

	// Calculate ray direction
	rayDirection = normalize(worldPosition.xyz - vec3(eyePosition));
}