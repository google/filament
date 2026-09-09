; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC9M4N32U0S0 = type { i8* }
%dx.types.LinAlgMatrixC9M4N8U0S2 = type { i8* }
%dx.types.LinAlgMatrixC9M4N8U1S0 = type { i8* }
%dx.types.LinAlgMatrixC9M4N8U0S0 = type { i8* }
%dx.types.LinAlgMatrixC21M4N8U0S0 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %3 = call %dx.types.LinAlgMatrixC9M4N32U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N32U0S0(i32 -2147483634, %dx.types.Handle %2, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK: Function: main: error: Component type 'I1' from InputInterp not allowed in LinAlg Matrix operations.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N32U0S0.v8f32.v4f32
  %4 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N32U0S0.v8f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N32U0S0 %3, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 1, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %5 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %6 = call %dx.types.LinAlgMatrixC9M4N8U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U0S2(i32 -2147483634, %dx.types.Handle %5, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'ThreadGroup' does not match expected scope Thread.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S2.v8f32.v4f32
  %7 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S2.v8f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U0S2 %6, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 9, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %9 = call %dx.types.LinAlgMatrixC9M4N8U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U1S0(i32 -2147483634, %dx.types.Handle %8, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix use 'B' does not match expected use A.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U1S0.v8f32.v4f32
  %10 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U1S0.v8f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U1S0 %9, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 9, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %11 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %12 = call %dx.types.LinAlgMatrixC9M4N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U0S0(i32 -2147483634, %dx.types.Handle %11, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input vector size '4' must be 8 for input matrix with K '8' and Type 'F32'
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v4f32.v4f32
  %13 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v4f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U0S0 %12, i1 true, <4 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00>, i32 9, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)
  %14 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %15 = call %dx.types.LinAlgMatrixC21M4N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC21M4N8U0S0(i32 -2147483634, %dx.types.Handle %14, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input vector size '8' must be 2 for input matrix with K '8' and Type 'F8_E4M3FN'
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC21M4N8U0S0.v8f32.v4f32
  %16 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC21M4N8U0S0.v8f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC21M4N8U0S0 %15, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 21, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)

  ; CHECK-NEXT: Function: main: error: Output vector size '2' must match input matrix M dimension '4'
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v2f32.mC9M4N8U0S0.v8f32.v2f32
  ; CHECK-NEXT: Function: main: error: Bias vector size '2' must match input matrix M dimension '4'
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v2f32.mC9M4N8U0S0.v8f32.v2f32
  %17 = call <2 x float> @dx.op.linAlgMatVecMulAdd.v2f32.mC9M4N8U0S0.v8f32.v2f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U0S0 %12, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 9, <2 x float> <float 3.000000e+00, float 5.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)

  ; CHECK-NEXT: Function: main: error: Float-like type 'float' must be signed
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4f32
  %18 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4f32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U0S0 %12, i1 false, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 9, <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)

  ; CHECK-NEXT: Function: main: error: Output vector element type 'float' must match bias vector element type 'i32'
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4i32
  %19 = call <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4i32(i32 -2147483622, %dx.types.LinAlgMatrixC9M4N8U0S0 %12, i1 true, <8 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00>, i32 9, <4 x i32> zeroinitializer)  ; LinAlgMatVecMulAdd(matrix,isOutputSigned,inputVector,inputInterpretation,biasVector)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC9M4N32U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N32U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N32U0S0.v8f32.v4f32(i32, %dx.types.LinAlgMatrixC9M4N32U0S0, i1, <8 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC9M4N8U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S2.v8f32.v4f32(i32, %dx.types.LinAlgMatrixC9M4N8U0S2, i1, <8 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC9M4N8U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U1S0.v8f32.v4f32(i32, %dx.types.LinAlgMatrixC9M4N8U1S0, i1, <8 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC9M4N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC9M4N8U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v4f32.v4f32(i32, %dx.types.LinAlgMatrixC9M4N8U0S0, i1, <4 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC21M4N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC21M4N8U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC21M4N8U0S0.v8f32.v4f32(i32, %dx.types.LinAlgMatrixC21M4N8U0S0, i1, <8 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare <2 x float> @dx.op.linAlgMatVecMulAdd.v2f32.mC9M4N8U0S0.v8f32.v2f32(i32, %dx.types.LinAlgMatrixC9M4N8U0S0, i1, <8 x float>, i32, <2 x float>) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4f32(i32, %dx.types.LinAlgMatrixC9M4N8U0S0, i1, <8 x float>, i32, <4 x float>) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.linAlgMatVecMulAdd.v4f32.mC9M4N8U0S0.v8f32.v4i32(i32, %dx.types.LinAlgMatrixC9M4N8U0S0, i1, <8 x float>, i32, <4 x i32>) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}
!dx.version = !{!6}
!dx.valver = !{!6}
!dx.shaderModel = !{!7}
!dx.resources = !{!8}
!dx.entryPoints = !{!11}

!0 = !{%dx.types.LinAlgMatrixC9M4N32U0S0 undef, i32 9, i32 4, i32 32, i32 0, i32 0}
!1 = !{%dx.types.LinAlgMatrixC9M4N8U0S2 undef, i32 9, i32 4, i32 8, i32 0, i32 2}
!2 = !{%dx.types.LinAlgMatrixC9M4N8U1S0 undef, i32 9, i32 4, i32 8, i32 1, i32 0}
!3 = !{%dx.types.LinAlgMatrixC9M4N8U0S0 undef, i32 9, i32 4, i32 8, i32 0, i32 0}
!4 = !{%dx.types.LinAlgMatrixC21M4N8U0S0 undef, i32 21, i32 4, i32 8, i32 0, i32 0}
!5 = !{!"dxc(private) 1.9.0.5414 (linalg-matvecmul, b61cafa02-dirty)"}
!6 = !{i32 1, i32 10}
!7 = !{!"cs", i32 6, i32 10}
!8 = !{!9, null, null, null}
!9 = !{!10}
!10 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!11 = !{void ()* @main, !"main", null, !8, !12}
!12 = !{i32 0, i64 8388624, i32 4, !13}
!13 = !{i32 1, i32 1, i32 1}
