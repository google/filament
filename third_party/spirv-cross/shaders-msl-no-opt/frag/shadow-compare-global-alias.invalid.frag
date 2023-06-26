#version 450

layout(location = 0) out float FragColor;
layout(binding = 0) uniform sampler2DShadow uSampler;
layout(location = 0) in vec3 vUV;

layout(binding = 1) uniform texture2D uTex;
layout(binding = 2) uniform samplerShadow uSamp;

float Samp(vec3 uv)
{
	return texture(sampler2DShadow(uTex, uSamp), uv);
}

float Samp2(vec3 uv)
{
	return texture(uSampler, vUV);
}

float Samp3(texture2D uT, samplerShadow uS, vec3 uv)
{
	return texture(sampler2DShadow(uT, uS), vUV);
}

float Samp4(sampler2DShadow uS, vec3 uv)
{
	return texture(uS, vUV);
}

void main()
{
	FragColor = texture(uSampler, vUV);
	FragColor += texture(sampler2DShadow(uTex, uSamp), vUV);
	FragColor += Samp(vUV);
	FragColor += Samp2(vUV);
	FragColor += Samp3(uTex, uSamp, vUV);
	FragColor += Samp4(uSampler, vUV);
}
