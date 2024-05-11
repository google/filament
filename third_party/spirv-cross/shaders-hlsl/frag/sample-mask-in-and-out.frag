#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0);
	gl_SampleMask[0] = gl_SampleMaskIn[0];
}
