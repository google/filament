#version 310 es

layout(location = 0) out mediump vec2 vViewDirVS;

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vViewDirVS = vec2(texcoord * 2.0 + -1.0);
	gl_Position = vec4(vViewDirVS, 0.0, 1.0);
}