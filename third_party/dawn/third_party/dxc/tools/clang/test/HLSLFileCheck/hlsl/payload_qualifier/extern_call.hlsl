// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers | FileCheck -input-file=stderr %s

// CHECK: warning: passing a qualified payload to an extern function can cause undefined behavior if payload qualifiers mismatch

struct [raypayload] Payload
{
    int a      : read(closesthit) : write(caller);
};

struct Attribs
{
    float2 barys;
};

void foo( inout Payload );

[shader("closesthit")]
void ClosestHitInOut( inout Payload payload, in Attribs attribs )
{
    foo(payload); // warn about passing an inout payload to an external function
}