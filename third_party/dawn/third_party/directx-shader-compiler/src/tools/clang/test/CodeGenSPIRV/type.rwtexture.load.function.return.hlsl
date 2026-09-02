// RUN: %dxc -T cs_6_7 -E main -spirv -fspv-target-env=vulkan1.1 %s -fcgl | FileCheck %s

RWTexture2D<float2> textures[];

cbuffer CB {
	uint index;
}

RWTexture2D<float2> GetTexture() {
	return textures[index];
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
// We need to make sure there is no OpLoad on the already loaded texture.
// CHECK: [[img:%[0-9]+]] = OpFunctionCall %type_2d_image %GetTexture
// CHECK:                   OpImageWrite [[img]] {{.*}} {{.*}} None
	GetTexture()[tid.xy] = float2(1.0, 0.0);

// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %textures %int_0
// CHECK: [[tmp:%[0-9]+]] = OpLoad %type_2d_image [[ptr]]
// CHECK:                   OpImageWrite [[tmp]] {{.*}} {{.*}} None
  textures[0][tid.xy] = float2(1.0, 0.0);

// CHECK: [[img:%[0-9]+]] = OpFunctionCall %type_2d_image %GetTexture
// CHECK:                   OpStore %tex [[img]]
// CHECK: [[tmp:%[0-9]+]] = OpLoad %type_2d_image %tex
// CHECK:                   OpImageWrite [[tmp]] {{.*}} {{.*}} None
	RWTexture2D<float2> tex = GetTexture();
  tex[tid.xy] = float2(1.0, 0.0);
}
