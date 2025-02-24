#version 320 es

layout(constant_id = 0) const int RenderBloom = 0;

layout(set = 0, binding = 0) uniform mediump sampler2D sTexture;
layout(set = 0, binding = 1) uniform mediump sampler2D sOffScreenTexture;
layout(location = 0) in mediump vec2 vTexCoords[9];

layout(location = 0) out mediump vec4 oColor;

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

layout(push_constant) uniform pushConstantsBlock{
	mediump vec2 blurConfigs[8];
	mediump float linearExposure;
};

void main()
{
	highp float blurredColor = texture(sTexture, vTexCoords[1]).r;
	blurredColor += texture(sTexture, vTexCoords[2]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[3]).r;
	blurredColor += texture(sTexture, vTexCoords[4]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[5]).r;
	blurredColor += texture(sTexture, vTexCoords[6]).r * 2.0;
	blurredColor += texture(sTexture, vTexCoords[7]).r;
	blurredColor += texture(sTexture, vTexCoords[8]).r * 2.0;
	blurredColor *= 0.08333333333333333333333333333333;

	highp vec3 hdrColor;

	if(RenderBloom == 1)
	{
		hdrColor = vec3(blurredColor);
	}
	else
	{
		// Retrieve the original hdr colour attachment and combine it with the blurred image
		highp vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[0]).rgb;

		hdrColor = offscreenTexture * linearExposure + vec3(blurredColor);
	}

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tone mapping
	mediump vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords[0] - vec2(0.5));
	// determine the vector length of the centre position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	oColor = vec4(ldrColor * vignette, 1.0);
}