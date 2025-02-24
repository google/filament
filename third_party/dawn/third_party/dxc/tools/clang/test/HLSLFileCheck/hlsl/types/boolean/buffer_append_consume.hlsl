// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Check that bool register to memory representation conversions happen
// when using AppendStructuredBuffer/ConsumeStructuredBuffer,
// since this is handled by special-purpose code.
// Regression test for GitHub #1882, where this was crashing.

ConsumeStructuredBuffer<bool> boolIn;
ConsumeStructuredBuffer<bool2> boolVecIn;
AppendStructuredBuffer<bool> boolOut;
AppendStructuredBuffer<bool2> boolVecOut;
void main()
{
    // CHECK: rawBufferLoad
    // CHECK: icmp ne i32 %{{.*}}, 0
    // CHECK: zext i1 %{{.*}} to i32
    // CHECK: rawBufferStore
    boolOut.Append(boolIn.Consume());
    
    // CHECK: rawBufferLoad
    // CHECK: icmp ne i32 %{{.*}}, 0
    // CHECK: icmp ne i32 %{{.*}}, 0
    // CHECK: zext i1 %{{.*}} to i32
    // CHECK: zext i1 %{{.*}} to i32
    // CHECK: rawBufferStore
    boolVecOut.Append(boolVecIn.Consume());
}