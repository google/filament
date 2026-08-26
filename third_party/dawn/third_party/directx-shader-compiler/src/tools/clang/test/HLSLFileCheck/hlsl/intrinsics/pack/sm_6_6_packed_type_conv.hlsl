// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// CHECK:  and i32 
// CHECK:  add i32 
// CHECK:  uitofp i32 %{{[a-z0-9]+}} to float
// CHECK:  fmul fast float
// CHECK:  fadd fast float
// CHECK:  uitofp i32 %{{[a-z0-9]+}} to float
// CHECK:  fptosi float %{{[a-z0-9]+}} to i32

int main(int8_t4_packed input1 : Inputs1, uint8_t4_packed input2 : Inputs2) : SV_Target {
  int8_t4_packed p = input1 & input2;
  uint a = p + input2;
  float f = input1 * 13.5f + input2 / 1.8f;
  return a * f;
}
