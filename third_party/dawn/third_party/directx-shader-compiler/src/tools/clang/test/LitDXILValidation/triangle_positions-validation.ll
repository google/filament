; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Test DXIL validation for TriangleObjectPosition intrinsics with invalid inputs

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.HitObject = type { i8* }

; CHECK-DAG: Function: test_candidate_trianglepos_invalid_handle: error: Instructions should not read uninitialized value.
; CHECK-DAG: Function: test_committed_trianglepos_invalid_handle: error: Instructions should not read uninitialized value.
; CHECK-DAG: Function: test_hitobject_trianglepos_undef: error: HitObject is undef.
; CHECK: Validation failed

; Test RayQuery CandidateTriangleObjectPosition with invalid rayQueryHandle (undef)
define void @test_candidate_trianglepos_invalid_handle() #0 {
entry:
  ; Using undef as rayQueryHandle should fail validation
  %0 = call <9 x float> @dx.op.rayQuery_CandidateTriangleObjectPosition.f32(i32 -2147483640, i32 undef)
  ret void
}

; Test RayQuery CommittedTriangleObjectPosition with invalid rayQueryHandle (undef)
define void @test_committed_trianglepos_invalid_handle() #0 {
entry:
  ; Using undef as rayQueryHandle should fail validation
  %0 = call <9 x float> @dx.op.rayQuery_CommittedTriangleObjectPosition.f32(i32 -2147483639, i32 undef)
  ret void
}

; Test HitObject TriangleObjectPosition with undef HitObject
define void @test_hitobject_trianglepos_undef() #0 {
entry:
  ; Using undef HitObject should fail validation
  %0 = call <9 x float> @dx.op.hitObject_TriangleObjectPosition.f32(i32 -2147483638, %dx.types.HitObject undef)
  ret void
}

; Function Attrs: nounwind readonly
declare <9 x float> @dx.op.rayQuery_CandidateTriangleObjectPosition.f32(i32, i32) #1

; Function Attrs: nounwind readonly
declare <9 x float> @dx.op.rayQuery_CommittedTriangleObjectPosition.f32(i32, i32) #1

; Function Attrs: nounwind readonly
declare <9 x float> @dx.op.hitObject_TriangleObjectPosition.f32(i32, %dx.types.HitObject) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.typeAnnotations = !{!2}
!dx.entryPoints = !{!6, !9, !10}

!0 = !{i32 1, i32 10}
!1 = !{!"lib", i32 6, i32 10}
!2 = !{i32 1, void ()* @test_candidate_trianglepos_invalid_handle, !3, void ()* @test_committed_trianglepos_invalid_handle, !3, void ()* @test_hitobject_trianglepos_undef, !3}
!3 = !{!4}
!4 = !{i32 1, !5, !5}
!5 = !{}
!6 = !{void ()* @test_candidate_trianglepos_invalid_handle, !"test_candidate_trianglepos_invalid_handle", null, null, !7}
!7 = !{i32 8, i32 7, i32 5, !8}
!8 = !{i32 0}
!9 = !{void ()* @test_committed_trianglepos_invalid_handle, !"test_committed_trianglepos_invalid_handle", null, null, !7}
!10 = !{void ()* @test_hitobject_trianglepos_undef, !"test_hitobject_trianglepos_undef", null, null, !7}
