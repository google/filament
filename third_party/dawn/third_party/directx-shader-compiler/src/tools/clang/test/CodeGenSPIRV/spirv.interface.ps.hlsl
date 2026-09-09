// RUN: %dxc -T ps_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ClipDistance
// CHECK: OpCapability CullDistance

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

struct Inner {
    float2 cull2 : SV_CullDistance2;            // Builtin CullDistance
    float3 foo   : FOO;                         // Input variable
};

struct PsIn {
    float4 pos   : SV_Position;                 // Builtin FragCoord
    float2 clip0 : SV_ClipDistance0;            // Builtin ClipDistance
    Inner  s;
};

// CHECK: OpEntryPoint Fragment %main "main" %gl_ClipDistance %gl_CullDistance %gl_FragCoord %in_var_FOO %in_var_BAR %out_var_SV_Target

// CHECK: OpDecorate %gl_ClipDistance BuiltIn ClipDistance
// CHECK: OpDecorateString %gl_ClipDistance UserSemantic "SV_ClipDistance"
// CHECK: OpDecorate %gl_CullDistance BuiltIn CullDistance
// CHECK: OpDecorateString %gl_CullDistance UserSemantic "SV_CullDistance"
// CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
// CHECK: OpDecorateString %gl_FragCoord UserSemantic "SV_Position"
// CHECK: OpDecorateString %in_var_FOO UserSemantic "FOO"
// CHECK: OpDecorateString %in_var_BAR UserSemantic "BAR"
// CHECK: OpDecorateString %out_var_SV_Target UserSemantic "SV_Target"
// CHECK: OpDecorate %in_var_FOO Location 0
// CHECK: OpDecorate %in_var_BAR Location 1
// CHECK: OpDecorate %out_var_SV_Target Location 0

// Input : clip0 + clip1 : 3 floats
// Input : cull1 + cull2 : 5 floats
// CHECK:   %gl_ClipDistance = OpVariable %_ptr_Input__arr_float_uint_3 Input
// CHECK:   %gl_CullDistance = OpVariable %_ptr_Input__arr_float_uint_5 Input
// CHECK:      %gl_FragCoord = OpVariable %_ptr_Input_v4float Input
// CHECK:        %in_var_FOO = OpVariable %_ptr_Input_v3float Input
// CHECK:        %in_var_BAR = OpVariable %_ptr_Input_float Input
// CHECK: %out_var_SV_Target = OpVariable %_ptr_Output_v4float Output

float4 main(   PsIn   psIn,
               float  clip1 : SV_ClipDistance1, // Builtin ClipDistance
               float3 cull1 : SV_CullDistance1, // Builtin CullDistance
            in float  bar   : BAR               // Input variable
           ) : SV_Target {                      // Output variable
    return psIn.pos + float4(clip1 + bar, cull1);
// Layout of input ClipDistance array:
//   clip0: 2 floats, offset 0
//   clip1: 1 floats, offset 2

// Layout of input CullDistance array:
//   cull1: 3 floats, offset 0
//   cull2: 2 floats, offset 3

// CHECK:        [[pos:%[0-9]+]] = OpLoad %v4float %gl_FragCoord

// CHECK-NEXT:  [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0
// CHECK-NEXT:  [[val0:%[0-9]+]] = OpLoad %float [[ptr0]]
// CHECK-NEXT:  [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_1
// CHECK-NEXT:  [[val1:%[0-9]+]] = OpLoad %float [[ptr1]]
// CHECK-NEXT: [[clip0:%[0-9]+]] = OpCompositeConstruct %v2float [[val0]] [[val1]]

// CHECK-NEXT:  [[ptr0_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_3
// CHECK-NEXT:  [[val0_0:%[0-9]+]] = OpLoad %float [[ptr0_0]]
// CHECK-NEXT:  [[ptr1_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_4
// CHECK-NEXT:  [[val1_0:%[0-9]+]] = OpLoad %float [[ptr1_0]]
// CHECK-NEXT: [[cull2:%[0-9]+]] = OpCompositeConstruct %v2float [[val0_0]] [[val1_0]]

// CHECK-NEXT:   [[foo:%[0-9]+]] = OpLoad %v3float %in_var_FOO

// CHECK-NEXT: [[inner:%[0-9]+]] = OpCompositeConstruct %Inner [[cull2]] [[foo]]

// CHECK-NEXT:  [[psin:%[0-9]+]] = OpCompositeConstruct %PsIn [[pos]] [[clip0]] [[inner]]
// CHECK-NEXT:                  OpStore %param_var_psIn [[psin]]

// CHECK-NEXT:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_2
// CHECK-NEXT: [[clip1:%[0-9]+]] = OpLoad %float [[ptr]]
// CHECK-NEXT:                  OpStore %param_var_clip1 [[clip1]]

// CHECK-NEXT:  [[ptr0_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0
// CHECK-NEXT:  [[val0_1:%[0-9]+]] = OpLoad %float [[ptr0_1]]
// CHECK-NEXT:  [[ptr1_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1
// CHECK-NEXT:  [[val1_1:%[0-9]+]] = OpLoad %float [[ptr1_1]]
// CHECK-NEXT:  [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2
// CHECK-NEXT:  [[val2:%[0-9]+]] = OpLoad %float [[ptr2]]
// CHECK-NEXT: [[cull1:%[0-9]+]] = OpCompositeConstruct %v3float [[val0_1]] [[val1_1]] [[val2]]
// CHECK-NEXT:                  OpStore %param_var_cull1 [[cull1]]

// CHECK-NEXT:   [[bar:%[0-9]+]] = OpLoad %float %in_var_BAR
// CHECK-NEXT:                  OpStore %param_var_bar [[bar]]

// CHECK-NEXT:   [[ret:%[0-9]+]] = OpFunctionCall %v4float %src_main %param_var_psIn %param_var_clip1 %param_var_cull1 %param_var_bar
// CHECK-NEXT:                  OpStore %out_var_SV_Target [[ret]]
}
