; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC2M5N4U1S2 = type { i8* }
%dx.types.LinAlgMatrixC4M8N4U1S2 = type { i8* }
%dx.types.LinAlgMatrixC4M5N8U1S2 = type { i8* }
%dx.types.LinAlgMatrixC4M5N4U1S2 = type { i8* }
%dx.types.LinAlgMatrixC2M4N5U1S1 = type { i8* }
%dx.types.LinAlgMatrixC2M4N5U1S0 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %h1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %bab = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %h1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer

  %1 = call %dx.types.LinAlgMatrixC2M5N4U1S2 @dx.op.linAlgFillMatrix.mC2M5N4U1S2.i32(i32 -2147483636, i32 1)  ; LinAlgFillMatrix(value)

  ; CHECK: Function: main: error: Destination matrix dimension '8x4' must match source matrix dimension '5x4'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC4M8N4U1S2.mC2M5N4U1S2
  %2 = call %dx.types.LinAlgMatrixC4M8N4U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M8N4U1S2.mC2M5N4U1S2(i32 -2147483635, %dx.types.LinAlgMatrixC2M5N4U1S2 %1, i1 false)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  ; CHECK-NEXT: Function: main: error: Destination matrix dimension '5x8' must match source matrix dimension '5x4'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC4M5N8U1S2.mC2M5N4U1S2
  %3 = call %dx.types.LinAlgMatrixC4M5N8U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M5N8U1S2.mC2M5N4U1S2(i32 -2147483635, %dx.types.LinAlgMatrixC2M5N4U1S2 %1, i1 false)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  ; CHECK-NEXT: Function: main: error: Destination matrix dimension '5x4' must match source matrix dimension '4x5'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC4M5N4U1S2.mC2M5N4U1S2
  %4 = call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M5N4U1S2.mC2M5N4U1S2(i32 -2147483635, %dx.types.LinAlgMatrixC2M5N4U1S2 %1, i1 true)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  ; CHECK-NEXT: Function: main: error: Destination matrix scope 'Wave' must match source matrix scope 'ThreadGroup'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S1.mC2M5N4U1S2
  %5 = call %dx.types.LinAlgMatrixC2M4N5U1S1 @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S1.mC2M5N4U1S2(i32 -2147483635, %dx.types.LinAlgMatrixC2M5N4U1S2 %1, i1 true)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  %6 = call %dx.types.LinAlgMatrixC2M4N5U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC2M4N5U1S0(i32 -2147483634, %dx.types.Handle %bab, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Destination matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S0.mC2M4N5U1S0
  ; CHECK-NEXT: Function: main: error: Source matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S0.mC2M4N5U1S0
  %7 = call %dx.types.LinAlgMatrixC2M4N5U1S0 @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S0.mC2M4N5U1S0(i32 -2147483635, %dx.types.LinAlgMatrixC2M4N5U1S0 %6, i1 false)  ; LinAlgCopyConvertMatrix(srcMatrix,transpose)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC2M5N4U1S2 @dx.op.linAlgFillMatrix.mC2M5N4U1S2.i32(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC4M8N4U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M8N4U1S2.mC2M5N4U1S2(i32, %dx.types.LinAlgMatrixC2M5N4U1S2, i1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC4M5N8U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M5N8U1S2.mC2M5N4U1S2(i32, %dx.types.LinAlgMatrixC2M5N4U1S2, i1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgCopyConvertMatrix.mC4M5N4U1S2.mC2M5N4U1S2(i32, %dx.types.LinAlgMatrixC2M5N4U1S2, i1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC2M4N5U1S1 @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S1.mC2M5N4U1S2(i32, %dx.types.LinAlgMatrixC2M5N4U1S2, i1) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC2M4N5U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC2M4N5U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC2M4N5U1S0 @dx.op.linAlgCopyConvertMatrix.mC2M4N5U1S0.mC2M4N5U1S0(i32, %dx.types.LinAlgMatrixC2M4N5U1S0, i1) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2, !3, !4, !5}
!llvm.ident = !{!6}
!dx.version = !{!7}
!dx.valver = !{!7}
!dx.shaderModel = !{!8}
!dx.resources = !{!12}
!dx.entryPoints = !{!9}

!0 = !{%dx.types.LinAlgMatrixC2M5N4U1S2 undef, i32 2, i32 5, i32 4, i32 1, i32 2}
!1 = !{%dx.types.LinAlgMatrixC4M8N4U1S2 undef, i32 4, i32 8, i32 4, i32 1, i32 2}
!2 = !{%dx.types.LinAlgMatrixC4M5N8U1S2 undef, i32 4, i32 5, i32 8, i32 1, i32 2}
!3 = !{%dx.types.LinAlgMatrixC4M5N4U1S2 undef, i32 4, i32 5, i32 4, i32 1, i32 2}
!4 = !{%dx.types.LinAlgMatrixC2M4N5U1S1 undef, i32 2, i32 4, i32 5, i32 1, i32 1}
!5 = !{%dx.types.LinAlgMatrixC2M4N5U1S0 undef, i32 2, i32 4, i32 5, i32 1, i32 0}
!6 = !{!"dxc(private) 1.9.0.5389 (linalg-validation-component-type, b8b639b7d-dirty)"}
!7 = !{i32 1, i32 10}
!8 = !{!"cs", i32 6, i32 10}
!9 = !{void ()* @main, !"main", null, null, !10}
!10 = !{i32 4, !11}
!11 = !{i32 1, i32 1, i32 1}
!12 = !{!13, null, null, null}
!13 = !{!14}
!14 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
