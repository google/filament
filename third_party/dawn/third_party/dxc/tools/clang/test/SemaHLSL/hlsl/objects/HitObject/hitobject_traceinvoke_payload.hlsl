// RUN: %dxc -T lib_6_9 %s -D TEST_NUM=0 %s -verify
// RUN: %dxc -T lib_6_9 %s -D TEST_NUM=1 %s -verify

RaytracingAccelerationStructure scene : register(t0);

struct Payload
{
    int a : read (caller, closesthit, miss) : write(caller, closesthit, miss);
};

struct Attribs
{
    float2 barys;
};

[shader("raygeneration")]
void RayGen()
{
// expected-error@+1{{type 'Payload' used as payload requires that it is annotated with the [raypayload] attribute}}
    Payload payload_in_rg;
    RayDesc ray;
#if TEST_NUM == 0
    dx::HitObject::TraceRay( scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload_in_rg );
#else
    dx::HitObject::Invoke( dx::HitObject(), payload_in_rg );
#endif
}