#version 450

in float gl_ClipDistance[2];
in float gl_CullDistance[1];

layout(location = 0) out float FragColor;

void main()
{
	FragColor = gl_ClipDistance[0] + gl_CullDistance[0] + gl_ClipDistance[1];
}

