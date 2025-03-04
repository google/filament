; RUN: %opt-exe %s -normalizedxil -S | %FileCheck %s

; CHECK:  @main
; CHECK-NOT: %.reg2mem = alloca %dx.types.Handle
; CHECK:     %0 = call %dx.types.Handle @dx.op.createHandle(i32 {{[0-9]+}}, i8 0, i32 0, i32 0, i1 false)
; CHECK-NOT: store %dx.types.Handle %0, %dx.types.Handle* %.reg2mem
; CHECK-NOT: %.reload = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
; CHECK:     %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 {{[0-9]+}}, %dx.types.Handle %0, i32 %7, i32 undef)

%dx.types.u32 = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }

define void @main() {
entry:
  %.reg2mem = alloca %dx.types.Handle
  %"reg2mem alloca point" = bitcast i32 0 to i32
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
  store %dx.types.Handle %0, %dx.types.Handle* %.reg2mem
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)
  call void @dx.op.tempRegStore.f32(i32 1, i32 0, float %1)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 0)
  br label %loop0

loop0:                                            ; preds = %loop0.breakc0, %entry
  %2 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %3 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  %4 = icmp sge i32 %2, %3
  call void @dx.op.tempRegStore.i1(i32 1, i32 2, i1 %4)
  %5 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %loop0.end, label %loop0.breakc0

loop0.breakc0:                                    ; preds = %loop0
  %7 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %.reload = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
  %8 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload, i32 %7, i32 undef)
  %9 = extractvalue %dx.types.ResRet.i32 %8, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %9)
  %10 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %11 = uitofp i32 %10 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 2, float %11)
  %12 = call float @dx.op.tempRegLoad.f32(i32 0, i32 2)
  %13 = call float @dx.op.tempRegLoad.f32(i32 0, i32 0)
  %14 = fadd float %12, %13
  call void @dx.op.tempRegStore.f32(i32 1, i32 0, float %14)
  %15 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %16 = add i32 %15, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %16)
  br label %loop0

loop0.end:                                        ; preds = %loop0
  %17 = call float @dx.op.tempRegLoad.f32(i32 0, i32 0)
  %18 = call float @dx.op.tertiary.f32(i32 47, float %17, float 3.000000e+00, float 2.000000e+00)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %18)
  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.f32(i32, i32, float) #1

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #1

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #2

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i1(i32, i32, i1) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #2

; Function Attrs: nounwind readonly
declare float @dx.op.tempRegLoad.f32(i32, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.tertiary.f32(i32, float, float, float) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }