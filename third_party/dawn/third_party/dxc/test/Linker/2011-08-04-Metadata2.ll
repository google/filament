; This file is used by 2011-08-04-Metadata.ll, so it doesn't actually do anything itself
;
; RUN: true


target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-apple-macosx10.7.0"

@x = internal global i32 0, align 4

define void @bar() nounwind uwtable ssp {
entry:
  store i32 1, i32* @x, align 4, !dbg !7
  ret void, !dbg !7
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!11}
!llvm.dbg.sp = !{!1}
!llvm.dbg.gv = !{!5}

!0 = !DICompileUnit(language: DW_LANG_C99, producer: "clang version 3.0 ()", isOptimized: true, emissionKind: 0, file: !9, enums: !{}, retainedTypes: !{}, subprograms: !10)
!1 = !DISubprogram(name: "bar", line: 2, isLocal: false, isDefinition: true, virtualIndex: 6, isOptimized: false, file: !9, scope: !2, type: !3, function: void ()* @bar)
!2 = !DIFile(filename: "/tmp/two.c", directory: "/Volumes/Lalgate/Slate/D")
!3 = !DISubroutineType(types: !4)
!4 = !{null}
!5 = !DIGlobalVariable(name: "x", line: 1, isLocal: true, isDefinition: true, scope: !0, file: !2, type: !6, variable: i32* @x)
!6 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!7 = !DILocation(line: 2, column: 14, scope: !8)
!8 = distinct !DILexicalBlock(line: 2, column: 12, file: !9, scope: !1)
!9 = !DIFile(filename: "/tmp/two.c", directory: "/Volumes/Lalgate/Slate/D")
!10 = !{!1}
!11 = !{i32 1, !"Debug Info Version", i32 3}
