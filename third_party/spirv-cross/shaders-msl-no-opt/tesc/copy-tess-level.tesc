#version 450
layout(vertices = 1) out;

void main()
{
	gl_TessLevelInner = float[](1.0, 2.0);
	gl_TessLevelOuter = float[](1.0, 2.0, 3.0, 4.0);

	float inner[2] = gl_TessLevelInner;
	float outer[4] = gl_TessLevelOuter;
	gl_out[gl_InvocationID].gl_Position = vec4(1.0);
}
