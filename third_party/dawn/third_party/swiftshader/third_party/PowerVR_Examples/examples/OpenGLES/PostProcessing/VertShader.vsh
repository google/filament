#version 310 es

#define VERTEX_ARRAY 0
#define NORMAL_ARRAY 1
#define TEXCOORD_ARRAY 2
#define TANGENT_ARRAY 3

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;
layout(location = 3) in mediump vec3 inTangent;

layout(std140, binding = 0) uniform PerMesh
{
	highp mat4 mvpMatrix;
	highp mat4 worldMatrix;
};

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out highp vec3 T;
layout(location = 2) out highp vec3 B;
layout(location = 3) out highp vec3 N;

void main()
{
	// Transform position
	gl_Position = mvpMatrix * vec4(inVertex, 1.0);
	vTexCoord = inTexCoord;

	// Note that we pass through T, B and N separately rather than constructing the matrix in the vertex shader to work around
	// an issue we encountered with a desktop compiler.
	T = normalize(worldMatrix * vec4(inTangent, 0.0)).xyz;
	N = normalize(worldMatrix * vec4(inNormal, 0.0)).xyz;
	B = cross(T, N);
}