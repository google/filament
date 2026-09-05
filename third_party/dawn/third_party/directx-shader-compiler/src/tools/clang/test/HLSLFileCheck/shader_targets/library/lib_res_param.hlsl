// RUN: %dxc -T lib_6_3  %s | FileCheck %s

// resources in return/params disallowed for lib_6_3
// CHECK: error: Exported function
// CHECK: resStruct
// CHECK: must not contain a resource in parameter or return type

struct T {
RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;
};

struct D {
  float4 a;
  int4 b;
};

struct T2 {
   RWStructuredBuffer<D> uav;
};

T2 resStruct(T t, uint2 id);

RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;

[shader("compute")]
[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
    T t = {outputBuffer,outputBuffer2};
    T2 t2 = resStruct(t, id);
    uint counter = t2.uav.IncrementCounter();
    t2.uav[counter].b.xy = id;
}