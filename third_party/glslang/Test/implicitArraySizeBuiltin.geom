#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 204) out;

void f2(float x)
{
    gl_ClipDistance[6] = gl_in[0].gl_ClipDistance[6];
}
void f3(float x)
{
    gl_CullDistance[1] = gl_in[0].gl_CullDistance[1];
}

void main(){
  #if defined(CLIP)
  f2(0.1);
  #endif
  f3(0.1);
}