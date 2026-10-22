#version 310 es

layout(binding = 0) uniform sampler2D tex;

void main()
{
	gl_Position = texture(tex, vec2(0.4, 0.6));
}
