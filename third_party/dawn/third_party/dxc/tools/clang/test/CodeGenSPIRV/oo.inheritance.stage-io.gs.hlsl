// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Empty { };

struct Base : Empty {
    float4 a  : AAA;
    float4 pos: SV_Position;
};

struct Derived : Base {
    float4 b  : BBB;
};

// CHECK-LABEL: %main = OpFunction

// CHECK:         [[empty0:%[0-9]+]] = OpCompositeConstruct %Empty
// CHECK-NEXT:    [[empty1:%[0-9]+]] = OpCompositeConstruct %Empty
// CHECK-NEXT: [[empty_arr:%[0-9]+]] = OpCompositeConstruct %_arr_Empty_uint_2 [[empty0]] [[empty1]]

// CHECK-NEXT:     [[a_arr:%[0-9]+]] = OpLoad %_arr_v4float_uint_2 %in_var_AAA

// CHECK-NEXT:   [[pos_arr:%[0-9]+]] = OpLoad %_arr_v4float_uint_2 %gl_Position

// CHECK-NEXT:    [[empty0_0:%[0-9]+]] = OpCompositeExtract %Empty [[empty_arr]] 0
// CHECK-NEXT:        [[a0:%[0-9]+]] = OpCompositeExtract %v4float [[a_arr]] 0
// CHECK-NEXT:      [[pos0:%[0-9]+]] = OpCompositeExtract %v4float [[pos_arr]] 0
// CHECK-NEXT:     [[base0:%[0-9]+]] = OpCompositeConstruct %Base [[empty0_0]] [[a0]] [[pos0]]

// CHECK-NEXT:    [[empty1_0:%[0-9]+]] = OpCompositeExtract %Empty [[empty_arr]] 1
// CHECK-NEXT:        [[a1:%[0-9]+]] = OpCompositeExtract %v4float [[a_arr]] 1
// CHECK-NEXT:      [[pos1:%[0-9]+]] = OpCompositeExtract %v4float [[pos_arr]] 1
// CHECK-NEXT:     [[base1:%[0-9]+]] = OpCompositeConstruct %Base [[empty1_0]] [[a1]] [[pos1]]

// CHECK-NEXT:  [[base_arr:%[0-9]+]] = OpCompositeConstruct %_arr_Base_uint_2 [[base0]] [[base1]]

// CHECK-NEXT:     [[b_arr:%[0-9]+]] = OpLoad %_arr_v4float_uint_2 %in_var_BBB

// CHECK-NEXT:     [[base0_0:%[0-9]+]] = OpCompositeExtract %Base [[base_arr]] 0
// CHECK-NEXT:        [[b0:%[0-9]+]] = OpCompositeExtract %v4float [[b_arr]] 0
// CHECK-NEXT:  [[derived0:%[0-9]+]] = OpCompositeConstruct %Derived [[base0_0]] [[b0]]

// CHECK-NEXT:     [[base1_0:%[0-9]+]] = OpCompositeExtract %Base [[base_arr]] 1
// CHECK-NEXT:        [[b1:%[0-9]+]] = OpCompositeExtract %v4float [[b_arr]] 1
// CHECK-NEXT:  [[derived1:%[0-9]+]] = OpCompositeConstruct %Derived [[base1_0]] [[b1]]

// CHECK-NEXT:    [[inData:%[0-9]+]] = OpCompositeConstruct %_arr_Derived_uint_2 [[derived0]] [[derived1]]
// CHECK-NEXT:                      OpStore %param_var_inData [[inData]]

// CHECK-LABEL: %src_main = OpFunction

[maxvertexcount(2)]
void main(in    line Derived             inData[2],
          inout      LineStream<Derived> outData)
{
// CHECK:            [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_Derived %inData %int_0
// CHECK-NEXT:   [[inData0:%[0-9]+]] = OpLoad %Derived [[ptr]]
// CHECK-NEXT:      [[base:%[0-9]+]] = OpCompositeExtract %Base [[inData0]] 0

// CHECK-NEXT:           {{%[0-9]+}} = OpCompositeExtract %Empty [[base]] 0

// CHECK-NEXT:         [[a:%[0-9]+]] = OpCompositeExtract %v4float [[base]] 1
// CHECK-NEXT:                      OpStore %out_var_AAA [[a]]

// CHECK-NEXT:       [[pos:%[0-9]+]] = OpCompositeExtract %v4float [[base]] 2
// CHECK-NEXT:                      OpStore %gl_Position_0 [[pos]]

// CHECK-NEXT:         [[b:%[0-9]+]] = OpCompositeExtract %v4float [[inData0]] 1
// CHECK-NEXT:                      OpStore %out_var_BBB [[b]]

// CHECK-NEXT:       OpEmitVertex
    outData.Append(inData[0]);

    outData.RestartStrip();
}
