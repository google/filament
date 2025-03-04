; RUN: opt < %s -sroa -S | FileCheck %s
target datalayout = "e-p:64:64:64-p1:16:16:16-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-n8:16:32:64"

declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture readonly, i32, i32, i1)
declare void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* nocapture, i8* nocapture readonly, i32, i32, i1)
declare void @llvm.memcpy.p0i8.p1i8.i32(i8* nocapture, i8 addrspace(1)* nocapture readonly, i32, i32, i1)
declare void @llvm.memcpy.p1i8.p1i8.i32(i8 addrspace(1)* nocapture, i8 addrspace(1)* nocapture readonly, i32, i32, i1)


; Make sure an illegal bitcast isn't introduced
define void @test_address_space_1_1(<2 x i64> addrspace(1)* %a, i16 addrspace(1)* %b) {
; CHECK-LABEL: @test_address_space_1_1(
; CHECK: load <2 x i64>, <2 x i64> addrspace(1)* %a, align 2
; CHECK: store <2 x i64> {{.*}}, <2 x i64> addrspace(1)* {{.*}}, align 2
; CHECK: ret void
  %aa = alloca <2 x i64>, align 16
  %aptr = bitcast <2 x i64> addrspace(1)* %a to i8 addrspace(1)*
  %aaptr = bitcast <2 x i64>* %aa to i8*
  call void @llvm.memcpy.p0i8.p1i8.i32(i8* %aaptr, i8 addrspace(1)* %aptr, i32 16, i32 2, i1 false)
  %bptr = bitcast i16 addrspace(1)* %b to i8 addrspace(1)*
  call void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* %bptr, i8* %aaptr, i32 16, i32 2, i1 false)
  ret void
}

define void @test_address_space_1_0(<2 x i64> addrspace(1)* %a, i16* %b) {
; CHECK-LABEL: @test_address_space_1_0(
; CHECK: load <2 x i64>, <2 x i64> addrspace(1)* %a, align 2
; CHECK: store <2 x i64> {{.*}}, <2 x i64>* {{.*}}, align 2
; CHECK: ret void
  %aa = alloca <2 x i64>, align 16
  %aptr = bitcast <2 x i64> addrspace(1)* %a to i8 addrspace(1)*
  %aaptr = bitcast <2 x i64>* %aa to i8*
  call void @llvm.memcpy.p0i8.p1i8.i32(i8* %aaptr, i8 addrspace(1)* %aptr, i32 16, i32 2, i1 false)
  %bptr = bitcast i16* %b to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %bptr, i8* %aaptr, i32 16, i32 2, i1 false)
  ret void
}

define void @test_address_space_0_1(<2 x i64>* %a, i16 addrspace(1)* %b) {
; CHECK-LABEL: @test_address_space_0_1(
; CHECK: load <2 x i64>, <2 x i64>* %a, align 2
; CHECK: store <2 x i64> {{.*}}, <2 x i64> addrspace(1)* {{.*}}, align 2
; CHECK: ret void
  %aa = alloca <2 x i64>, align 16
  %aptr = bitcast <2 x i64>* %a to i8*
  %aaptr = bitcast <2 x i64>* %aa to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %aaptr, i8* %aptr, i32 16, i32 2, i1 false)
  %bptr = bitcast i16 addrspace(1)* %b to i8 addrspace(1)*
  call void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* %bptr, i8* %aaptr, i32 16, i32 2, i1 false)
  ret void
}

%struct.struct_test_27.0.13 = type { i32, float, i64, i8, [4 x i32] }

; Function Attrs: nounwind
define void @copy_struct([5 x i64] %in.coerce) {
; CHECK-LABEL: @copy_struct(
; CHECK-NOT: memcpy
for.end:
  %in = alloca %struct.struct_test_27.0.13, align 8
  %0 = bitcast %struct.struct_test_27.0.13* %in to [5 x i64]*
  store [5 x i64] %in.coerce, [5 x i64]* %0, align 8
  %scevgep9 = getelementptr %struct.struct_test_27.0.13, %struct.struct_test_27.0.13* %in, i32 0, i32 4, i32 0
  %scevgep910 = bitcast i32* %scevgep9 to i8*
  call void @llvm.memcpy.p1i8.p0i8.i32(i8 addrspace(1)* undef, i8* %scevgep910, i32 16, i32 4, i1 false)
  ret void
}
 
