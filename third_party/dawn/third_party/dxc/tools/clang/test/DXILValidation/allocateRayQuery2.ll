; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RaytracingAccelerationStructure = type { i32 }
@nonconstVal = global i32 5 ; this is a non-const value

define void @main() {
  ; test for incompatible flags
  ; CHECK-DAG: error: RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS must be set for RayQueryFlags when RAY_FLAG_FORCE_OMM_2_STATE is set for constRayFlags on AllocateRayQuery2 operation.
  ; CHECK-DAG: note: at '%1 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 0)' in block '#0' of function 'main'.
  %1 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 0)  ; AllocateRayQuery2(constRayFlags, RayQueryFlags)
  %ptr = getelementptr i32, i32* @nonconstVal, i32 0 ; Get the pointer to the global variable
  %val = load i32, i32* %ptr ; Dereference the pointer to load the value
  
  ; test for all combinations of non-constant flags
  ; CHECK-DAG: error: constRayFlags and RayQueryFlags arguments of AllocateRayQuery2 must be constant
  ; CHECK-DAG: note: at '%2 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 %val)' in block '#0' of function 'main'.
  %2 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 %val)  ; AllocateRayQuery2(constRayFlags, RayQueryFlags)
  ; CHECK-DAG: error: constRayFlags and RayQueryFlags arguments of AllocateRayQuery2 must be constant
  ; CHECK-DAG: note: at '%3 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 %val, i32 1)' in block '#0' of function 'main'.
  %3 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 %val, i32 1)  ; AllocateRayQuery2(constRayFlags, RayQueryFlags)
  ; CHECK-DAG: error: constRayFlags and RayQueryFlags arguments of AllocateRayQuery2 must be constant
  ; CHECK-DAG: note: at '%4 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 %val, i32 %val)' in block '#0' of function 'main'.
  %4 = call i32 @dx.op.allocateRayQuery2(i32 258, i32 %val, i32 %val)  ; AllocateRayQuery2(constRayFlags, RayQueryFlags)

  ret void
}

; Function Attrs: nounwind
declare i32 @dx.op.allocateRayQuery2(i32, i32, i32) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!7}
!dx.entryPoints = !{!8}

!0 = !{!"dxc(private) 1.8.0.4793 (OMM00, ef1472a14a0d-dirty)"}
!1 = !{i32 1, i32 9}
!2 = !{!"vs", i32 6, i32 9}
!3 = !{!4, null, null, null}
!4 = !{!5}
!5 = !{i32 0, %struct.RaytracingAccelerationStructure* undef, !"", i32 0, i32 0, i32 1, i32 16, i32 0, !6}
!6 = !{i32 0, i32 4}
!7 = !{[2 x i32] [i32 17, i32 0]}
!8 = !{void ()* @main, !"main", !9, !3, !22}
!9 = !{!10, null, null}
!10 = !{!11, !13, !15, !18, !20}
!11 = !{i32 0, !"IDX", i8 5, i8 0, !12, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!12 = !{i32 0}
!13 = !{i32 1, !"RAYDESC", i8 9, i8 0, !12, i8 0, i32 1, i8 3, i32 1, i8 0, !14}
!14 = !{i32 3, i32 7}
!15 = !{i32 2, !"RAYDESC", i8 9, i8 0, !16, i8 0, i32 1, i8 1, i32 2, i8 0, !17}
!16 = !{i32 1}
!17 = !{i32 3, i32 1}
!18 = !{i32 3, !"RAYDESC", i8 9, i8 0, !19, i8 0, i32 1, i8 3, i32 3, i8 0, !14}
!19 = !{i32 2}
!20 = !{i32 4, !"RAYDESC", i8 9, i8 0, !21, i8 0, i32 1, i8 1, i32 4, i8 0, !17}
!21 = !{i32 3}
!22 = !{i32 0, i64 33554432}
