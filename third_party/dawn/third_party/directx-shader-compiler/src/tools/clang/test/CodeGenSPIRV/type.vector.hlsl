// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-DAG: %float = OpTypeFloat 32
// CHECK-DAG: %_ptr_Function_float = OpTypePointer Function %float
    float1 a;
// CHECK-DAG: %v2float = OpTypeVector %float 2
// CHECK-DAG: %_ptr_Function_v2float = OpTypePointer Function %v2float
    float2 b;
// CHECK-DAG: %v3float = OpTypeVector %float 3
// CHECK-DAG: %_ptr_Function_v3float = OpTypePointer Function %v3float
    float3 c;
// CHECK-DAG: %v4float = OpTypeVector %float 4
// CHECK-DAG: %_ptr_Function_v4float = OpTypePointer Function %v4float
    float4 d;

// CHECK-DAG: %int = OpTypeInt 32 1
// CHECK-DAG: %_ptr_Function_int = OpTypePointer Function %int
    vector<int, 1> e;
// CHECK-DAG: %v2uint = OpTypeVector %uint 2
// CHECK-DAG: %_ptr_Function_v2uint = OpTypePointer Function %v2uint
    vector<uint, 2> f;
// CHECK-DAG: %v3bool = OpTypeVector %bool 3
// CHECK-DAG: %_ptr_Function_v3bool = OpTypePointer Function %v3bool
    vector<bool, 3> g;
// CHECK-DAG: %v4int = OpTypeVector %int 4
// CHECK-DAG: %_ptr_Function_v4int = OpTypePointer Function %v4int
    vector<int, 4> h;

// CHECK-LABEL: %bb_entry = OpLabel

// CHECK-NEXT: %a = OpVariable %_ptr_Function_float Function
// CHECK-NEXT: %b = OpVariable %_ptr_Function_v2float Function
// CHECK-NEXT: %c = OpVariable %_ptr_Function_v3float Function
// CHECK-NEXT: %d = OpVariable %_ptr_Function_v4float Function

// CHECK-NEXT: %e = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %f = OpVariable %_ptr_Function_v2uint Function
// CHECK-NEXT: %g = OpVariable %_ptr_Function_v3bool Function
// CHECK-NEXT: %h = OpVariable %_ptr_Function_v4int Function
}
