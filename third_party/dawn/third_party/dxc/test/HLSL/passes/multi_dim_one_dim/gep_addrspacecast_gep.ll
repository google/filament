; RUN: opt -S -multi-dim-one-dim %s | FileCheck %s
;
; Tests for the pass that changes multi-dimension global variable accesses into
; a flattened one-dimensional access. The tests focus on the case where the geps
; need to be merged but are separated by an addrspacecast operation. This was
; causing the pass to fail because it could not merge the gep through the
; addrspace cast.

; Naming convention: gep0_addrspacecast_gep1

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

@ArrayOfArray = addrspace(3) global [256 x [9 x float]] undef, align 4
@ArrayOfArrayOfArray = addrspace(3) global [256 x [9 x [3 x float]]] undef, align 4

; Test that we can merge the geps when all parts are instructions.
; CHECK-LABEL: @merge_gep_instr_instr_instr
; CHECK:  load float, float* addrspacecast (float addrspace(3)* getelementptr inbounds ([2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 1) to float*)
define void @merge_gep_instr_instr_instr() {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 0
  %asc  = addrspacecast [9 x float] addrspace(3)* %gep0 to [9 x float]*
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* %asc, i32 0, i32 1
  %load = load float, float* %gep1
  ret void
}

; Test that we can merge the geps when the inner gep are constants.
; CHECK-LABEL: @merge_gep_instr_instr_const
; CHECK:  load float, float* addrspacecast (float addrspace(3)* getelementptr inbounds ([2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 1) to float*)
define void @merge_gep_instr_instr_const() {
entry:
  %asc  = addrspacecast [9 x float] addrspace(3)* getelementptr inbounds ([256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 0) to [9 x float]*
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* %asc, i32 0, i32 1
  %load = load float, float* %gep1
  ret void
}

; Test that we can merge the geps when the addrspace and inner gep are constants.
; CHECK-LABEL: @merge_gep_instr_const_const
; CHECK:  load float, float* addrspacecast (float addrspace(3)* getelementptr inbounds ([2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 1) to float*)
define void @merge_gep_instr_const_const() {
entry:
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* addrspacecast ([9 x float] addrspace(3)* getelementptr inbounds ([256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 0) to [9 x float]*), i32 0, i32 1
  %load = load float, float* %gep1
  ret void
}

; Test that we can merge the geps when all parts are constants.
; CHECK-LABEL: @merge_gep_const_const
; CHECK: load float, float* addrspacecast (float addrspace(3)* getelementptr inbounds ([2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 1) to float*)
define void @merge_gep_const_const_const() {
entry:
  %load = load float, float* getelementptr inbounds ([9 x float], [9 x float]* addrspacecast ([9 x float] addrspace(3)* getelementptr inbounds ([256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 0) to [9 x float]*), i32 0, i32 1)
  ret void
}

; Test that we compute the correct index when the outer array has
; a non-zero constant index.
; CHECK-LABEL: @merge_gep_const_outer_array_index
; CHECK:  load float, float* addrspacecast (float addrspace(3)* getelementptr inbounds ([2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 66) to float*)
define void @merge_gep_const_outer_array_index() {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 7
  %asc  = addrspacecast [9 x float] addrspace(3)* %gep0 to [9 x float]*
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* %asc, i32 0, i32 3
  %load = load float, float* %gep1
  ret void
}

; Test that we compute the correct index when the outer array has
; a non-constant index.
; CHECK-LABEL: @merge_gep_dynamic_outer_array_index
; CHECK:   %0 = mul i32 %idx, 9
; CHECK:   %1 = add i32 3, %0
; CHECK:   %2 = getelementptr [2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 %1
; CHECK:   %3 = addrspacecast float addrspace(3)* %2 to float*
; CHECK:   load float, float* %3
define void @merge_gep_dynamic_outer_array_index(i32 %idx) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 %idx
  %asc  = addrspacecast [9 x float] addrspace(3)* %gep0 to [9 x float]*
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* %asc, i32 0, i32 3
  %load = load float, float* %gep1
  ret void
}

; Test that we compute the correct index when the both arrays have
; a non-constant index.
; CHECK-LABEL: @merge_gep_dynamic_array_index
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = getelementptr [2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 %1
; CHECK:  %3 = addrspacecast float addrspace(3)* %2 to float*
; CHECK:  load float, float* %3
define void @merge_gep_dynamic_array_index(i32 %idx0, i32 %idx1) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 %idx0
  %asc  = addrspacecast [9 x float] addrspace(3)* %gep0 to [9 x float]*
  %gep1 = getelementptr inbounds [9 x float], [9 x float]* %asc, i32 0, i32 %idx1
  %load = load float, float* %gep1
  ret void
}

; Test that we compute the correct index when there are multiple
; geps after the addrspacecast. This also exercises the case
; where one of the outer geps ends in an array which hits
; an early return in MergeGEP.
; CHECK-LABEL: @merge_gep_multi_level_end_in_sequential_with_addrspace
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = getelementptr [2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 %1
; CHECK:  %3 = addrspacecast float addrspace(3)* %2 to float*
; CHECK:  load float, float* %3
define void @merge_gep_multi_level_end_in_sequential_with_addrspace(i32 %idx0, i32 %idx1) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0
  %asc  = addrspacecast [256 x [9 x float]] addrspace(3)* %gep0 to [256 x [9 x float]]*
  %gep1 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]]* %asc, i32 0, i32 %idx0
  %gep2 = getelementptr inbounds [9 x float], [9 x float]* %gep1, i32 0, i32 %idx1
  %load = load float, float* %gep2
  ret void
}

; Test that we compute the correct index when there are three levels of geps.
; This also exercises the case where one of the outer geps ends in an
; array which hits an early return in MergeGEP.
; CHECK-LABEL: @merge_gep_multi_level_end_in_sequential
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = getelementptr [2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 %1
; CHECK:  load float, float addrspace(3)* %2
define void @merge_gep_multi_level_end_in_sequential(i32 %idx0, i32 %idx1) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0
  %gep1 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* %gep0, i32 0, i32 %idx0
  %gep2 = getelementptr inbounds [9 x float], [9 x float] addrspace(3)* %gep1, i32 0, i32 %idx1
  %load = load float, float addrspace(3)* %gep2
  ret void
}

; Test that we compute the correct index when the global has 3 levels of
; nested arrays and an addrspacecast.
; CHECK-LABEL: @merge_gep_multi_level_with_addrspace
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = mul i32 %1, 3
; CHECK:  %3 = add i32 %idx2, %2
; CHECK:  %4 = getelementptr [6912 x float], [6912 x float] addrspace(3)* @ArrayOfArrayOfArray.1dim, i32 0, i32 %3
; CHECK:  %5 = addrspacecast float addrspace(3)* %4 to float*
; CHECK:  load float, float* %5
define void @merge_gep_multi_level_with_addrspace(i32 %idx0, i32 %idx1, i32 %idx2) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x [3 x float]]], [256 x [9 x [3 x float]]] addrspace(3)* @ArrayOfArrayOfArray, i32 0, i32 %idx0
  %asc  = addrspacecast [9 x [3 x float]] addrspace(3)* %gep0 to [9 x [3 x float]]*
  %gep1 = getelementptr inbounds [9 x [3 x float]], [9 x [3 x float]]* %asc, i32 0, i32 %idx1
  %gep2 = getelementptr inbounds [3 x float], [3 x float]* %gep1, i32 0, i32 %idx2
  %load = load float, float* %gep2
  ret void
}

; Test that we compute the correct index when the global has 3 levels of
; nested arrays.
; CHECK-LABEL: @merge_gep_multi_level
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = mul i32 %1, 3
; CHECK:  %3 = add i32 %idx2, %2
; CHECK:  %4 = getelementptr [6912 x float], [6912 x float] addrspace(3)* @ArrayOfArrayOfArray.1dim, i32 0, i32 %3
; CHECK:  load float, float addrspace(3)* %4
define void @merge_gep_multi_level(i32 %idx0, i32 %idx1, i32 %idx2) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x [3 x float]]], [256 x [9 x [3 x float]]] addrspace(3)* @ArrayOfArrayOfArray, i32 0, i32 %idx0
  %gep1 = getelementptr inbounds [9 x [3 x float]], [9 x [3 x float]] addrspace(3)* %gep0, i32 0, i32 %idx1
  %gep2 = getelementptr inbounds [3 x float], [3 x float] addrspace(3)* %gep1, i32 0, i32 %idx2
  %load = load float, float addrspace(3)* %gep2
  ret void
}

; Test that we compute the correct index when the addrspacecast includes both a
; change in address space and a change in the underlying type. I did not see
; this pattern in IR generated from hlsl, but we can handle this case so I am
; adding a test for it anyway.
; CHECK-LABEL: addrspace_cast_new_type
; CHECK:  %0 = mul i32 %idx0, 9
; CHECK:  %1 = add i32 %idx1, %0
; CHECK:  %2 = getelementptr [2304 x float], [2304 x float] addrspace(3)* @ArrayOfArray.1dim, i32 0, i32 %1
; CHECK:  %3 = addrspacecast float addrspace(3)* %2 to i32*
; CHECK:  load i32, i32* %3
define void @addrspace_cast_new_type(i32 %idx0, i32 %idx1) {
entry:
  %gep0 = getelementptr inbounds [256 x [9 x float]], [256 x [9 x float]] addrspace(3)* @ArrayOfArray, i32 0, i32 %idx0
  %asc  = addrspacecast [9 x float] addrspace(3)* %gep0 to [3 x i32]*
  %gep1 = getelementptr inbounds [3 x i32], [3 x i32]* %asc, i32 0, i32 %idx1
  %load = load i32, i32* %gep1
  ret void
}