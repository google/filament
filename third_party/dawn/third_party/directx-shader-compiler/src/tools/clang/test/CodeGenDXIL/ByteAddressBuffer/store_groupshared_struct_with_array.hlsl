// This is a regression test for a shader that crashed the compiler. The crash
// occured because we failed to flatten the multi-level gep into a single-level
// gep which caused a crash in the multi-dimension global flattening pass.
//
// The shader is compiled with three different index values (two different
// constants and a dynamic index value). Each of these crashed in different
// places in the compiler as I was developing a fix. Originally, the /Od
// version crashed with index 0 or 1 and the /O3 version crashed with
// index 1.

// RUN: dxc /Tcs_6_0     /DINDEX=0 %s   | FileCheck %s
// RUN: dxc /Tcs_6_0 /Od /DINDEX=0 %s   | FileCheck %s
// RUN: dxc /Tcs_6_0     /DINDEX=1 %s   | FileCheck %s
// RUN: dxc /Tcs_6_0 /Od /DINDEX=1 %s   | FileCheck %s
// RUN: dxc /Tcs_6_0     /DINDEX=idx %s | FileCheck %s
// RUN: dxc /Tcs_6_0 /Od /DINDEX=idx %s | FileCheck %s


// CHECK: @main
// CHECK-DAG: [[ELEM_0:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_0]]

// CHECK-DAG: [[ELEM_1:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_1]]

// CHECK-DAG: [[ELEM_2:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_2]]

// CHECK-DAG: [[ELEM_3:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_3]]

// CHECK-DAG: [[ELEM_4:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_4]]

// CHECK-DAG: [[ELEM_5:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_5]]

// CHECK-DAG: [[ELEM_6:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_6]]

// CHECK-DAG: [[ELEM_7:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_7]]

// CHECK-DAG: [[ELEM_8:%[0-9]+]] = load float, float addrspace(3)*
// CHECK-DAG: call void @dx.op.bufferStore.f32({{.*}}, float [[ELEM_8]]

struct Vec9 { float data[9]; };
groupshared Vec9 A[256];
 
RWByteAddressBuffer buf;
  
[numthreads(1,1,1)]
[RootSignature("DescriptorTable(SRV(t0, numDescriptors=10), UAV(u0,numDescriptors=10))")]
void main(uint idx : SV_GroupID) 
{
    buf.Store<Vec9>(idx, A[INDEX]);
}
