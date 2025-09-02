// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<bool4>  buffer;

// CHECK-DAG: [[v4_0:%[0-9]+]] = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_0
// CHECK-DAG: [[v4_1:%[0-9]+]] = OpConstantComposite %v4uint %uint_1 %uint_1 %uint_1 %uint_1

[numthreads(1, 1, 1)]
void main()
{
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4uint %buffer %int_0 %uint_0
// CHECK: [[load:%[0-9]+]] = OpLoad %v4uint [[ptr]]
// CHECK: [[cast:%[0-9]+]] = OpINotEqual %v4bool [[load]] [[v4_0]]
// CHECK: [[shuf:%[0-9]+]] = OpVectorShuffle %v3bool [[cast]] [[cast]] 0 1 2
// CHECK:                    OpStore %a [[shuf]]
  bool3 a = buffer[0].xyz;

// CHECK:    [[a:%[0-9]+]] = OpLoad %v3bool %a
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4uint %buffer %int_0 %uint_1
// CHECK: [[load:%[0-9]+]] = OpLoad %v4uint [[ptr]]
// CHECK: [[cast:%[0-9]+]] = OpINotEqual %v4bool [[load]] [[v4_0]]
// CHECK: [[shuf:%[0-9]+]] = OpVectorShuffle %v4bool [[cast]] [[a]] 4 5 6 3
// CHECK: [[cast:%[0-9]+]] = OpSelect %v4uint [[shuf]] [[v4_1]] [[v4_0]]
// CHECK:                    OpStore [[ptr]] [[cast]]
  buffer[1].xyz = a;
}
