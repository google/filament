// RUN: %dxc -T cs_6_6 -fspv-target-env=vulkan1.3 -E main %s -spirv -fcgl | FileCheck %s

enum : uint {
  UA = 1
};

enum : int {
  SA = 3
};

static float p;

[numthreads(1, 1, 1)]
void main() {
//CHECK:         OpStore %foo %float_1
  float foo = UA;

//CHECK:         OpStore %bar %float_3
  float bar = SA;
}
