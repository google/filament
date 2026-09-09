; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Test DXIL validation for ClusterID intrinsics with invalid inputs

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.HitObject = type { i8* }

; CHECK-DAG: Function: test_candidate_clusterid_invalid_handle: error: Instructions should not read uninitialized value.
; CHECK-DAG: Function: test_committed_clusterid_invalid_handle: error: Instructions should not read uninitialized value.
; CHECK-DAG: Function: test_hitobject_clusterid_undef: error: HitObject is undef.
; CHECK: Validation failed

; Test CandidateClusterID with invalid rayQueryHandle (undef)
define void @test_candidate_clusterid_invalid_handle() #0 {
entry:
  ; Using undef as rayQueryHandle should fail validation
  %0 = call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483644, i32 undef)
  ret void
}

; Test CommittedClusterID with invalid rayQueryHandle (undef)
define void @test_committed_clusterid_invalid_handle() #0 {
entry:
  ; Using undef as rayQueryHandle should fail validation
  %0 = call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483643, i32 undef)
  ret void
}

; Test HitObject GetClusterID with undef HitObject
define void @test_hitobject_clusterid_undef() #0 {
entry:
  ; Using undef HitObject should fail validation
  %0 = call i32 @dx.op.hitObject_StateScalar.i32(i32 -2147483642, %dx.types.HitObject undef)
  ret void
}

declare i32 @dx.op.rayQuery_StateScalar.i32(i32, i32) #1
declare i32 @dx.op.hitObject_StateScalar.i32(i32, %dx.types.HitObject) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !10, !11}

!0 = !{!"dxc(private)"}
!1 = !{i32 1, i32 10}
!2 = !{!"lib", i32 6, i32 10}
!3 = !{i32 1, void ()* @test_candidate_clusterid_invalid_handle, !4, void ()* @test_committed_clusterid_invalid_handle, !4, void ()* @test_hitobject_clusterid_undef, !4}
!4 = !{!5}
!5 = !{i32 1, !6, !6}
!6 = !{}
!7 = !{void ()* @test_candidate_clusterid_invalid_handle, !"test_candidate_clusterid_invalid_handle", null, null, !8}
!8 = !{i32 8, i32 7, i32 5, !9}
!9 = !{i32 0}
!10 = !{void ()* @test_committed_clusterid_invalid_handle, !"test_committed_clusterid_invalid_handle", null, null, !8}
!11 = !{void ()* @test_hitobject_clusterid_undef, !"test_hitobject_clusterid_undef", null, null, !8}
