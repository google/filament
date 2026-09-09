; RUN: %opt %s -dxil-cond-mem2reg -S | FileCheck %s

; Regression test for a crash in dxilutil::DeleteDeadAllocas

; CHECK: @main
; CHECK-NOT: = alloca

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%cb = type { [5 x float] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@cb = external constant %cb

; Function Attrs: nounwind readnone
declare %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32, %cb*, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb) #0

; Function Attrs: convergent
declare void @dx.noop() #1

; Function Attrs: nounwind
define void @main(float* noalias) #2 {
entry:
  %retval = alloca float, align 4, !dx.temp !3
  %alloca = alloca [5 x float], align 4

  ; Test is here. There was a crash when a user is immediately after the alloca
  ; when calling dxilutil::DeleteDeadAllocas, because we deleted instructions
  ; in the forward order.
  %my_index = getelementptr inbounds [5 x float], [5 x float]* %alloca, i32 0, i32 0
  store float %7, float* %my_index, align 4

  %i = alloca i32, align 4
  %1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32 0, %cb* @cb, i32 0)
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32 11, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 13, i32 68 }, %cb undef)
  %3 = call %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %2, i32 0)
  %4 = getelementptr inbounds %cb, %cb* %3, i32 0, i32 0, i32 0
  call void @dx.noop(), !dbg !4
  store i32 0, i32* %i, align 4, !dbg !4
  %5 = load i32, i32* %i, align 4, !dbg !8
  %cmp.1 = icmp slt i32 %5, 5, !dbg !9
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end, !dbg !10

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body, !dbg !10

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %6 = load i32, i32* %i, align 4, !dbg !11
  %arrayidx3 = getelementptr inbounds %cb, %cb* %3, i32 0, i32 0, i32 %6, !dbg !12
  %7 = load float, float* %arrayidx3, align 4, !dbg !12
  %8 = load i32, i32* %i, align 4, !dbg !13
  %arrayidx2 = getelementptr inbounds [5 x float], [5 x float]* %alloca, i32 0, i32 %8, !dbg !14
  call void @dx.noop(), !dbg !15
  store float %7, float* %arrayidx2, align 4, !dbg !15
  br label %for.inc, !dbg !16

for.inc:                                          ; preds = %for.body
  %9 = load i32, i32* %i, align 4, !dbg !17
  %inc = add nsw i32 %9, 1, !dbg !17
  call void @dx.noop(), !dbg !17
  store i32 %inc, i32* %i, align 4, !dbg !17
  %10 = load i32, i32* %i, align 4, !dbg !8
  %cmp = icmp slt i32 %10, 5, !dbg !9
  %tobool = icmp ne i1 %cmp, false, !dbg !9
  %tobool1 = icmp ne i1 %tobool, false, !dbg !10
  br i1 %tobool1, label %for.body, label %for.cond.for.end_crit_edge, !dbg !10, !llvm.loop !18

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end, !dbg !10

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %11 = load float, float* %4, align 4, !dbg !20
  store float %11, float* %retval, !dbg !21
  %12 = load float, float* %retval, !dbg !21
  call void @dx.noop(), !dbg !21
  store float %12, float* %0, !dbg !21
  ret void, !dbg !21
}

attributes #0 = { nounwind readnone }
attributes #1 = { convergent }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!3 = !{}
!4 = !DILocation(line: 9, column: 12, scope: !5)
!5 = !DISubprogram(name: "main", scope: !6, file: !6, line: 6, type: !7, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, function: void (float*)* @main)
!6 = !DIFile(filename: "F:\5Ctest\5Cod_fails_may_2022\5Csimple.hlsl", directory: "")
!7 = !DISubroutineType(types: !3)
!8 = !DILocation(line: 9, column: 19, scope: !5)
!9 = !DILocation(line: 9, column: 21, scope: !5)
!10 = !DILocation(line: 9, column: 3, scope: !5)
!11 = !DILocation(line: 10, column: 21, scope: !5)
!12 = !DILocation(line: 10, column: 17, scope: !5)
!13 = !DILocation(line: 10, column: 12, scope: !5)
!14 = !DILocation(line: 10, column: 5, scope: !5)
!15 = !DILocation(line: 10, column: 15, scope: !5)
!16 = !DILocation(line: 11, column: 3, scope: !5)
!17 = !DILocation(line: 9, column: 27, scope: !5)
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.unroll.disable"}
!20 = !DILocation(line: 13, column: 10, scope: !5)
!21 = !DILocation(line: 13, column: 3, scope: !5)