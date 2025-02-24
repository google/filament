// RUN: %dxc -T cs_6_7 -E main -spirv -fspv-target-env=vulkan1.3 %s

// CHECK: OpCapability StorageImageWriteWithoutFormat

// CHECK: %[[#image:]] = OpTypeImage %float 3D 2 0 0 2 Unknown
[[vk::image_format("unknown")]] RWTexture3D<float32_t2> untypedImage;

[numthreads(8,8,8)]
void main(uint32_t3 gl_GlobalInvocationID : SV_DispatchThreadID)
{
// CHECK: %[[#tmp:]] = OpLoad %[[#image]] %[[#]]
// CHECK:              OpImageWrite [[#tmp]] %[[#]] %[[#]] None
    untypedImage[gl_GlobalInvocationID] = float32_t2(4,5);
}
