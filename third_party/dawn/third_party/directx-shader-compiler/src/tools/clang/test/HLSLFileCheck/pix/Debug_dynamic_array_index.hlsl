// RUN: %dxc -Tcs_6_0 /Od %s | %opt -S -dxil-annotate-with-virtual-regs -hlsl-dxil-debug-instrumentation,UAVSize=128,upstreamSVPositionRow=2 -hlsl-dxilemit | %FileCheck %s

// Check that there is a block precis that correctly returns that the array is a 4-value float array
// CHECK: Block#0
// CHECK-SAME: d,0-4

RWByteAddressBuffer RawUAV: register(u1);

[numthreads(1, 1, 1)]
void main()
{
    float local_array[4];
    local_array[RawUAV.Load(0)] = 8;
    local_array[RawUAV.Load(1)] = 128;

    RawUAV.Store(64+0,local_array[0]);
    RawUAV.Store(64+4,local_array[1]);
}

