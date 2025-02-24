#version 310 es

/*
	If the current vertex is affected by bones then the vertex position and
	normal will be transformed by the bone matrices. Each vertex wil have up
	to 4 bone indices (inBoneIndex) and bone weights (inBoneWeights).

	The indices are used to index into the array of bone matrices
	(BoneMatrixArray) to get the required bone matrix for transformation. The
	amount of influence a particular bone has on a vertex is determined by the
	weights which should always total 1. So if a vertex is affected by 2 bones
	the vertex position in world space is given by the following equation:

	position = (BoneMatrixArray[Index0] * inVertex) * Weight0 +
	           (BoneMatrixArray[Index1] * inVertex) * Weight1

	The same proceedure is applied to the normals but the translation part of
	the transformation is ignored.

	After this the position is multiplied by the view and projection matrices
	only as the bone matrices already contain the model transform for this
	particular mesh. The two-step transformation is required because lighting
	will not work properly in clip space.
*/

in highp vec3 inVertex;
in mediump vec3 inNormal;
in mediump vec3 inTangent;
in mediump vec3 inBiNormal;
in mediump vec2 inTexCoord;
in highp vec4 inBoneWeights;
in highp vec4 inBoneIndex;

struct Bone{
	highp mat4 boneMatrix;
	highp mat3 boneMatrixIT;
};

layout (std140, binding = 0) uniform MyUBlock
{
	highp mat4 ViewProjMatrix;
	highp vec3 LightPos;
};

layout (std140, binding = 0) buffer MyBBlock
{
    Bone bones[];
};

out highp vec3 vLight;
out mediump vec2 vTexCoord;

out highp vec3 worldPosition;
out mediump float vOneOverAttenuation;

void main()
{
	// On PowerVR GPUs it is possible to index the components of a vector
	// with the [] operator. However this can cause trouble with PC
	// emulation on some hardware so we "rotate" the vectors instead.
	mediump ivec4 boneIndex = ivec4(inBoneIndex);
	mediump vec4 boneWeights = inBoneWeights;

	mediump vec3 worldTangent = vec3(0, 0, 0);
	mediump vec3 worldBiNormal = vec3(0, 0, 0);

	highp vec4 position = vec4(0, 0, 0, 0);
	mediump vec3 worldNormal = vec3(0, 0, 0);

	for (mediump int i = 0; i < 4; ++i)
	{
		Bone b = bones[boneIndex.x];
		position += b.boneMatrix * vec4(inVertex, 1.0) * boneWeights.x;
		worldNormal += b.boneMatrixIT  * inNormal * boneWeights.x;

		worldTangent += b.boneMatrixIT * inTangent * boneWeights.x;
		worldBiNormal += b.boneMatrixIT * inBiNormal * boneWeights.x;

		// "rotate" the vector components
		boneIndex = boneIndex.yzwx;
		boneWeights = boneWeights.yzwx;
	}

	worldPosition = position.xyz / position.w;

	gl_Position = ViewProjMatrix * position;

	// lighting
	mediump vec3 tmpLightDir = LightPos - position.xyz;
	mediump float light_distance = length(tmpLightDir);
	tmpLightDir /= light_distance;

	vOneOverAttenuation = 1.0 / (1.0 + 0.00005 * light_distance * light_distance);

	vLight.x = dot(normalize(worldTangent), tmpLightDir);
	vLight.y = dot(normalize(worldBiNormal), tmpLightDir);
	vLight.z = dot(normalize(worldNormal), tmpLightDir);

	// Pass through texcoords
	vTexCoord = inTexCoord;

}
