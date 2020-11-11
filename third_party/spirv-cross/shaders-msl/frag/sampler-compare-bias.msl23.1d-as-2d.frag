#version 450

layout(binding = 0) uniform texture1DArray uTex;
layout(binding = 1) uniform samplerShadow uShadow;
layout(location = 0) in vec3 vUV;
layout(location = 0) out float FragColor;

void main()
{
	FragColor = texture(sampler1DArrayShadow(uTex, uShadow), vUV, 1.0);
}
