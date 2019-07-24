#version 450

layout(location = 0) out vec2 FragColor;
layout(set = 0, binding = 0) uniform sampler2D uSampler2D;
layout(set = 0, binding = 1) uniform sampler3D uSampler3D;
layout(set = 0, binding = 2) uniform samplerCube uSamplerCube;
layout(set = 0, binding = 3) uniform sampler uSampler;
layout(set = 0, binding = 4) uniform texture2D uTexture2D;
layout(set = 0, binding = 5) uniform texture3D uTexture3D;
layout(set = 0, binding = 6) uniform textureCube uTextureCube;
layout(location = 0) in vec3 vUV;

void from_function()
{
	FragColor += textureQueryLod(uSampler2D, vUV.xy);
	FragColor += textureQueryLod(uSampler3D, vUV);
	FragColor += textureQueryLod(uSamplerCube, vUV);
	FragColor += textureQueryLod(sampler2D(uTexture2D, uSampler), vUV.xy);
	FragColor += textureQueryLod(sampler3D(uTexture3D, uSampler), vUV);
	FragColor += textureQueryLod(samplerCube(uTextureCube, uSampler), vUV);
}

void main()
{
	FragColor = vec2(0.0);
	FragColor += textureQueryLod(uSampler2D, vUV.xy);
	FragColor += textureQueryLod(uSampler3D, vUV);
	FragColor += textureQueryLod(uSamplerCube, vUV);
	FragColor += textureQueryLod(sampler2D(uTexture2D, uSampler), vUV.xy);
	FragColor += textureQueryLod(sampler3D(uTexture3D, uSampler), vUV);
	FragColor += textureQueryLod(samplerCube(uTextureCube, uSampler), vUV);
	from_function();
}
