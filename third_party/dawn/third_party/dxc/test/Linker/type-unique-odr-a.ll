; REQUIRES: object-emission
;
; RUN: llvm-link %s %p/type-unique-odr-b.ll -S -o - | %llc_dwarf -filetype=obj -O0 | llvm-dwarfdump -debug-dump=info - | FileCheck %s
;
; Test ODR-based type uniquing for C++ class members.
; rdar://problem/15851313.
;
; $ cat -n type-unique-odr-a.cpp
;     1	class A {
;     2	  int data;
;     3	protected:
;     4	  void getFoo();
;     5	};
;     6
;     7	static void bar() {
;     8	  A a;
;     9	}
;    10
;    11	void baz() { bar(); }
;; #include "ab.h"
; foo_t bar() {
;     return A().getFoo();
; }
;
; CHECK:      DW_TAG_class_type
; CHECK-NEXT:   DW_AT_name {{.*}} "A"
; CHECK-NOT:  DW_TAG
; CHECK:      DW_TAG_member
; CHECK-NEXT:   DW_AT_name {{.*}} "data"
; CHECK-NOT:  DW_TAG
; CHECK:      DW_TAG_subprogram
; CHECK-NOT: DW_TAG
; CHECK:   DW_AT_linkage_name {{.*}} "_ZN1A6getFooEv"
; CHECK-NOT: DW_TAG
; CHECK:   DW_AT_name {{.*}} "getFoo"
; CHECK:      DW_TAG_subprogram
; CHECK-NOT: DW_TAG
; CHECK:   DW_AT_linkage_name {{.*}} "_Z3bazv"
; CHECK:      DW_TAG_subprogram
; CHECK-NOT: DW_TAG
; CHECK:   DW_AT_linkage_name {{.*}} "_ZL3barv"

; getFoo and A may only appear once.
; CHECK-NOT:  AT_name{{.*(getFoo)|("A")}}


; ModuleID = 'type-unique-odr-a.cpp'

%class.A = type { i32 }

; Function Attrs: nounwind
define void @_Z3bazv() #0 {
entry:
  call void @_ZL3barv(), !dbg !23
  ret void, !dbg !23
}

; Function Attrs: nounwind
define internal void @_ZL3barv() #0 {
entry:
  %a = alloca %class.A, align 4
  call void @llvm.dbg.declare(metadata %class.A* %a, metadata !24, metadata !DIExpression()), !dbg !25
  ret void, !dbg !26
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!20, !21}
!llvm.ident = !{!22}

!0 = !DICompileUnit(language: DW_LANG_C_plus_plus, producer: "clang version 3.5.0 ", isOptimized: false, emissionKind: 1, file: !1, enums: !2, retainedTypes: !3, subprograms: !14, globals: !2, imports: !2)
!1 = !DIFile(filename: "<unknown>", directory: "")
!2 = !{}
!3 = !{!4}
!4 = !DICompositeType(tag: DW_TAG_class_type, name: "A", line: 1, size: 32, align: 32, file: !5, elements: !6, identifier: "_ZTS1A")
!5 = !DIFile(filename: "type-unique-odr-a.cpp", directory: "")
!6 = !{!7, !9}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "data", line: 2, size: 32, align: 32, flags: DIFlagPrivate, file: !5, scope: !"_ZTS1A", baseType: !8)
!8 = !DIBasicType(tag: DW_TAG_base_type, name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!9 = !DISubprogram(name: "getFoo", linkageName: "_ZN1A6getFooEv", line: 4, isLocal: false, isDefinition: false, virtualIndex: 6, flags: DIFlagProtected | DIFlagPrototyped, isOptimized: false, scopeLine: 4, file: !5, scope: !"_ZTS1A", type: !10)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 64, align: 64, flags: DIFlagArtificial | DIFlagObjectPointer, baseType: !"_ZTS1A")
!14 = !{!15, !19}
!15 = !DISubprogram(name: "baz", linkageName: "_Z3bazv", line: 11, isLocal: false, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 11, file: !5, scope: !16, type: !17, function: void ()* @_Z3bazv, variables: !2)
!16 = !DIFile(filename: "type-unique-odr-a.cpp", directory: "")
!17 = !DISubroutineType(types: !18)
!18 = !{null}
!19 = !DISubprogram(name: "bar", linkageName: "_ZL3barv", line: 7, isLocal: true, isDefinition: true, virtualIndex: 6, flags: DIFlagPrototyped, isOptimized: false, scopeLine: 7, file: !5, scope: !16, type: !17, function: void ()* @_ZL3barv, variables: !2)
!20 = !{i32 2, !"Dwarf Version", i32 4}
!21 = !{i32 1, !"Debug Info Version", i32 3}
!22 = !{!"clang version 3.5.0 "}
!23 = !DILocation(line: 11, scope: !15)
!24 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "a", line: 8, scope: !19, file: !16, type: !"_ZTS1A")
!25 = !DILocation(line: 8, scope: !19)
!26 = !DILocation(line: 9, scope: !19)
