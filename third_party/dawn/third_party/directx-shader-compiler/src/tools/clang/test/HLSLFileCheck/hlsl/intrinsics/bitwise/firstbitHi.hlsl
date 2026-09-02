// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call {{.*}} FirstbitHi
// CHECK: sub i32 31
// CHECK: icmp eq i32 {{.*}}, -1
// CHECK: select
// CHECK: i32 -1

// CHECK: call {{.*}} FirstbitSHi
// CHECK: sub i32 31
// CHECK: icmp eq i32 {{.*}}, -1
// CHECK: select
// CHECK: i32 -1

// CHECK: call void @dx.op.bufferStore.i32{{.*}}, i32 5 
// CHECK: call void @dx.op.bufferStore.i32{{.*}}, i32 8

// CHECK: dx.op.unaryBits.i64(i32 33, i64
// CHECK: sub i32 63
// CHECK: icmp eq i32 {{.*}}, -1
// CHECK: select
// CHECK: i32 -1

uint a;
int2 b;

RWByteAddressBuffer outputUAV;

float4 main() : SV_TARGET
{
    outputUAV.Store(0, firstbithigh(a));
    outputUAV.Store(1, firstbithigh(b).y);

    outputUAV.Store(2, firstbithigh(32));
    outputUAV.Store(3, firstbithigh(-512));

    uint64_t c = b.x + 1;
    outputUAV.Store(4, firstbithigh(c));
    outputUAV.Store(5, countbits(c));
    return 1.0;
}
