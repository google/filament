#version 450
#extension GL_ARB_shader_ballot : enable
#extension GL_KHR_shader_subgroup_basic : enable

layout(location = 0) out uint result;

void main (void)
{
  result = gl_SubGroupSizeARB;
}
