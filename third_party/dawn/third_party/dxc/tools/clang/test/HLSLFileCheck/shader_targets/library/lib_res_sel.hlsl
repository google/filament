// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// resource uses must resolve to a single resource global variable (single rangeID)
// CHECK: error: local resource not guaranteed to map to unique global resource

RWStructuredBuffer<float2> buf0;
RWStructuredBuffer<float2> buf1;


void Store(bool bBufX, float2 v, uint idx) {
  RWStructuredBuffer<float2> buf = bBufX ? buf0: buf1;
  buf[idx] = v;
}