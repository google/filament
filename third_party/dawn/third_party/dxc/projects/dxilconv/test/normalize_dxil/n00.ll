; RUN: %opt-exe %s -normalizedxil -S | %FileCheck %s

; CHECK:  @main
; CHECK-NOT: %.reg2mem = alloca %dx.types.Handle
; CHECK: %0 = call %dx.types.Handle @dx.op.createHandle(i32 {{[0-9]+}}, i8 0, i32 0, i32 0, i1 false)
; CHECK: %1 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 {{[0-9]+}}, %dx.types.Handle %0, i32 %x, i32 undef)
; CHECK: %2 = extractvalue %dx.types.ResRet.i32 %1, 0

%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }

declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1)
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32)

define i32 @main(i32 %x) {
entry:
  %.reg2mem = alloca %dx.types.Handle
  %"reg2mem alloca point" = bitcast i32 0 to i32
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
  store %dx.types.Handle %0, %dx.types.Handle* %.reg2mem
  %.reload.1 = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
  %1 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload.1, i32 %x, i32 undef)
  %2 = extractvalue %dx.types.ResRet.i32 %1, 0
  ret i32 %2
}
