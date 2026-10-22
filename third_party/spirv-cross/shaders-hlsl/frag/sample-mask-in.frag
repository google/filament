#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
	if ((gl_SampleMaskIn[0] & (1 << gl_SampleID)) != 0)
	{
		FragColor = vec4(1.0);
	}
}
