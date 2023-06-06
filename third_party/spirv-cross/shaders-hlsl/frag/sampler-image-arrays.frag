#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in vec2 vTex;
layout(location = 1) flat in int vIndex;
layout(binding = 0) uniform sampler2D uSampler[4];
layout(binding = 4) uniform sampler uSamplers[4];
layout(binding = 8) uniform texture2D uTextures[4];

vec4 sample_from_argument(sampler2D samplers[4])
{
	return texture(samplers[vIndex], vTex + 0.2);
}

vec4 sample_single_from_argument(sampler2D samp)
{
	return texture(samp, vTex + 0.3);
}

vec4 sample_from_global()
{
	return texture(uSampler[vIndex], vTex + 0.1);
}

void main()
{
	FragColor = vec4(0.0);
	FragColor += texture(sampler2D(uTextures[2], uSamplers[1]), vTex);
	FragColor += texture(uSampler[vIndex], vTex);
	FragColor += sample_from_global();
	FragColor += sample_from_argument(uSampler);
	FragColor += sample_single_from_argument(uSampler[3]);
}
