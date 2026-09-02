// RUN: %dxc -T vs_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ClipDistance
// CHECK: OpCapability CullDistance

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK: OpEntryPoint Vertex %main "main" %gl_ClipDistance %gl_CullDistance %gl_ClipDistance_0 %gl_CullDistance_0 %in_var_TEXCOORD %in_var_SV_Position %in_var_SV_ClipDistance %in_var_SV_CullDistance0 %gl_PointSize %out_var_COLOR %gl_Position %out_var_TEXCOORD

// CHECK: OpDecorate %gl_ClipDistance BuiltIn ClipDistance
// CHECK: OpDecorateString %gl_ClipDistance UserSemantic "SV_ClipDistance"
// CHECK: OpDecorate %gl_CullDistance BuiltIn CullDistance
// CHECK: OpDecorateString %gl_CullDistance UserSemantic "SV_CullDistance0"
// CHECK: OpDecorate %gl_ClipDistance_0 BuiltIn ClipDistance
// CHECK: OpDecorateString %gl_ClipDistance_0 UserSemantic "SV_ClipDistance"
// CHECK: OpDecorate %gl_CullDistance_0 BuiltIn CullDistance
// CHECK: OpDecorateString %gl_CullDistance_0 UserSemantic "SV_CullDistance"

// CHECK: OpDecorateString %in_var_TEXCOORD UserSemantic "TEXCOORD"
// CHECK: OpDecorateString %in_var_SV_Position UserSemantic "SV_Position"
// CHECK: OpDecorateString %in_var_SV_ClipDistance UserSemantic "SV_ClipDistance"
// CHECK: OpDecorateString %in_var_SV_CullDistance0 UserSemantic "SV_CullDistance0"
// CHECK: OpDecorate %gl_PointSize BuiltIn PointSize
// CHECK: OpDecorateString %gl_PointSize UserSemantic "PSize"
// CHECK: OpDecorateString %out_var_COLOR UserSemantic "COLOR"
// CHECK: OpDecorate %gl_Position BuiltIn Position
// CHECK: OpDecorateString %gl_Position UserSemantic "SV_Position"
// CHECK: OpDecorateString %out_var_TEXCOORD UserSemantic "TEXCOORD"

// CHECK: OpDecorate %in_var_TEXCOORD Location 0
// CHECK: OpDecorate %in_var_SV_Position Location 1
// CHECK: OpDecorate %in_var_SV_ClipDistance Location 2
// CHECK: OpDecorate %in_var_SV_CullDistance0 Location 3
// CHECK: OpDecorate %out_var_COLOR Location 0
// CHECK: OpDecorate %out_var_TEXCOORD Location 1

//     clipdis0 + clipdis1            : 5 floats
//     culldis3 + culldis5 + culldis6 : 3 floats

// CHECK: %gl_ClipDistance = OpVariable %_ptr_Input__arr_float_uint_2 Input
// CHECK: %gl_CullDistance = OpVariable %_ptr_Input__arr_float_uint_3 Input
// CHECK: %gl_ClipDistance_0 = OpVariable %_ptr_Output__arr_float_uint_5 Output
// CHECK: %gl_CullDistance_0 = OpVariable %_ptr_Output__arr_float_uint_3 Output

// CHECK: %in_var_TEXCOORD = OpVariable %_ptr_Input_v4float Input
// CHECK: %in_var_SV_Position = OpVariable %_ptr_Input_v4float Input
// CHECK: %in_var_SV_ClipDistance = OpVariable %_ptr_Input_v2float Input
// CHECK: %in_var_SV_CullDistance0 = OpVariable %_ptr_Input_v3float Input
// CHECK: %gl_PointSize = OpVariable %_ptr_Output_float Output

// CHECK: %out_var_COLOR = OpVariable %_ptr_Output_v4float Output
// CHECK: %gl_Position = OpVariable %_ptr_Output_v4float Output
// CHECK: %out_var_TEXCOORD = OpVariable %_ptr_Output_v4float Output

struct InnerInnerStruct {
  float4           position : SV_Position;      // -> BuiltIn Position in gl_Pervertex
};

struct InnerStruct {
  float2           clipdis1 : SV_ClipDistance1; // -> BuiltIn ClipDistance in gl_PerVertex
  InnerInnerStruct s;
};

struct VSOut {
  float4           color    : COLOR;            // -> Output variable
  InnerStruct s;
};

