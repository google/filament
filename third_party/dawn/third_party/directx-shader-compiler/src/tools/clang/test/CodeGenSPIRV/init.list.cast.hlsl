// RUN: %dxc -T cs_6_6 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<float> buffer;
RWStructuredBuffer<float4> buffer4;

[numthreads(1, 1, 1)]
void main() {
  {
    // CHECK: [[f:%[0-9]+]] = OpLoad %float {{.*}}
    // CHECK: [[s:%[0-9]+]] = OpConvertFToS %int [[f]]
    // CHECK: [[u:%[0-9]+]] = OpBitcast %uint [[s]]
    // CHECK:                 OpStore {{.*}} [[u]]
    uint p = uint(int(buffer[0]));
  }

  {
    // CHECK: [[f:%[0-9]+]] = OpLoad %v4float {{.*}}
    // CHECK: [[s:%[0-9]+]] = OpConvertFToS %v4int [[f]]
    // CHECK: [[u:%[0-9]+]] = OpBitcast %v4uint [[s]]
    // CHECK:                 OpStore {{.*}} [[u]]
    uint4 p4 = uint4(int4(buffer4[0]));
  }

  {
    // CHECK: [[f:%[0-9]+]] = OpLoad %v4float {{.*}}
    // CHECK: [[s:%[0-9]+]] = OpConvertFToS %v4int [[f]]
    // CHECK: [[u:%[0-9]+]] = OpBitcast %v4uint [[s]]
    // CHECK:                 OpStore {{.*}} [[u]]
    uint4 p4 = int4(buffer4[0]);
  }

  {
    // CHECK: [[f:%[0-9]+]] = OpLoad %v4float {{.*}}
    // CHECK: [[u:%[0-9]+]] = OpConvertFToU %v4uint [[f]]
    // CHECK:                 OpStore {{.*}} [[u]]
    uint4 p4 = uint4(uint4(buffer4[0]));
  }
}
