#version 450
#extension GL_ARB_post_depth_coverage : require

layout(post_depth_coverage) in;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(gl_SampleMaskIn[0]);
}
