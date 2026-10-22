#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(binding = 0) uniform texture2D uSamplers[8];
layout(binding = 8) uniform sampler2D uCombinedSamplers[8];
layout(binding = 1) uniform sampler uSamps[7];
layout(location = 0) flat in int vIndex;
layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform UBO 
{
	vec4 v[64];
} ubos[2];

layout(set = 0, binding = 2) readonly buffer SSBO
{
	vec4 v[];
} ssbos[2];

void main()
{
	int i = vIndex;
	FragColor = texture(sampler2D(uSamplers[nonuniformEXT(i + 10)], uSamps[nonuniformEXT(i + 40)]), vUV);
	FragColor = texture(uCombinedSamplers[nonuniformEXT(i + 10)], vUV);
	FragColor += ubos[nonuniformEXT(i + 20)].v[nonuniformEXT(i + 40)];
	FragColor += ssbos[nonuniformEXT(i + 50)].v[nonuniformEXT(i + 60)];
}
