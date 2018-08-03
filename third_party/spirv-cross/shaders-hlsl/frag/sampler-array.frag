#version 450

layout(binding = 0) uniform sampler2D uCombined[4];
layout(binding = 4) uniform texture2D uTex[4];
layout(binding = 8) uniform sampler uSampler[4];
layout(binding = 12, rgba32f) uniform writeonly image2D uImage[8];
layout(location = 0) in vec2 vTex;
layout(location = 1) flat in int vIndex;

vec4 sample_in_function(sampler2D samp)
{
	return texture(samp, vTex);
}

vec4 sample_in_function2(texture2D tex, sampler samp)
{
	return texture(sampler2D(tex, samp), vTex);
}

void main()
{
	vec4 color = texture(uCombined[vIndex], vTex);
	color += texture(sampler2D(uTex[vIndex], uSampler[vIndex]), vTex);
	color += sample_in_function(uCombined[vIndex + 1]);
	color += sample_in_function2(uTex[vIndex + 1], uSampler[vIndex + 1]);

	imageStore(uImage[vIndex], ivec2(gl_FragCoord.xy), color);
}
