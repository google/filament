#version 310 es
#extension GL_EXT_shader_io_blocks : require

layout(location = 0) out VertexOut
{
   vec4 color;
   vec3 normal;
} vout;

layout(location = 0) in vec4 Position;

void main()
{
   gl_Position = Position;
   vout.color = vec4(1.0);
   vout.normal = vec3(0.5);
}
