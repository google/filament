// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers -D TEST_NUM=0 %s | FileCheck -check-prefix=CHK0 %s
// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers -D TEST_NUM=1 %s | FileCheck -check-prefix=CHK1 %s

// CHK0: error: type 'Payload' used as payload requires that it is annotated with the [raypayload] attribute
// CHK1: error: type 'Payload' used as payload requires that it is annotated with the [raypayload] attribute

// Check for payload annotations when payload used on trace.

RaytracingAccelerationStructure scene : register(t0);

struct Payload
{
    int a : read (caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct Attribs
{
    float2 barys;
};

#if TEST_NUM == 0
[shader("raygeneration")]
void RayGen()
{
    Payload payload_in_rg;
    RayDesc ray;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload_in_rg );

}
#endif

#if TEST_NUM == 1
[shader("closesthit")]
void Closesthit( inout Payload payload, in Attribs attribs )
{
    Payload payload_in_ch;
    RayDesc ray;
    TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload_in_ch );

}
#endif
