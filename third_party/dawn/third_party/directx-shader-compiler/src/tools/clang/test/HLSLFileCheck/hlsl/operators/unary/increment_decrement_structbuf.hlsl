// RUN: %dxc /T vs_6_2 /E main %s | FileCheck %s

// Check that pre/post increment/decrement operators on
// matrices have the intended semantics for both the original
// variable and the returned value, for matrices living in resources.

RWStructuredBuffer<int1x1> buf;
AppendStructuredBuffer<int> results;

void main()
{
  // Post-increment
  // CHECK: rawBufferLoad
  // CHECK: add i32 %{{.*}}, 1
  // CHECK: rawBufferStore
  // CHECK-NOT: rawBufferLoad
  // CHECK: rawBufferStore
  results.Append((buf[0]++)._11);
  
  // Post-decrement
  // CHECK: rawBufferLoad
  // CHECK: add i32 %{{.*}}, -1
  // CHECK: rawBufferStore
  // CHECK-NOT: rawBufferLoad
  // CHECK: rawBufferStore
  results.Append((buf[0]--)._11);
  
  // Pre-increment
  // CHECK: rawBufferLoad
  // CHECK: add i32 %{{.*}}, 1
  // CHECK: rawBufferStore
  // CHECK: rawBufferLoad
  // CHECK: rawBufferStore
  results.Append((++buf[0])._11);
  
  // Pre-decrement
  // CHECK: rawBufferLoad
  // CHECK: add i32 %{{.*}}, -1
  // CHECK: rawBufferStore
  // CHECK: rawBufferLoad
  // CHECK: rawBufferStore
  results.Append((--buf[0])._11);
}