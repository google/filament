#version 450

layout(location = 0) out float FragColor;
layout(binding = 0) uniform sampler2DShadow uSampler2D;
layout(binding = 1) uniform sampler2DArrayShadow uSampler2DArray;
layout(binding = 2) uniform samplerCubeShadow uSamplerCube;
layout(binding = 3) uniform samplerCubeArrayShadow uSamplerCubeArray;

layout(location = 0) in vec3 vUVRef;
layout(location = 1) in vec4 vDirRef;

void main()
{
	float s0 = textureOffset(uSampler2D, vUVRef, ivec2(-1));
	float s1 = textureOffset(uSampler2DArray, vDirRef, ivec2(-1));
	float s2 = texture(uSamplerCube, vDirRef);
	float s3 = texture(uSamplerCubeArray, vDirRef, 0.5);

	float l0 = textureLodOffset(uSampler2D, vUVRef, 0.0, ivec2(-1));
	float l1 = textureGradOffset(uSampler2DArray, vDirRef, vec2(0.0), vec2(0.0), ivec2(-1));
	float l2 = textureGrad(uSamplerCube, vDirRef, vec3(0.0), vec3(0.0));

	float p0 = textureProjOffset(uSampler2D, vDirRef, ivec2(+1));
	float p1 = textureProjLodOffset(uSampler2D, vDirRef, 0.0, ivec2(+1));

	FragColor = s0 + s1 + s2 + s3 + l0 + l1 + l2 + p0 + p1;
}
