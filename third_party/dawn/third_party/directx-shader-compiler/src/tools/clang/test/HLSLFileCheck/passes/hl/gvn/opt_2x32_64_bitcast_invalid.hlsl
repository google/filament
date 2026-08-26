// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -Zpr %s | FileCheck %s

// Make sure GVN does not do illegal bitcast for DXIL
// CHECK-NOT: bitcast i32* {{.*}} to i64*

Make sure compile was successful (function props for main)
// void ()* @main, i32 5

struct FOO
{
  int val0;
  int val1;
};

void externFunc(inout FOO);
void append(int);

[numthreads(1,1,1)]
void main()
{
  FOO foo;
  foo.val0 = 0;
  foo.val0 = 1;
  externFunc(foo);
  append(foo.val0);
  append(foo.val1);
}