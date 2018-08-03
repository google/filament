#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in float vTex;
layout(binding = 0) uniform sampler1D uSampler;

void main()
{
	FragColor += texture(uSampler, vTex, 2.0) +
		textureLod(uSampler, vTex, 3.0) +
		textureGrad(uSampler, vTex, 5.0, 8.0);
}
