#version 450

layout(set = 0, binding = 0) uniform sampler1D uSamp;
layout(set = 0, binding = 1) uniform sampler1DShadow uSampShadow;
layout(location = 0) in vec4 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
	// 1D
	FragColor = texture(uSamp, vUV.x);
	FragColor += textureProj(uSamp, vUV.xy);

	// 1D Shadow
	FragColor += texture(uSampShadow, vUV.xyz);
	FragColor += textureProj(uSampShadow, vUV);
}
