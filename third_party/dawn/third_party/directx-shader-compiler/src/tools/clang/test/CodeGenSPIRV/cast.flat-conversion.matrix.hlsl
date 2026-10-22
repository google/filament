// RUN: %dxc -T ps_6_0 -E main %s -spirv -Zpc | FileCheck %s -check-prefix=COL -check-prefix=CHECK
// RUN: %dxc -T ps_6_0 -E main %s -spirv -Zpr | FileCheck %s -check-prefix=ROW -check-prefix=CHECK

struct S {
  float2x3 a;
};

struct T {
  float a[6];
};

RWStructuredBuffer<S> s_output;
RWStructuredBuffer<T> t_output;

// See https://github.com/microsoft/DirectXShaderCompiler/blob/438781364eea22d188b337be1dfa4174ed7d928c/docs/SPIR-V.rst#L723.
// COL: OpMemberDecorate %S 0 RowMajor
// ROW: OpMemberDecorate %S 0 ColMajor

// The DXIL generated for the two cases seem to produce the same results,
// and this matches that behaviour.
// CHECK: [[array_const:%[0-9]+]] = OpConstantComposite %_arr_float_uint_6 %float_0 %float_1 %float_2 %float_3 %float_4 %float_5
// CHECK: [[t:%[0-9]+]] = OpConstantComposite %T [[array_const]]

// The DXIL that is generates different order for the values depending on
// whether the matrix is column or row major. However, for SPIR-V, the value
// stored in both cases is the same because the decoration, which is checked
// above, is what determines the layout in memory for the value.

// CHECK: [[row0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_1 %float_2
// CHECK: [[row1:%[0-9]+]] = OpConstantComposite %v3float %float_3 %float_4 %float_5
// CHECK: [[mat:%[0-9]+]] = OpConstantComposite %mat2v3float %33 %34
// CHECK: [[s:%[0-9]+]] = OpConstantComposite %S %35

void main() {
  S s;
  [unroll]
  for( int i = 0; i < 2; ++i) {
    [unroll]
    for( int j = 0; j < 3; ++j) {
      s.a[i][j] = i*3+j;
    }
  }
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_T %t_output %int_0 %uint_0
// CHECK: OpStore [[ac]] [[t]]
  T t = (T)(s);
  t_output[0] = t;

// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %s_output %int_0 %uint_0
// CHECK: OpStore [[ac]] [[s]]
  s = (S)t;
  s_output[0] = s;
}
