; RUN: %opt-exe %s -S -dxil-cleanup -verify -o %t.ll.converted
; RUN: fc %t.ll.converted %b.ref


%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%dx.types.splitdouble = type { i32, i32 }
%dx.types.CBufRet.f64 = type { double, double }
%dx.types.i8x80 = type { [80 x i8] }

define void @main() {
entry:
  %dx.v32.x0 = alloca [24 x float], align 4
  %dx.v32.x1 = alloca [16 x float], align 4
  %0 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)
  %1 = bitcast [24 x float]* %dx.v32.x0 to double*
  %2 = getelementptr double, double* %1, i32 0
  store double 8.000000e+00, double* %2, align 8
  %3 = bitcast [24 x float]* %dx.v32.x0 to double*
  %4 = getelementptr double, double* %3, i32 4
  store double 1.000000e+00, double* %4, align 8
  %5 = bitcast [24 x float]* %dx.v32.x0 to double*
  %6 = getelementptr double, double* %5, i32 8
  store double 2.000000e+00, double* %6, align 8
  %7 = bitcast [24 x float]* %dx.v32.x0 to double*
  %8 = getelementptr double, double* %7, i32 12
  store double 0x400C7AE140000000, double* %8, align 8
  %9 = bitcast [24 x float]* %dx.v32.x0 to double*
  %10 = getelementptr double, double* %9, i32 16
  store double 7.000000e+00, double* %10, align 8
  %11 = bitcast [24 x float]* %dx.v32.x0 to double*
  %12 = getelementptr double, double* %11, i32 20
  store double 3.300000e+01, double* %12, align 8
  %13 = bitcast [16 x float]* %dx.v32.x1 to double*
  %14 = getelementptr double, double* %13, i32 0
  store double 1.000000e+00, double* %14, align 8
  %15 = bitcast [16 x float]* %dx.v32.x1 to double*
  %16 = getelementptr double, double* %15, i32 4
  store double 2.000000e+00, double* %16, align 8
  %17 = bitcast [16 x float]* %dx.v32.x1 to double*
  %18 = getelementptr double, double* %17, i32 8
  store double 0x400C7AE140000000, double* %18, align 8
  %19 = bitcast [16 x float]* %dx.v32.x1 to double*
  %20 = getelementptr double, double* %19, i32 12
  store double 7.000000e+00, double* %20, align 8
  %21 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %0, i32 0)
  %22 = extractvalue %dx.types.CBufRet.i32 %21, 0
  %23 = extractvalue %dx.types.CBufRet.i32 %21, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %22)
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %23)
  %24 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %25 = mul i32 %24, 4
  %26 = add i32 %25, 0
  %27 = bitcast [16 x float]* %dx.v32.x1 to double*
  %28 = getelementptr double, double* %27, i32 %26
  %29 = load double, double* %28, align 8
  %30 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %29)
  %31 = extractvalue %dx.types.splitdouble %30, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %31)
  %32 = extractvalue %dx.types.splitdouble %30, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %32)
  %33 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %34 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %35 = call double @dx.op.makeDouble.f64(i32 101, i32 %33, i32 %34)
  %36 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %37 = add i32 %36, 1
  %38 = call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %0, i32 %37)
  %39 = extractvalue %dx.types.CBufRet.f64 %38, 0
  %40 = fadd fast double %35, %39
  %41 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %40)
  %42 = extractvalue %dx.types.splitdouble %41, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %42)
  %43 = extractvalue %dx.types.splitdouble %41, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %43)
  %44 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %45 = mul i32 %44, 4
  %46 = add i32 %45, 0
  %47 = bitcast [24 x float]* %dx.v32.x0 to double*
  %48 = getelementptr double, double* %47, i32 %46
  %49 = load double, double* %48, align 8
  %50 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %49)
  %51 = extractvalue %dx.types.splitdouble %50, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 4, i32 %51)
  %52 = extractvalue %dx.types.splitdouble %50, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 5, i32 %52)
  %53 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %54 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %55 = call double @dx.op.makeDouble.f64(i32 101, i32 %53, i32 %54)
  %56 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 4)
  %57 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 5)
  %58 = call double @dx.op.makeDouble.f64(i32 101, i32 %56, i32 %57)
  %59 = fadd fast double %55, %58
  %60 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %59)
  %61 = extractvalue %dx.types.splitdouble %60, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %61)
  %62 = extractvalue %dx.types.splitdouble %60, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %62)
  %63 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %64 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 1, i32 undef)
  call void @dx.op.tempRegStore.i32(i32 1, i32 4, i32 %63)
  call void @dx.op.tempRegStore.i32(i32 1, i32 5, i32 %64)
  %65 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  %66 = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 1, i32 undef)
  call void @dx.op.tempRegStore.i32(i32 1, i32 6, i32 %65)
  call void @dx.op.tempRegStore.i32(i32 1, i32 7, i32 %66)
  %67 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 6)
  %68 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 7)
  %69 = call double @dx.op.makeDouble.f64(i32 101, i32 %67, i32 %68)
  %70 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 4)
  %71 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 5)
  %72 = call double @dx.op.makeDouble.f64(i32 101, i32 %70, i32 %71)
  %73 = fadd fast double %69, %72
  %74 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %73)
  %75 = extractvalue %dx.types.splitdouble %74, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 4, i32 %75)
  %76 = extractvalue %dx.types.splitdouble %74, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 5, i32 %76)
  %77 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  %78 = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 1, i32 undef)
  call void @dx.op.tempRegStore.i32(i32 1, i32 6, i32 %77)
  call void @dx.op.tempRegStore.i32(i32 1, i32 7, i32 %78)
  %79 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 6)
  %80 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 7)
  %81 = call double @dx.op.makeDouble.f64(i32 101, i32 %79, i32 %80)
  %82 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 4)
  %83 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 5)
  %84 = call double @dx.op.makeDouble.f64(i32 101, i32 %82, i32 %83)
  %85 = fadd fast double %81, %84
  %86 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %85)
  %87 = extractvalue %dx.types.splitdouble %86, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 4, i32 %87)
  %88 = extractvalue %dx.types.splitdouble %86, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 5, i32 %88)
  %89 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %90 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %91 = call double @dx.op.makeDouble.f64(i32 101, i32 %89, i32 %90)
  %92 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 4)
  %93 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 5)
  %94 = call double @dx.op.makeDouble.f64(i32 101, i32 %92, i32 %93)
  %95 = fadd fast double %91, %94
  %96 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %95)
  %97 = extractvalue %dx.types.splitdouble %96, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 2, i32 %97)
  %98 = extractvalue %dx.types.splitdouble %96, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 3, i32 %98)
  %99 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %100 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %101 = call double @dx.op.makeDouble.f64(i32 101, i32 %99, i32 %100)
  %102 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %103 = mul i32 %102, 4
  %104 = add i32 %103, 0
  %105 = bitcast [16 x float]* %dx.v32.x1 to double*
  %106 = getelementptr double, double* %105, i32 %104
  store double %101, double* %106, align 8
  %107 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %108 = mul i32 %107, 4
  %109 = add i32 %108, 0
  %110 = bitcast [16 x float]* %dx.v32.x1 to double*
  %111 = getelementptr double, double* %110, i32 %109
  %112 = load double, double* %111, align 8
  %113 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %112)
  %114 = extractvalue %dx.types.splitdouble %113, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %114)
  %115 = extractvalue %dx.types.splitdouble %113, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %115)
  %116 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %117 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  %118 = call double @dx.op.makeDouble.f64(i32 101, i32 %116, i32 %117)
  %119 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 2)
  %120 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 3)
  %121 = call double @dx.op.makeDouble.f64(i32 101, i32 %119, i32 %120)
  %122 = fadd fast double %118, %121
  %123 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102, double %122)
  %124 = extractvalue %dx.types.splitdouble %123, 0
  call void @dx.op.tempRegStore.i32(i32 1, i32 0, i32 %124)
  %125 = extractvalue %dx.types.splitdouble %123, 1
  call void @dx.op.tempRegStore.i32(i32 1, i32 1, i32 %125)
  %126 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 0)
  %127 = call i32 @dx.op.tempRegLoad.i32(i32 0, i32 1)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %126)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 %127)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 %126)
  call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 %127)
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind
declare void @dx.op.tempRegStore.i32(i32, i32, i32) #1

