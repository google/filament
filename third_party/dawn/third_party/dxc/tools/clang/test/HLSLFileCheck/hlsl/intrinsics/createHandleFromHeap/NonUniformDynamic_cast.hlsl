// RUN: %dxc -T cs_6_6 -enable-16bit-types %s | %FileCheck %s
// RUN: %dxc -T cs_6_6 -enable-16bit-types -Od %s | %FileCheck %s
// RUN: %dxc -T cs_6_6 -enable-16bit-types -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKOD
// RUN: %dxc -T cs_6_6 -enable-16bit-types -Od -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKOD

// CHECK: Note: shader requires additional functionality:
// CHECK: Resource descriptor heap indexing

// Make sure nonUniformIndex is true.
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 true)
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 true)

float read(uint64_t ID) {
  Buffer<float> buf = ResourceDescriptorHeap[NonUniformResourceIndex(ID)];
  return buf[0];
}

void write(uint16_t ID, float f) {
  RWBuffer<float> buf = ResourceDescriptorHeap[NonUniformResourceIndex(ID)];
  buf[0] = f;
}

[numthreads(8, 8, 1)]
void main( uint2 ID : SV_DispatchThreadID) {
  float v = read((uint64_t)ID.x);
  write((uint16_t)ID.y, v);
}

// Exclude quoted source file (see readme)
// CHECKOD-LABEL: {{!"[^"]*\\0A[^"]*"}}