// RUN: %dxc -T ds_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Tessellation
// CHECK: OpCapability ClipDistance
// CHECK: OpCapability CullDistance

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// HS PCF output

struct HsPcfOut {
  float  outTessFactor[4]   : SV_TessFactor;        // Builtin TessLevelOuter
  float  inTessFactor[2]    : SV_InsideTessFactor;  // Builtin TessLevelInner

  float3 foo                : FOO;                  // Input variable
};

// Per-vertex input structs

struct Inner2PerVertexIn {
  float               clip3 : SV_ClipDistance3;     // Builtin ClipDistance
  float4              texco : TEXCOORD;             // Input variable
};

struct InnerPerVertexIn {
  float4              pos   : SV_Position;          // Builtin Position
  float               clip0 : SV_ClipDistance0;     // Builtin ClipDistance
  Inner2PerVertexIn   s;
  float2              cull2 : SV_CullDistance2;     // Builtin CullDistance
};

struct PerVertexIn {
  float4              cull3 : SV_CullDistance3;     // Builtin CullDistance
  InnerPerVertexIn    s;
  float2              bar   : BAR;                  // Input variable
  [[vk::builtin("PointSize")]]
  float  ptSize             : PSIZE;                // Builtin PointSize
};

// Per-vertex output structs

struct Inner2PerVertexOut {
  float3 foo                : FOO;                  // Output variable
  float2 cull3              : SV_CullDistance3;     // Builtin CullDistance
  float  clip0              : SV_ClipDistance0;     // Builtin ClipDistance
};

struct InnerPerVertexOut {
  Inner2PerVertexOut s;
  float2 cull4              : SV_CullDistance4;     // Builtin CullDistance
  float4 bar                : BAR;                  // Output variable
  [[vk::builtin("PointSize")]]
  float  ptSize             : PSIZE;                // Builtin PointSize
};

struct DsOut {
  float4 pos                : SV_Position;
  InnerPerVertexOut s;
};

// Per-vertex    input  builtin : Position, PointSize, ClipDistance, CullDistance
// Per-vertex    output builtin : Position, PointSize, ClipDistance, CullDistance
// Per-vertex    input  variable: TEXCOORD, BAR
// Per-vertex    output variable: FOO, BAR

// Per-primitive input builtin  : TessLevelInner, TessLevelOuter, TessCoord (SV_DomainLocation)
// Per-primitive input variable : FOO

// CHECK: OpEntryPoint TessellationEvaluation %main "main" %gl_ClipDistance %gl_CullDistance %gl_ClipDistance_0 %gl_CullDistance_0 %gl_Position %in_var_TEXCOORD %in_var_BAR %gl_PointSize %gl_TessCoord %gl_TessLevelOuter %gl_TessLevelInner %in_var_FOO %gl_Position_0 %out_var_FOO %out_var_BAR %gl_PointSize_0

// CHECK: OpDecorate %gl_ClipDistance BuiltIn ClipDistance
// CHECK: OpDecorateString %gl_ClipDistance UserSemantic "SV_ClipDistance"
// CHECK: OpDecorate %gl_CullDistance BuiltIn CullDistance
// CHECK: OpDecorateString %gl_CullDistance UserSemantic "SV_CullDistance"
// CHECK: OpDecorate %gl_ClipDistance_0 BuiltIn ClipDistance
// CHECK: OpDecorateString %gl_ClipDistance_0 UserSemantic "SV_ClipDistance"
// CHECK: OpDecorate %gl_CullDistance_0 BuiltIn CullDistance
// CHECK: OpDecorateString %gl_CullDistance_0 UserSemantic "SV_CullDistance"

// CHECK: OpDecorate %gl_Position BuiltIn Position
// CHECK: OpDecorateString %gl_Position UserSemantic "SV_Position"
// CHECK: OpDecorateString %in_var_TEXCOORD UserSemantic "TEXCOORD"
// CHECK: OpDecorateString %in_var_BAR UserSemantic "BAR"
// CHECK: OpDecorate %gl_PointSize BuiltIn PointSize
// CHECK: OpDecorateString %gl_PointSize UserSemantic "PSIZE"
// CHECK: OpDecorate %gl_TessCoord BuiltIn TessCoord
// CHECK: OpDecorateString %gl_TessCoord UserSemantic "SV_DomainLocation"
// CHECK: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
// CHECK: OpDecorateString %gl_TessLevelOuter UserSemantic "SV_TessFactor"
// CHECK: OpDecorate %gl_TessLevelOuter Patch
// CHECK: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
// CHECK: OpDecorateString %gl_TessLevelInner UserSemantic "SV_InsideTessFactor"
// CHECK: OpDecorate %gl_TessLevelInner Patch
// CHECK: OpDecorateString %in_var_FOO UserSemantic "FOO"
// CHECK: OpDecorate %in_var_FOO Patch

