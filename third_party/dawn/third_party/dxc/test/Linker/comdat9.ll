; RUN: llvm-link %s -S -o - | FileCheck %s

$c = comdat any
@a = alias void ()* @f
define internal void @f() comdat($c) {
  ret void
}

; CHECK-DAG: $c = comdat any
; CHECK-DAG: @a = alias void ()* @f
; CHECK-DAG: define internal void @f() comdat($c)

$f2 = comdat largest
define internal void @f2() comdat($f2) {
  ret void
}

; CHECK-DAG: $f2 = comdat largest
; CHECK-DAG: define internal void @f2() comdat {
