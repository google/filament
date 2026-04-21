#version 410

// Should generate contradictory input vertex spacing and triangle ordering errors
layout(triangles, fractional_even_spacing, ccw) in;

vec4 transformPosition(vec4 position)
{
  return 2.0 * position;
}
