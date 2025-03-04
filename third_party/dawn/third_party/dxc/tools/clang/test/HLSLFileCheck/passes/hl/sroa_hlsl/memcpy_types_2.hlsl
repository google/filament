// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test a bug where passing array struct element to function causes
// a crash due to memcpy src and dest type mismatch.
// This test makes sure that casting to a different type with the same layout still works.

// CHECK: @main

struct my_struct
{
  float x : POSITION0;
  float y : TEXCOORD0;
};

struct my_struct_2
{
  float x;
  float y;
};
 
uint foo(my_struct_2 s)
{
  return 0;
}
 
void main(my_struct a[1])
{
  uint r = foo((my_struct_2)a[0]);
}


