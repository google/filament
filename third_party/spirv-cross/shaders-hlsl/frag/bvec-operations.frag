#version 310 es
precision mediump float;

layout(location = 0) in vec2 value;

layout(location = 0) out vec4 FragColor;

void main()
{
	bvec2 bools1 = not(bvec2(value.x == 0.0, value.y == 0.0));
	bvec2 bools2 = lessThanEqual(value, vec2(1.5, 0.5));
	FragColor = vec4(1.0, 0.0, bools1.x ? 1.0 : 0.0, bools2.x ? 1.0 : 0.0);
}
