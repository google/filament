#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 1) in VertexOut
{
   vec4 color;
   highp vec3 normal;
} vin;

layout(location = 0) out vec4 FragColor;

void main()
{
   FragColor = vin.color + vin.normal.xyzz;
}
