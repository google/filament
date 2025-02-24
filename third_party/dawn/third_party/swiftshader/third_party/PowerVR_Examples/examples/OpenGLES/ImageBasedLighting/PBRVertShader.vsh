#version 310 es
layout(location = 0)in highp   vec3 inVertex;
layout(location = 1)in highp   vec3 inNormal;
layout(location = 2)in mediump vec2 inTexCoord;
layout(location = 3)in highp   vec4 inTangent;

layout(std140, binding = 1) uniform UboDynamic
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
	mediump float emissiveIntensity;
	mediump float exposure;
} uboDynamic;

layout(std140, binding = 2) uniform UboPerModel
{
	highp mat4 modelMatrix;
} uboPerModel;

layout(location = 0) out highp vec3 outWorldPos;
layout(location = 1) out mediump vec3 outNormal;
layout(location = 2) flat out mediump int outInstanceIndex;

// Material textures
layout(location = 3) out mediump vec2 outTexCoord;
layout(location = 4) out mediump vec3 outTangent;
layout(location = 5) out mediump vec3 outBitTangent;

void main()
{
	highp vec4 posTmp = uboPerModel.modelMatrix * vec4(inVertex, 1.0);
	outNormal = normalize(transpose(inverse(mat3(uboPerModel.modelMatrix))) * inNormal);

	outInstanceIndex = gl_InstanceID;

#ifdef MATERIAL_TEXTURES
	// Helmet - Uses tangent space and material textures
	outTexCoord = inTexCoord;
	outTangent = inTangent.xyz;
	outBitTangent = cross(inNormal, inTangent.xyz) * inTangent.w;
#else
	// Sphere - Uses instancing to modify the world position, so as to create the grid
	highp float x = -float(gl_InstanceID % 6) * 10.0 + 25.;
	highp float y = -float(gl_InstanceID / 6) * 10.0 + 15.;
	posTmp.xy += vec2(x, y);
#endif

	outWorldPos = posTmp.xyz;
	gl_Position = uboDynamic.VPMatrix * posTmp;
}
