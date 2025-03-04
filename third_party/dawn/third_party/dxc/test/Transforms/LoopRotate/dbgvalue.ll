; RUN: opt -S -loop-rotate < %s | FileCheck %s

declare void @llvm.dbg.declare(metadata, metadata, metadata) nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) nounwind readnone

define i32 @tak(i32 %x, i32 %y, i32 %z) nounwind ssp {
; CHECK-LABEL: define i32 @tak(
; CHECK: entry
; CHECK-NEXT: call void @llvm.dbg.value(metadata i32 %x

entry:
  br label %tailrecurse

tailrecurse:                                      ; preds = %if.then, %entry
  %x.tr = phi i32 [ %x, %entry ], [ %call, %if.then ]
  %y.tr = phi i32 [ %y, %entry ], [ %call9, %if.then ]
  %z.tr = phi i32 [ %z, %entry ], [ %call14, %if.then ]
  tail call void @llvm.dbg.value(metadata i32 %x.tr, i64 0, metadata !6, metadata !DIExpression()), !dbg !7
  tail call void @llvm.dbg.value(metadata i32 %y.tr, i64 0, metadata !8, metadata !DIExpression()), !dbg !9
  tail call void @llvm.dbg.value(metadata i32 %z.tr, i64 0, metadata !10, metadata !DIExpression()), !dbg !11
  %cmp = icmp slt i32 %y.tr, %x.tr, !dbg !12
  br i1 %cmp, label %if.then, label %if.end, !dbg !12

if.then:                                          ; preds = %tailrecurse
  %sub = sub nsw i32 %x.tr, 1, !dbg !14
  %call = tail call i32 @tak(i32 %sub, i32 %y.tr, i32 %z.tr), !dbg !14
  %sub6 = sub nsw i32 %y.tr, 1, !dbg !14
  %call9 = tail call i32 @tak(i32 %sub6, i32 %z.tr, i32 %x.tr), !dbg !14
  %sub11 = sub nsw i32 %z.tr, 1, !dbg !14
  %call14 = tail call i32 @tak(i32 %sub11, i32 %x.tr, i32 %y.tr), !dbg !14
  br label %tailrecurse

if.end:                                           ; preds = %tailrecurse
  br label %return, !dbg !16

return:                                           ; preds = %if.end
  ret i32 %z.tr, !dbg !17
}

@channelColumns = external global i64
@horzPlane = external global i8*, align 8

define void @FindFreeHorzSeg(i64 %startCol, i64 %row, i64* %rowStart) {
; Ensure that the loop increment basic block is rotated into the tail of the
; body, even though it contains a debug intrinsic call.
; CHECK-LABEL: define void @FindFreeHorzSeg(
; CHECK: %dec = add
; CHECK-NEXT: tail call void @llvm.dbg.value
; CHECK: %cmp = icmp
; CHECK: br i1 %cmp
; CHECK: phi i64 [ %{{[^,]*}}, %{{[^,]*}} ]
; CHECK-NEXT: br label %for.end


entry:
  br label %for.cond

for.cond:
  %i.0 = phi i64 [ %startCol, %entry ], [ %dec, %for.inc ]
  %cmp = icmp eq i64 %i.0, 0
  br i1 %cmp, label %for.end, label %for.body

for.body:
  %0 = load i64, i64* @channelColumns, align 8
  %mul = mul i64 %0, %row
  %add = add i64 %mul, %i.0
  %1 = load i8*, i8** @horzPlane, align 8
  %arrayidx = getelementptr inbounds i8, i8* %1, i64 %add
  %2 = load i8, i8* %arrayidx, align 1
  %tobool = icmp eq i8 %2, 0
  br i1 %tobool, label %for.inc, label %for.end

for.inc:
  %dec = add i64 %i.0, -1
  tail call void @llvm.dbg.value(metadata i64 %dec, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !0), metadata !DIExpression()), !dbg !DILocation(scope: !0)
  br label %for.cond

for.end:
  %add1 = add i64 %i.0, 1
  store i64 %add1, i64* %rowStart, align 8
  ret void
}

!llvm.module.flags = !{!20}
!llvm.dbg.sp = !{!0}

!0 = !DISubprogram(name: "tak", line: 32, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, file: !18, scope: !1, type: !3, function: i32 (i32, i32, i32)* @tak)
!1 = !DIFile(filename: "/Volumes/Lalgate/cj/llvm/projects/llvm-test/SingleSource/Benchmarks/BenchmarkGame/recursive.c", directory: "/Volumes/Lalgate/cj/D/projects/llvm-test/SingleSource/Benchmarks/BenchmarkGame")
!2 = !DICompileUnit(language: DW_LANG_C99, producer: "clang version 2.9 (trunk 125492)", isOptimized: true, emissionKind: 0, file: !18, enums: !19, retainedTypes: !19)
!3 = !DISubroutineType(types: !4)
!4 = !{!5}
!5 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!6 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "x", line: 32, arg: 0, scope: !0, file: !1, type: !5)
!7 = !DILocation(line: 32, column: 13, scope: !0)
!8 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "y", line: 32, arg: 0, scope: !0, file: !1, type: !5)
!9 = !DILocation(line: 32, column: 20, scope: !0)
!10 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "z", line: 32, arg: 0, scope: !0, file: !1, type: !5)
!11 = !DILocation(line: 32, column: 27, scope: !0)
!12 = !DILocation(line: 33, column: 3, scope: !13)
!13 = distinct !DILexicalBlock(line: 32, column: 30, file: !18, scope: !0)
!14 = !DILocation(line: 34, column: 5, scope: !15)
!15 = distinct !DILexicalBlock(line: 33, column: 14, file: !18, scope: !13)
!16 = !DILocation(line: 36, column: 3, scope: !13)
!17 = !DILocation(line: 37, column: 1, scope: !13)
!18 = !DIFile(filename: "/Volumes/Lalgate/cj/llvm/projects/llvm-test/SingleSource/Benchmarks/BenchmarkGame/recursive.c", directory: "/Volumes/Lalgate/cj/D/projects/llvm-test/SingleSource/Benchmarks/BenchmarkGame")
!19 = !{i32 0}
!20 = !{i32 1, !"Debug Info Version", i32 3}
