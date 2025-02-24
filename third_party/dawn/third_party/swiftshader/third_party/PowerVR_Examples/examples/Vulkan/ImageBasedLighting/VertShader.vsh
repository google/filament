#version 320 es

layout(constant_id = 0) const bool HAS_MATERIAL_TEXTURES = false;

layout(location = 0) in highp vec3 inVertex;
layout(location = 1) in mediump vec3 inNormal;
layout(location = 2) in mediump vec2 inTexCoord;
layout(location = 3) in mediump vec4 inTangent;

layout(std140, set = 0, binding = 0) uniform Dynamics
{
	highp mat4 VPMatrix;
	highp vec3 camPos;
	mediump float emissiveIntensity;
	mediump float exposure;
};

#define MODEL_MAT4_ARRAY_SIZE 25
layout(std140, set = 0, binding = 1) uniform Model
{
	highp mat4 modelMatrix; 
};

layout(location = 0) out highp vec3 outWorldPos;
layout(location = 1) out mediump vec3 outNormal;
layout(location = 2) flat out mediump int outInstanceIndex;

// Material textures
layout(location = 3) out mediump vec2 outTexCoord;
layout(location = 4) out mediump vec3 outTangent;
layout(location = 5) out mediump vec3 outBitTangent;

void main()
{
	highp vec4 posTmp = modelMatrix * vec4(inVertex, 1.0);

	outInstanceIndex = gl_InstanceIndex;

	if (HAS_MATERIAL_TEXTURES)
	{
		// Helmet - Uses tangent space and material textures
		outTexCoord = inTexCoord;
		outTangent = inTangent.xyz;
		outBitTangent = cross(inNormal, inTangent.xyz) * inTangent.w;
	}
	else
	{
		// Sphere - Uses instancing to modify the world position, so as to create the grid
		highp float x = -float(gl_InstanceIndex % 6) * 10.0 + 25.;
		highp float y = -float(gl_InstanceIndex / 6) * 10.0 + 15.;
		posTmp.xy += vec2(x, y);
	}

	outNormal = normalize(transpose(inverse(mat3(modelMatrix))) * inNormal);
	outWorldPos = posTmp.xyz;
	gl_Position = VPMatrix * posTmp;
}
