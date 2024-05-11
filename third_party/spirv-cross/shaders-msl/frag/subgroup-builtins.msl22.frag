#version 450
#extension GL_KHR_shader_subgroup_basic : require

layout(location = 0) out uvec2 FragColor;

void main()
{
	FragColor.x = gl_SubgroupSize;
	FragColor.y = gl_SubgroupInvocationID;
}
