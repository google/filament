#version 450

layout(location = 0) out vec4 FragColor;

in float gl_CullDistance[2];
in float gl_ClipDistance[2];

vec4 read_in_func()
{
	return vec4(
			gl_CullDistance[0],
			gl_CullDistance[1],
			gl_ClipDistance[0],
			gl_ClipDistance[1]);
}

void main()
{
	FragColor = read_in_func();
}
