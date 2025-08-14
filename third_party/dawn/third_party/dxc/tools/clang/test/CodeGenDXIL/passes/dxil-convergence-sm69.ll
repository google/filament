; RUN: %dxopt %s -hlsl-passes-resume -hlsl-dxil-convergent-mark -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
declare <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32, <2 x float>) #1

; Function Attrs: nounwind
define void @main(<2 x float>* noalias %arg, <2 x float> %arg1, <2 x float> %arg2, i32 %arg3) #0 {
bb:

  %tmp = fadd <2 x float> %arg1, %arg2
  ; CHECK: [[vec:%.*]] = call <2 x float> @"dxil.convergent.marker.<2 x float>"(<2 x float> %tmp)
  %tmp4 = icmp sgt i32 %arg3, 2
  %tmp5 = icmp ne i1 %tmp4, false
  %tmp6 = icmp ne i1 %tmp5, false
  br i1 %tmp6, label %bb7, label %bb10

bb7:                                              ; preds = %bb
  ; CHECK: call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32 128, <2 x float> [[vec]])
  %tmp8 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32 128, <2 x float> %tmp)
  %tmp9 = fsub <2 x float> zeroinitializer, %tmp8
  br label %bb10

bb10:                                             ; preds = %bb7, %bb
  %res.0 = phi <2 x float> [ %tmp9, %bb7 ], [ zeroinitializer, %bb ]
  store <2 x float> %res.0, <2 x float>* %arg
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5}
!dx.entryPoints = !{!18}
!dx.fnprops = !{!19}
!dx.options = !{!20, !21}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4959 (coopvec-tests, 43e1db83c-dirty)"}
!3 = !{i32 1, i32 9}
!4 = !{!"ps", i32 6, i32 9}
!5 = !{i32 1, void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, !6}
!6 = !{!7, !9, !12, !14, !16}
!7 = !{i32 0, !8, !8}
!8 = !{}
!9 = !{i32 1, !10, !11}
!10 = !{i32 4, !"SV_Target", i32 7, i32 9}
!11 = !{i32 0}
!12 = !{i32 0, !13, !11}
!13 = !{i32 4, !"A", i32 7, i32 9}
!14 = !{i32 0, !15, !11}
!15 = !{i32 4, !"B", i32 7, i32 9}
!16 = !{i32 0, !17, !11}
!17 = !{i32 4, !"C", i32 7, i32 4}
!18 = !{void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, !"main", null, null, null}
!19 = !{void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, i32 0, i1 false}
!20 = !{i32 64}
!21 = !{i32 -1}
