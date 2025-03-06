// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    bool a : A;
    uint2 b: B;
    float2x3 c: C;
};

struct T {
    S x;
    int y: D;
    bool2 z : E;
};

// CHECK: [[v2uint0:%[0-9]+]] = OpConstantComposite %v2uint %uint_0 %uint_0
// CHECK: [[v2uint1:%[0-9]+]] = OpConstantComposite %v2uint %uint_1 %uint_1

// CHECK:       %in_var_A = OpVariable %_ptr_Input_uint Input
// CHECK-NEXT:  %in_var_B = OpVariable %_ptr_Input_v2uint Input
// CHECK-NEXT:  %in_var_C = OpVariable %_ptr_Input_mat2v3float Input
// CHECK-NEXT:  %in_var_D = OpVariable %_ptr_Input_int Input
// CHECK-NEXT:  %in_var_E = OpVariable %_ptr_Input_v2uint Input

// CHECK-NEXT: %out_var_A = OpVariable %_ptr_Output_uint Output
// CHECK-NEXT: %out_var_B = OpVariable %_ptr_Output_v2uint Output
// CHECK-NEXT: %out_var_C = OpVariable %_ptr_Output_mat2v3float Output
// CHECK-NEXT: %out_var_D = OpVariable %_ptr_Output_int Output
// CHECK-NEXT: %out_var_E = OpVariable %_ptr_Output_v2uint Output

// CHECK-NEXT:        %main = OpFunction %void None {{%[0-9]+}}
// CHECK-NEXT:     {{%[0-9]+}} = OpLabel

// CHECK-NEXT: %param_var_input = OpVariable %_ptr_Function_T Function
// CHECK-NEXT:     [[inA:%[0-9]+]] = OpLoad %uint %in_var_A
// CHECK-NEXT: [[inAbool:%[0-9]+]] = OpINotEqual %bool [[inA]] %uint_0
// CHECK-NEXT:     [[inB:%[0-9]+]] = OpLoad %v2uint %in_var_B
// CHECK-NEXT:     [[inC:%[0-9]+]] = OpLoad %mat2v3float %in_var_C
// CHECK-NEXT:     [[inS:%[0-9]+]] = OpCompositeConstruct %S [[inAbool]] [[inB]] [[inC]]
// CHECK-NEXT:     [[inD:%[0-9]+]] = OpLoad %int %in_var_D
// CHECK-NEXT:     [[inE:%[0-9]+]] = OpLoad %v2uint %in_var_E
// CHECK-NEXT: [[inEbool:%[0-9]+]] = OpINotEqual %v2bool [[inE]] [[v2uint0]]
// CHECK-NEXT:     [[inT:%[0-9]+]] = OpCompositeConstruct %T [[inS]] [[inD]] [[inEbool]]
// CHECK-NEXT:                    OpStore %param_var_input [[inT]]

// CHECK-NEXT:      [[ret:%[0-9]+]] = OpFunctionCall %T %src_main %param_var_input
// CHECK-NEXT:     [[outS:%[0-9]+]] = OpCompositeExtract %S [[ret]] 0
// CHECK-NEXT:     [[outA:%[0-9]+]] = OpCompositeExtract %bool [[outS]] 0
// CHECK-NEXT: [[outAuint:%[0-9]+]] = OpSelect %uint [[outA]] %uint_1 %uint_0
// CHECK-NEXT:                     OpStore %out_var_A [[outAuint]]
// CHECK-NEXT:     [[outB:%[0-9]+]] = OpCompositeExtract %v2uint [[outS]] 1
// CHECK-NEXT:                     OpStore %out_var_B [[outB]]
// CHECK-NEXT:     [[outC:%[0-9]+]] = OpCompositeExtract %mat2v3float [[outS]] 2
// CHECK-NEXT:                     OpStore %out_var_C [[outC]]
// CHECK-NEXT:     [[outD:%[0-9]+]] = OpCompositeExtract %int [[ret]] 1
// CHECK-NEXT:                     OpStore %out_var_D [[outD]]
// CHECK-NEXT:     [[outE:%[0-9]+]] = OpCompositeExtract %v2bool [[ret]] 2
// CHECK-NEXT: [[outEuint:%[0-9]+]] = OpSelect %v2uint [[outE]] [[v2uint1]] [[v2uint0]]
// CHECK-NEXT:                     OpStore %out_var_E [[outEuint]]

// CHECK-NEXT:                 OpReturn
// CHECK-NEXT:                 OpFunctionEnd
T main(T input) {
    return input;
}
