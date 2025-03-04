// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// NOTE: According to Item "Data rules" of SPIR-V Spec 2.16.1 "Universal
// Validation Rules":
//   Matrix types can only be parameterized with floating-point types.
//
// So we need special handling for matrices with non-fp elements. An extension
// to SPIR-V to lessen the above rule is a possible way, which will enable the
// generation of SPIR-V currently commented out. Or we can emulate them using
// other types.

void main() {
// CHECK: %float = OpTypeFloat 32
   float1x1 mat11;
// CHECK: %v2int = OpTypeVector %int 2
   int1x2   mat12;
// CHECK: %v3uint = OpTypeVector %uint 3
   uint1x3  mat13;
// CHECK: %v4bool = OpTypeVector %bool 4
   bool1x4  mat14;

   int2x1   mat21;
// CHECK: %_arr_v2uint_uint_2 = OpTypeArray %v2uint %uint_2
   uint2x2  mat22;
// CHECK: %v3bool = OpTypeVector %bool 3
// CHECK-NEXT: %_arr_v3bool_uint_2 = OpTypeArray %v3bool %uint_2
   bool2x3  mat23;
// CHECK: %v4float = OpTypeVector %float 4
// CHECK-NEXT: %mat2v4float = OpTypeMatrix %v4float 2
   float2x4 mat24;

   uint3x1  mat31;
// CHECK: %v2bool = OpTypeVector %bool 2
// CHECK: _arr_v2bool_uint_3 = OpTypeArray %v2bool %uint_3
   bool3x2  mat32;
// CHECK: %v3float = OpTypeVector %float 3
// CHECK-NEXT: %mat3v3float = OpTypeMatrix %v3float 3
   float3x3 mat33;
// CHECK: %v4int = OpTypeVector %int 4
// CHECK-NEXT: %_arr_v4int_uint_3 = OpTypeArray %v4int %uint_3
   int3x4   mat34;

   bool4x1  mat41;
// CHECK: %v2float = OpTypeVector %float 2
// CHECK-NEXT: %mat4v2float = OpTypeMatrix %v2float 4
   float4x2 mat42;
// CHECK: %v3int = OpTypeVector %int 3
// CHECK: %_arr_v3int_uint_4 = OpTypeArray %v3int %uint_4
   int4x3   mat43;
// CHECK: %v4uint = OpTypeVector %uint 4
// CHECK: %_arr_v4uint_uint_4 = OpTypeArray %v4uint %uint_4
   uint4x4  mat44;

// CHECK: %mat4v4float = OpTypeMatrix %v4float 4
    matrix mat;

    matrix<int, 1, 1>   imat11;
    matrix<uint, 1, 3>  umat23;
    matrix<float, 2, 1> fmat21;
    matrix<float, 1, 2> fmat12;
// CHECK: %_arr_v4bool_uint_3 = OpTypeArray %v4bool %uint_3
    matrix<bool, 3, 4>  bmat34;

// CHECK-LABEL: %bb_entry = OpLabel

// CHECK-NEXT: %mat11 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT: %mat12 = OpVariable %_ptr_Function_v2int Function
// CHECK-NEXT: %mat13 = OpVariable %_ptr_Function_v3uint Function
// CHECK-NEXT: %mat14 = OpVariable %_ptr_Function_v4bool Function

// CHECK-NEXT: %mat21 = OpVariable %_ptr_Function_v2int Function
// CHECK-NEXT: %mat22 = OpVariable %_ptr_Function__arr_v2uint_uint_2 Function
// CHECK-NEXT: %mat23 = OpVariable %_ptr_Function__arr_v3bool_uint_2 Function
// CHECK-NEXT: %mat24 = OpVariable %_ptr_Function_mat2v4float Function

// CHECK-NEXT: %mat31 = OpVariable %_ptr_Function_v3uint Function
// CHECK-NEXT: %mat32 = OpVariable %_ptr_Function__arr_v2bool_uint_3 Function
// CHECK-NEXT: %mat33 = OpVariable %_ptr_Function_mat3v3float Function
// CHECK-NEXT: %mat34 = OpVariable %_ptr_Function__arr_v4int_uint_3 Function

// CHECK-NEXT: %mat41 = OpVariable %_ptr_Function_v4bool Function
// CHECK-NEXT: %mat42 = OpVariable %_ptr_Function_mat4v2float Function
// CHECK-NEXT: %mat43 = OpVariable %_ptr_Function__arr_v3int_uint_4 Function
// CHECK-NEXT: %mat44 = OpVariable %_ptr_Function__arr_v4uint_uint_4 Function

// CHECK-NEXT: %mat = OpVariable %_ptr_Function_mat4v4float Function

// CHECK-NEXT: %imat11 = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %umat23 = OpVariable %_ptr_Function_v3uint Function
// CHECK-NEXT: %fmat21 = OpVariable %_ptr_Function_v2float Function
// CHECK-NEXT: %fmat12 = OpVariable %_ptr_Function_v2float Function
// CHECK-NEXT: %bmat34 = OpVariable %_ptr_Function__arr_v4bool_uint_3 Function
}
