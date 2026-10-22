#version 310 es

uniform Buffer
{
	mat2 m2;
	mat3 m3;
	mat4 m4;
	mediump mat2 m2r;
	mediump mat3 m3r;
	mediump mat4 m4r;
};

layout(location = 0) in vec4 Position;

layout(location = 0) out vec3 dets;
layout(location = 1) out mat2 o2;
layout(location = 3) out mat3 o3;
layout(location = 6) out mat4 o4;
layout(location = 10) out mat2 o2r;
layout(location = 12) out mat3 o3r;
layout(location = 15) out mat4 o4r;

void main()
{
	dets.x = determinant(m2);
	dets.y = determinant(m3);
	dets.z = determinant(m4);

	o2 = inverse(m2);
	o3 = inverse(m3);
	o4 = inverse(m4);

	o2r = inverse(m2r);
	o3r = inverse(m3r);
	o4r = inverse(m4r);

	gl_Position = vec4(0.0);
}

