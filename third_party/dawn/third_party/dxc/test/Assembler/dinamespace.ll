; RUN: llvm-as < %s | llvm-dis | llvm-as | llvm-dis | FileCheck %s
; RUN: verify-uselistorder %s

; CHECK: !named = !{!0, !1, !2, !3, !4, !4}
!named = !{!0, !1, !2, !3, !4, !5}

!0 = !DIFile(filename: "file.cpp", directory: "/path/to/dir")
!1 = distinct !{}
!2 = !DIFile(filename: "path/to/file", directory: "/path/to/dir")

; CHECK: !3 = !DINamespace(name: "Namespace", scope: !0, file: !2, line: 7)
!3 = !DINamespace(name: "Namespace", scope: !0, file: !2, line: 7)

; CHECK: !4 = !DINamespace(scope: !0)
!4 = !DINamespace(name: "", scope: !0, file: null, line: 0)
!5 = !DINamespace(scope: !0)
