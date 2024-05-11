#version 450

layout(set = 0, binding = 0) uniform texture2D uTexture;
layout(set = 0, binding = 1) uniform sampler uSampler;
layout(set = 0, binding = 2) uniform samplerShadow uSamplerShadow;

layout(location = 0) out float FragColor;
layout(location = 0) in vec3 vUV;

float sample_normal2(texture2D tex)
{
	return texture(sampler2D(tex, uSampler), vUV.xy).x;
}

float sample_normal(texture2D tex)
{
	return sample_normal2(tex);
}

float sample_comp(texture2D tex)
{
	return texture(sampler2DShadow(tex, uSamplerShadow), vUV);
}

void main()
{
	FragColor = sample_normal(uTexture);
	FragColor += sample_comp(uTexture);
}
