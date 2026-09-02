// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv -fvk-use-gl-layout | FileCheck %s

// CHECK: OpDecorate %in_var_A Location 0
// CHECK: OpDecorate %in_var_B Location 1
// CHECK: OpDecorate %in_var_C Location 2
// CHECK: OpDecorate %in_var_D Location 4
// CHECK: OpDecorate %in_var_E Location 6
// CHECK: OpDecorate %in_var_F Location 8
// CHECK: OpDecorate %in_var_G Location 16

// CHECK: OpDecorate %out_var_A Location 0
// CHECK: OpDecorate %out_var_B Location 2
// CHECK: OpDecorate %out_var_C Location 3
// CHECK: OpDecorate %out_var_D Location 4
// CHECK: OpDecorate %out_var_E Location 5
// CHECK: OpDecorate %out_var_F Location 11
// CHECK: OpDecorate %out_var_G Location 13
// CHECK: OpDecorate %out_var_H Location 14

struct S {
    half2x3  matrix2x3 : A; // 0 (+2)
    float1x2 vector1x2 : B; // 2 (+1)
    float3x1 vector3x1 : C; // 3 (+1)
    float1x1 scalar1x1 : D; // 4 (+1)
};

struct T {
    S        s;
    float2x3 array1[3] : E; // 5  (+2*3)
    half1x2  array2[2] : F; // 11 (+1*2)
    half3x1  array3[1] : G; // 13 (+1*1)
    float    array4[4] : H; // 14 (+1*4)
};

T main(
    double    a   : A, // 0  (+1)
    double2   b   : B, // 1  (+1)
    double3   c   : C, // 2  (+2)
    double4   d   : D, // 4  (+2)
    double2x2 e   : E, // 6  (+1*2)
    double2x3 f[2]: F, // 8  (+2*2*2)
    double2x3 g   : G  // 16 (+2x2)
) {
    T t = (T)0;
    return t;
}
