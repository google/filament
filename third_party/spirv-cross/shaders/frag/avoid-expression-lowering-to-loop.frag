#version 310 es
precision mediump float;
precision mediump int;

layout(binding = 0) uniform mediump sampler2D tex;
layout(binding = 1) uniform Count
{
	float count;
};

layout(location = 0) in highp vec4 vertex;
layout(location = 0) out vec4 fragColor;

void main() {

	highp float size = 1.0 / float(textureSize(tex, 0).x);
	float r = 0.0;
	float d = dFdx(vertex.x);
	for (float i = 0.0; i < count ; i += 1.0)
		r += size * d;

	fragColor = vec4(r);
}
