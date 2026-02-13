// RUN: %dxc -HV 2021 -T cs_6_7 -E main -fcgl  %s -spirv | FileCheck %s

template<typename MatT>
struct matrix_traits;

template<typename T, int32_t N, int32_t M>
struct matrix_traits<matrix<T,N,M> >
{
    static const uint32_t RowCount = N;
    static const uint32_t ColumnCount = M;
};

template<typename MatT>
uint32_t elementCount()
{
    return matrix_traits<MatT>::RowCount * matrix_traits<MatT>::ColumnCount;
}

RWBuffer<int> o;

// Initialize the static members at the start of wrapper
// CHECK: %main = OpFunction %void None 
// CHECK: OpStore %RowCount %uint_4
// CHECK: OpStore %ColumnCount %uint_4
// CHECK: OpStore %RowCount_0 %uint_3
// CHECK: OpStore %ColumnCount_0 %uint_2
// CHECK: OpFunctionEnd



// CHECK: %src_main = OpFunction %void None
[numthreads(64,1,1)]
void main()
{
// CHECK: OpFunctionCall %uint %elementCount
    o[0] = elementCount<float32_t4x4>();
// CHECK: OpFunctionCall %uint %elementCount_0
    o[1] = elementCount<float32_t3x2>();
}

// CHECK: %elementCount = OpFunction %uint None
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[rc:%[0-9]+]] = OpLoad %uint %RowCount
// CHECK-NEXT: [[cc:%[0-9]+]] = OpLoad %uint %ColumnCount
// CHECK-NEXT: [[mul:%[0-9]+]] = OpIMul %uint [[rc]] [[cc]]
// CHECK-NEXT: OpReturnValue [[mul]]
// CHECK-NEXT: OpFunctionEnd

// CHECK: %elementCount_0 = OpFunction %uint None
// CHECK-NEXT: %bb_entry_1 = OpLabel
// CHECK-NEXT: [[rc:%[0-9]+]] = OpLoad %uint %RowCount_0
// CHECK-NEXT: [[cc:%[0-9]+]] = OpLoad %uint %ColumnCount_0
// CHECK-NEXT: [[mul:%[0-9]+]] = OpIMul %uint [[rc]] [[cc]]
// CHECK-NEXT: OpReturnValue [[mul]]
// CHECK-NEXT: OpFunctionEnd
