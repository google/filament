#version	450 
#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2
#define TANGENT_ARRAY	3

layout(location = VERTEX_ARRAY) in highp vec4 inVertex;
layout(location = NORMAL_ARRAY) in mediump vec3 inNormal;
layout(location = TEXCOORD_ARRAY) in mediump vec2 inTexCoord;
layout(location = TANGENT_ARRAY) in mediump vec3 inTangent;

layout (set = 1, binding = 0)uniform PerMesh
{
	highp mat4 MVPMatrix; // model view projection transformation
	mediump vec3 LightDirModel;	// Light position (point light) in model space
};
layout(location = 0) out mediump vec3 LightVec;
layout(location = 1) out mediump vec2 TexCoord;
void main()
{
	// Transform position
	gl_Position = MVPMatrix * inVertex;
	highp vec3 lightDirection = normalize(LightDirModel);
	// transform light direction from model space to tangent space
	mediump vec3 bitangent = cross(inNormal, inTangent);
	mediump mat3 tangentSpaceXform = mat3(inTangent, bitangent, inNormal);
	LightVec = lightDirection * tangentSpaceXform;
	TexCoord = inTexCoord;
}
