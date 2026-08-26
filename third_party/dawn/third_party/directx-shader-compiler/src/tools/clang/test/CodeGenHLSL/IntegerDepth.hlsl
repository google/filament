// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

int main(snorm float b : B, float c:C) : SV_DEPTH
{
  return b;
}