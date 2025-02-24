#version 320 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2
#define TANGENT_ARRAY	3

layout(location = VERTEX_ARRAY) in highp vec3 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;
layout(location = TANGENT_ARRAY) in mediump vec3 inTangent;

layout(set = 0, binding = 3) uniform PerMesh
{
	highp mat4 mvpMatrix;
	highp mat4 worldMatrix;
};

layout(location = 0) out mediump vec2 vTexCoord;
layout(location = 1) out highp vec3 worldPosition;
layout(location = 2) out highp mat3 TBN_worldSpace;

void main()
{
	// Transform position
	gl_Position = mvpMatrix * vec4(inVertex, 1.0);
	vTexCoord = inTexCoord;

	highp vec3 T = normalize(worldMatrix * vec4(inTangent, 0.0)).xyz;
	highp vec3 N = normalize(worldMatrix * vec4(inNormal, 0.0)).xyz;
	highp vec3 B = cross(T, N);
	TBN_worldSpace = mat3(T, B, N);

	worldPosition = (worldMatrix * vec4(inVertex, 1.0)).xyz;
}