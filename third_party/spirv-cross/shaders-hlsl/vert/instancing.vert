#version 310 es

void main()
{
	gl_Position = vec4(float(gl_VertexIndex + gl_InstanceIndex));
}
