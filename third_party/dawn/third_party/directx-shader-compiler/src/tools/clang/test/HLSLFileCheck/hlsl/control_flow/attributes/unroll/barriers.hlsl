// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Regression test for loops that contain barriers

// CHECK: @main

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

// CHECK: barrier
// CHECK: bufferUpdateCounter
// CHECK: bufferStore

AppendStructuredBuffer<float4> buf0;
AppendStructuredBuffer<float4> buf1;
AppendStructuredBuffer<float4> buf2;
AppendStructuredBuffer<float4> buf3;
uint g_cond;


[numthreads( 128, 1, 1 )]

void main() {

  AppendStructuredBuffer<float4> buffers[] = { buf0, buf1, buf2, buf3, };

  float ret = 0;
  [unroll]
  for (uint i = 0; i < 2; i++) {

    GroupMemoryBarrierWithGroupSync();
    buffers[i].Append(i);

    [unroll]
    for (uint j = 0; j < 2; j++) {
      ret++;

      GroupMemoryBarrierWithGroupSync();
      buffers[i].Append(j);

      GroupMemoryBarrierWithGroupSync();
      buffers[i].Append(j);
    }
  }
}

