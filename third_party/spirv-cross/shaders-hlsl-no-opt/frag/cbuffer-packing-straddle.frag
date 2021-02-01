#version 450

layout(set = 0, binding = 0) uniform UBO
{
	vec4 a[2]; // 0
	vec4 b; // 32
	vec4 c; // 48
	mat4x4 d; // 64

	float e; // 128
	vec2 f; // 136

	float g; // 144
	vec2 h; // 152

	float i; // 160
	vec2 j; // 168

	float k;
	vec2 l;

	float m;
	float n;
	float o;

	vec4 p;
	vec4 q;
	vec3 r;
	vec4 s;
	vec4 t;
	vec4 u;
	float v;
	float w;
	float x;
	float y;
	float z;
	float aa;
	float ab;
	float ac;
	float ad;
	float ae;
	vec4 ef;
};

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = a[1];
}