// CHECK: OpDecorate %gl_Position_0 BuiltIn Position
// CHECK: OpDecorateString %gl_Position_0 UserSemantic "SV_Position"
// CHECK: OpDecorateString %out_var_FOO UserSemantic "FOO"
// CHECK: OpDecorateString %out_var_BAR UserSemantic "BAR"
// CHECK: OpDecorate %gl_PointSize_0 BuiltIn PointSize
// CHECK: OpDecorateString %gl_PointSize_0 UserSemantic "PSIZE"
// CHECK: OpDecorate %in_var_BAR Location 0
// CHECK: OpDecorate %in_var_FOO Location 1
// CHECK: OpDecorate %in_var_TEXCOORD Location 2
// CHECK: OpDecorate %out_var_FOO Location 0
// CHECK: OpDecorate %out_var_BAR Location 1

// Input : clip0 + clip3 : 2 floats
// Input : cull2 + cull3 : 6 floats

// Output: clip0 + clip5 : 4 floats
// Output: cull3 + cull4 : 4 floats

// CHECK:   %gl_ClipDistance = OpVariable %_ptr_Input__arr__arr_float_uint_2_uint_3 Input
// CHECK:   %gl_CullDistance = OpVariable %_ptr_Input__arr__arr_float_uint_6_uint_3 Input
// CHECK: %gl_ClipDistance_0 = OpVariable %_ptr_Output__arr_float_uint_4 Output
// CHECK: %gl_CullDistance_0 = OpVariable %_ptr_Output__arr_float_uint_4 Output

// CHECK:       %gl_Position = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
// CHECK:   %in_var_TEXCOORD = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
// CHECK:        %in_var_BAR = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
// CHECK:      %gl_PointSize = OpVariable %_ptr_Input__arr_float_uint_3 Input
// CHECK:      %gl_TessCoord = OpVariable %_ptr_Input_v3float Input
// CHECK: %gl_TessLevelOuter = OpVariable %_ptr_Input__arr_float_uint_4 Input
// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input
// CHECK:        %in_var_FOO = OpVariable %_ptr_Input_v3float Input

// CHECK:     %gl_Position_0 = OpVariable %_ptr_Output_v4float Output
// CHECK:       %out_var_FOO = OpVariable %_ptr_Output_v3float Output
// CHECK:       %out_var_BAR = OpVariable %_ptr_Output_v4float Output
// CHECK:    %gl_PointSize_0 = OpVariable %_ptr_Output_float Output

