// RUN: %dxc -T cs_6_0 -E main %s | FileCheck %s

// Make sure report error when external function used.
// CHECK: External function used in non-library profile

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

[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
    T t = {outputBuffer,outputBuffer2};
    T2 t2 = resStruct(t, id);
    uint counter = t2.uav.IncrementCounter();
    t2.uav[counter].b.xy = id;
}