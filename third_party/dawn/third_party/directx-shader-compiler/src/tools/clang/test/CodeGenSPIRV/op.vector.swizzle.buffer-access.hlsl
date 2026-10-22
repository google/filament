// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

Buffer<float>  myBuffer1;
Buffer<float3> myBuffer3;

float4 main(in float4 pos : SV_Position) : SV_Target0
{
    uint index = (uint)pos.x;

// CHECK:        [[img:%[0-9]+]] = OpLoad %type_buffer_image %myBuffer1
// CHECK-NEXT: [[fetch:%[0-9]+]] = OpImageFetch %v4float [[img]] {{%[0-9]+}}
// CHECK-NEXT:   [[val:%[0-9]+]] = OpCompositeExtract %float [[fetch]] 0
// CHECK-NEXT:                  OpStore %a [[val]]
    float  a = myBuffer1[index].x;

// CHECK:        [[img_0:%[0-9]+]] = OpLoad %type_buffer_image %myBuffer1
// CHECK-NEXT: [[fetch_0:%[0-9]+]] = OpImageFetch %v4float [[img_0]] {{%[0-9]+}}
// CHECK-NEXT:    [[ex:%[0-9]+]] = OpCompositeExtract %float [[fetch_0]] 0
// CHECK-NEXT:   [[val_0:%[0-9]+]] = OpCompositeConstruct %v2float [[ex]] [[ex]]
// CHECK-NEXT:                  OpStore %b [[val_0]]
    float2 b = myBuffer1[index].xx;

// CHECK:        [[img_1:%[0-9]+]] = OpLoad %type_buffer_image_0 %myBuffer3
// CHECK-NEXT: [[fetch_1:%[0-9]+]] = OpImageFetch %v4float [[img_1]] {{%[0-9]+}}
// CHECK-NEXT:   [[val_1:%[0-9]+]] = OpVectorShuffle %v3float [[fetch_1]] [[fetch_1]] 0 1 2
// CHECK-NEXT:       OpStore %c [[val_1]]
    float3 c = myBuffer3[index].xyz;

// CHECK:        [[img_2:%[0-9]+]] = OpLoad %type_buffer_image_0 %myBuffer3
// CHECK-NEXT: [[fetch_2:%[0-9]+]] = OpImageFetch %v4float [[img_2]] {{%[0-9]+}}
// CHECK-NEXT:   [[val_2:%[0-9]+]] = OpVectorShuffle %v3float [[fetch_2]] [[fetch_2]] 0 1 2
// CHECK-NEXT:     [[d:%[0-9]+]] = OpVectorShuffle %v4float [[val_2]] [[val_2]] 1 1 0 2
// CHECK-NEXT:                  OpStore %d [[d]]
    float4 d = myBuffer3[index].yyxz;

    return float4(c, a) + d + float4(b, b);
}
