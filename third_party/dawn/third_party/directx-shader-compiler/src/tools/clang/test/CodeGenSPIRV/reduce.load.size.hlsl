// RUN: %dxc -T cs_6_0 -E main -fspv-reduce-load-size -O0  %s -spirv | FileCheck %s

struct S {
  uint f;
};

cbuffer gBuffer { uint a[6]; };

RWStructuredBuffer<S> gRWSBuffer;

// CHECK-NOT: OpCompositeExtract

// CHECK: [[p0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_0
// CHECK: OpLoad %uint [[p0]]
// CHECK: [[p1:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_1
// CHECK: OpLoad %uint [[p1]]
// CHECK: [[p2:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_2
// CHECK: OpLoad %uint [[p2]]
// CHECK: [[p3:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_3
// CHECK: OpLoad %uint [[p3]]
// CHECK: [[p4:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_4
// CHECK: OpLoad %uint [[p4]]
// CHECK: [[p5:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint {{%[a-zA-Z0-9_]+}} %uint_5
// CHECK: OpLoad %uint [[p5]]
uint foo(uint p[6]) {
  return p[0] + p[1] + p[2] + p[3] + p[4] + p[5];
}

[numthreads(1,1,1)]
void main() {
  gRWSBuffer[0].f = foo(a);
}