[[vk::builtin("PointSize")]]
float main(out VSOut  vsOut,
           out   float3 clipdis0 : SV_ClipDistance0, // -> BuiltIn ClipDistance in gl_PerVertex
           inout float4 coord    : TEXCOORD,         // -> Input & output variable
           out   float  culldis5 : SV_CullDistance5, // -> BuiltIn CullDistance in gl_PerVertex
           out   float  culldis3 : SV_CullDistance3, // -> BuiltIn CullDistance in gl_PerVertex
           out   float  culldis6 : SV_CullDistance6, // -> BuiltIn CullDistance in gl_PerVertex
           in    float4 inPos    : SV_Position,      // -> Input variable
           in    float2 inClip   : SV_ClipDistance,  // -> Input variable
           in    float3 inCull   : SV_CullDistance0  // -> Input variable
         ) : PSize {                                 // -> Builtin PointSize
    vsOut    = (VSOut)0;
    clipdis0 = 1.;
    coord    = 2.;
    culldis5 = 3.;
    culldis3 = 4.;
    culldis6 = 5.;

    return inPos.x + inClip.x + inCull.x;

// Layout of ClipDistance array:
//   clipdis0: 3 floats, offset 0
//   clipdis1: 2 floats, offset 3

// Layout of CullDistance array:
//   culldis3: 1 floats, offset 0
//   culldis5: 1 floats, offset 1
//   culldis6: 1 floats, offset 2

// CHECK:      [[texcoord:%[0-9]+]] = OpLoad %v4float %in_var_TEXCOORD
// CHECK-NEXT:                     OpStore %param_var_coord [[texcoord]]
// CHECK-NEXT:      [[pos:%[0-9]+]] = OpLoad %v4float %in_var_SV_Position
// CHECK-NEXT:                     OpStore %param_var_inPos [[pos]]
// CHECK-NEXT:   [[inClip:%[0-9]+]] = OpLoad %v2float %in_var_SV_ClipDistance
// CHECK-NEXT:                     OpStore %param_var_inClip [[inClip]]
// CHECK-NEXT:   [[inCull:%[0-9]+]] = OpLoad %v3float %in_var_SV_CullDistance0
// CHECK-NEXT:                     OpStore %param_var_inCull [[inCull]]

// CHECK-NEXT:   [[ptSize:%[0-9]+]] = OpFunctionCall %float %src_main

// CHECK-NEXT:                     OpStore %gl_PointSize [[ptSize]]

// Write out COLOR
// CHECK-NEXT:    [[vsOut:%[0-9]+]] = OpLoad %VSOut %param_var_vsOut
// CHECK-NEXT: [[outColor:%[0-9]+]] = OpCompositeExtract %v4float [[vsOut]] 0
// CHECK-NEXT:                     OpStore %out_var_COLOR [[outColor]]

// CHECK-NEXT:   [[innerS:%[0-9]+]] = OpCompositeExtract %InnerStruct [[vsOut]] 1

// Write out SV_ClipDistance1
// CHECK-NEXT:    [[clip1:%[0-9]+]] = OpCompositeExtract %v2float [[innerS]] 0
// CHECK-NEXT:    [[ind10:%[0-9]+]] = OpIAdd %uint %uint_3 %uint_0
// CHECK-NEXT: [[clipArr3:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind10]]
// CHECK-NEXT:   [[clip10:%[0-9]+]] = OpCompositeExtract %float [[clip1]] 0
// CHECK-NEXT:                     OpStore [[clipArr3]] [[clip10]]
// CHECK-NEXT:    [[ind11:%[0-9]+]] = OpIAdd %uint %uint_3 %uint_1
// CHECK-NEXT: [[clipArr4:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind11]]
// CHECK-NEXT:   [[clip11:%[0-9]+]] = OpCompositeExtract %float [[clip1]] 1
// CHECK-NEXT:                     OpStore [[clipArr4]] [[clip11]]

// CHECK-NEXT: [[inner2S:%[0-9]+]] = OpCompositeExtract %InnerInnerStruct [[innerS]] 1

// Write out SV_Position
// CHECK-NEXT:     [[pos_0:%[0-9]+]] = OpCompositeExtract %v4float [[inner2S]] 0
// CHECK-NEXT:                    OpStore %gl_Position [[pos_0]]

// Write out SV_ClipDistance0
// CHECK-NEXT:    [[clip0:%[0-9]+]] = OpLoad %v3float %param_var_clipdis0
// CHECK-NEXT:    [[ind00:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_0
// CHECK-NEXT: [[clipArr0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind00]]
// CHECK-NEXT:   [[clip00:%[0-9]+]] = OpCompositeExtract %float [[clip0]] 0
// CHECK-NEXT:                     OpStore [[clipArr0]] [[clip00]]
// CHECK-NEXT:    [[ind01:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_1
// CHECK-NEXT: [[clipArr1:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind01]]
// CHECK-NEXT:   [[clip01:%[0-9]+]] = OpCompositeExtract %float [[clip0]] 1
// CHECK-NEXT:                     OpStore [[clipArr1]] [[clip01]]
// CHECK-NEXT:    [[ind02:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_2
// CHECK-NEXT: [[clipArr2:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind02]]
// CHECK-NEXT:   [[clip02:%[0-9]+]] = OpCompositeExtract %float [[clip0]] 2
// CHECK-NEXT:                     OpStore [[clipArr2]] [[clip02]]

// Write out TEXCOORD
// CHECK-NEXT:  [[texcord:%[0-9]+]] = OpLoad %v4float %param_var_coord
// CHECK-NEXT:                     OpStore %out_var_TEXCOORD [[texcord]]

// Write out SV_CullDistance5
// CHECK-NEXT:    [[cull5:%[0-9]+]] = OpLoad %float %param_var_culldis5
// CHECK-NEXT: [[cullArr1:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 %uint_1
// CHECK-NEXT:                     OpStore [[cullArr1]] [[cull5]]

// Write out SV_CullDistance3
// CHECK-NEXT:    [[cull3:%[0-9]+]] = OpLoad %float %param_var_culldis3
// CHECK-NEXT: [[cullArr0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 %uint_0
// CHECK-NEXT:                     OpStore [[cullArr0]] [[cull3]]

// Write out SV_CullDistance6
// CHECK-NEXT:    [[cull6:%[0-9]+]] = OpLoad %float %param_var_culldis6
// CHECK-NEXT: [[cullArr2:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 %uint_2
// CHECK-NEXT:                     OpStore [[cullArr2]] [[cull6]]
}
