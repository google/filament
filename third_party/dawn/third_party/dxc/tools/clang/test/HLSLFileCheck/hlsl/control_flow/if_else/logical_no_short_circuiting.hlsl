// RUN: %dxc -E main -T vs_6_0 -HV 2018 %s | FileCheck %s

// HLSL's logical operators are not short-circuiting.
// Test that the right-hand side is always executed.

AppendStructuredBuffer<int4> buf;

bool set10(inout int i, bool retval) { i = 10; return retval; }

void main() {
  int4 vec;
  
  // &&
  // CHECK: i32 10, i32 10, i32 10, i32 11, i8 15)
  vec = 0;
  if (false && set10(vec.x, false)) vec.x++;
  if (false && set10(vec.y, true)) vec.y++;
  if (true && set10(vec.z, false)) vec.z++;
  if (true && set10(vec.w, true)) vec.w++;
  buf.Append(vec);
  
  // ||
  // CHECK: i32 10, i32 11, i32 11, i32 11, i8 15)
  vec = 0;
  if (false || set10(vec.x, false)) vec.x++;
  if (false || set10(vec.y, true)) vec.y++;
  if (true || set10(vec.z, false)) vec.z++;
  if (true || set10(vec.w, true)) vec.w++;
  buf.Append(vec);
}

