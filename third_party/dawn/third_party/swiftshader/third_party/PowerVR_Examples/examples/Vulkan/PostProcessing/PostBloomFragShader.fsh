#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;

layout(set = 0, binding = 0) uniform mediump sampler2D sBlurTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sOffScreenTexture;

layout(location = 0) in mediump vec2 vTexCoords;

layout(location = 0) out mediump vec4 oColor;

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

layout(push_constant) uniform pushConstantsBlock{
	mediump float linearExposure;
};

void main()
{
	highp vec3 hdrColor;

	if(RenderBloom == 1)
	{
		hdrColor = vec3(texture(sBlurTexture, vTexCoords).r);
	}
	else
	{
		// Retrieve the two hdr colour attachments and combine them
		highp vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords).rgb;
		highp vec3 bloomTexture = vec3(texture(sBlurTexture, vTexCoords).r);
		hdrColor = offscreenTexture * linearExposure + bloomTexture;
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tone mapping
	mediump vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords - vec2(0.5));
	// determine the vector length of the centre position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	oColor = vec4(ldrColor * vignette, 1.0);
}
