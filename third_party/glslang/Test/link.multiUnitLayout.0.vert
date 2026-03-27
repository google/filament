#version 450

layout(xfb_buffer = 1, xfb_stride = 32) out;

layout(location = 0) in vec4 in_position;

vec4 computePosition(vec4 position);

void main()
{
  gl_Position = computePosition(in_position);
}
