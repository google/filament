#version 320 es

layout(location = 0) out mediump vec3 RayDir;

layout(std140, set = 0, binding = 1) uniform Dynamic
{
	highp mat4 InvVPMatrix;
	highp vec4 EyePos;
	float exposure;
};

void main()
{
		const highp vec3 quadVertices[6] = vec3[6](
		vec3(-1., -1., 1.f), // upper left
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1., -1., 1.f), // upper right
		vec3( 1., -1., 1.f), // upper right
		vec3(-1.,  1., 1.f), // lower left
		vec3( 1.,  1., 1.f) // lower right
	);
	
	highp vec3 inVertex = quadVertices[gl_VertexIndex];

	// Set position
	gl_Position = vec4(inVertex, 1.0);

	// Calculate world space vertex position
	highp vec4 pos = gl_Position;

	vec4 WorldPos = InvVPMatrix * pos;
	// flip the y here to convert from vulkan +Y down coordinate to OpenGL +Y up.
	WorldPos /= WorldPos.w;

	// Calculate ray direction
	RayDir = normalize(WorldPos.xyz - vec3(EyePos));
}
