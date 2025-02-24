#version 320 es

layout(location = 0) out mediump vec2 vTexCoords;

void main()
{
	mediump vec2 coord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(coord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoords = vec2(coord.x, 1.0 - coord.y) ;
}