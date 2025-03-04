; RUN: %opt-exe %s -normalizedxil -S | %FileCheck %s
; Should not remove the alloca because it is written to more than once
; CHECK:   %.reg2mem = alloca %dx.types.Handle
; CHECK:   %.reload1 = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
; CHECK:   %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload1, i32 %5, i32 undef)
; CHECK:   %.reload = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
; CHECK:   %14 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload, i32 %13, i32 undef)
  
%dx.types.u32 = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }

define void @main() {
entry:
  %.reg2mem = alloca %dx.types.Handle
  %"reg2mem alloca point" = bitcast i32 0 to i32
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
  %x = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 1, i1 false)
  store %dx.types.Handle %0, %dx.types.Handle* %.reg2mem
  %1 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  %2 = icmp slt i32 100, %1
  call void @dx.op.tempRegStore.i1(i32 1, i32 0, i1 %2)
  %3 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %4 = icmp ne i32 %3, 0
  br i1 %4, label %if0.then, label %if0.else

if0.then:                                         ; preds = %entry
  %5 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  %.reload1 = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
  %6 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload1, i32 %5, i32 undef)
  %7 = extractvalue %dx.types.ResRet.i32 %6, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %7)
  %8 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %9 = uitofp i32 %8 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 0, float %9)
  %10 = call float @dx.op.tempRegLoad.f32(i32 0, i32 0)
  %11 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %12 = fadd float %10, %11
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %12)
  store %dx.types.Handle %x, %dx.types.Handle* %.reg2mem
  br label %if0.end

if0.else:                                         ; preds = %entry
  %13 = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 0, i32 undef)
  %.reload = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
  %14 = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload, i32 %13, i32 undef)
  %15 = extractvalue %dx.types.ResRet.i32 %14, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %15)
  %16 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %17 = uitofp i32 %16 to float
  call void @dx.op.tempRegStore.f32(i32 1, i32 0, float %17)
  %18 = call float @dx.op.tempRegLoad.f32(i32 0, i32 0)
  %19 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %20 = fadd float %18, %19
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %20)
  br label %if0.end

if0.end:                                          ; preds = %if0.else, %if0.then
  %.reload2 = load %dx.types.Handle, %dx.types.Handle* %.reg2mem
  %r = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 69, %dx.types.Handle %.reload2, i32 3, i32 undef)
  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i1(i32, i32, i1) #1

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.f32(i32, i32, float) #1

; Function Attrs: nounwind readonly
declare float @dx.op.tempRegLoad.f32(i32, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }
