#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D sBaseTex;
layout(set = 0, binding = 1) uniform mediump sampler2D sNormalMap;
layout(set = 0, binding = 2) uniform mediump samplerCube irradianceMap;

layout(push_constant) uniform pushConstantsBlock{
	mediump float linearExposure;
	mediump float threshold;
};

layout(location = 0) in mediump vec2 vTexCoord;
layout(location = 1) in highp vec3 worldPosition;
layout(location = 2) in highp mat3 TBN_worldSpace;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

mediump float luma(mediump vec3 color)
{
	return max(dot(color, vec3(0.2126, 0.7152, 0.0722)), 0.0001);
}

void main()
{
	// read the per-pixel normal from the normal map and expand to [-1, 1]
	mediump vec3 normal = texture(sNormalMap, vTexCoord).rgb * 2.0 - 1.0;
	mediump vec3 worldSpaceNormal = normalize(TBN_worldSpace * normal);

	mediump vec3 texColor = texture(sBaseTex, vTexCoord).rgb;
	mediump vec3 diffuseIrradiance = texColor * texture(irradianceMap, worldSpaceNormal).rgb;
	
	oColor = vec4(diffuseIrradiance, 1.0);

	// Calculate an exposure value
	// linearExposure = keyValue / averageLuminance
	mediump float exposure = log2(max(linearExposure, 0.0001f));

	// Reduce by the threshold value
	exposure -= threshold;

	// Apply the exposure value
	exposure = exp2(exposure);
	oFilter = luma(exposure * oColor.rgb);;
}