#version 310 es

layout(binding = 0) uniform mediump sampler2D sBlurTexture;
layout(binding = 1) uniform mediump sampler2D sOffScreenTexture;
layout(location = 0) in mediump vec2 vTexCoords[9];

layout(location = 0) out mediump vec4 oColor;

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

uniform mediump float linearExposure;

void main()
{
	highp float blurredColor = texture(sBlurTexture, vTexCoords[0]).r;
	blurredColor += texture(sBlurTexture, vTexCoords[1]).r * 2.0;
	blurredColor += texture(sBlurTexture, vTexCoords[2]).r;
	blurredColor += texture(sBlurTexture, vTexCoords[3]).r * 2.0;
	blurredColor += texture(sBlurTexture, vTexCoords[4]).r;
	blurredColor += texture(sBlurTexture, vTexCoords[5]).r * 2.0;
	blurredColor += texture(sBlurTexture, vTexCoords[6]).r;
	blurredColor += texture(sBlurTexture, vTexCoords[7]).r * 2.0;
	blurredColor *= 0.08333333333333333333333333333333;

	highp vec3 hdrColor;

#ifdef RENDER_BLOOM
	hdrColor = vec3(blurredColor);
#else
	highp vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[8]).rgb;

	hdrColor = offscreenTexture * linearExposure + vec3(blurredColor);
#endif

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords[0] - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	mediump vec3 color = ldrColor * vignette;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif	

	oColor = vec4(color, 1.0);
}