// RUN: %dxc /Tlib_6_9 %s | FileCheck %s
// RUN: %dxc /Tlib_6_9 -fcgl %s | FileCheck %s -check-prefix=FCGL

// Make sure that we can use the BuiltInTriangleIntersectionAttributes struct
// as a template argument to GetAttributes.

// For -fcgl, just check the form of the HL call.
// FCGL: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.BuiltInTriangleIntersectionAttributes*)"(i32 364, %dx.types.HitObject* %{{[^ ]+}}, %struct.BuiltInTriangleIntersectionAttributes* %{{[^ ]+}})

// CHECK: %[[ATTR:[^ ]+]] = alloca %struct.BuiltInTriangleIntersectionAttributes
// CHECK: call void @dx.op.hitObject_Attributes.struct.BuiltInTriangleIntersectionAttributes(i32 289, %dx.types.HitObject %{{[^ ]+}}, %struct.BuiltInTriangleIntersectionAttributes* nonnull %[[ATTR]])

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);

struct [raypayload] RayPayload
{
    float4 color : write(caller, closesthit, miss) : read(caller);
};

typedef BuiltInTriangleIntersectionAttributes MyAttribs;

[shader("raygeneration")]
void MyRaygenShader()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload payload = { float4(0, 0, 0, 0) };
    float4 color = float4(1,1,1,1);

    dx::HitObject hit = dx::HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    MyAttribs attr;
    hit.GetAttributes(attr);
    payload.color += float4(attr,0,1);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}
