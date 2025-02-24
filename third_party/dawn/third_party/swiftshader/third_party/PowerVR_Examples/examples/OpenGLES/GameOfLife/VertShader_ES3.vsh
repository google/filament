#version 310 es

layout(location = 0) out mediump vec2 vTexCoord;

void main()
{

	mediump vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	// Pass through texcoords
	vTexCoord = vec2(texcoord.x, 1.0 - texcoord.y);
}
