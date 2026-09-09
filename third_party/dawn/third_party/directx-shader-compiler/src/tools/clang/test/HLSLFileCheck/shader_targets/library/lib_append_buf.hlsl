// RUN: %dxc -T lib_6_3 %s | FileCheck %s

// Make sure append/consume works for lib.
// CHECK: bufferUpdateCounter(i32 70, {{.*}}, i8 -1)
// CHECK: bufferUpdateCounter(i32 70, {{.*}}, i8 1)

// Append Structured Buffer (u3)
AppendStructuredBuffer<float4> appendUAVResource : register(u3);

// Consume Structured Buffer (u4) 
ConsumeStructuredBuffer<float4> consumeUAVResource : register(u4);

[numthreads(1,1,1)]
[shader("compute")]
void test()
{
  float4 consumeResourceOutput = consumeUAVResource.Consume();
  appendUAVResource.Append(consumeResourceOutput);
}