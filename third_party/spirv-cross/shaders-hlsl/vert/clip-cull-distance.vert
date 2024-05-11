#version 450
out float gl_ClipDistance[2];
out float gl_CullDistance[1];

void main()
{
	gl_Position = vec4(1.0);
	gl_ClipDistance[0] = 0.0;
	gl_ClipDistance[1] = 0.0;
	gl_CullDistance[0] = 4.0;
}
