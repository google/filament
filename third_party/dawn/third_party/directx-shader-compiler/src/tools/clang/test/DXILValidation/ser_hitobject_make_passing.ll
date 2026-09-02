; RUN: %dxv %s | FileCheck %s

; CHECK: Validation succeeded.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.HitObject = type { i8* }

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
  ; Test HitObject_MakeMiss (opcode 265)
  %r265 = call %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32 265, i32 4, i32 0, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 9.999000e+03)  ; HitObject_MakeMiss(RayFlags,MissShaderIndex,Origin_X,Origin_Y,Origin_Z,TMin,Direction_X,Direction_Y,Direction_Z,TMax)

  ; Test HitObject_MakeNop (opcode 266)
  %r266 = call %dx.types.HitObject @dx.op.hitObject_MakeNop(i32 266)  ; HitObject_MakeNop()

  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_MakeMiss(i32, i32, i32, float, float, float, float, float, float, float, float) #1

; Function Attrs: nounwind readnone
declare %dx.types.HitObject @dx.op.hitObject_MakeNop(i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.typeAnnotations = !{!2}
!dx.entryPoints = !{!9, !11}

!0 = !{i32 1, i32 9}
!1 = !{!"lib", i32 6, i32 9}
!2 = !{i32 1, void ()* @"\01?main@@YAXXZ", !3}
!3 = !{!4}
!4 = !{i32 1, !5, !5}
!5 = !{}
!9 = !{null, !"", null, null, !10}
!10 = !{i32 0, i64 0}
!11 = !{void ()* @"\01?main@@YAXXZ", !"\01?main@@YAXXZ", null, null, !12}
!12 = !{i32 8, i32 7, i32 5, !13}
!13 = !{i32 0}
