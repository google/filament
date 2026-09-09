// RUN: %dxc %s -T lib_6_8 -E main -spirv -enable-16bit-types

// CHECK-DAG: %ulong = OpTypeInt 64 0
// CHECK-DAG:  %long = OpTypeInt 64 1

template <typename T> struct S {
  static const T value = 0;
};

// CHECK: %u = OpFunction %void None
[shader("vertex")]
uint64_t u() : A
{
  // CHECK: OpStore %out_var_A %ulong_0
  return S<uint64_t>::value;
}

// CHECK: %s = OpFunction %void None
[shader("vertex")]
int64_t s() : B
{
  // CHECK: OpStore %out_var_A %long_0
  return S<int64_t>::value;
}