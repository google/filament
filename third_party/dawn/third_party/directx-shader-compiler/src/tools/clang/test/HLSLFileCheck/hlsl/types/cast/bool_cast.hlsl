// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// Make sure it compiles
// CHECK: uitofp i1

static bool t;
float main( float a:A) : SV_Target
{
    t.x = bool(a);
    return t;
}