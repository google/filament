#version 450

struct Input {
	vec2 v0;
	vec2 v1;
	vec3 v2;
	vec4 v3;
	float v4;
	float v5;
	float v6;
};

layout(location=0) in centroid noperspective Input inp;

layout(location=0) out vec4 FragColor;

void main() {
	FragColor = vec4(inp.v0.x + inp.v1.y, inp.v2.xy, inp.v3.w * inp.v4 + inp.v5 - inp.v6);
}
