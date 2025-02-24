// RUN: %dxc -T lib_6_4 -fcgl -HV 2021 %s | FileCheck %s

struct Doggo {
void operator()(inout float X, inout int Y, inout float Z) {
  X = 1.0;
  Y = 2;
  Z = 3.0;
}
};

void fn() {
  float X, Z = 0.0;
  int Y = 0;
  Doggo D;
  D(X, Y, Z);

  D(X, Y, X);
}

// CHECK: define internal void @"\01?fn{{[@$?.A-Za-z0-9_]+}}"()
// CHECK: [[Tmp1:%[0-9A-Z]+]] = alloca float
// CHECK: [[X:%[0-9A-Z]+]] = alloca float, align 4
// CHECK: [[Z:%[0-9A-Z]+]] = alloca float, align 4
// CHECK: [[Y:%[0-9A-Z]+]] = alloca i32, align 4
// CHECK: [[D:%[0-9A-Z]+]] = alloca %struct.Doggo

// First call has no copy-in/copy out parameters since all parameters are unique.
// CHECK: call void @"\01??RDoggo{{[@$?.A-Za-z0-9_]+}}"(%struct.Doggo* [[D]], float* dereferenceable(4) [[X]], i32* dereferenceable(4) [[Y]], float* dereferenceable(4) [[Z]])

// The second call copies X for the third parameter.
// CHECK: [[TmpX:%[0-9A-Z]+]] = load float, float* [[X]], align 4
// CHECK: store float [[TmpX]], float* [[Tmp1]]

// CHECK: call void @"\01??RDoggo{{[@$?.A-Za-z0-9_]+}}"(%struct.Doggo* [[D]], float* dereferenceable(4) [[X]], i32* dereferenceable(4) [[Y]], float* dereferenceable(4) [[Tmp1]])

// The third call stores parameter 3 to X after the call.
// CHECK: [[TmpX:%[0-9A-Z]+]] = load float, float* [[Tmp1]]
// CHECK: store float [[TmpX]], float* [[X]]
