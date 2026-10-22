#version 310 es
#extension GL_EXT_post_depth_coverage : require
#extension GL_OES_sample_variables : require
precision mediump float;

layout(early_fragment_tests, post_depth_coverage) in;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(gl_SampleMaskIn[0]);
}
