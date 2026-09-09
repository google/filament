// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    float1x1 mat;
    float3 vec3;
    float2 vec2;
    float1 vec1;
    float scalar;
    uint index;

    // 1 element (from lvalue)
// CHECK:      [[load0:%[0-9]+]] = OpLoad %float %mat
// CHECK-NEXT: OpStore %scalar [[load0]]
    scalar = mat._m00; // Used as rvalue
// CHECK-NEXT: [[load1:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: OpStore %mat [[load1]]
    mat._11 = scalar; // Used as lvalue

    // >1 elements (from lvalue)
// CHECK-NEXT: [[load2:%[0-9]+]] = OpLoad %float %mat
// CHECK-NEXT: [[load3:%[0-9]+]] = OpLoad %float %mat
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v2float [[load2]] [[load3]]
// CHECK-NEXT: OpStore %vec2 [[cc0]]
    vec2 = mat._11_11; // Used as rvalue

    // The following statements will trigger errors:
    //   invalid format for vector swizzle
    // scalar = (mat + mat)._m00;
    // vec2 = (mat * mat)._11_11;

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load9:%[0-9]+]] = OpLoad %float %mat
// CHECK-NEXT: OpStore %vec1 [[load9]]
    vec1 = mat[0]; // Used as rvalue

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load10:%[0-9]+]] = OpLoad %float %vec1
// CHECK-NEXT: OpStore %mat [[load10]]
    mat[index] = vec1; // Used as lvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[load11:%[0-9]+]] = OpLoad %float %mat
// CHECK-NEXT: OpStore %scalar [[load11]]
    scalar = mat[index][0]; // Used as rvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[load12:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: OpStore %mat [[load12]]
    mat[0][index] = scalar; // Used as lvalue

    // One/two level indexing (from rvalue)
    // The following statements will trigger errors:
    //   subscripted value is not an array, matrix, or vector
    // vec1 = (mat + mat)[0];
    // scalar = (mat * mat)[0][index];
}
