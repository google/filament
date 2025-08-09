#version 460 core

in float g[][3];
out float o[];

void f(){
  o[1] = g[1][1];
}