// RUN: %dxc -E main -T cs_6_3 -HV 202x -fcgl %s | FileCheck %s

struct Shared {
  int A;
  float F;
  double Arr[4];
};

groupshared Shared SharedData;

// CHECK-LABEL: @"\01?fn1@@YAXAGAUShared@@@Z"
// CHECK: [[D:%.*]] = alloca double, align 8
// CHECK: [[A:%.*]] = getelementptr inbounds %struct.Shared, %struct.Shared addrspace(3)* %Sh, i32 0, i32 0
// CHECK: store i32 10, i32 addrspace(3)* [[A]], align 4
// CHECK: [[F:%.*]] = getelementptr inbounds %struct.Shared, %struct.Shared addrspace(3)* %Sh, i32 0, i32 1
// CHECK: store float 0x40263851E0000000, float addrspace(3)* %F, align 4
// CHECK: store double 1.000000e+01, double* [[D]], align 8
// CHECK: [[Z:%.*]] = load double, double* [[D]], align 8
// CHECK: [[Arr:%.*]] = getelementptr inbounds %struct.Shared, %struct.Shared addrspace(3)* %Sh, i32 0, i32 2
// CHECK: [[ArrIdx:%.*]] = getelementptr inbounds [4 x double], [4 x double] addrspace(3)* [[Arr]], i32 0, i32 1
// CHECK: store double [[Z]], double addrspace(3)* [[ArrIdx]], align 4
void fn1(groupshared Shared Sh) {
  Sh.A = 10;
  Sh.F = 11.11;
  double D = 10.0;
  Sh.Arr[1] = D;
}

[numthreads(4, 1, 1)]
void main(uint3 TID : SV_GroupThreadID) {
  fn1(SharedData);
}
