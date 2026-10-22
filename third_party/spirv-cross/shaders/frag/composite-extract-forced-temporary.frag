#version 310 es
precision mediump float;
layout(binding = 0) uniform sampler2D Texture;
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vTexCoord;

void main()
{
	float f = texture(Texture, vTexCoord).x;
	FragColor = vec4(f * f);
}
