#version 450

// Should generate a contradictory xfb_stride error
layout(xfb_buffer = 1, xfb_stride = 34) out;

vec4 computePosition(vec4 position)
{
  return vec4(1.0) + 2.0 * position;
}
