; RUN: opt < %s -inline -S | grep call | count 1

define i32 @fn2() noinline {
  ret i32 1
}

define i32 @fn3() {
   %r = call i32 @fn2()
   ret i32 %r
}
