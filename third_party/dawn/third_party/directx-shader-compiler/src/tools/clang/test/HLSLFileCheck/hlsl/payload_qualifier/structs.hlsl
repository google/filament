// RUN: %dxc -T lib_6_6 %s -enable-payload-qualifiers | FileCheck %s

// CHECK: error: payload field 's1' has no payload access qualifiers.
// CHECK: error: payload field 'p3' is a payload struct. Payload access qualifiers are not allowed on payload types.
// CHECK: error: payload type 'P1' requires that all fields carry payload access qualifiers.

struct [raypayload] P2 {
    int c2 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
};

struct S1 {
    int c1;
};

struct [raypayload] P1 {
    int a : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
    int b : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
    S1 s1;
    S1 s2 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
    P2 p2;
    P2 p3 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
    matrix <float, 3, 3> matrix1 : write(miss, closesthit, anyhit, caller) : read(miss, closesthit, anyhit, caller);
};

[shader("miss")]
void Miss( inout P1 payload )
{
}