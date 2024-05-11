#version 450
layout(vertices = 4) out;

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 2.0;
		gl_TessLevelOuter[1] = 3.0;
		gl_TessLevelOuter[2] = 4.0;
		gl_TessLevelOuter[3] = 5.0;
		gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
		gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
}
