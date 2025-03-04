// RUN: %dxc -EClosestHit -Tlib_6_5 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=U0:0:10i0;U0:1:2i0;.0;0;0. | %FileCheck %s

// Check we added the UAV:
// CHECK: @PIXUAV0 = external constant %struct.RWByteAddressBuffer, align 4
// CHECK: load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @PIXUAV
// CHECK: call %dx.types.Handle @dx.op.createHandleForLib.struct.RWByteAddressBuffer
// 
// check for an attempt to write to the PIX UAV before the load from TerrainTextures:
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle
// CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad
// 
// And a write for the store to RenderTarget:
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle
// CHECK: void @dx.op.textureStore


RWTexture2D<float4> RenderTarget : register(u0);
RWTexture3D<float4> TerrainTextures[2] : register(u1);

struct RayPayload 
{
  float4 color;
};

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttributes attr) 
{
    RenderTarget[uint2(3,5)] = TerrainTextures[1].Load(uint3(2,2,2));
}