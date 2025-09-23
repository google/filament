#version 450
#extension GL_ARM_shader_core_builtins: enable
layout(location = 0) out uvec4 data;
void main (void)
{
  uint temp = gl_WarpMaxIDARM;
  data = uvec4(gl_CoreIDARM, gl_CoreCountARM, gl_CoreMaxIDARM, gl_WarpIDARM + temp);
}
