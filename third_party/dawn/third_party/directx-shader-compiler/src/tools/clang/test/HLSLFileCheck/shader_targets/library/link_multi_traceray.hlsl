// RUN: %dxc -T lib_6_6 -D ENTRY=MyRaygen0 -D TYPE=float4 -Fo lib0 %s | FileCheck %s -check-prefix=LIB0
// RUN: %dxc -T lib_6_6 -D ENTRY=MyRaygen1 -D TYPE=int3 -Fo lib1 %s | FileCheck %s -check-prefix=LIB1
// RUN: %dxl -T lib_6_6 lib0;lib1 %s  | FileCheck %s -check-prefixes=CHKLINK

// Ensures that colliding RayPayload structure which is different causes intrinsics to be renamed appropriately

// LIB0: %struct.RayPayload = type { <4 x float> }
// LIB0: define void {{.*}}MyRaygen0

// LIB1: %struct.RayPayload = type { <3 x i32> }
// LIB1: define void {{.*}}MyRaygen1

// CHKLINK-DAG: %struct.RayPayload = type
// CHKLINK-DAG: %struct.RayPayload.1 = type
// CHKLINK-DAG: declare void @dx.op.traceRay.struct.RayPayload(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.RayPayload*)
// CHKLINK-DAG: declare void @dx.op.traceRay.struct.RayPayload.1(i32, %dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, %struct.RayPayload.1*)

RaytracingAccelerationStructure scene : register(t0);

struct RayPayload
{
    TYPE color;
};

[shader("raygeneration")]
void ENTRY()
{
    RayDesc ray = {{0,0,0}, {0,0,1}, 0.05, 1000.0};
    RayPayload pld;
    TraceRay(scene, 0 /*rayFlags*/, 0xFF /*rayMask*/, 0 /*sbtRecordOffset*/, 1 /*sbtRecordStride*/, 0 /*missIndex*/, ray, pld);
}
