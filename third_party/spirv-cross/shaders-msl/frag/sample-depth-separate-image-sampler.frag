#version 450

layout(set = 0, binding = 0) uniform texture2D uDepth;
layout(set = 0, binding = 1) uniform texture2D uColor;
layout(set = 0, binding = 2) uniform sampler uSampler;
layout(set = 0, binding = 3) uniform samplerShadow uSamplerShadow;
layout(location = 0) out float FragColor;

float sample_depth_from_function(texture2D uT, samplerShadow uS)
{
	return texture(sampler2DShadow(uT, uS), vec3(0.5));
}

float sample_color_from_function(texture2D uT, sampler uS)
{
	return texture(sampler2D(uT, uS), vec2(0.5)).x;
}

void main()
{
	FragColor = sample_depth_from_function(uDepth, uSamplerShadow) + sample_color_from_function(uColor, uSampler);
}
