#version 310 es

void main()
{
   gl_Position = float(gl_VertexIndex + gl_InstanceIndex) * vec4(1.0, 2.0, 3.0, 4.0);
}
