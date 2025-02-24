#version 310 es

layout(binding = 0) uniform mediump samplerCube skybox;

layout(location = 0) in mediump vec3 rayDirection;

layout(location = 0) out mediump vec4 oColor;
layout(location = 1) out mediump float oFilter;

layout(location = 0) uniform mediump float linearExposure;
layout(location = 1) uniform mediump float threshold;

mediump float luma(mediump vec3 color)
{
	return dot(vec3(0.2126, 0.7152, 0.0722), color);
}

void main()
{
	// Sample skybox cube map
	oColor = texture(skybox, rayDirection);

	// Calculate an exposure value
	// linearExposure = keyValue / averageLuminance
	mediump float exposure = log2(max(linearExposure, 0.0001f));

	// Reduce by the threshold value
	exposure -= threshold;

	// Apply the exposure value
	exposure = exp2(exposure);
	mediump float luminance = luma(exposure * oColor.rgb);
	oFilter = mix(0.0, luminance, dot(luminance, 1.0/3.0) > 0.001);
}