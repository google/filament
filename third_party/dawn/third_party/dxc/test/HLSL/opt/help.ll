; RUN: opt -help | FileCheck %s

; Make sure the help message is printed.
; CHECK: -O1
; CHECK-SAME: - Optimization level 1. Similar to clang -O1
