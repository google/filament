#version 450

layout(binding=0, set=0, std430) buffer foo
{
	int x;
};

layout(location=0) out vec4 fragColor;

void main(void)
{
	if (gl_FragCoord.y == 7)
		discard;
	for (x = 0; x < gl_FragCoord.x; ++x)
		;
	fragColor = vec4(x, 0, 0, 1);
}
