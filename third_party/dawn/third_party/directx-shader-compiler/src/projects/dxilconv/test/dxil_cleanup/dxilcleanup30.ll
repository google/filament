; RUN: %opt-exe %s -S -dxil-cleanup -verify -o %t.ll.converted
; RUN: fc %t.ll.converted %b.ref


%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.i8x224 = type { [224 x i8] }

define void @main() {
entry:
  %dx.v32.x0 = alloca [24 x float], align 4
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %1)
  %2 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %2)
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 0
  %5 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 0
  %6 = call float @dx.op.bitcastI32toF32(i32 126, i32 %4)
  store float %6, float* %5, align 4
  %7 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %8 = add i32 %7, 6
  %9 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %8)
  %10 = extractvalue %dx.types.CBufRet.i32 %9, 0
  %11 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 4
  %12 = call float @dx.op.bitcastI32toF32(i32 126, i32 %10)
  store float %12, float* %11, align 4
  %13 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %14 = add i32 %13, 2
  %15 = add i32 %13, 3
  %16 = add i32 %13, 4
  %17 = add i32 %13, 5
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %14)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %15)
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %16)
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %17)
  %18 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %19 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %18)
  %20 = extractvalue %dx.types.CBufRet.i32 %19, 0
  %21 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 8
  %22 = call float @dx.op.bitcastI32toF32(i32 126, i32 %20)
  store float %22, float* %21, align 4
  %23 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %24 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %23)
  %25 = extractvalue %dx.types.CBufRet.i32 %24, 0
  %26 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 12
  %27 = call float @dx.op.bitcastI32toF32(i32 126, i32 %25)
  store float %27, float* %26, align 4
  %28 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %29 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %28)
  %30 = extractvalue %dx.types.CBufRet.i32 %29, 0
  %31 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 16
  %32 = call float @dx.op.bitcastI32toF32(i32 126, i32 %30)
  store float %32, float* %31, align 4
  %33 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %34 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 %33)
  %35 = extractvalue %dx.types.CBufRet.i32 %34, 0
  %36 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 20
  %37 = call float @dx.op.bitcastI32toF32(i32 126, i32 %35)
  store float %37, float* %36, align 4
  %38 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %39 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  %40 = add i32 %38, %39
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %40)
  %41 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %42 = mul i32 %41, 4
  %43 = add i32 %42, 0
  %44 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0, i32 %43
  %45 = load float, float* %44, align 4
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %45)
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #2

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.bitcastI32toF32(i32, i32) #1

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.entryPoints = !{!5}
!llvm.ident = !{!14}

!0 = !{i32 1, i32 0}
!1 = !{!"ps", i32 6, i32 0}
!2 = !{null, null, !3, null}
!3 = !{!4}
!4 = !{i32 0, %dx.types.i8x224 addrspace(2)* undef, !"CB0", i32 0, i32 0, i32 1, i32 224, null}
!5 = !{void ()* @main, !"main", !6, !2, null}
!6 = !{!7, !12, null}
!7 = !{!8, !10, !11}
!8 = !{i32 0, !"A", i8 9, i8 0, !9, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!9 = !{i32 0}
!10 = !{i32 1, !"B", i8 4, i8 0, !9, i8 1, i32 1, i8 1, i32 1, i8 0, null}
!11 = !{i32 2, !"C", i8 4, i8 0, !9, i8 1, i32 1, i8 1, i32 1, i8 1, null}
!12 = !{!13}
!13 = !{i32 0, !"SV_Target", i8 9, i8 16, !9, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!14 = !{!"dxbc2dxil 1.0"}
