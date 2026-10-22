// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

Texture2D    gTexture;
Texture2D    gTextures[1];
SamplerState gSamplers[2];

// Copy to static variable
// CHECK:      [[src:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %gTextures %int_0
// CHECK-NEXT: [[elm:%[0-9]+]] = OpLoad %type_2d_image [[src]]
// CHECK-NEXT: [[val:%[0-9]+]] = OpCompositeConstruct %_arr_type_2d_image_uint_1 [[elm]]
// CHECK-NEXT:                OpStore %sTextures [[val]]
static Texture2D sTextures[1] = gTextures;

struct Samplers {
    SamplerState samplers[2];
};

struct Resources {
    Texture2D textures[1];
    Samplers  samplers;
};

float4 doSample(Texture2D t, SamplerState s[2]);

float4 main() : SV_Target {
    // Initialize array with brace-enclosed list
// CHECK:      [[elem0:%[0-9]+]] = OpLoad %type_2d_image %gTexture
// CHECK-NEXT:  [[arr0:%[0-9]+]] = OpCompositeConstruct %_arr_type_2d_image_uint_1 [[elem0]]
// CHECK-NEXT:                  OpStore %textures0 [[arr0]]
    Texture2D textures0[1] = { gTexture };

    Resources r;
    // Copy to struct field
// CHECK:      OpAccessChain %_ptr_UniformConstant_type_2d_image %gTextures %int_0
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpCompositeConstruct %_arr_type_2d_image_uint_1
    r.textures          = gTextures;

// CHECK:      OpAccessChain %_ptr_UniformConstant_type_sampler %gSamplers %int_0
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpAccessChain %_ptr_UniformConstant_type_sampler %gSamplers %int_1
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpCompositeConstruct %_arr_type_sampler_uint_2
    r.samplers.samplers = gSamplers;

    // Copy to local variable
// CHECK:      [[r:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_type_2d_image_uint_1 %r %int_0
// CHECK-NEXT: OpAccessChain %_ptr_Function_type_2d_image [[r]] %int_0
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpCompositeConstruct %_arr_type_2d_image_uint_1
    Texture2D    textures[1] = r.textures;
    SamplerState samplers[2];
// CHECK:      [[r_0:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_type_sampler_uint_2 %r %int_1 %int_0
// CHECK-NEXT: OpAccessChain %_ptr_Function_type_sampler [[r_0]] %int_0
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpAccessChain %_ptr_Function_type_sampler [[r_0]] %int_1
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpCompositeConstruct %_arr_type_sampler_uint_2
    samplers = r.samplers.samplers;

// Copy to function parameter
// CHECK:      OpAccessChain %_ptr_Function_type_sampler %samplers %int_0
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpAccessChain %_ptr_Function_type_sampler %samplers %int_1
// CHECK-NEXT: OpLoad
// CHECK-NEXT: OpCompositeConstruct %_arr_type_sampler_uint_2
    return doSample(textures[0], samplers);
}

float4 doSample(Texture2D t, SamplerState s[2]) {
    return t.Sample(s[1], float2(0.1, 0.2));
}
