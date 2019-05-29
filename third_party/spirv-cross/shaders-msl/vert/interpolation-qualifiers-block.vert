#version 450

layout(location=0) in vec4 Position;

struct Output {
	vec2 v0;
	vec2 v1;
	vec3 v2;
	vec4 v3;
	float v4;
	float v5;
	float v6;
};

layout(location=0) out centroid noperspective Output outp;

void main() {
	outp.v0 = Position.xy;
	outp.v1 = Position.zw;
	outp.v2 = vec3(Position.x, Position.z * Position.y, Position.x);
	outp.v3 = Position.xxyy;
	outp.v4 = Position.w;
	outp.v5 = Position.y;
	outp.v6 = Position.x * Position.w;
	gl_Position = Position;
}