; Function Attrs: nounwind readonly
declare i32 @dx.op.tempRegLoad.i32(i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.splitdouble @dx.op.splitDouble.f64(i32, double) #2

; Function Attrs: nounwind readnone
declare double @dx.op.makeDouble.f64(i32, i32, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #2

; Function Attrs: nounwind
declare void @dx.op.storeOutput.i32(i32, i32, i32, i8, i32) #1

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.entryPoints = !{!5}
!llvm.ident = !{!15}

!0 = !{i32 1, i32 0}
!1 = !{!"ps", i32 6, i32 0}
!2 = !{null, null, !3, null}
!3 = !{!4}
!4 = !{i32 0, %dx.types.i8x80 addrspace(2)* undef, !"CB0", i32 0, i32 0, i32 1, i32 80, null}
!5 = !{void ()* @main, !"main", !6, !2, !14}
!6 = !{!7, !12, null}
!7 = !{!8, !10, !11}
!8 = !{i32 0, !"AAA", i8 5, i8 0, !9, i8 1, i32 1, i8 4, i32 0, i8 0, null}
!9 = !{i32 0}
!10 = !{i32 1, !"BBB", i8 5, i8 0, !9, i8 1, i32 1, i8 4, i32 1, i8 0, null}
!11 = !{i32 2, !"CCC", i8 5, i8 0, !9, i8 1, i32 1, i8 4, i32 2, i8 0, null}
!12 = !{!13}
!13 = !{i32 0, !"SV_Target", i8 5, i8 16, !9, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!14 = !{i32 0, i64 4}
!15 = !{!"dxbc2dxil 1.0"}
