// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: define void @main()
// CHECK: ret void

float main(int a
           : A) : SV_Target {
  float y;
  [branch] if (a > 0) {
    y = a;
  }
  else {
    a = a * a;
  }

  // Here "y" will appear as undef in the phi node when "else" block is taken.
  // The instcombine pass should not fold the below mul instruction to phi
  // node if phi has undef values.
  float z = y * 3.3f;
  z = z / a;
  return z;
}
