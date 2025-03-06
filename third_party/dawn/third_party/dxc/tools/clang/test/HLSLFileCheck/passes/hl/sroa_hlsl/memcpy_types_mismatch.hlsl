// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that casting here doesn't work.

// CHECK-NOT: @main

struct my_struct
{
  float x : POSITION0;
  float y : TEXCOORD0;
};

struct my_struct_2
{
  float x;
  float y;
  float z;
};
 
uint foo(my_struct_2 s)
{
  return 0;
}
 
void main(my_struct a[1])
{
  uint r = foo(a[0]);
}


