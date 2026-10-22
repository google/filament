// RUN: %dxc -O0 -T cs_6_4 %s | FileCheck %s
// CHECK: fadd

RWBuffer<float> buf;

cbuffer cb : register(b0) {
  float foo;
};

[numthreads(8, 8, 1)]
void main(uint id : SV_DispatchThreadId) {
    float x = foo;
#if defined(__spirv__)
    x -= 1;
#else
    x += 1;
#endif
    buf[id] = x;
}
