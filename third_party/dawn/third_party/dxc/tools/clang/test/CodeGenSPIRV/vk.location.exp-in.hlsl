// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    [[vk::location(2)]] int    a: A; // In nested struct --- A -> 2
};

struct T {
    [[vk::location(1)]] float4 b: B; // In struct --- B -> 1
                        S      s;
};

struct U {
                        float4 c: C;
};

float main([[vk::location(0)]] in  uint   m: M, // On function parameter --- M -> 0
                                   T      t,
                               out float  n: N,
                               out U      u,

                               out float4 pos: SV_Position
          ) : R {
    return 1.0;
}

// Explicit assignment
// CHECK: OpDecorate %in_var_M Location 0
// CHECK: OpDecorate %in_var_B Location 1
// CHECK: OpDecorate %in_var_A Location 2

// Declaration order assignment
// CHECK: OpDecorate %out_var_R Location 0
// CHECK: OpDecorate %out_var_N Location 1
// CHECK: OpDecorate %out_var_C Location 2
