// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    float1x3 mat;
    float3 vec3;
    float2 vec2;
    float scalar;
    uint index;

    // 1 element (from lvalue)
// CHECK:      [[access0:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_2
// CHECK-NEXT: [[load0:%[0-9]+]] = OpLoad %float [[access0]]
// CHECK-NEXT: OpStore %scalar [[load0]]
    scalar = mat._m02; // Used as rvalue
// CHECK-NEXT: [[load1:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: [[access1:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_1
// CHECK-NEXT: OpStore [[access1]] [[load1]]
    mat._12 = scalar; // Used as lvalue

    // > 1 elements (from lvalue)
// CHECK-NEXT: [[access2:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0
// CHECK-NEXT: [[load2:%[0-9]+]] = OpLoad %float [[access2]]
// CHECK-NEXT: [[access3:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_2
// CHECK-NEXT: [[load3:%[0-9]+]] = OpLoad %float [[access3]]
// CHECK-NEXT: [[access4:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_1
// CHECK-NEXT: [[load4:%[0-9]+]] = OpLoad %float [[access4]]
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v3float [[load2]] [[load3]] [[load4]]
// CHECK-NEXT: OpStore %vec3 [[cc0]]
    vec3 = mat._11_13_12; // Used as rvalue
// CHECK-NEXT: [[rhs0:%[0-9]+]] = OpLoad %v2float %vec2
// CHECK-NEXT: [[ce0:%[0-9]+]] = OpCompositeExtract %float [[rhs0]] 0
// CHECK-NEXT: [[access5:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_0
// CHECK-NEXT: OpStore [[access5]] [[ce0]]
// CHECK-NEXT: [[ce1:%[0-9]+]] = OpCompositeExtract %float [[rhs0]] 1
// CHECK-NEXT: [[access6:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %int_2
// CHECK-NEXT: OpStore [[access6]] [[ce1]]
    mat._m00_m02 = vec2; // Used as lvalue

    // The following statements will trigger errors:
    //   invalid format for vector swizzle
    // scalar = (mat + mat)._m02;
    // vec2 = (mat * mat)._11_12;

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load9:%[0-9]+]] = OpLoad %v3float %mat
// CHECK-NEXT: OpStore %vec3 [[load9]]
    vec3 = mat[0]; // Used as rvalue

    // One level indexing (from lvalue)
// CHECK-NEXT: [[load10:%[0-9]+]] = OpLoad %v3float %vec3
// CHECK-NEXT: OpStore %mat [[load10]]
    mat[index] = vec3; // Used as lvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[access9:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat %uint_1
// CHECK-NEXT: [[load11:%[0-9]+]] = OpLoad %float [[access9]]
// CHECK-NEXT: OpStore %scalar [[load11]]
    scalar = mat[index][1]; // Used as rvalue

    // Two level indexing (from lvalue)
// CHECK-NEXT: [[load12:%[0-9]+]] = OpLoad %float %scalar
// CHECK-NEXT: [[index0:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[access10:%[0-9]+]] = OpAccessChain %_ptr_Function_float %mat [[index0]]
// CHECK-NEXT: OpStore [[access10]] [[load12]]
    mat[0][index] = scalar; // Used as lvalue

    // On level indexing (from rvalue)
    // For the following case:
    // vec3 = (mat + mat)[0];
    // The AST for operator[] is actually accessing into a vector. Both mat will
    // be implicitly casted (HLSLMatrixToVectorCast) to vector before doing
    // operator+. So the whole rhs is actually a vector splatting.

    // Two level indexing (from rvalue)
    // The following statements will trigger errors:
    //   subscripted value is not an array, matrix, or vector
    // scalar = (mat * mat)[0][index];
}
