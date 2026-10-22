; RUN: %opt %s -reg2mem_hlsl -S | FileCheck %s

; Make sure store is after load.
; CHECK: while.body:
; CHECK: load i32
; CHECK: while.body.while.cond_crit_edge:
; CHECK: store i32 -1

; ModuleID = 'MyModule'
target triple = "dxil-ms-dx"

%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%class.RWTexture2D = type { <4 x float> }
%Viewport = type { <2 x float>, <2 x float> }
%Constants = type { %struct.DispatchRaysConstants.6 }
%struct.DispatchRaysConstants.6 = type { <2 x i32> }

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadId.i32(i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #2

define void @main() {
entry:
  %0 = call i32 @dx.op.threadId.i32(i32 93, i32 0)
  %1 = call i32 @dx.op.threadId.i32(i32 93, i32 1)
  %2 = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  br label %while.cond.outer

while.cond.outer:                                 ; preds = %sw.bb.5.i, %sw.bb.3.i, %sw.bb.i, %while.body, %entry
  %stateObj.0.0.0.i076.ph = phi float [ 0.000000e+00, %entry ], [ %11, %sw.bb.i ], [ %stateObj.0.0.0.i076.ph, %sw.bb.3.i ], [ %stateObj.0.0.0.i076.ph, %sw.bb.5.i ], [ %stateObj.0.0.0.i076.ph, %while.body ]
  %stateObj.0.0.0.i177.ph = phi float [ 0.000000e+00, %entry ], [ %16, %sw.bb.i ], [ %stateObj.0.0.0.i177.ph, %sw.bb.3.i ], [ %stateObj.0.0.0.i177.ph, %sw.bb.5.i ], [ %stateObj.0.0.0.i177.ph, %while.body ]
  %stateObj.0.0.0.i278.ph = phi float [ 0.000000e+00, %entry ], [ 0.000000e+00, %sw.bb.i ], [ %stateObj.0.0.0.i278.ph, %sw.bb.3.i ], [ %stateObj.0.0.0.i278.ph, %sw.bb.5.i ], [ %stateObj.0.0.0.i278.ph, %while.body ]
  %stateObj.0.1.0.ph = phi float [ 0.000000e+00, %entry ], [ 0.000000e+00, %sw.bb.i ], [ %stateObj.0.1.0.ph, %sw.bb.3.i ], [ %stateObj.0.1.0.ph, %sw.bb.5.i ], [ %stateObj.0.1.0.ph, %while.body ]
  %stateObj.0.2.0.i079.ph = phi float [ 0.000000e+00, %entry ], [ 0.000000e+00, %sw.bb.i ], [ %stateObj.0.2.0.i079.ph, %sw.bb.3.i ], [ %stateObj.0.2.0.i079.ph, %sw.bb.5.i ], [ %stateObj.0.2.0.i079.ph, %while.body ]
  %stateObj.0.2.0.i180.ph = phi float [ 0.000000e+00, %entry ], [ 0.000000e+00, %sw.bb.i ], [ %stateObj.0.2.0.i180.ph, %sw.bb.3.i ], [ %stateObj.0.2.0.i180.ph, %sw.bb.5.i ], [ %stateObj.0.2.0.i180.ph, %while.body ]
  %stateObj.0.2.0.i281.ph = phi float [ 0.000000e+00, %entry ], [ 1.000000e+00, %sw.bb.i ], [ %stateObj.0.2.0.i281.ph, %sw.bb.3.i ], [ %stateObj.0.2.0.i281.ph, %sw.bb.5.i ], [ %stateObj.0.2.0.i281.ph, %while.body ]
  %stateObj.0.3.0.ph = phi float [ 0.000000e+00, %entry ], [ 1.000000e+04, %sw.bb.i ], [ %stateObj.0.3.0.ph, %sw.bb.3.i ], [ %stateObj.0.3.0.ph, %sw.bb.5.i ], [ %stateObj.0.3.0.ph, %while.body ]
  %stateId.0.ph = phi i32 [ 0, %entry ], [ 1, %sw.bb.i ], [ -1, %sw.bb.3.i ], [ -1, %sw.bb.5.i ], [ 3, %while.body ]
  br label %while.cond

while.cond:                                       ; preds = %while.body, %while.cond.outer
  %stateId.0 = phi i32 [ -1, %while.body ], [ %stateId.0.ph, %while.cond.outer ]
  %cmp = icmp sgt i32 %stateId.0, -1
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  switch i32 %stateId.0, label %while.cond [
    i32 0, label %sw.bb.i
    i32 1, label %while.cond.outer
    i32 2, label %sw.bb.3.i
    i32 3, label %sw.bb.5.i
  ]

sw.bb.i:                                          ; preds = %while.body
  %Viewport_buffer.i = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false) #2
  %conv.i0.i = uitofp i32 %0 to float
  %conv.i1.i = uitofp i32 %1 to float
  %Constants_buffer.i.i = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 1, i32 4, i1 false) #2
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %Constants_buffer.i.i, i32 0) #2
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 0
  %5 = extractvalue %dx.types.CBufRet.i32 %3, 1
  %conv2.i0.i = uitofp i32 %4 to float
  %conv2.i1.i = uitofp i32 %5 to float
  %div.i0.i = fdiv float %conv.i0.i, %conv2.i0.i
  %div.i1.i = fdiv float %conv.i1.i, %conv2.i1.i
  %6 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Viewport_buffer.i, i32 0) #2
  %7 = extractvalue %dx.types.CBufRet.f32 %6, 2
  %8 = extractvalue %dx.types.CBufRet.f32 %6, 0
  %9 = fsub float %7, %8
  %10 = fmul float %div.i0.i, %9
  %11 = fadd float %8, %10
  %12 = extractvalue %dx.types.CBufRet.f32 %6, 3
  %13 = extractvalue %dx.types.CBufRet.f32 %6, 1
  %14 = fsub float %12, %13
  %15 = fmul float %div.i1.i, %14
  %16 = fadd float %13, %15
  br label %while.cond.outer

sw.bb.3.i:                                        ; preds = %while.body
  %RenderTarget_UAV_2d.i = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false) #2
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %RenderTarget_UAV_2d.i, i32 %0, i32 %1, i32 undef, float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, i8 15) #2
  br label %while.cond.outer

sw.bb.5.i:                                        ; preds = %while.body
  %RenderTarget_UAV_2d.i.1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false) #2
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %RenderTarget_UAV_2d.i.1, i32 %0, i32 %1, i32 undef, float 1.000000e+00, float 0.000000e+00, float 1.000000e+00, float 1.000000e+00, i8 15) #2
  br label %while.cond.outer

while.end:                                        ; preds = %while.cond
  ret void
}

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.typeAnnotations = !{!9}
!dx.entryPoints = !{!13}

!0 = !{i32 1, i32 0}
!1 = !{!"cs", i32 6, i32 0}
!2 = !{null, !3, !6, null}
!3 = !{!4}
!4 = !{i32 0, %class.RWTexture2D* undef, !"RenderTarget", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !5}
!5 = !{i32 0, i32 9}
!6 = !{!7, !8}
!7 = !{i32 0, %Viewport* undef, !"Viewport", i32 0, i32 0, i32 1, i32 16, null}
!8 = !{i32 1, %Constants* undef, !"Constants", i32 2147473647, i32 4, i32 1, i32 8, null}
!9 = !{i32 1, void ()* @main, !10}
!10 = !{!11}
!11 = !{i32 0, !12, !12}
!12 = !{}
!13 = !{void ()* @main, !"main", null, !2, !14}
!14 = !{i32 4, !15}
!15 = !{i32 8, i32 4, i32 1}