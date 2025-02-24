#version 460 core

void f1(float x)
{
    gl_ClipDistance[6] = x;
    gl_CullDistance[1] = x;
}

void main(){
  f1(0.1);
}