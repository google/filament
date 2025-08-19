#version 460 core
in float a[];
in float c[3];
out float b[5];

void main(){
  b[0] = a[1];
  b[1] = c[1];
}