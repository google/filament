// RUN: %dxc -T cs_6_7 -E main -spirv -fspv-target-env=vulkan1.3 %s

// CHECK: OpCapability StorageImageReadWithoutFormat

// CHECK: %[[#image:]] = OpTypeImage %float 1D 2 0 0 2 Unknown
[[vk::image_format("unknown")]] RWTexture1D<float32_t2> untypedImage;
RWStructuredBuffer<float32_t2> output;

[numthreads(8,8,8)]
void main()
{
// CHECK: %[[#tmp:]] = OpLoad %[[#image]] %[[#]]
// CHECK:              OpImageRead %[[#]] [[#tmp]] %[[#]] None
    output[0] = untypedImage[0];
}

