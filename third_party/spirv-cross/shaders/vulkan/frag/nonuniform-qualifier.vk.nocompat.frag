#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout(binding = 0) uniform texture2D uSamplers[];
layout(binding = 0) uniform texture2DMS uSamplersMS[];
layout(binding = 4) uniform sampler2D uCombinedSamplers[];
layout(binding = 1) uniform sampler uSamps[];
layout(location = 0) flat in int vIndex;
layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

layout(r32f, binding = 5) uniform image2D uImages[];
layout(r32ui, binding = 5) uniform uimage2D uImagesU32[];

layout(set = 0, binding = 2) uniform UBO 
{
	vec4 v[64];
} ubos[];

layout(set = 0, binding = 3) buffer SSBO
{
	uint counter;
	vec4 v[];
} ssbos[];

void main()
{
	int i = vIndex;
	FragColor = texture(nonuniformEXT(sampler2D(uSamplers[i + 10], uSamps[i + 40])), vUV);
	FragColor = texture(uCombinedSamplers[nonuniformEXT(i + 10)], vUV);
	FragColor += ubos[nonuniformEXT(i + 20)].v[nonuniformEXT(i + 40)];
	FragColor += ssbos[nonuniformEXT(i + 50)].v[nonuniformEXT(i + 60)];
	ssbos[nonuniformEXT(i + 60)].v[nonuniformEXT(i + 70)] = vec4(20.0);

	FragColor = texelFetch(uSamplers[nonuniformEXT(i + 10)], ivec2(vUV), 0);
	atomicAdd(ssbos[nonuniformEXT(i + 100)].counter, 100u);

	vec2 queried = textureQueryLod(nonuniformEXT(sampler2D(uSamplers[i + 10], uSamps[i + 40])), vUV);
	queried += textureQueryLod(uCombinedSamplers[nonuniformEXT(i + 10)], vUV);
	FragColor.xy += queried;

	FragColor.x += float(textureQueryLevels(uSamplers[nonuniformEXT(i + 20)]));
	FragColor.y += float(textureSamples(uSamplersMS[nonuniformEXT(i + 20)]));
	FragColor.xy += vec2(textureSize(uSamplers[nonuniformEXT(i + 20)], 0));

	FragColor += imageLoad(uImages[nonuniformEXT(i + 50)], ivec2(vUV));
	FragColor.xy += vec2(imageSize(uImages[nonuniformEXT(i + 20)]));
	imageStore(uImages[nonuniformEXT(i + 60)], ivec2(vUV), vec4(50.0));

	imageAtomicAdd(uImagesU32[nonuniformEXT(i + 70)], ivec2(vUV), 40u);
}
