// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Tests struct variable initialization from different storage class

struct S {
    float4 pos : POSITION;
};

cbuffer Constants {
  S uniform_struct;
}

// uniform_struct is of type %S, while (fn_)private_struct and s is of type %S_0.
// So we need to decompose and assign for uniform_struct -> (fn_)private_struct,
// but not private_struct -> s.

// CHECK-LABEL:           %main = OpFunction

// CHECK:          [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %Constants %int_0
// CHECK-NEXT: [[uniform:%[0-9]+]] = OpLoad %S [[ptr]]
// CHECK-NEXT:     [[vec:%[0-9]+]] = OpCompositeExtract %v4float [[uniform]] 0
// CHECK-NEXT:  [[struct:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec]]
// CHECK-NEXT:                    OpStore %private_struct [[struct]]
static const S private_struct = uniform_struct; // Unifrom -> Private

float4 foo();

S main()
// CHECK-LABEL:       %src_main = OpFunction
{
    S Out;
// CHECK:      [[private:%[0-9]+]] = OpLoad %S_0 %private_struct
// CHECK-NEXT:                    OpStore %s [[private]]
    S s = private_struct; // Private -> Function
    Out.pos = s.pos + foo();
    return Out;
}

float4 foo()
// CHECK-LABEL:            %foo = OpFunction
{
// CHECK:          [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %Constants %int_0
// CHECK-NEXT: [[uniform_0:%[0-9]+]] = OpLoad %S [[ptr_0]]
// CHECK-NEXT:     [[vec_0:%[0-9]+]] = OpCompositeExtract %v4float [[uniform_0]] 0
// CHECK-NEXT:  [[struct_0:%[0-9]+]] = OpCompositeConstruct %S_0 [[vec_0]]
// CHECK-NEXT:                    OpStore %fn_private_struct [[struct_0]]
    static S fn_private_struct = uniform_struct; // Uniform -> Private
    return fn_private_struct.pos;
}
