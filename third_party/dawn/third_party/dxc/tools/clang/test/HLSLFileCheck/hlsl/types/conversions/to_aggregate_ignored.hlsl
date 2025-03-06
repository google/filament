// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Regression test for GitHub #1978, where converting to an aggregate type
// and ignoring the result would cause a crash, because clang would not
// allocate an AggValueSlot, and so our destination pointer would be nullptr.

// CHECK: ret void

struct S { int f; };
void main()
{
  (S)0;
  (int[1])0;
}