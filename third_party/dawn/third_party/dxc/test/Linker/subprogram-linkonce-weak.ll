; RUN: llvm-link %s %S/Inputs/subprogram-linkonce-weak.ll -S -o %t1
; RUN: FileCheck %s -check-prefix=LW -check-prefix=CHECK <%t1
; RUN: llvm-link %S/Inputs/subprogram-linkonce-weak.ll %s -S -o %t2
; RUN: FileCheck %s -check-prefix=WL -check-prefix=CHECK <%t2

; This testcase tests the following flow:
;  - File A defines a linkonce version of @foo which has inlined into @bar.
;  - File B defines a weak version of @foo (different definition).
;  - Linkage rules state File B version of @foo wins.
;  - @bar still has inlined debug info related to the linkonce @foo.
;
; This should fix PR22792, although the testcase was hand-written.  There's a
; RUN line with a crasher for llc at the end with checks for the DWARF output.

; The LW prefix means linkonce (this file) first, then weak (the other file).
; The WL prefix means weak (the other file) first, then linkonce (this file).

; We'll see @bar before @foo if this file is first.
; LW-LABEL: define i32 @bar(
; LW: %sum = add i32 %a, %b, !dbg ![[FOOINBAR:[0-9]+]]
; LW: ret i32 %sum, !dbg ![[BARRET:[0-9]+]]
; LW-LABEL: define weak i32 @foo(
; LW: %sum = call i32 @fastadd(i32 %a, i32 %b), !dbg ![[FOOCALL:[0-9]+]]
; LW: ret i32 %sum, !dbg ![[FOORET:[0-9]+]]

; We'll see @foo before @bar if this file is second.
; WL-LABEL: define weak i32 @foo(
; WL: %sum = call i32 @fastadd(i32 %a, i32 %b), !dbg ![[FOOCALL:[0-9]+]]
; WL: ret i32 %sum, !dbg ![[FOORET:[0-9]+]]
; WL-LABEL: define i32 @bar(
; WL: %sum = add i32 %a, %b, !dbg ![[FOOINBAR:[0-9]+]]
; WL: ret i32 %sum, !dbg ![[BARRET:[0-9]+]]

define i32 @bar(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b, !dbg !DILocation(line: 2, scope: !4,
                                          inlinedAt: !DILocation(line: 12, scope: !3))
  ret i32 %sum, !dbg !DILocation(line: 13, scope: !3)
}

define linkonce i32 @foo(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b, !dbg !DILocation(line: 2, scope: !4)
  ret i32 %sum, !dbg !DILocation(line: 3, scope: !4)
}

!llvm.module.flags = !{!0}
!0 = !{i32 2, !"Debug Info Version", i32 3}

; CHECK-LABEL: !llvm.dbg.cu =
; LW-SAME: !{![[LCU:[0-9]+]], ![[WCU:[0-9]+]]}
; WL-SAME: !{![[WCU:[0-9]+]], ![[LCU:[0-9]+]]}
!llvm.dbg.cu = !{!1}

; LW: ![[LCU]] = !DICompileUnit({{.*}} subprograms: ![[LSPs:[0-9]+]]
; LW: ![[LSPs]] = !{![[BARSP:[0-9]+]], ![[FOOSP:[0-9]+]]}
; LW: ![[BARSP]] = !DISubprogram(name: "bar",
; LW-SAME: function: i32 (i32, i32)* @bar
; LW: ![[FOOSP]] = {{.*}}!DISubprogram(name: "foo",
; LW-NOT: function:
; LW-SAME: ){{$}}
; LW: ![[WCU]] = !DICompileUnit({{.*}} subprograms: ![[WSPs:[0-9]+]]
; LW: ![[WSPs]] = !{![[WEAKFOOSP:[0-9]+]]}
; LW: ![[WEAKFOOSP]] = !DISubprogram(name: "foo",
; LW-SAME: function: i32 (i32, i32)* @foo
; LW: ![[FOOINBAR]] = !DILocation(line: 2, scope: ![[FOOSP]], inlinedAt: ![[BARIA:[0-9]+]])
; LW: ![[BARIA]] = !DILocation(line: 12, scope: ![[BARSP]])
; LW: ![[BARRET]] = !DILocation(line: 13, scope: ![[BARSP]])
; LW: ![[FOOCALL]] = !DILocation(line: 52, scope: ![[WEAKFOOSP]])
; LW: ![[FOORET]] = !DILocation(line: 53, scope: ![[WEAKFOOSP]])

; Same as above, but reordered.
; WL: ![[WCU]] = !DICompileUnit({{.*}} subprograms: ![[WSPs:[0-9]+]]
; WL: ![[WSPs]] = !{![[WEAKFOOSP:[0-9]+]]}
; WL: ![[WEAKFOOSP]] = !DISubprogram(name: "foo",
; WL-SAME: function: i32 (i32, i32)* @foo
; WL: ![[LCU]] = !DICompileUnit({{.*}} subprograms: ![[LSPs:[0-9]+]]
; WL: ![[LSPs]] = !{![[BARSP:[0-9]+]], ![[FOOSP:[0-9]+]]}
; WL: ![[BARSP]] = !DISubprogram(name: "bar",
; WL-SAME: function: i32 (i32, i32)* @bar
; WL: ![[FOOSP]] = {{.*}}!DISubprogram(name: "foo",
; Note, for symmetry, this should be "NOT: function:" and "SAME: ){{$}}".
; WL-SAME: function: i32 (i32, i32)* @foo
; WL: ![[FOOCALL]] = !DILocation(line: 52, scope: ![[WEAKFOOSP]])
; WL: ![[FOORET]] = !DILocation(line: 53, scope: ![[WEAKFOOSP]])
; WL: ![[FOOINBAR]] = !DILocation(line: 2, scope: ![[FOOSP]], inlinedAt: ![[BARIA:[0-9]+]])
; WL: ![[BARIA]] = !DILocation(line: 12, scope: ![[BARSP]])
; WL: ![[BARRET]] = !DILocation(line: 13, scope: ![[BARSP]])

!1 = !DICompileUnit(language: DW_LANG_C99, file: !2, subprograms: !{!3, !4}, emissionKind: 1)
!2 = !DIFile(filename: "bar.c", directory: "/path/to/dir")
!3 = !DISubprogram(file: !2, scope: !2, line: 11, name: "bar", function: i32 (i32, i32)* @bar, type: !5)
!4 = !DISubprogram(file: !2, scope: !2, line: 1, name: "foo", function: i32 (i32, i32)* @foo, type: !5)
!5 = !DISubroutineType(types: !{})

; Crasher for llc.
; REQUIRES: object-emission
; RUN: %llc_dwarf -filetype=obj -O0 %t1 -o %t1.o
; RUN: llvm-dwarfdump %t1.o -debug-dump=all | FileCheck %s -check-prefix=DWLW -check-prefix=DW
; RUN: %llc_dwarf -filetype=obj -O0 %t2 -o %t2.o
; RUN: llvm-dwarfdump %t2.o -debug-dump=all | FileCheck %s -check-prefix=DWWL -check-prefix=DW
; Check that the debug info for the discarded linkonce version of @foo doesn't
; reference any code, and that the other subprograms look correct.

; DW-LABEL: .debug_info contents:
; DWLW:     DW_TAG_compile_unit
; DWLW:       DW_AT_name {{.*}}"bar.c"
; DWLW:       DW_TAG_subprogram
; DWLW-NOT:     DW_AT_low_pc
; DWLW-NOT:     DW_AT_high_pc
; DWLW:         DW_AT_name {{.*}}foo
; DWLW:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}bar.c"
; DWLW:         DW_AT_decl_line {{.*}}(1)
; DWLW:       DW_TAG_subprogram
; DWLW:         DW_AT_low_pc
; DWLW:         DW_AT_high_pc
; DWLW:         DW_AT_name {{.*}}bar
; DWLW:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}bar.c"
; DWLW:         DW_AT_decl_line {{.*}}(11)

; DWLW:         DW_TAG_inlined_subroutine
; DWLW:           DW_AT_abstract_origin
; DWLW:     DW_TAG_compile_unit
; DWLW:       DW_AT_name {{.*}}"foo.c"
; DWLW:       DW_TAG_subprogram
; DWLW:         DW_AT_low_pc
; DWLW:         DW_AT_high_pc
; DWLW:         DW_AT_name {{.*}}foo
; DWLW:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}foo.c"
; DWLW:         DW_AT_decl_line {{.*}}(51)

; The DWARF output is already symmetric (just reordered).
; DWWL:     DW_TAG_compile_unit
; DWWL:       DW_AT_name {{.*}}"foo.c"
; DWWL:       DW_TAG_subprogram
; DWWL:         DW_AT_low_pc
; DWWL:         DW_AT_high_pc
; DWWL:         DW_AT_name {{.*}}foo
; DWWL:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}foo.c"
; DWWL:         DW_AT_decl_line {{.*}}(51)
; DWWL:     DW_TAG_compile_unit
; DWWL:       DW_AT_name {{.*}}"bar.c"
; DWWL:       DW_TAG_subprogram
; DWWL-NOT:     DW_AT_low_pc
; DWWL-NOT:     DW_AT_high_pc
; DWWL:         DW_AT_name {{.*}}foo
; DWWL:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}bar.c"
; DWWL:         DW_AT_decl_line {{.*}}(1)
; DWWL:       DW_TAG_subprogram
; DWWL:         DW_AT_low_pc
; DWWL:         DW_AT_high_pc
; DWWL:         DW_AT_name {{.*}}bar
; DWWL:         DW_AT_decl_file {{.*}}"/path/to/dir{{/|\\}}bar.c"
; DWWL:         DW_AT_decl_line {{.*}}(11)
; DWWL:         DW_TAG_inlined_subroutine
; DWWL:           DW_AT_abstract_origin

; DW-LABEL:   .debug_line contents:
; Check that we have the right things in the line table as well.

; DWLW-LABEL: file_names[{{ *}}1]{{.*}} bar.c
; DWLW:        2 0 1 0 0 is_stmt prologue_end
; DWLW-LABEL: file_names[{{ *}}1]{{.*}} foo.c
; DWLW:       52 0 1 0 0 is_stmt prologue_end
; DWLW-NOT:                      prologue_end

; DWWL-LABEL: file_names[{{ *}}1]{{.*}} foo.c
; DWWL:       52 0 1 0 0 is_stmt prologue_end
; DWWL-LABEL: file_names[{{ *}}1]{{.*}} bar.c
; DWWL:        2 0 1 0 0 is_stmt prologue_end
; DWWL-NOT:                      prologue_end
