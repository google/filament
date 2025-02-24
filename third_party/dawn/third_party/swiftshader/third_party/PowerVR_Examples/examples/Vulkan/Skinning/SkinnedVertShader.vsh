#version 320 es

/*
	If the current vertex is affected by bones then the vertex position and
	normal will be transformed by the bone matrices. Each vertex will have up
	to 4 bone indices (inBoneIndex) and bone weights (inBoneWeights).

	The indices are used to index into the array of bone matrices
	(BoneMatrixArray) to get the required bone matrix for transformation. The
	amount of influence a particular bone has on a vertex is determined by the
	weights which should always total 1. So if a vertex is affected by 2 bones
	the vertex position in world space is given by the following equation:

	position = (BoneMatrixArray[Index0] * inVertex) * Weight0 +
	           (BoneMatrixArray[Index1] * inVertex) * Weight1

	The same procedure is applied to the normals but the translation part of
	the transformation is ignored.

	After this the position is multiplied by the view and projection matrices
	only as the bone matrices already contain the model transform for this
	particular mesh. The two-step transformation is required because lighting
	will not work properly in clip space.
*/

#define MAX_BONE_COUNT 8

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec3 inTangent;
layout(location = 3) in mediump vec3 inBiNormal;
layout(location = 4) in mediump vec2 inTexCoord;
layout(location = 5) in mediump vec4 inBoneWeights;
layout(location = 6) in uvec4 inBoneIndex;

// per frame / per mesh
layout(std140,set = 1, binding = 0) uniform Dynamics
{
	mat4 BoneMatrixArray[24];
    mat3x3 BoneMatrixArrayIT[24];
    int BoneCount;
};

// static throughout the lifetime
layout(std140,set = 2, binding = 0) uniform Statics
{
    highp mat4 ViewProjMatrix;
    highp vec3 LightPos;
};

layout(location = 0) out mediump vec3 vLight;
layout(location = 1) out mediump vec2 vTexCoord;
layout(location = 2) out mediump float vOneOverAttenuation;

void main()
{
	// On PowerVR GPUs it is possible to index the components of a vector
	// with the [] operator. However this can cause trouble with PC
	// emulation on some hardware so we "rotate" the vectors instead.
	mediump uvec4 boneIndex = uvec4(inBoneIndex);

	mediump vec4 boneWeights = inBoneWeights;

	highp mat4 boneMatrix;
	mediump mat3 normalMatrix;
	
	mediump vec3 worldTangent = vec3(0,0,0);
	mediump vec3 worldBiNormal = vec3(0,0,0);
	
	highp vec4 position = vec4(0,0,0,0);
	mediump vec3 worldNormal = vec3(0,0,0);
	
	for (mediump int i = 0; i < BoneCount; ++i)
	{
		boneMatrix = BoneMatrixArray[boneIndex.x];
		normalMatrix = BoneMatrixArrayIT[boneIndex.x];

		position += boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		worldNormal += normalMatrix * inNormal * boneWeights.x;

		worldTangent += normalMatrix * inTangent * boneWeights.x;
		worldBiNormal += normalMatrix * inBiNormal * boneWeights.x;

		// "rotate" the vector components
		boneIndex = boneIndex.yzwx;
		boneWeights = boneWeights.yzwx;
	}
	
	gl_Position = ViewProjMatrix * position;

	// lighting
	mediump vec3 tmpLightDir = LightPos - position.xyz;
	mediump float light_distance = length(tmpLightDir);
	tmpLightDir /= light_distance;

	vLight.x = dot(normalize(worldTangent), tmpLightDir);
	vLight.y = dot(normalize(worldBiNormal), tmpLightDir);
	vLight.z = dot(normalize(worldNormal), tmpLightDir);

	vOneOverAttenuation = 1.0 / (1.0 + 0.00005 * light_distance * light_distance);

	// Pass through texcoords
	vTexCoord = inTexCoord;
}
