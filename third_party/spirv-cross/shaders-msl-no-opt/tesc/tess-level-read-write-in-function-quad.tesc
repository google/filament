#version 450
layout(vertices = 1) out;

float load_tess_level_in_func()
{
	return gl_TessLevelInner[0] + gl_TessLevelOuter[1];
}

void store_tess_level_in_func()
{
	gl_TessLevelInner[0] = 1.0;
	gl_TessLevelInner[1] = 2.0;
	gl_TessLevelOuter[0] = 3.0;
	gl_TessLevelOuter[1] = 4.0;
	gl_TessLevelOuter[2] = 5.0;
	gl_TessLevelOuter[3] = 6.0;
}

void main()
{
	store_tess_level_in_func();
	float v = load_tess_level_in_func();
	gl_out[gl_InvocationID].gl_Position = vec4(v);
}
