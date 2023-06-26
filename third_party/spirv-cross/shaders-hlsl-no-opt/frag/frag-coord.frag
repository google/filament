#version 450

layout(location = 0) out vec3 FragColor;

void main()
{
	FragColor = gl_FragCoord.xyz / gl_FragCoord.w;
}
