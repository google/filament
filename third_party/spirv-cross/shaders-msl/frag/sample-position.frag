#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(gl_SamplePosition, gl_SampleID, 1.0);
}
