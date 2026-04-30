#version 450

vec4 computePosition(vec4 position)
{
  return vec4(1.0) + 2.0 * position;
}
