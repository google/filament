#version 450

in float gl_ClipDistance[4];
in float gl_CullDistance[3];

layout(location = 0) out float FragColor;

void main()
{
	FragColor = gl_ClipDistance[0] + gl_CullDistance[0];
}

