// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Int64
// CHECK: OpCapability Float64

// CHECK: OpDecorate %m16i RelaxedPrecision
// CHECK: OpDecorate %m12i RelaxedPrecision
// CHECK: OpDecorate %m16u RelaxedPrecision
// CHECK: OpDecorate %m16f RelaxedPrecision
// CHECK: OpDecorate %m10f RelaxedPrecision
// CHECK: OpDecorate %m16i1 RelaxedPrecision
// CHECK: OpDecorate %m12i1 RelaxedPrecision
// CHECK: OpDecorate %m16u1 RelaxedPrecision
// CHECK: OpDecorate %m16f1 RelaxedPrecision
// CHECK: OpDecorate %m10f1 RelaxedPrecision

// CHECK-DAG: %void = OpTypeVoid
// CHECK-DAG: %{{[0-9]+}} = OpTypeFunction %void
void main() {

// CHECK-DAG: %bool = OpTypeBool
// CHECK-DAG: %_ptr_Function_bool = OpTypePointer Function %bool
  bool boolvar;

// CHECK-DAG: %int = OpTypeInt 32 1
// CHECK-DAG: %_ptr_Function_int = OpTypePointer Function %int
  int      intvar;
  min16int m16i;
  min12int m12i;

// CHECK-DAG: %uint = OpTypeInt 32 0
// CHECK-DAG: %_ptr_Function_uint = OpTypePointer Function %uint
  uint      uintvar;
  dword     dwordvar;
  min16uint m16u;

// CHECK-DAG: %long = OpTypeInt 64 1
// CHECK-DAG: %_ptr_Function_long = OpTypePointer Function %long
  int64_t    int64var;

// CHECK-DAG: %ulong = OpTypeInt 64 0
// CHECK-DAG: %_ptr_Function_ulong = OpTypePointer Function %ulong
  uint64_t   uint64var;

// CHECK-DAG: %float = OpTypeFloat 32
// CHECK-DAG: %_ptr_Function_float = OpTypePointer Function %float
  float      floatvar;
  half       halfvar;
  min16float m16f;
  min10float m10f;

  snorm float snf;
  unorm float unf;

// CHECK-DAG: %double = OpTypeFloat 64
// CHECK-DAG: %_ptr_Function_double = OpTypePointer Function %double
  double doublevar;

// These following variables should use the types already defined above.
  bool1       boolvar1;
  int1        intvar1;
  min16int1   m16i1;
  min12int1   m12i1;
  uint1       uintvar1;
  dword1      dwordvar1;
  min16uint1  m16u1;
  float1      floatvar1;
  half1       halfvar1;
  min16float1 m16f1;
  min10float1 m10f1;
  snorm float1 snf1;
  unorm float1 unf1;
  double1     doublevar1;

// CHECK:         %boolvar = OpVariable %_ptr_Function_bool Function
// CHECK-NEXT:     %intvar = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:       %m16i = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:       %m12i = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:    %uintvar = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:   %dwordvar = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:       %m16u = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:   %int64var = OpVariable %_ptr_Function_long Function
// CHECK-NEXT:  %uint64var = OpVariable %_ptr_Function_ulong Function
// CHECK-NEXT:   %floatvar = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:    %halfvar = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:       %m16f = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:       %m10f = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:        %snf = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:        %unf = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:  %doublevar = OpVariable %_ptr_Function_double Function
// CHECK-NEXT:   %boolvar1 = OpVariable %_ptr_Function_bool Function
// CHECK-NEXT:    %intvar1 = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:      %m16i1 = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:      %m12i1 = OpVariable %_ptr_Function_int Function
// CHECK-NEXT:   %uintvar1 = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:  %dwordvar1 = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:      %m16u1 = OpVariable %_ptr_Function_uint Function
// CHECK-NEXT:  %floatvar1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:   %halfvar1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:      %m16f1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:      %m10f1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:       %snf1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT:       %unf1 = OpVariable %_ptr_Function_float Function
// CHECK-NEXT: %doublevar1 = OpVariable %_ptr_Function_double Function
}
