; RUN: opt < %s -S -inline -inline-threshold=2 | FileCheck %s
; RUN: opt < %s -S -strip-debug -inline -inline-threshold=2 | FileCheck %s
;
; The purpose of this test is to check that debug info doesn't influence
; inlining decisions.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

define <4 x float> @inner_vectors(<4 x float> %a, <4 x float> %b) {
entry:
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  %mul = fmul <4 x float> %a, <float 3.000000e+00, float 3.000000e+00, float 3.000000e+00, float 3.000000e+00>
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  %mul1 = fmul <4 x float> %b, <float 5.000000e+00, float 5.000000e+00, float 5.000000e+00, float 5.000000e+00>
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  %add = fadd <4 x float> %mul, %mul1
  ret <4 x float> %add
}

define float @outer_vectors(<4 x float> %a, <4 x float> %b) {
; CHECK-LABEL: @outer_vectors(
; CHECK-NOT: call <4 x float> @inner_vectors(
; CHECK: ret float

entry:
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  %call = call <4 x float> @inner_vectors(<4 x float> %a, <4 x float> %b)
  call void @llvm.dbg.value(metadata i32 undef, i64 0, metadata !DILocalVariable(tag: DW_TAG_auto_variable, scope: !6), metadata !DIExpression()), !dbg !DILocation(scope: !6)
  %vecext = extractelement <4 x float> %call, i32 0
  %vecext1 = extractelement <4 x float> %call, i32 1
  %add = fadd float %vecext, %vecext1
  %vecext2 = extractelement <4 x float> %call, i32 2
  %add3 = fadd float %add, %vecext2
  %vecext4 = extractelement <4 x float> %call, i32 3
  %add5 = fadd float %add3, %vecext4
  ret float %add5
}

attributes #0 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = !DICompileUnit(language: DW_LANG_C_plus_plus, isOptimized: false, emissionKind: 0, file: !1, enums: !2, retainedTypes: !2, subprograms: !{!6}, globals: !2, imports: !2)
!1 = !DIFile(filename: "test.c", directory: "")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 1, !"Debug Info Version", i32 3}
!5 = !{!""}
!6 = !DISubprogram()
