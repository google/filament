// RUN: %dxc -T lib_6_3 %s | %opt -S -hlsl-dxil-pix-dxr-invocations-log,maxNumEntriesInLog=24 | %FileCheck %s


// Check that handles for the two UAVs were created:
// CHECK: load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @PIXUAV
// CHECK: createHandleForLib
// CHECK: load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @PIXUAV
// CHECK: createHandleForLib

// Check that an increment was added (to the counter UAV):
// CHECK: dx.op.atomicBinOp.i32
// CHECK: i32 1

// Now check that at least three functions were modified (the hit group shaders):

// -------- one ----------
// Check for out-of-bounds clamp:
// CHECK: mul i32
// CHECK: 52

// Check for writes to the data UAV:
// CHECK: dx.op.bufferStore.i32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.i32
// Check that an increment was added (to the counter UAV):
// CHECK: dx.op.atomicBinOp.i32
// CHECK: i32 1

// -------- two ----------
// Check for out-of-bounds clamp:
// CHECK: mul i32
// CHECK: 52

// Check for writes to the data UAV:
// CHECK: dx.op.bufferStore.i32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.i32

// -------- three ----------
// Check that an increment was added (to the counter UAV):
// CHECK: dx.op.atomicBinOp.i32
// CHECK: i32 1

// Check for out-of-bounds clamp:
// CHECK: mul i32
// CHECK: 52

// Check for writes to the data UAV:
// CHECK: dx.op.bufferStore.i32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.f32
// CHECK: dx.op.bufferStore.i32

RWTexture2D<float4> RenderTarget : register(u0);
RWTexture3D<float4> TerrainTextures[2] : register(u1);

struct MyPayload {
  float4 color;
};

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

[shader("closesthit")]
void MyClosestHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{  
}

[shader("anyhit")]
void MyAnyHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("miss")]
void MyMiss(inout MyPayload payload)
{
}
