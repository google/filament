#version 450

layout(location=0) in vec2 v0;
layout(location=1) in noperspective vec2 v1;
layout(location=2) in centroid vec3 v2;
layout(location=3) in centroid noperspective vec4 v3;
layout(location=4) in sample float v4;
layout(location=5) in sample noperspective float v5;
layout(location=6) in flat float v6;

layout(location=0) out vec4 FragColor;

void main() {
	FragColor = vec4(v0.x + v1.y, v2.xy, v3.w * v4 + v5 - v6);
}
