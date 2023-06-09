#version 450

out float gl_CullDistance[2];

void main()
{
	gl_CullDistance[0] = 1.0;
	gl_CullDistance[1] = 3.0;
	gl_Position = vec4(1.0);
}
