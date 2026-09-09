; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M8N8U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M6N8U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U2S2 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M8N6U2S2 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %3 = call %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S2(i32 -2147483634, %dx.types.Handle %2, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %5 = call %dx.types.LinAlgMatrixC8M8N8U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U1S2(i32 -2147483634, %dx.types.Handle %4, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %7 = call %dx.types.LinAlgMatrixC8M8N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S0(i32 -2147483634, %dx.types.Handle %6, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %9 = call %dx.types.LinAlgMatrixC8M8N8U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U1S0(i32 -2147483634, %dx.types.Handle %8, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %10 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %11 = call %dx.types.LinAlgMatrixC8M6N8U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M6N8U1S2(i32 -2147483634, %dx.types.Handle %10, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK: Function: main: error: A matrix use 'B' does not match expected use A.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U1S2.mC8M8N8U1S2
  %12 = call %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U1S2.mC8M8N8U1S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U1S2 %5, %dx.types.LinAlgMatrixC8M8N8U1S2 %5)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: B matrix use 'Accumulator' does not match expected use B.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U2S2
  %13 = call %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U2S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S2 %3, %dx.types.LinAlgMatrixC8M8N8U2S2 %12)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: Return matrix use 'A' does not match expected use Accumulator.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U0S2.mC8M8N8U0S2.mC8M8N8U1S2
  %14 = call %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U0S2.mC8M8N8U0S2.mC8M8N8U1S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S2 %3, %dx.types.LinAlgMatrixC8M8N8U1S2 %5)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: A matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S0.mC8M8N8U1S2
  ; CHECK-NEXT: Function: main: error: Matrix scope must be the same for all matrices. A 'Thread', B 'ThreadGroup', Return 'ThreadGroup'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S0.mC8M8N8U1S2
  %15 = call %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S0.mC8M8N8U1S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S0 %7, %dx.types.LinAlgMatrixC8M8N8U1S2 %5)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: B matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U1S0
  ; CHECK-NEXT: Function: main: error: Matrix scope must be the same for all matrices. A 'ThreadGroup', B 'Thread', Return 'ThreadGroup'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U1S0
  %16 = call %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U1S0(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S2 %14, %dx.types.LinAlgMatrixC8M8N8U1S0 %9)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: A matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S0.mC8M8N8U0S0.mC8M8N8U1S0
  ; CHECK-NEXT: Function: main: error: B matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S0.mC8M8N8U0S0.mC8M8N8U1S0
  ; CHECK-NEXT: Function: main: error: Return matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S0.mC8M8N8U0S0.mC8M8N8U1S0
  %17 = call %dx.types.LinAlgMatrixC8M8N8U2S0 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S0.mC8M8N8U0S0.mC8M8N8U1S0(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S0 %7, %dx.types.LinAlgMatrixC8M8N8U1S0 %9)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: K dim of A matrix '8x8' must match K dim of B matrix '6x8'. 8 != 6.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M6N8U1S2
  %18 = call %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M6N8U1S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S2 %14, %dx.types.LinAlgMatrixC8M6N8U1S2 %11)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Function: main: error: Return matrix dimension '8x6' must match A.MxB.N '8x8'.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixMultiply.mC8M8N6U2S2.mC8M8N8U0S2.mC8M8N8U1S2
  %19 = call %dx.types.LinAlgMatrixC8M8N6U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N6U2S2.mC8M8N8U0S2.mC8M8N8U1S2(i32 -2147483625, %dx.types.LinAlgMatrixC8M8N8U0S2 %14, %dx.types.LinAlgMatrixC8M8N8U1S2 %5)  ; LinAlgMatrixMultiply(matrixA,matrixB)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M6N8U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M6N8U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U1S2.mC8M8N8U1S2(i32, %dx.types.LinAlgMatrixC8M8N8U1S2, %dx.types.LinAlgMatrixC8M8N8U1S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U2S2(i32, %dx.types.LinAlgMatrixC8M8N8U0S2, %dx.types.LinAlgMatrixC8M8N8U2S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U0S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U0S2.mC8M8N8U0S2.mC8M8N8U1S2(i32, %dx.types.LinAlgMatrixC8M8N8U0S2, %dx.types.LinAlgMatrixC8M8N8U1S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S0.mC8M8N8U1S2(i32, %dx.types.LinAlgMatrixC8M8N8U0S0, %dx.types.LinAlgMatrixC8M8N8U1S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M8N8U1S0(i32, %dx.types.LinAlgMatrixC8M8N8U0S2, %dx.types.LinAlgMatrixC8M8N8U1S0) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S0 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S0.mC8M8N8U0S0.mC8M8N8U1S0(i32, %dx.types.LinAlgMatrixC8M8N8U0S0, %dx.types.LinAlgMatrixC8M8N8U1S0) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N8U2S2.mC8M8N8U0S2.mC8M6N8U1S2(i32, %dx.types.LinAlgMatrixC8M8N8U0S2, %dx.types.LinAlgMatrixC8M6N8U1S2) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N6U2S2 @dx.op.linAlgMatrixMultiply.mC8M8N6U2S2.mC8M8N8U0S2.mC8M8N8U1S2(i32, %dx.types.LinAlgMatrixC8M8N8U0S2, %dx.types.LinAlgMatrixC8M8N8U1S2) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}
!dx.version = !{!9}
!dx.valver = !{!9}
!dx.shaderModel = !{!10}
!dx.resources = !{!11}
!dx.entryPoints = !{!14}

!0 = !{%dx.types.LinAlgMatrixC8M8N8U0S2 undef, i32 8, i32 8, i32 8, i32 0, i32 2}
!1 = !{%dx.types.LinAlgMatrixC8M8N8U1S2 undef, i32 8, i32 8, i32 8, i32 1, i32 2}
!2 = !{%dx.types.LinAlgMatrixC8M8N8U2S2 undef, i32 8, i32 8, i32 8, i32 2, i32 2}
!3 = !{%dx.types.LinAlgMatrixC8M6N8U1S2 undef, i32 8, i32 6, i32 8, i32 1, i32 2}
!4 = !{%dx.types.LinAlgMatrixC8M8N6U2S2 undef, i32 8, i32 8, i32 6, i32 2, i32 2}
!5 = !{%dx.types.LinAlgMatrixC8M8N8U0S0 undef, i32 8, i32 8, i32 8, i32 0, i32 0}
!6 = !{%dx.types.LinAlgMatrixC8M8N8U1S0 undef, i32 8, i32 8, i32 8, i32 1, i32 0}
!7 = !{%dx.types.LinAlgMatrixC8M8N8U2S0 undef, i32 8, i32 8, i32 8, i32 2, i32 0}
!8 = !{!"dxc(private) 1.9.0.5450 (linalg-vali-refactor, 8c87acfc6-dirty)"}
!9 = !{i32 1, i32 10}
!10 = !{!"cs", i32 6, i32 10}
!11 = !{!12, null, null, null}
!12 = !{!13}
!13 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!14 = !{void ()* @main, !"main", null, !11, !15}
!15 = !{i32 0, i64 8388624, i32 4, !16}
!16 = !{i32 1, i32 1, i32 1}

