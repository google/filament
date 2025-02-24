// RUN: %dxc -T cs_6_0 -E main -fspv-fix-func-call-arguments -O0  %s -spirv | FileCheck %s
RWStructuredBuffer< float4 > output : register(u1);

[noinline]
float4 foo(inout float f0, inout int f1)
{
    return 0;
}

// CHECK: [[s39:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_int Function
// CHECK: [[s36:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_float Function
// CHECK: [[s33:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_float {{%[a-zA-Z0-9_]+}} %int_0
// CHECK: [[s34:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function_int {{%[a-zA-Z0-9_]+}} %int_1
// CHECK: [[s37:%[a-zA-Z0-9_]+]] = OpLoad %float [[s33]]
// CHECK:                OpStore [[s36]] [[s37]]
// CHECK: [[s40:%[a-zA-Z0-9_]+]] = OpLoad %int [[s34]]
// CHECK:                OpStore [[s39]] [[s40]]
// CHECK: {{%[a-zA-Z0-9_]+}} = OpFunctionCall %v4float %foo [[s36]] [[s39]]
// CHECK: [[s41:%[a-zA-Z0-9_]+]] = OpLoad %int [[s39]]
// CHECK:                OpStore [[s34]] [[s41]]
// CHECK: [[s38:%[a-zA-Z0-9_]+]] = OpLoad %float [[s36]]
// CHECK:                OpStore [[s33]] [[s38]]

struct Stru {
  int x;
  int y;
};

[numthreads(1,1,1)]
void main()
{
    Stru stru;
    foo(output[0].x, stru.y);
}
