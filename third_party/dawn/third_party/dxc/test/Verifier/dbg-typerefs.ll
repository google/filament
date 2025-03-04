; RUN: not llvm-as -disable-output <%s 2>&1 | FileCheck %s
; Check that the debug info verifier gives nice errors for bad type refs
; (rather than crashing).
!llvm.module.flags = !{!0}
!0 = !{i32 2, !"Debug Info Version", i32 3}

; Make a bunch of type references.  Note that !4 references !"0.bad" (instead
; of !"4.bad") to test error ordering.
!typerefs = !{!1, !2, !3, !4}
!1 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, baseType: !"1.good")
!2 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, baseType: !"2.bad")
!3 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, baseType: !"3.good")
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, size: 32, align: 32, baseType: !"0.bad")

; Add a minimal compile unit to resolve some of the type references.
!llvm.dbg.cu = !{!5}
!5 = !DICompileUnit(file: !6, language: DW_LANG_C99, retainedTypes: !7)
!6 = !DIFile(filename: "file.c", directory: "/path/to/dir")
!7 = !{!8, !9}
!8 = !DICompositeType(tag: DW_TAG_structure_type, identifier: "1.good")
!9 = !DICompositeType(tag: DW_TAG_structure_type, identifier: "3.good")

; CHECK:      assembly parsed, but does not verify
; CHECK-NEXT: unresolved type ref
; CHECK-NEXT: !"0.bad"
; CHECK-NEXT: !DIDerivedType(tag: DW_TAG_pointer_type
; CHECK-SAME:                baseType: !"0.bad"
; CHECK-NEXT: unresolved type ref
; CHECK-NEXT: !"2.bad"
; CHECK-NEXT: !DIDerivedType(tag: DW_TAG_pointer_type
; CHECK-SAME:                baseType: !"2.bad"
; CHECK-NOT:  unresolved
