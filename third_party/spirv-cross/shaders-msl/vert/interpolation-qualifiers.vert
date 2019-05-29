#version 450

layout(location=0) in vec4 Position;

layout(location=0) out vec2 v0;
layout(location=1) out noperspective vec2 v1;
layout(location=2) out centroid vec3 v2;
layout(location=3) out centroid noperspective vec4 v3;
layout(location=4) out sample float v4;
layout(location=5) out sample noperspective float v5;
layout(location=6) out flat float v6;

void main() {
	v0 = Position.xy;
	v1 = Position.zw;
	v2 = vec3(Position.x, Position.z * Position.y, Position.x);
	v3 = Position.xxyy;
	v4 = Position.w;
	v5 = Position.y;
	v6 = Position.x * Position.w;
	gl_Position = Position;
}
