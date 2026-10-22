// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
void main()
{
  return;
}