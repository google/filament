#version 450
#extension GL_ARM_shader_core_builtins: enable
layout(set = 0, binding = 0, std430) buffer Output
{
  uvec4 result;
};

void main (void)
{
  uint temp = gl_WarpMaxIDARM;
  result = uvec4(gl_CoreIDARM, gl_CoreCountARM, gl_CoreMaxIDARM, gl_WarpIDARM + temp);
}
