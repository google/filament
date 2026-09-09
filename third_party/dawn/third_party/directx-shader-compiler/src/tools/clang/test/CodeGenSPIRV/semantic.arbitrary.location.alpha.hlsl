// RUN: %dxc -T vs_6_0 -E main -fvk-stage-io-order=alpha -fcgl  %s -spirv | FileCheck %s

struct S {
    float c: C;
    float a: A;
    float b: B;
};

struct T {
    float e: E;
    S     s;
    float d: D;
};

// CHECK:      OpDecorate %in_var_A Location 0
// CHECK-NEXT: OpDecorate %in_var_B Location 1
// CHECK-NEXT: OpDecorate %in_var_C Location 2
// CHECK-NEXT: OpDecorate %in_var_D Location 3
// CHECK-NEXT: OpDecorate %in_var_E Location 4
// CHECK-NEXT: OpDecorate %in_var_M Location 5
// CHECK-NEXT: OpDecorate %in_var_N Location 6

// CHECK-NEXT: OpDecorate %out_var_A Location 0
// CHECK-NEXT: OpDecorate %out_var_B Location 1
// CHECK-NEXT: OpDecorate %out_var_C Location 2
// CHECK-NEXT: OpDecorate %out_var_D Location 3
// CHECK-NEXT: OpDecorate %out_var_E Location 4
// CHECK-NEXT: OpDecorate %out_var_P Location 5
// CHECK-NEXT: OpDecorate %out_var_Q Location 6

// Input semantics by declaration order: N, E, C, A, B, D, M
//                 by alphabetic order:  A, B, C, D, E, M, N

// Output semantics by declaration order: E, C, A, B, D, Q, P
//                  by alphabetic order:  A, B, C, D, E, P, Q

void main(in  float input1 : N,
          in  T     input2 ,
          in  float input3 : M,
          out T     output1,
          out float output2: Q,
          out float output3: P
         ) {
}
