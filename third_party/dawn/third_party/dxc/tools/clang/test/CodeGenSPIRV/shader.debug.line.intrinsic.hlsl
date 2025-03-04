// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -no-warnings -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void %2 DebugSource [[file]]

static int dest_i;

void main() {
  float2 v2f;
  uint4 v4i;
  float2x2 m2x2f;

// CHECK:                     DebugLine [[src]] %uint_16 %uint_22 %uint_3 %uint_14
// CHECK-NEXT: [[mod:%[0-9]+]] = OpExtInst %ModfStructType {{%[0-9]+}} ModfStruct {{%[0-9]+}}
// CHECK-NEXT:     {{%[0-9]+}} = OpCompositeExtract %v2float [[mod]] 1
  modf(v2f,
// CHECK:      [[mod_0:%[0-9]+]] = OpConvertFToU %v2uint {{%[0-9]+}}
// CHECK-NEXT:                DebugLine [[src]] %uint_22 %uint_22 %uint_8 %uint_8
// CHECK-NEXT: [[v4i:%[0-9]+]] = OpLoad %v4uint %v4i
// CHECK-NEXT: [[v4i_0:%[0-9]+]] = OpVectorShuffle %v4uint [[v4i]] [[mod_0]] 4 5 2 3
// CHECK-NEXT:                OpStore %v4i [[v4i_0]]
       v4i.xy);

// CHECK:                      DebugLine [[src]] %uint_29 %uint_29 %uint_9 %uint_9
// CHECK-NEXT: [[v4ix:%[0-9]+]] = OpCompositeExtract %uint {{%[0-9]+}} 0
// CHECK-NEXT:      {{%[0-9]+}} = OpShiftLeftLogical %uint [[v4ix]] %uint_8
// CHECK-NEXT:      {{%[0-9]+}} = OpShiftLeftLogical %uint [[v4ix]] %uint_16
// CHECK-NEXT:      {{%[0-9]+}} = OpShiftLeftLogical %uint [[v4ix]] %uint_24
  v4i = msad4(v4i.x, v4i.xy, v4i);
// CHECK:      DebugLine [[src]] %uint_29 %uint_29 %uint_3 %uint_33
// CHECK-NEXT: OpStore %v4i {{%[0-9]+}}

// CHECK:      DebugLine [[src]] %uint_35 %uint_35 %uint_23 %uint_67
// CHECK:      OpExtInst %v2float {{%[0-9]+}} Fma
  /* comment */ v4i = mad(m2x2f, float2x2(v4i), float2x2(v2f, v2f));
// CHECK:      DebugLine [[src]] %uint_35 %uint_35 %uint_17 %uint_67
// CHECK-NEXT: OpStore %v4i

// CHECK:                 DebugLine [[src]] %uint_41 %uint_41 %uint_9 %uint_23
// CHECK-NEXT: {{%[0-9]+}} = OpMatrixTimesVector %v2float
  v2f = mul(v2f, m2x2f);

// CHECK:                 DebugLine [[src]] %uint_45 %uint_45 %uint_11 %uint_26
// CHECK-NEXT: {{%[0-9]+}} = OpDot %float
  v2f.x = dot(v4i.xy, v2f);

// CHECK:                 DebugLine [[src]] %uint_49 %uint_49 %uint_11 %uint_25
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %v2float {{%[0-9]+}} UnpackHalf2x16
  v2f.x = f16tof32(v4i.x);

// CHECK:                 DebugLine [[src]] %uint_53 %uint_53 %uint_11 %uint_20
// CHECK-NEXT: {{%[0-9]+}} = OpDPdx %v2float
  m2x2f = ddx(m2x2f);

// CHECK:                       DebugLine [[src]] %uint_60 %uint_60 %uint_11 %uint_36
// CHECK-NEXT: [[fmod0:%[0-9]+]] = OpFRem %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK:                       DebugLine [[src]] %uint_60 %uint_60 %uint_11 %uint_36
// CHECK-NEXT: [[fmod1:%[0-9]+]] = OpFRem %v2float {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:       {{%[0-9]+}} = OpCompositeConstruct %mat2v2float [[fmod0]] [[fmod1]]
  m2x2f = fmod(m2x2f, float2x2(v4i));

// CHECK:                     DebugLine [[src]] %uint_65 %uint_65 %uint_7 %uint_7
// CHECK-NEXT: [[v2f:%[0-9]+]] = OpFOrdNotEqual %v2bool
// CHECK:          {{%[0-9]+}} = OpAll %bool [[v2f]]
  if (all(v2f))
// CHECK:                      DebugLine [[src]] %uint_72 %uint_72 %uint_5 %uint_31
// CHECK:       [[sin:%[0-9]+]] = OpExtInst %float {{%[0-9]+}} Sin {{%[0-9]+}}
// CHECK-NEXT:                 DebugLine [[src]] %uint_72 %uint_72 %uint_19 %uint_23
// CHECK-NEXT: [[v2fx:%[0-9]+]] = OpAccessChain %_ptr_Function_float %v2f %int_1
// CHECK-NEXT:                 DebugLine [[src]] %uint_72 %uint_72 %uint_5 %uint_31
// CHECK-NEXT:                 OpStore [[v2fx]] [[sin]]
    sincos(v2f.x, v2f.y, v2f.x);

// CHECK:                 DebugLine [[src]] %uint_76 %uint_76 %uint_9 %uint_21
// CHECK-NEXT: {{%[0-9]+}} = OpExtInst %v2float {{%[0-9]+}} FClamp
  v2f = saturate(v2f);

// CHECK: DebugLine [[src]] %uint_80 %uint_80 %uint_26 %uint_33
// CHECK: OpAny
  /* comment */ dest_i = any(v4i);

// CHECK:                     DebugLine [[src]] %uint_87 %uint_87 %uint_35 %uint_47
// CHECK-NEXT: [[idx:%[0-9]+]] = OpIAdd %uint
// CHECK:                     DebugLine [[src]] %uint_87 %uint_87 %uint_3 %uint_48
// CHECK-NEXT: [[v4i_1:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %v4i %int_0
// CHECK-NEXT:                OpStore [[v4i_1]] {{%[0-9]+}}
  v4i.x = NonUniformResourceIndex(v4i.y + v4i.z);

// CHECK:      DebugLine [[src]] %uint_93 %uint_93 %uint_11 %uint_39
// CHECK-NEXT: OpImageSparseTexelsResident %bool
// CHECK:      DebugLine [[src]] %uint_93 %uint_93 %uint_3 %uint_39
// CHECK-NEXT: OpAccessChain %_ptr_Function_uint %v4i %int_2
  v4i.z = CheckAccessFullyMapped(v4i.w);

// CHECK:                     DebugLine [[src]] %uint_101 %uint_101 %uint_19 %uint_36
// CHECK-NEXT: [[add:%[0-9]+]] = OpFAdd %v2float
// CHECK-NEXT:                DebugLine [[src]] %uint_101 %uint_101 %uint_12 %uint_39
// CHECK-NEXT:                OpBitcast %v2uint [[add]]
// CHECK-NEXT:                DebugLine [[src]] %uint_101 %uint_101 %uint_3 %uint_39
// CHECK-NEXT:                OpLoad %v4uint %v4i
  v4i.xy = asuint(m2x2f._m00_m11 + v2f);

// CHECK:      DebugLine [[src]] %uint_107 %uint_107 %uint_8 %uint_23
// CHECK-NEXT: OpFMul %v2float
// CHECK-NEXT: DebugLine [[src]] %uint_107 %uint_107 %uint_3 %uint_31
// CHECK-NEXT: OpFOrdLessThan %v2bool
  clip(v4i.yz * m2x2f._m00_m11);

  float4 v4f;

// CHECK:      DebugLine [[src]] %uint_115 %uint_115 %uint_9 %uint_37
// CHECK:      OpFMul %float
// CHECK-NEXT: OpCompositeConstruct %v4float
// CHECK-NEXT: OpConvertFToU %v4uint
  v4i = dst(v4f + 3 * v4f, v4f - v4f);

// CHECK:      DebugLine [[src]] %uint_121 %uint_121 %uint_17 %uint_43
// CHECK-NEXT: OpExtInst %float {{%[0-9]+}} Exp2
// CHECK:      DebugLine [[src]] %uint_121 %uint_121 %uint_11 %uint_44
// CHECK-NEXT: OpBitcast %int
  v4i.x = asint(ldexp(v4f.x + v4f.y, v4f.w));

// CHECK:      DebugLine [[src]] %uint_129 %uint_129 %uint_19 %uint_31
// CHECK-NEXT: OpFAdd %float
// CHECK-NEXT: DebugLine [[src]] %uint_129 %uint_129 %uint_34 %uint_38
// CHECK-NEXT: OpAccessChain %_ptr_Function_float %v4f %int_3
// CHECK-NEXT: DebugLine [[src]] %uint_129 %uint_129 %uint_13 %uint_39
// CHECK-NEXT: OpExtInst %FrexpStructType {{%[0-9]+}} FrexpStruct
  v4f = lit(frexp(v4f.x + v4f.y, v4f.w),
// CHECK:                     DebugLine [[src]] %uint_133 %uint_133 %uint_13 %uint_17
// CHECK-NEXT: [[v4f:%[0-9]+]] = OpAccessChain %_ptr_Function_float %v4f %int_2
// CHECK-NEXT:                OpLoad %float [[v4f]]
            v4f.z,
// CHECK:                       DebugLine [[src]] %uint_140 %uint_140 %uint_13 %uint_58
// CHECK-NEXT: [[clamp:%[0-9]+]] = OpExtInst %uint {{%[0-9]+}} UClamp
// CHECK-NEXT:                  OpConvertUToF %float [[clamp]]
// CHECK-NEXT:                  DebugLine [[src]] %uint_129 %uint_140 %uint_9 %uint_59
// CHECK-NEXT:                  OpExtInst %float {{%[0-9]+}} FMax %float_0
// CHECK-NEXT:                  OpExtInst %float {{%[0-9]+}} FMin
            clamp(v4i.x + v4i.y, 2 * v4i.z, v4i.w - v4i.z));

// CHECK:                      DebugLine [[src]] %uint_146 %uint_146 %uint_33 %uint_59
// CHECK-NEXT: [[sign:%[0-9]+]] = OpExtInst %v3float {{%[0-9]+}} FSign
// CHECK-NEXT:                 DebugLine [[src]] %uint_146 %uint_146 %uint_38 %uint_38
// CHECK-NEXT:                 OpConvertFToS %v3int [[sign]]
  v4i = D3DCOLORtoUBYTE4(float4(sign(v4f.xyz - 2 * v4f.xyz),
// CHECK:      DebugLine [[src]] %uint_149 %uint_149 %uint_33 %uint_43
// CHECK-NEXT: OpExtInst %float {{%[0-9]+}} FSign
                                sign(v4f.w)));
// CHECK:                     DebugLine [[src]] %uint_146 %uint_149 %uint_9 %uint_45
// CHECK-NEXT: [[arg:%[0-9]+]] = OpVectorShuffle %v4float {{%[0-9]+}} {{%[0-9]+}} 2 1 0 3
// CHECK-NEXT:                OpVectorTimesScalar %v4float [[arg]]

// CHECK:      DebugLine [[src]] %uint_156 %uint_156 %uint_7 %uint_19
// CHECK-NEXT: OpIsNan %v4bool
  if (isfinite(v4f).x)
// CHECK:                     DebugLine [[src]] %uint_161 %uint_161 %uint_15 %uint_30
// CHECK-NEXT: [[rcp:%[0-9]+]] = OpFDiv %v4float
// CHECK-NEXT:                DebugLine [[src]] %uint_161 %uint_161 %uint_11 %uint_31
// CHECK-NEXT:                OpExtInst %v4float {{%[0-9]+}} Sin [[rcp]]
    v4f = sin(rcp(v4f / v4i.x));

// CHECK:                     DebugLine [[src]] %uint_168 %uint_168 %uint_20 %uint_47
// CHECK-NEXT:                OpExtInst %float {{%[0-9]+}} Log2
// CHECK:                     DebugLine [[src]] %uint_168 %uint_168 %uint_11 %uint_48
// CHECK-NEXT: [[arg_0:%[0-9]+]] = OpCompositeConstruct %v2float
// CHECK-NEXT:                OpExtInst %uint {{%[0-9]+}} PackHalf2x16 [[arg_0]]
  v4i.x = f32tof16(log10(v2f.x * v2f.y + v4f.x));

// CHECK:      DebugLine [[src]] %uint_172 %uint_172 %uint_3 %uint_26
// CHECK-NEXT: OpTranspose %mat2v2float
  transpose(m2x2f + m2x2f);

// CHECK:                     DebugLine [[src]] %uint_180 %uint_180 %uint_25 %uint_42
// CHECK-NEXT: [[abs:%[0-9]+]] = OpExtInst %float {{%[0-9]+}} FAbs
// CHECK-NEXT:                DebugLine [[src]] %uint_180 %uint_180 %uint_20 %uint_43
// CHECK-NEXT:                OpExtInst %float {{%[0-9]+}} Sqrt [[abs]]
// CHECK:      DebugLine [[src]] %uint_180 %uint_180 %uint_7 %uint_52
// CHECK-NEXT: OpExtInst %uint {{%[0-9]+}} FindSMsb
  max(firstbithigh(sqrt(abs(v2f.x * v4f.w)) + v4i.x),
// CHECK:      DebugLine [[src]] %uint_183 %uint_183 %uint_7 %uint_16
// CHECK-NEXT: OpExtInst %float {{%[0-9]+}} Cos
      cos(v4f.x));
// CHECK:      DebugLine [[src]] %uint_180 %uint_183 %uint_3 %uint_17
// CHECK-NEXT: OpExtInst %float {{%[0-9]+}} NMax
}
