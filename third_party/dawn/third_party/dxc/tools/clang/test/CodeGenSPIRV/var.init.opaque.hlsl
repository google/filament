// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Inner {
    Texture3D    t;
};

struct Combined1 {
    Inner        others;
    SamplerState s1;
    Texture2D    t1;
    Texture2D    t2;
    SamplerState s2;
};

struct Combined2 {
    Texture3D    t;
    SamplerState s;
};

Texture3D    gTex3D;
SamplerState gSampler;
Texture2D    gTex2D;

float main() : A {

// CHECK:       [[tex3d:%[0-9]+]] = OpLoad %type_3d_image %gTex3D
// CHECK-NEXT:  [[sampl:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:  [[comb2:%[0-9]+]] = OpCompositeConstruct %Combined2 [[tex3d]] [[sampl]]
// CHECK-NEXT:                  OpStore %comb2 [[comb2]]
    Combined2 comb2 = {gTex3D, gSampler};

// CHECK-NEXT:  [[tex3d_0:%[0-9]+]] = OpLoad %Combined2 %comb2
// CHECK-NEXT: [[tex2d1:%[0-9]+]] = OpLoad %type_2d_image %gTex2D
// CHECK-NEXT: [[tex2d2:%[0-9]+]] = OpLoad %type_2d_image %gTex2D
// CHECK-NEXT: [[sampl2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:  [[tex3d_1:%[0-9]+]] = OpCompositeExtract %type_3d_image %32 0
// CHECK-NEXT: [[sampl1:%[0-9]+]] = OpCompositeExtract %type_sampler %32 1
// CHECK-NEXT:  [[inner:%[0-9]+]] = OpCompositeConstruct %Inner [[tex3d_1]]
// CHECK-NEXT:  [[comb1:%[0-9]+]] = OpCompositeConstruct %Combined1 [[inner]] [[sampl1]] [[tex2d1]] [[tex2d2]] [[sampl2]]
// CHECK-NEXT:                   OpStore %comb1 [[comb1]]
    Combined1 comb1 = {comb2, {gTex2D, gTex2D}, gSampler};

    return 1.0;
}
