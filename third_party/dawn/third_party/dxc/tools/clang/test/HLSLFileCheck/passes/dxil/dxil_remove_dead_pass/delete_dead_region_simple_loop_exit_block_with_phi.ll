; RUN: %opt %s -dxil-erase-dead-region -S | FileCheck %s

; Regression test for a bug caused by an incorrect assumption that the exit block of simple
; loops with no exiting values definitely don't have any phis.
;
; In the code below, the loop `for.body` has no value that's used outside of the loop, but the
; exit block `for.cond.for.end_crit_edge` DOES have a phi. The incoming value for the phi comes from
; outside the loop. It was left over from lcssa, when the incoming value did originally come from
; inside the loop, but was replaced later.

; Check 1) compiler didn't crash or assert. 2) Loop was actually deleted by the pass. 3) phi's incoming block was correctly replaced.

;;// Original HLSL source, stopped right before dxil-erase-dead-region.
;; 
;; StructuredBuffer<float3> buf : register(t0);
;; 
;; float4 main(float4 input : INPUT, int bound : BOUND) : SV_Target {
;;   float4 loopInput = input;
;; 
;;   float4 loopOutput = loopInput;
;;   for (int i = 0; i < bound; i++) {
;;     float4 loopVal;
;;     loopVal.rgb = loopInput.rgb += buf[i];
;;     loopVal.a = loopInput.a;
;;     loopOutput = loopVal;
;;   }
;; 
;;   float4 ret = loopOutput;
;;   ret.rgb *= loopOutput.a;
;;   ret.rgb *= 0;
;;   return ret;
;; }

; CHECK: @main
; CHECK-NOT: for.body:
; CHECK-NOT: for.inc
; CHECK: %loopInput.0.i3.lcssa = phi float [ %4, %for.body.lr.ph ]

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.StructuredBuffer<vector<float, 3> >" = type { <3 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }

@"\01?buf@@3V?$StructuredBuffer@V?$vector@M$02@@@@A" = external global %"class.StructuredBuffer<vector<float, 3> >", align 4
@llvm.used = appending global [1 x i8*] [i8* bitcast (%"class.StructuredBuffer<vector<float, 3> >"* @"\01?buf@@3V?$StructuredBuffer@V?$vector@M$02@@@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<vector<float, 3> >\22)"(i32, %"class.StructuredBuffer<vector<float, 3> >") #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<vector<float, 3> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.StructuredBuffer<vector<float, 3> >") #0

; Function Attrs: convergent
declare void @dx.noop() #1

; Function Attrs: nounwind
define void @main(<4 x float>* noalias, <4 x float>, i32) #2 {
entry:
  %3 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef), !dbg !3
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef), !dbg !3
  call void @dx.noop(), !dbg !3
  call void @dx.noop(), !dbg !8
  call void @dx.noop(), !dbg !9
  %cmp.1 = icmp slt i32 0, %3, !dbg !10
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end, !dbg !11

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body, !dbg !11

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.0 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  call void @dx.noop(), !dbg !12
  call void @dx.noop(), !dbg !12
  call void @dx.noop(), !dbg !12
  call void @dx.noop(), !dbg !13
  call void @dx.noop(), !dbg !13
  call void @dx.noop(), !dbg !13
  call void @dx.noop(), !dbg !14
  call void @dx.noop(), !dbg !15
  br label %for.inc, !dbg !16

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !dbg !17
  call void @dx.noop(), !dbg !17
  %cmp = icmp slt i32 %inc, %3, !dbg !10
  %tobool = icmp ne i1 %cmp, false, !dbg !10
  %tobool1 = icmp ne i1 %tobool, false, !dbg !11
  br i1 %tobool1, label %for.body, label %for.cond.for.end_crit_edge, !dbg !11

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  %loopInput.0.i3.lcssa = phi float [ %4, %for.inc ]
  br label %for.end, !dbg !11

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %loopOutput.0.i3 = phi float [ %loopInput.0.i3.lcssa, %for.cond.for.end_crit_edge ], [ %4, %entry ]
  call void @dx.noop(), !dbg !18
  call void @dx.noop(), !dbg !19
  call void @dx.noop(), !dbg !19
  call void @dx.noop(), !dbg !19
  call void @dx.noop(), !dbg !20
  call void @dx.noop(), !dbg !20
  call void @dx.noop(), !dbg !20
  call void @dx.noop(), !dbg !21
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00), !dbg !21
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00), !dbg !21
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00), !dbg !21
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %loopOutput.0.i3), !dbg !21
  ret void, !dbg !21
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32, %dx.types.Handle, i32, i32, i8, i32) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleForLib.class.StructuredBuffer<vector<float, 3> >"(i32, %"class.StructuredBuffer<vector<float, 3> >") #3

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!3 = !DILocation(line: 6, column: 10, scope: !4)
!4 = !DISubprogram(name: "main", scope: !5, file: !5, line: 5, type: !6, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, function: void (<4 x float>*, <4 x float>, i32)* @main)
!5 = !DIFile(filename: "F:\5Ctest\5Cbethesda_od\5Csimple_repro.hlsl", directory: "")
!6 = !DISubroutineType(types: !7)
!7 = !{}
!8 = !DILocation(line: 8, column: 10, scope: !4)
!9 = !DILocation(line: 9, column: 12, scope: !4)
!10 = !DILocation(line: 9, column: 21, scope: !4)
!11 = !DILocation(line: 9, column: 3, scope: !4)
!12 = !DILocation(line: 11, column: 33, scope: !4)
!13 = !DILocation(line: 11, column: 17, scope: !4)
!14 = !DILocation(line: 12, column: 15, scope: !4)
!15 = !DILocation(line: 13, column: 16, scope: !4)
!16 = !DILocation(line: 14, column: 3, scope: !4)
!17 = !DILocation(line: 9, column: 31, scope: !4)
!18 = !DILocation(line: 16, column: 10, scope: !4)
!19 = !DILocation(line: 17, column: 11, scope: !4)
!20 = !DILocation(line: 18, column: 11, scope: !4)
!21 = !DILocation(line: 19, column: 3, scope: !4)