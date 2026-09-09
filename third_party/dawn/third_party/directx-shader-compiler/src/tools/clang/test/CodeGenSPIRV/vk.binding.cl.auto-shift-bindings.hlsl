// RUN: %dxc -T ps_6_0 -E main -fvk-b-shift 100 0 -fvk-t-shift 200 0 -fvk-t-shift 300 1 -fvk-s-shift 500 0 -fvk-u-shift 700 0 -fvk-auto-shift-bindings -fspv-flatten-resource-arrays -fcgl  %s -spirv | FileCheck %s

// Tests that we can set shift for more than one sets of the same register type
// Tests that we can override shift for the same set

struct S {
    float4 f;
};

// Explicit binding assignment is unaffected by -auto-shift-bindings
//
// CHECK: OpDecorate %cbuffer1 DescriptorSet 0
// CHECK: OpDecorate %cbuffer1 Binding 42
[[vk::binding(42)]]
ConstantBuffer<S> cbuffer1;

// "-fvk-t-shift 300 1" should shift the binding for this resource.
//
// CHECK: OpDecorate %texture1 DescriptorSet 1
// CHECK: OpDecorate %texture1 Binding 301
Texture1D<float4> texture1: register(t1, space1);

// "-fvk-t-shift 200 0" should shift it by 200. So we should get binding slots 200-209.
//
// CHECK: OpDecorate %textureArray_1 DescriptorSet 0
// CHECK: OpDecorate %textureArray_1 Binding 200
Texture1DArray<float4> textureArray_1[10] : register(t0);

// "-fvk-t-shift 200 0" should shift it by 200. So we should get binding slots 215-224.
//
// CHECK: OpDecorate %textureArray_2 DescriptorSet 0
// CHECK: OpDecorate %textureArray_2 Binding 215
Texture2D<float4> textureArray_2[10] : register(t15);

// Missing 'register' assignment.
// "-auto-shift-bindings" should detect that this is register of type 't'.
// "-fvk-t-shift 200 0" should shift it by 200.
// This is an array of 6, so it requires 6 binding slots.
// Binding slot 200-209 is taken. There's a gap of 5 slots (210-214) inclusive
// which is not large enough to for this resource. So, it should be placed
// after the previous textures, starting at binding slot 225. Consumes 225-230, inclusive.
//
// CHECK: OpDecorate %textureArray_3 DescriptorSet 0
// CHECK: OpDecorate %textureArray_3 Binding 225
Texture2DArray<float4> textureArray_3[6];

// The following all have the 't' register type and they take the next
// available binding slots starting at 200.
//
// CHECK: OpDecorate %t3d DescriptorSet 0
// CHECK: OpDecorate %t3d Binding 210
Texture3D<float4> t3d;
// CHECK: OpDecorate %tc DescriptorSet 0
// CHECK: OpDecorate %tc Binding 211
TextureCube<float4> tc;
// CHECK: OpDecorate %tcarr DescriptorSet 0
// CHECK: OpDecorate %tcarr Binding 212
TextureCubeArray<float4> tcarr;
// CHECK: OpDecorate %t2dms DescriptorSet 0
// CHECK: OpDecorate %t2dms Binding 213
Texture2DMS<float4> t2dms;
// CHECK: OpDecorate %t2dmsarr DescriptorSet 0
// CHECK: OpDecorate %t2dmsarr Binding 214
Texture2DMSArray<float4> t2dmsarr;
// CHECK: OpDecorate %sb DescriptorSet 0
// CHECK: OpDecorate %sb Binding 231
StructuredBuffer<float4> sb;
// CHECK: OpDecorate %bab DescriptorSet 0
// CHECK: OpDecorate %bab Binding 232
ByteAddressBuffer bab;
// CHECK: OpDecorate %buf DescriptorSet 0
// CHECK: OpDecorate %buf Binding 233
Buffer<float4> buf;
// CHECK: OpDecorate %myTbuffer DescriptorSet 0
// CHECK: OpDecorate %myTbuffer Binding 234
tbuffer myTbuffer { float x; };

// "-auto-shift-bindings" should detect that this is register of type 's'.
// "-fvk-s-shift 500 0" should shift it by 500.
//
// CHECK: OpDecorate %ss DescriptorSet 0
// CHECK: OpDecorate %ss Binding 500
SamplerState ss;
// CHECK: OpDecorate %scs DescriptorSet 0
// CHECK: OpDecorate %scs Binding 501
SamplerComparisonState scs;

// "-auto-shift-bindings" should detect that this is register of type 'u'.
// "-fvk-u-shift 700 0" should shift it by 700.
//
// CHECK: OpDecorate %rwbab DescriptorSet 0
// CHECK: OpDecorate %rwbab Binding 700
RWByteAddressBuffer rwbab;
// CHECK: OpDecorate %rwsb DescriptorSet 0
// CHECK: OpDecorate %rwsb Binding 701
RWStructuredBuffer<float4> rwsb;
// CHECK: OpDecorate %asb DescriptorSet 0
// CHECK: OpDecorate %asb Binding 702
AppendStructuredBuffer<float4> asb;
// CHECK: OpDecorate %csb DescriptorSet 0
// CHECK: OpDecorate %csb Binding 703
ConsumeStructuredBuffer<float4> csb;
// CHECK: OpDecorate %rwbuffer DescriptorSet 0
// CHECK: OpDecorate %rwbuffer Binding 704
RWBuffer<float4> rwbuffer;
// CHECK: OpDecorate %rwtexture1d DescriptorSet 0
// CHECK: OpDecorate %rwtexture1d Binding 705
RWTexture1D<float4> rwtexture1d;
// CHECK: OpDecorate %rwtexture1darray DescriptorSet 0
// CHECK: OpDecorate %rwtexture1darray Binding 706
RWTexture1DArray<float4> rwtexture1darray;
// CHECK: OpDecorate %rwtexture2d DescriptorSet 0
// CHECK: OpDecorate %rwtexture2d Binding 707
RWTexture2D<float4> rwtexture2d;
// CHECK: OpDecorate %rwtexture2darray DescriptorSet 0
// CHECK: OpDecorate %rwtexture2darray Binding 708
RWTexture2DArray<float4> rwtexture2darray;
// CHECK: OpDecorate %rwtexture3d DescriptorSet 0
// CHECK: OpDecorate %rwtexture3d Binding 709
RWTexture3D<float4> rwtexture3d;

// Missing 'register' assignment.
// "-auto-shift-bindings" should detect that this is register of type 'b'.
// "-fvk-b-shift 100 0" should shift it by 100.
//
// CHECK: OpDecorate %cbuffer2 DescriptorSet 0
// CHECK: OpDecorate %cbuffer2 Binding 100
ConstantBuffer<S> cbuffer2;

float4 main() : SV_Target {
    return 0.xxxx;
}
