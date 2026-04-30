#version 450

void Wobble(vec2 uu, out vec4 offset[3])
{
	offset[0] = vec4(uu, uu);
	offset[1] = vec4(uu, uu);
	offset[2] = vec4(uu, uu);
}

layout(location = 3) in vec3 dd[1];

layout(location = 1) out Vertex {
	vec2 umbrage;
	vec4 offset[3];
} aa;

void main()
{
	Wobble(dd[0].st, aa.offset);
	gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
}
