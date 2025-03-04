; RUN: opt < %s -instcombine -inline -S | FileCheck %s
; PR3142

; CHECK-NOT: -715827882

define i32 @a(i32 %X) nounwind readnone {
entry:
       %0 = sub i32 0, %X
       %1 = sdiv i32 %0, -3
       ret i32 %1
}

define i32 @b(i32 %X) nounwind readnone {
entry:
       %0 = call i32 @a(i32 -2147483648)
       ret i32 %0
}

define i32 @c(i32 %X) nounwind readnone {
entry:
       %0 = sub i32 0, -2147483648
       %1 = sdiv i32 %0, -3
       ret i32 %1
}