[domain("quad")]
DsOut main(    const OutputPatch<PerVertexIn, 3> patch,
               float2   loc     : SV_DomainLocation,
               HsPcfOut pcfData,
           out float3   clip5   : SV_ClipDistance5)     // Builtin ClipDistance
{
  DsOut dsOut;
  dsOut = (DsOut)0;
  dsOut.pos = patch[0].ptSize;
  clip5 = pcfData.foo + float3(loc, 1);
  return dsOut;
// Layout of input ClipDistance array:
//   clip0: 1 floats, offset 0
//   clip3: 1 floats, offset 1

// Layout of input CullDistance array:
//   cull2: 2 floats, offset 0
//   cull3: 4 floats, offset 2

// Layout of output ClipDistance array:
//   clip0: 1 floats, offset 0
//   clip5: 3 floats, offset 1

// Layout of output CullDistance array:
//   cull3: 2 floats, offset 0
//   cull4: 2 floats, offset 2

// Read gl_CullDistance[0] and compose patch[0].cull3 (SV_CullDistance3)
// CHECK:              [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_2
// CHECK-NEXT:         [[val0:%[0-9]+]] = OpLoad %float [[ptr0]]
// CHECK-NEXT:         [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_3
// CHECK-NEXT:         [[val1:%[0-9]+]] = OpLoad %float [[ptr1]]
// CHECK-NEXT:         [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_4
// CHECK-NEXT:         [[val2:%[0-9]+]] = OpLoad %float [[ptr2]]
// CHECK-NEXT:         [[ptr3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_5
// CHECK-NEXT:         [[val3:%[0-9]+]] = OpLoad %float [[ptr3]]
// CHECK-NEXT:  [[patch0cull3:%[0-9]+]] = OpCompositeConstruct %v4float [[val0]] [[val1]] [[val2]] [[val3]]

// Read gl_CullDistance[1] and compose patch[1].cull3 (SV_CullDistance3)
// CHECK-NEXT:         [[ptr0_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_2
// CHECK-NEXT:         [[val0_0:%[0-9]+]] = OpLoad %float [[ptr0_0]]
// CHECK-NEXT:         [[ptr1_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_3
// CHECK-NEXT:         [[val1_0:%[0-9]+]] = OpLoad %float [[ptr1_0]]
// CHECK-NEXT:         [[ptr2_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_4
// CHECK-NEXT:         [[val2_0:%[0-9]+]] = OpLoad %float [[ptr2_0]]
// CHECK-NEXT:         [[ptr3_0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_5
// CHECK-NEXT:         [[val3_0:%[0-9]+]] = OpLoad %float [[ptr3_0]]
// CHECK-NEXT:  [[patch1cull3:%[0-9]+]] = OpCompositeConstruct %v4float [[val0_0]] [[val1_0]] [[val2_0]] [[val3_0]]

// Read gl_CullDistance[2] and compose patch[2].cull3 (SV_CullDistance3)
// CHECK-NEXT:         [[ptr0_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_2
// CHECK-NEXT:         [[val0_1:%[0-9]+]] = OpLoad %float [[ptr0_1]]
// CHECK-NEXT:         [[ptr1_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_3
// CHECK-NEXT:         [[val1_1:%[0-9]+]] = OpLoad %float [[ptr1_1]]
// CHECK-NEXT:         [[ptr2_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_4
// CHECK-NEXT:         [[val2_1:%[0-9]+]] = OpLoad %float [[ptr2_1]]
// CHECK-NEXT:         [[ptr3_1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_5
// CHECK-NEXT:         [[val3_1:%[0-9]+]] = OpLoad %float [[ptr3_1]]
// CHECK-NEXT:  [[patch2cull3:%[0-9]+]] = OpCompositeConstruct %v4float [[val0_1]] [[val1_1]] [[val2_1]] [[val3_1]]

// Compose an array of input SV_CullDistance3 for later use
// CHECK-NEXT:   [[inCull3Arr:%[0-9]+]] = OpCompositeConstruct %_arr_v4float_uint_3 [[patch0cull3]] [[patch1cull3]] [[patch2cull3]]

// Read gl_Position
// CHECK-NEXT:     [[inPosArr:%[0-9]+]] = OpLoad %_arr_v4float_uint_3 %gl_Position

// Read gl_ClipDistance[0] as patch[0].s.clip0 (SV_ClipDistance0)
// CHECK-NEXT:         [[ptr0_2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0 %uint_0
// CHECK-NEXT:         [[val0_2:%[0-9]+]] = OpLoad %float [[ptr0_2]]

// Read gl_ClipDistance[1] as patch[1].s.clip0 (SV_ClipDistance0)
// CHECK-NEXT:         [[ptr1_2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_1 %uint_0
// CHECK-NEXT:         [[val1_2:%[0-9]+]] = OpLoad %float [[ptr1_2]]

// Read gl_ClipDistance[2] as patch[2].s.clip0 (SV_ClipDistance0)
// CHECK-NEXT:         [[ptr2_2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_2 %uint_0
// CHECK-NEXT:         [[val2_2:%[0-9]+]] = OpLoad %float [[ptr2_2]]

// Compose an array of input SV_ClipDistance0 for later use
// CHECK-NEXT:   [[inClip0Arr:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_3 [[val0_2]] [[val1_2]] [[val2_2]]

// Read gl_ClipDistance[0] as patch[0].s.s.clip3 (SV_ClipDistance3)
// CHECK-NEXT:         [[ptr0_3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0 %uint_1
// CHECK-NEXT:         [[val0_3:%[0-9]+]] = OpLoad %float [[ptr0_3]]

// Read gl_ClipDistance[1] as patch[1].s.s.clip3 (SV_ClipDistance3)
// CHECK-NEXT:         [[ptr1_3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_1 %uint_1
// CHECK-NEXT:         [[val1_3:%[0-9]+]] = OpLoad %float [[ptr1_3]]

// Read gl_ClipDistance[2] as patch[2].s.s.clip3 (SV_ClipDistance3)
// CHECK-NEXT:         [[ptr2_3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_2 %uint_1
// CHECK-NEXT:         [[val2_3:%[0-9]+]] = OpLoad %float [[ptr2_3]]

// Compose an array of input SV_ClipDistance3 for later use
// CHECK-NEXT:   [[inClip3Arr:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_3 [[val0_3]] [[val1_3]] [[val2_3]]

// CHECK-NEXT:      [[texcord:%[0-9]+]] = OpLoad %_arr_v4float_uint_3 %in_var_TEXCOORD

// Decompose temporary arrays created before to compose Inner2PerVertexIn

// CHECK-NEXT:  [[inClip3Arr0:%[0-9]+]] = OpCompositeExtract %float [[inClip3Arr]] 0
// CHECK-NEXT:     [[texcord0:%[0-9]+]] = OpCompositeExtract %v4float [[texcord]] 0
// CHECK-NEXT:         [[val0_4:%[0-9]+]] = OpCompositeConstruct %Inner2PerVertexIn [[inClip3Arr0]] [[texcord0]]

// CHECK-NEXT:  [[inClip3Arr1:%[0-9]+]] = OpCompositeExtract %float [[inClip3Arr]] 1
// CHECK-NEXT:     [[texcord1:%[0-9]+]] = OpCompositeExtract %v4float [[texcord]] 1
// CHECK-NEXT:         [[val1_4:%[0-9]+]] = OpCompositeConstruct %Inner2PerVertexIn [[inClip3Arr1]] [[texcord1]]

// CHECK-NEXT:  [[inClip3Arr2:%[0-9]+]] = OpCompositeExtract %float [[inClip3Arr]] 2
// CHECK-NEXT:     [[texcord2:%[0-9]+]] = OpCompositeExtract %v4float [[texcord]] 2
// CHECK-NEXT:         [[val2_4:%[0-9]+]] = OpCompositeConstruct %Inner2PerVertexIn [[inClip3Arr2]] [[texcord2]]

// Compose an array of input Inner2PerVertexIn for later use
// CHECK-NEXT:   [[inIn2PVArr:%[0-9]+]] = OpCompositeConstruct %_arr_Inner2PerVertexIn_uint_3 [[val0_4]] [[val1_4]] [[val2_4]]

// Read gl_CullDistance[0] as patch[0].s.cull2 (SV_CullDistance2)
// CHECK-NEXT:         [[ptr0_4:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_0
// CHECK-NEXT:         [[val0_5:%[0-9]+]] = OpLoad %float [[ptr0_4]]
// CHECK-NEXT:         [[ptr1_4:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_0 %uint_1
// CHECK-NEXT:         [[val1_5:%[0-9]+]] = OpLoad %float [[ptr1_4]]
// CHECK-NEXT:  [[patch0cull2:%[0-9]+]] = OpCompositeConstruct %v2float [[val0_5]] [[val1_5]]

// Read gl_CullDistance[1] as patch[1].s.cull2 (SV_CullDistance2)
// CHECK-NEXT:         [[ptr0_5:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_0
// CHECK-NEXT:         [[val0_6:%[0-9]+]] = OpLoad %float [[ptr0_5]]
// CHECK-NEXT:         [[ptr1_5:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_1 %uint_1
// CHECK-NEXT:         [[val1_6:%[0-9]+]] = OpLoad %float [[ptr1_5]]
// CHECK-NEXT:  [[patch1cull2:%[0-9]+]] = OpCompositeConstruct %v2float [[val0_6]] [[val1_6]]

// Read gl_CullDistance[2] as patch[2].s.cull2 (SV_CullDistance2)
// CHECK-NEXT:         [[ptr0_6:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_0
// CHECK-NEXT:         [[val0_7:%[0-9]+]] = OpLoad %float [[ptr0_6]]
// CHECK-NEXT:         [[ptr1_6:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_CullDistance %uint_2 %uint_1
// CHECK-NEXT:         [[val1_7:%[0-9]+]] = OpLoad %float [[ptr1_6]]
// CHECK-NEXT:  [[patch2cull2:%[0-9]+]] = OpCompositeConstruct %v2float [[val0_7]] [[val1_7]]

// Compose an array of input SV_CullDistance2 for later use
// CHECK-NEXT:   [[inCull2Arr:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_3 [[patch0cull2]] [[patch1cull2]] [[patch2cull2]]

// Decompose temporary arrays created before to compose InnerPerVertexIn

// CHECK-NEXT:       [[field0:%[0-9]+]] = OpCompositeExtract %v4float [[inPosArr]] 0
// CHECK-NEXT:       [[field1:%[0-9]+]] = OpCompositeExtract %float [[inClip0Arr]] 0
// CHECK-NEXT:       [[field2:%[0-9]+]] = OpCompositeExtract %Inner2PerVertexIn [[inIn2PVArr]] 0
// CHECK-NEXT:       [[field3:%[0-9]+]] = OpCompositeExtract %v2float [[inCull2Arr]] 0
// CHECK-NEXT:         [[val0_8:%[0-9]+]] = OpCompositeConstruct %InnerPerVertexIn [[field0]] [[field1]] [[field2]] [[field3]]

// CHECK-NEXT:       [[field0_0:%[0-9]+]] = OpCompositeExtract %v4float [[inPosArr]] 1
// CHECK-NEXT:       [[field1_0:%[0-9]+]] = OpCompositeExtract %float [[inClip0Arr]] 1
// CHECK-NEXT:       [[field2_0:%[0-9]+]] = OpCompositeExtract %Inner2PerVertexIn [[inIn2PVArr]] 1
// CHECK-NEXT:       [[field3_0:%[0-9]+]] = OpCompositeExtract %v2float [[inCull2Arr]] 1
// CHECK-NEXT:         [[val1_8:%[0-9]+]] = OpCompositeConstruct %InnerPerVertexIn [[field0_0]] [[field1_0]] [[field2_0]] [[field3_0]]

// CHECK-NEXT:       [[field0_1:%[0-9]+]] = OpCompositeExtract %v4float [[inPosArr]] 2
// CHECK-NEXT:       [[field1_1:%[0-9]+]] = OpCompositeExtract %float [[inClip0Arr]] 2
// CHECK-NEXT:       [[field2_1:%[0-9]+]] = OpCompositeExtract %Inner2PerVertexIn [[inIn2PVArr]] 2
// CHECK-NEXT:       [[field3_1:%[0-9]+]] = OpCompositeExtract %v2float [[inCull2Arr]] 2
// CHECK-NEXT:         [[val2_5:%[0-9]+]] = OpCompositeConstruct %InnerPerVertexIn [[field0_1]] [[field1_1]] [[field2_1]] [[field3_1]]

// Compose an array of input InnerPerVertexIn for later use
// CHECK-NEXT:    [[inInPVArr:%[0-9]+]] = OpCompositeConstruct %_arr_InnerPerVertexIn_uint_3 [[val0_8]] [[val1_8]] [[val2_5]]

// CHECK-NEXT:     [[inBarArr:%[0-9]+]] = OpLoad %_arr_v2float_uint_3 %in_var_BAR

// Read gl_PointSize
// CHECK-NEXT:  [[inPtSizeArr:%[0-9]+]] = OpLoad %_arr_float_uint_3 %gl_PointSize

// Decompose temporary arrays created before to compose PerVertexIn

// CHECK-NEXT:       [[field0_2:%[0-9]+]] = OpCompositeExtract %v4float [[inCull3Arr]] 0
// CHECK-NEXT:       [[field1_2:%[0-9]+]] = OpCompositeExtract %InnerPerVertexIn [[inInPVArr]] 0
// CHECK-NEXT:       [[field2_2:%[0-9]+]] = OpCompositeExtract %v2float [[inBarArr]] 0
// CHECK-NEXT:       [[field3_2:%[0-9]+]] = OpCompositeExtract %float [[inPtSizeArr]] 0
// CHECK-NEXT:         [[val0_9:%[0-9]+]] = OpCompositeConstruct %PerVertexIn [[field0_2]] [[field1_2]] [[field2_2]] [[field3_2]]

// CHECK-NEXT:       [[field0_3:%[0-9]+]] = OpCompositeExtract %v4float [[inCull3Arr]] 1
// CHECK-NEXT:       [[field1_3:%[0-9]+]] = OpCompositeExtract %InnerPerVertexIn [[inInPVArr]] 1
// CHECK-NEXT:       [[field2_3:%[0-9]+]] = OpCompositeExtract %v2float [[inBarArr]] 1
// CHECK-NEXT:       [[field3_3:%[0-9]+]] = OpCompositeExtract %float [[inPtSizeArr]] 1
// CHECK-NEXT:         [[val1_9:%[0-9]+]] = OpCompositeConstruct %PerVertexIn [[field0_3]] [[field1_3]] [[field2_3]] [[field3_3]]

// CHECK-NEXT:       [[field0_4:%[0-9]+]] = OpCompositeExtract %v4float [[inCull3Arr]] 2
// CHECK-NEXT:       [[field1_4:%[0-9]+]] = OpCompositeExtract %InnerPerVertexIn [[inInPVArr]] 2
// CHECK-NEXT:       [[field2_4:%[0-9]+]] = OpCompositeExtract %v2float [[inBarArr]] 2
// CHECK-NEXT:       [[field3_4:%[0-9]+]] = OpCompositeExtract %float [[inPtSizeArr]] 2
// CHECK-NEXT:         [[val2_6:%[0-9]+]] = OpCompositeConstruct %PerVertexIn [[field0_4]] [[field1_4]] [[field2_4]] [[field3_4]]

// The final value for the patch parameter!
// CHECK-NEXT:        [[patch:%[0-9]+]] = OpCompositeConstruct %_arr_PerVertexIn_uint_3 [[val0_9]] [[val1_9]] [[val2_6]]
// CHECK-NEXT:                         OpStore %param_var_patch [[patch]]

// Write SV_DomainLocation to tempoary variable for function call
// CHECK-NEXT:    [[tesscoord:%[0-9]+]] = OpLoad %v3float %gl_TessCoord
// CHECK-NEXT:      [[shuffle:%[0-9]+]] = OpVectorShuffle %v2float [[tesscoord]] [[tesscoord]] 0 1
// CHECK-NEXT:                         OpStore %param_var_loc [[shuffle]]

// Compose pcfData and write to tempoary variable for function call
// CHECK-NEXT:          [[tlo:%[0-9]+]] = OpLoad %_arr_float_uint_4 %gl_TessLevelOuter
// CHECK-NEXT:          [[tli:%[0-9]+]] = OpLoad %_arr_float_uint_2 %gl_TessLevelInner
// CHECK-NEXT:        [[inFoo:%[0-9]+]] = OpLoad %v3float %in_var_FOO
// CHECK-NEXT:      [[pcfData:%[0-9]+]] = OpCompositeConstruct %HsPcfOut [[tlo]] [[tli]] [[inFoo]]
// CHECK-NEXT:                         OpStore %param_var_pcfData [[pcfData]]

// Make the call!
// CHECK-NEXT:          [[ret:%[0-9]+]] = OpFunctionCall %DsOut %src_main %param_var_patch %param_var_loc %param_var_pcfData %param_var_clip5

// Decompose DsOut and write out output SV_Position
// CHECK-NEXT:       [[outPos:%[0-9]+]] = OpCompositeExtract %v4float [[ret]] 0
// CHECK-NEXT:                         OpStore %gl_Position_0 [[outPos]]

// CHECK-NEXT:      [[outInPV:%[0-9]+]] = OpCompositeExtract %InnerPerVertexOut [[ret]] 1
// CHECK-NEXT:     [[outIn2PV:%[0-9]+]] = OpCompositeExtract %Inner2PerVertexOut [[outInPV]] 0

// Decompose Inner2PerVertexOut and write out DsOut.s.s.foo (FOO)
// CHECK-NEXT:          [[foo:%[0-9]+]] = OpCompositeExtract %v3float [[outIn2PV]] 0
// CHECK-NEXT:                         OpStore %out_var_FOO [[foo]]

// Decompose Inner2PerVertexOut and write out DsOut.s.s.cull3 (SV_CullDistance3) at offset 0
// CHECK-NEXT:        [[cull3:%[0-9]+]] = OpCompositeExtract %v2float [[outIn2PV]] 1
// CHECK-NEXT:        [[ind00:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_0
// CHECK-NEXT:         [[ptr0_7:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 [[ind00]]
// CHECK-NEXT:         [[val0_10:%[0-9]+]] = OpCompositeExtract %float [[cull3]] 0
// CHECK-NEXT:                         OpStore [[ptr0_7]] [[val0_10]]
// CHECK-NEXT:        [[ind01:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_1
// CHECK-NEXT:         [[ptr1_7:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 [[ind01]]
// CHECK-NEXT:         [[val1_10:%[0-9]+]] = OpCompositeExtract %float [[cull3]] 1
// CHECK-NEXT:                         OpStore [[ptr1_7]] [[val1_10]]

// Decompose Inner2PerVertexOut and write out DsOut.s.s.clip0 (SV_ClipDistance0) at offset 0
// CHECK-NEXT:        [[clip0:%[0-9]+]] = OpCompositeExtract %float [[outIn2PV]] 2
// CHECK-NEXT:          [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 %uint_0
// CHECK-NEXT:                         OpStore [[ptr]] [[clip0]]

// Decompose InnerPerVertexOut and write out DsOut.s.cull4 (SV_CullDistance4) at offset 2
// CHECK-NEXT:        [[cull4:%[0-9]+]] = OpCompositeExtract %v2float [[outInPV]] 1
// CHECK-NEXT:        [[ind20:%[0-9]+]] = OpIAdd %uint %uint_2 %uint_0
// CHECK-NEXT:         [[ptr0_8:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 [[ind20]]
// CHECK-NEXT:         [[val0_11:%[0-9]+]] = OpCompositeExtract %float [[cull4]] 0
// CHECK-NEXT:                         OpStore [[ptr0_8]] [[val0_11]]
// CHECK-NEXT:        [[ind21:%[0-9]+]] = OpIAdd %uint %uint_2 %uint_1
// CHECK-NEXT:         [[ptr1_8:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_CullDistance_0 [[ind21]]
// CHECK-NEXT:         [[val1_11:%[0-9]+]] = OpCompositeExtract %float [[cull4]] 1
// CHECK-NEXT:                         OpStore [[ptr1_8]] [[val1_11]]

// Decompose InnerPerVertexOut and write out DsOut.s.bar (BAR)
// CHECK-NEXT:          [[bar:%[0-9]+]] = OpCompositeExtract %v4float [[outInPV]] 2
// CHECK-NEXT:                         OpStore %out_var_BAR [[bar]]

// Decompose InnerPerVertexOut and write out DsOut.s.ptSize (PointSize)
// CHECK-NEXT:       [[ptSize:%[0-9]+]] = OpCompositeExtract %float [[outInPV]] 3
// CHECK-NEXT:                         OpStore %gl_PointSize_0 [[ptSize]]

// Write out clip5 (SV_ClipDistance5) at offset 1
// CHECK-NEXT: [[clip5:%[0-9]+]] = OpLoad %v3float %param_var_clip5
// CHECK-NEXT: [[ind10:%[0-9]+]] = OpIAdd %uint %uint_1 %uint_0
// CHECK-NEXT:  [[ptr0_9:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind10]]
// CHECK-NEXT:  [[val0_12:%[0-9]+]] = OpCompositeExtract %float [[clip5]] 0
// CHECK-NEXT:                  OpStore [[ptr0_9]] [[val0_12]]
// CHECK-NEXT: [[ind11:%[0-9]+]] = OpIAdd %uint %uint_1 %uint_1
// CHECK-NEXT:  [[ptr1_9:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind11]]
// CHECK-NEXT:  [[val1_12:%[0-9]+]] = OpCompositeExtract %float [[clip5]] 1
// CHECK-NEXT:                  OpStore [[ptr1_9]] [[val1_12]]
// CHECK-NEXT: [[ind12:%[0-9]+]] = OpIAdd %uint %uint_1 %uint_2
// CHECK-NEXT:  [[ptr2_4:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance_0 [[ind12]]
// CHECK-NEXT:  [[val2_7:%[0-9]+]] = OpCompositeExtract %float [[clip5]] 2
// CHECK-NEXT:                  OpStore [[ptr2_4]] [[val2_7]]
}
