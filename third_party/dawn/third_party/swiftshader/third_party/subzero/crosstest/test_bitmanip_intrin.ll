; Wrappers around the bit manipulation intrinsics, which use name mangling
; for encoding the type in the name instead of plain "C" suffixes.
; E.g., my_ctpop(unsigned long long) vs __builtin_popcountll(...)
; Also, normalize the intrinsic to take a single parameter when there
; can be two, as is the case for ctlz and cttz.

declare i32 @llvm.ctlz.i32(i32, i1)
declare i64 @llvm.ctlz.i64(i64, i1)

declare i32 @llvm.cttz.i32(i32, i1)
declare i64 @llvm.cttz.i64(i64, i1)

declare i32 @llvm.ctpop.i32(i32)
declare i64 @llvm.ctpop.i64(i64)

define i32 @_Z7my_ctlzj(i32 %a) {
  %x = call i32 @llvm.ctlz.i32(i32 %a, i1 0)
  ret i32 %x
}

define i64 @_Z7my_ctlzy(i64 %a) {
  %x = call i64 @llvm.ctlz.i64(i64 %a, i1 0)
  ret i64 %x
}

define i32 @_Z7my_cttzj(i32 %a) {
  %x = call i32 @llvm.cttz.i32(i32 %a, i1 0)
  ret i32 %x
}

define i64 @_Z7my_cttzy(i64 %a) {
  %x = call i64 @llvm.cttz.i64(i64 %a, i1 0)
  ret i64 %x
}

define i32 @_Z8my_ctpopj(i32 %a) {
  %x = call i32 @llvm.ctpop.i32(i32 %a)
  ret i32 %x
}

define i64 @_Z8my_ctpopy(i64 %a) {
  %x = call i64 @llvm.ctpop.i64(i64 %a)
  ret i64 %x
}
