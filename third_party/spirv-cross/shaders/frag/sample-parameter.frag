#version 310 es
#extension GL_OES_sample_variables : require
precision mediump float;

layout(location = 0) out vec2 FragColor;

void main()
{
	FragColor = gl_SamplePosition + vec2(gl_SampleMaskIn[0]) + float(gl_SampleID);
	gl_SampleMask[0] = 1;
}
