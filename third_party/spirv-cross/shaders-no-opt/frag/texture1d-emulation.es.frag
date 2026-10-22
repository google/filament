#version 450

layout(set = 0, binding = 0) uniform sampler1D uSamp;
layout(set = 0, binding = 1) uniform sampler1DShadow uSampShadow;
layout(set = 0, binding = 2) uniform sampler1DArray uSampArray;
layout(set = 0, binding = 3) uniform sampler1DArrayShadow uSampArrayShadow;
layout(set = 0, binding = 4, r32f) uniform image1D uImage;
layout(location = 0) in vec4 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
	// 1D
	FragColor = texture(uSamp, vUV.x);
	FragColor += textureProj(uSamp, vUV.xy);
	FragColor += texelFetch(uSamp, int(vUV.x), 0);

	// 1D Shadow
	FragColor += texture(uSampShadow, vUV.xyz);
	FragColor += textureProj(uSampShadow, vUV);

	// 1D Array
	FragColor = texture(uSampArray, vUV.xy);
	FragColor += texelFetch(uSampArray, ivec2(vUV.xy), 0);

	// 1D Array Shadow
	FragColor += texture(uSampArrayShadow, vUV.xyz);

	// 1D images
	FragColor += imageLoad(uImage, int(vUV.x));
	imageStore(uImage, int(vUV.x), FragColor);
}
