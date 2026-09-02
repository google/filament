; RUN: %opt %s -dxil-cond-mem2reg -S | FileCheck %s

; Make sure that dxilutil::DeleteDeadAllocas handles memcpy's

; CHECK: @main
; CHECK-NOT: = alloca %struct.Data

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.Data = type { [5 x float] }
%ConstantBuffer = type opaque
%cb = type { %struct.Data, %struct.Data }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?foo@cb@@3UData@@B" = external global %struct.Data, align 4
@"\01?bar@cb@@3UData@@B" = external global %struct.Data, align 4
@"$Globals" = external constant %ConstantBuffer
@cb = external constant %cb

; Function Attrs: nounwind
define float @main() #0 {
entry:
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32 0, %cb* @cb, i32 0)
  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32 11, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 13, i32 148 }, %cb undef)
  %2 = call %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %1, i32 0)
  %3 = getelementptr inbounds %cb, %cb* %2, i32 0, i32 0
  %4 = getelementptr inbounds %cb, %cb* %2, i32 0, i32 1, i32 0, i32 0
  %retval = alloca float, align 4, !dx.temp !3
  %alloca = alloca %struct.Data, align 4
  %i = alloca i32, align 4
  %5 = bitcast %struct.Data* %alloca to i8*, !dbg !4
  %6 = bitcast %struct.Data* %3 to i8*, !dbg !4
  call void @dx.noop(), !dbg !4
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %5, i8* %6, i64 20, i32 1, i1 false), !dbg !4
  call void @dx.noop(), !dbg !8
  store i32 0, i32* %i, align 4, !dbg !8
  br label %for.cond, !dbg !9

for.cond:                                         ; preds = %for.inc, %entry
  %7 = load i32, i32* %i, align 4, !dbg !10
  %cmp = icmp slt i32 %7, 5, !dbg !11
  %tobool = icmp ne i1 %cmp, false, !dbg !11
  %tobool1 = icmp ne i1 %tobool, false, !dbg !12
  br i1 %tobool1, label %for.body, label %for.end, !dbg !12

for.body:                                         ; preds = %for.cond
  %8 = load i32, i32* %i, align 4, !dbg !13
  %arrayidx3 = getelementptr inbounds %cb, %cb* %2, i32 0, i32 1, i32 0, i32 %8, !dbg !14
  %9 = load float, float* %arrayidx3, align 4, !dbg !14
  %10 = load i32, i32* %i, align 4, !dbg !15
  %member = getelementptr inbounds %struct.Data, %struct.Data* %alloca, i32 0, i32 0, !dbg !16
  %arrayidx2 = getelementptr inbounds [5 x float], [5 x float]* %member, i32 0, i32 %10, !dbg !17
  call void @dx.noop(), !dbg !18
  store float %9, float* %arrayidx2, align 4, !dbg !18
  br label %for.inc, !dbg !19

for.inc:                                          ; preds = %for.body
  %11 = load i32, i32* %i, align 4, !dbg !20
  %inc = add nsw i32 %11, 1, !dbg !20
  call void @dx.noop(), !dbg !20
  store i32 %inc, i32* %i, align 4, !dbg !20
  br label %for.cond, !dbg !12, !llvm.loop !21

for.end:                                          ; preds = %for.cond
  %12 = load float, float* %4, align 4, !dbg !23
  store float %12, float* %retval, !dbg !24
  %13 = load float, float* %retval, !dbg !24
  call void @dx.noop(), !dbg !24
  ret float %13, !dbg !24
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind readnone
declare %cb* @"dx.hl.subscript.cb.%cb* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cb*, i32)"(i32, %cb*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cb) #1

; Function Attrs: convergent
declare void @dx.noop() #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { convergent }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!3 = !{}
!4 = !DILocation(line: 12, column: 17, scope: !5)
!5 = !DISubprogram(name: "main", scope: !6, file: !6, line: 11, type: !7, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, function: float ()* @main)
!6 = !DIFile(filename: "F:\5Ctest\5Cod_fails_may_2022\5Cmemcpy.hlsl", directory: "")
!7 = !DISubroutineType(types: !3)
!8 = !DILocation(line: 14, column: 12, scope: !5)
!9 = !DILocation(line: 14, column: 8, scope: !5)
!10 = !DILocation(line: 14, column: 19, scope: !5)
!11 = !DILocation(line: 14, column: 21, scope: !5)
!12 = !DILocation(line: 14, column: 3, scope: !5)
!13 = !DILocation(line: 15, column: 35, scope: !5)
!14 = !DILocation(line: 15, column: 24, scope: !5)
!15 = !DILocation(line: 15, column: 19, scope: !5)
!16 = !DILocation(line: 15, column: 12, scope: !5)
!17 = !DILocation(line: 15, column: 5, scope: !5)
!18 = !DILocation(line: 15, column: 22, scope: !5)
!19 = !DILocation(line: 16, column: 3, scope: !5)
!20 = !DILocation(line: 14, column: 27, scope: !5)
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.unroll.disable"}
!23 = !DILocation(line: 18, column: 10, scope: !5)
!24 = !DILocation(line: 18, column: 3, scope: !5)