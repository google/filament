#version 450
layout(quads) in;

vec4 read_tess_levels()
{
	return vec4(
		gl_TessLevelOuter[0],
		gl_TessLevelOuter[1],
		gl_TessLevelOuter[2],
		gl_TessLevelOuter[3]) +
		vec2(gl_TessLevelInner[0], gl_TessLevelInner[1]).xyxy;
}

void main()
{
	gl_Position = read_tess_levels();
}
