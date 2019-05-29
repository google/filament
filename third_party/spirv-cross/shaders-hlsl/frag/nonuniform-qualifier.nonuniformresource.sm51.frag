#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform texture2D uSamplers[];
layout(set = 1, binding = 0) uniform sampler2D uCombinedSamplers[];
layout(set = 2, binding = 0) uniform sampler uSamps[];
layout(location = 0) flat in int vIndex;
layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

layout(set = 3, binding = 0) uniform UBO 
{
	vec4 v[64];
} ubos[];

layout(set = 4, binding = 0) readonly buffer SSBO
{
	vec4 v[];
} ssbos[];

void main()
{
	int i = vIndex;
	FragColor = texture(sampler2D(uSamplers[nonuniformEXT(i + 10)], uSamps[nonuniformEXT(i + 40)]), vUV);
	FragColor = texture(uCombinedSamplers[nonuniformEXT(i + 10)], vUV);
	FragColor += ubos[nonuniformEXT(i + 20)].v[nonuniformEXT(i + 40)];
	FragColor += ssbos[nonuniformEXT(i + 50)].v[nonuniformEXT(i + 60)];
}
