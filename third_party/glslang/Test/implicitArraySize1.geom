#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 204) out;

void f();

in float g[][3];
out float o[3];

void main(){
  f();
  o[1] = g[2][1];
}