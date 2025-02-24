#version 310 es

layout(binding = 0) uniform mediump sampler2D sCurrentBlurredImage;
layout(binding = 1) uniform mediump sampler2D sDownsampledCurrentMipLevel;
layout(binding = 2) uniform mediump sampler2D sOffScreenTexture;
layout(location = 0) in mediump vec2 vTexCoords[9];
layout(location = 0) out mediump vec4 oColor;

const mediump float weights[9] = float[9](0.25, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125, 0.0625, 0.125);

// Radius for our vignette
const mediump float VignetteRadius = 0.85;

// Soft for our vignette, between 0.0 and 1.0
const mediump float VignetteSoftness = 0.35;

uniform mediump float linearExposure;

void main()
{	
	highp float blurredColor = texture(sDownsampledCurrentMipLevel, vTexCoords[0]).r * weights[0];
	
	for(int i = 0; i < 9; ++i)
	{
		blurredColor += texture(sCurrentBlurredImage, vTexCoords[i]).r * weights[i];
	}

	highp vec3 hdrColor;

#ifdef RENDER_BLOOM
	hdrColor = vec3(blurredColor);
#else
	mediump vec3 offscreenTexture = texture(sOffScreenTexture, vTexCoords[0]).rgb;

	hdrColor = offscreenTexture * linearExposure + vec3(blurredColor);
#endif

	// http://filmicworlds.com/blog/filmic-tonemapping-operators/
	// Reinhard tonemapping
	mediump vec3 ldrColor = hdrColor / (1.0 + hdrColor);

	// apply a simple vignette
	mediump vec2 vtc = vec2(vTexCoords[8] - vec2(0.5));
	// determine the vector length of the center position
	mediump float lenPos = length(vtc);
	mediump float vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteSoftness, lenPos);

	mediump vec3 color = ldrColor * vignette;
#ifndef FRAMEBUFFER_SRGB
	color = pow(color, vec3(0.4545454545)); // Do gamma correction
#endif	

	oColor = vec4(color, 1.0);
}