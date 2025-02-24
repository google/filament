// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

Texture2D<float4> myTexture;

float4 main(in float4 pos : SV_Position) : SV_Target0
{
    int2 coord = (int2)pos.xy;

// CHECK:        [[img:%[0-9]+]] = OpLoad %type_2d_image %myTexture
// CHECK-NEXT: [[fetch:%[0-9]+]] = OpImageFetch %v4float [[img]] {{%[0-9]+}} Lod %uint_0
// CHECK-NEXT:   [[val:%[0-9]+]] = OpCompositeExtract %float [[fetch]] 2
// CHECK-NEXT:                  OpStore %a [[val]]
    float  a = myTexture[coord].z;

// CHECK:        [[img_0:%[0-9]+]] = OpLoad %type_2d_image %myTexture
// CHECK-NEXT: [[fetch_0:%[0-9]+]] = OpImageFetch %v4float [[img_0]] {{%[0-9]+}} Lod %uint_0
// CHECK-NEXT:   [[val_0:%[0-9]+]] = OpVectorShuffle %v2float [[fetch_0]] [[fetch_0]] 1 0
// CHECK-NEXT:                  OpStore %b [[val_0]]
    float2 b = myTexture[coord].yx;

// CHECK:        [[img_1:%[0-9]+]] = OpLoad %type_2d_image %myTexture
// CHECK-NEXT: [[fetch_1:%[0-9]+]] = OpImageFetch %v4float [[img_1]] {{%[0-9]+}} Lod %uint_0
// CHECK-NEXT:   [[val_1:%[0-9]+]] = OpVectorShuffle %v3float [[fetch_1]] [[fetch_1]] 2 3 0
// CHECK-NEXT:       OpStore %c [[val_1]]
    float3 c = myTexture[coord].zwx;

// CHECK:        [[img_2:%[0-9]+]] = OpLoad %type_2d_image %myTexture
// CHECK-NEXT: [[fetch_2:%[0-9]+]] = OpImageFetch %v4float [[img_2]] {{%[0-9]+}} Lod %uint_0
// CHECK-NEXT:                  OpStore %d [[fetch_2]]
    float4 d = myTexture[coord].xyzw;

    return float4(c, a) + d + float4(b, b);
}
