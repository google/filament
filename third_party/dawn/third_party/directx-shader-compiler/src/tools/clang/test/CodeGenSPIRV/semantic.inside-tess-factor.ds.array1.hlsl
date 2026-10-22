// RUN: %dxc -T ds_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input

struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[3]        : SV_TessFactor;
  // According to HLSL doc, this should actually be a scalar float.
  // But developers sometimes use float[1].
  float Inside[1]       : SV_InsideTessFactor;
};

// Output control point (output of hull shader)
struct BEZIER_CONTROL_POINT
{
  float3 vPosition	: BEZIERPOS;
};

// The domain shader outputs
struct DS_OUTPUT
{
  float4 vPosition  : SV_POSITION;
};

[domain("tri")]
DS_OUTPUT main( HS_CONSTANT_DATA_OUTPUT input,
                float2 UV : SV_DomainLocation,
                const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch )
{
// CHECK:       [[tli:%[0-9]+]] = OpLoad %_arr_float_uint_2 %gl_TessLevelInner
// CHECK-NEXT:   [[e0:%[0-9]+]] = OpCompositeExtract %float [[tli]] 0
// CHECK-NEXT: [[arr1:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_1 [[e0]]
// CHECK-NEXT:      {{%[0-9]+}} = OpCompositeConstruct %HS_CONSTANT_DATA_OUTPUT {{%[0-9]+}} [[arr1]]
  DS_OUTPUT Output;
  return Output;
}

