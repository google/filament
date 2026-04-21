#version 310 es
#extension GL_EXT_tessellation_shader : require

layout(cw, triangles, fractional_even_spacing, point_mode) in;

void main()
{
   gl_Position = vec4(1.0);
}

