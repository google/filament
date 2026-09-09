; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M1025N1025U2S0 = type { i8* }
%dx.types.LinAlgMatrixC4M16N16U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M129N16U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N129U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M3N16U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N3U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M129N16U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N129U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M3N16U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N3U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M1025N16U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M16N1025U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M16N0U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M3N16U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M1025N129U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M129N1025U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M0N129U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M129N3U1S2 = type { i8* }
%dx.types.LinAlgMatrixC8M128N128U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M128N128U1S0 = type { i8* }
%dx.types.LinAlgMatrixC8M1024N1024U0S2 = type { i8* }
%dx.types.LinAlgMatrixC8M1024N1024U1S2 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)

  ; Matrix<F16, 1025, 1025, Accumulator, Thread> - its not possible to statically determine which dim is K on Accumulator so no validation occurs
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %3 = call %dx.types.LinAlgMatrixC8M1025N1025U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N1025U2S0(i32 -2147483634, %dx.types.Handle %2, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK: Function: main: error: Return matrix must have well-formed metadata.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC4M16N16U0S2
  ; Matrix<I32, 16, 16, A, ThreadGroup> - missing metadata
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %5 = call %dx.types.LinAlgMatrixC4M16N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC4M16N16U0S2(i32 -2147483634, %dx.types.Handle %4, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=129 must be >= 4 and <= 128.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N129U0S0
  ; Matrix<F16, 129, 16, A, Thread> - N is K so pass
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %7 = call %dx.types.LinAlgMatrixC8M129N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N16U0S0(i32 -2147483634, %dx.types.Handle %6, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 16, 129, A, Thread> - N is K so fail
  %8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %9 = call %dx.types.LinAlgMatrixC8M16N129U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N129U0S0(i32 -2147483634, %dx.types.Handle %8, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=3 must be >= 4 and <= 128.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N3U0S0
  ; Matrix<F16, 3, 16, A, Thread> - N is K so pass
  %10 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %11 = call %dx.types.LinAlgMatrixC8M3N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U0S0(i32 -2147483634, %dx.types.Handle %10, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 16, 3, A, Thread> - N is K so fail
  %12 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %13 = call %dx.types.LinAlgMatrixC8M16N3U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N3U0S0(i32 -2147483634, %dx.types.Handle %12, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=129 must be >= 4 and <= 128.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N16U1S0
  ; Matrix<F16, 129, 16, B, Thread> - M is K so fail
  %14 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %15 = call %dx.types.LinAlgMatrixC8M129N16U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N16U1S0(i32 -2147483634, %dx.types.Handle %14, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 16, 129, B, Thread> - M is K so pass
  %16 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %17 = call %dx.types.LinAlgMatrixC8M16N129U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N129U1S0(i32 -2147483634, %dx.types.Handle %16, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=3 must be >= 4 and <= 128.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U1S0
  ; Matrix<F16, 3, 16, B, Thread> - M is K so fail
  %18 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %19 = call %dx.types.LinAlgMatrixC8M3N16U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U1S0(i32 -2147483634, %dx.types.Handle %18, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 16, 3, B, Thread> - M is K so pass
  %20 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %21 = call %dx.types.LinAlgMatrixC8M16N3U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N3U1S0(i32 -2147483634, %dx.types.Handle %20, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=1025 must be >= 1 and <= 1024.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N1025U0S2
  ; Matrix<F16, 1025, 16, A, ThreadGroup> - N is K so pass
  %22 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %23 = call %dx.types.LinAlgMatrixC8M1025N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N16U0S2(i32 -2147483634, %dx.types.Handle %22, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 16, 1025, A, ThreadGroup> - N is K so fail
  %24 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %25 = call %dx.types.LinAlgMatrixC8M16N1025U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N1025U0S2(i32 -2147483634, %dx.types.Handle %24, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=0 must be >= 1 and <= 1024.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N0U0S2
  ; Matrix<F16, 16, 3, A, ThreadGroup> - N is K so fail
  %26 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %27 = call %dx.types.LinAlgMatrixC8M16N0U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N0U0S2(i32 -2147483634, %dx.types.Handle %26, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 3, 16, A, ThreadGroup> - N is K so pass
  %28 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %29 = call %dx.types.LinAlgMatrixC8M3N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U0S2(i32 -2147483634, %dx.types.Handle %28, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=1025 must be >= 1 and <= 1024.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N129U1S2
  ; Matrix<F16, 1025, 129, B, ThreadGroup> - M is K so fail
  %30 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %31 = call %dx.types.LinAlgMatrixC8M1025N129U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N129U1S2(i32 -2147483634, %dx.types.Handle %30, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 129, 1025, B, ThreadGroup> - M is K so pass
  %32 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %33 = call %dx.types.LinAlgMatrixC8M129N1025U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N1025U1S2(i32 -2147483634, %dx.types.Handle %32, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Return matrix K dimension out of bounds. K=0 must be >= 1 and <= 1024.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M0N129U1S2
  ; Matrix<F16, 3, 129, B, ThreadGroup> - M is K so fail
  %34 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %35 = call %dx.types.LinAlgMatrixC8M0N129U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M0N129U1S2(i32 -2147483634, %dx.types.Handle %34, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  ; Matrix<F16, 129, 3, B, ThreadGroup> - M is K so pass
  %36 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %37 = call %dx.types.LinAlgMatrixC8M129N3U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N3U1S2(i32 -2147483634, %dx.types.Handle %36, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; Below are just barely in bounds. No validation errors should be emitted.

  ; Matrix<F16, 128, 128, A, Thread>
  %38 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %39 = call %dx.types.LinAlgMatrixC8M128N128U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M128N128U0S0(i32 -2147483634, %dx.types.Handle %38, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; Matrix<F16, 128, 128, B, Thread>
  %40 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %41 = call %dx.types.LinAlgMatrixC8M128N128U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M128N128U1S0(i32 -2147483634, %dx.types.Handle %40, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; Matrix<F16, 1024, 1024, A, ThreadGroup>
  %42 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %43 = call %dx.types.LinAlgMatrixC8M1024N1024U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1024N1024U0S2(i32 -2147483634, %dx.types.Handle %42, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; Matrix<F16, 1024, 1024, B, ThreadGroup>
  %44 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %45 = call %dx.types.LinAlgMatrixC8M1024N1024U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1024N1024U1S2(i32 -2147483634, %dx.types.Handle %44, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M1025N1025U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N1025U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC4M16N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC4M16N16U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M129N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N16U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N129U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N129U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M3N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N3U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N3U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M129N16U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N16U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N129U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N129U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M3N16U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N3U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N3U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M1025N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N16U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N1025U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N1025U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N0U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N0U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M3N16U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M3N16U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M1025N129U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1025N129U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M129N1025U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N1025U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M0N129U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M0N129U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M129N3U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M129N3U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M128N128U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M128N128U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M128N128U1S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M128N128U1S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M1024N1024U0S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1024N1024U0S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M1024N1024U1S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M1024N1024U1S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

; !1 is intentionally removed. See below.
!dx.targetTypes = !{!0, !2, !3, !4, !5, !6, !7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21}
!llvm.ident = !{!22}
!dx.version = !{!23}
!dx.valver = !{!23}
!dx.shaderModel = !{!24}
!dx.resources = !{!25}
!dx.entryPoints = !{!28}

!0 = !{%dx.types.LinAlgMatrixC8M1025N1025U2S0 undef, i32 8, i32 1025, i32 1025, i32 2, i32 0}
; Below is intentionally removed to cause a missing metadata error
; !1 = !{%dx.types.LinAlgMatrixC4M16N16U0S2 undef, i32 4, i32 16, i32 16, i32 0, i32 2}
!2 = !{%dx.types.LinAlgMatrixC8M129N16U0S0 undef, i32 8, i32 129, i32 16, i32 0, i32 0}
!3 = !{%dx.types.LinAlgMatrixC8M16N129U0S0 undef, i32 8, i32 16, i32 129, i32 0, i32 0}
!4 = !{%dx.types.LinAlgMatrixC8M3N16U0S0 undef, i32 8, i32 3, i32 16, i32 0, i32 0}
!5 = !{%dx.types.LinAlgMatrixC8M16N3U0S0 undef, i32 8, i32 16, i32 3, i32 0, i32 0}
!6 = !{%dx.types.LinAlgMatrixC8M129N16U1S0 undef, i32 8, i32 129, i32 16, i32 1, i32 0}
!7 = !{%dx.types.LinAlgMatrixC8M16N129U1S0 undef, i32 8, i32 16, i32 129, i32 1, i32 0}
!8 = !{%dx.types.LinAlgMatrixC8M3N16U1S0 undef, i32 8, i32 3, i32 16, i32 1, i32 0}
!9 = !{%dx.types.LinAlgMatrixC8M16N3U1S0 undef, i32 8, i32 16, i32 3, i32 1, i32 0}
!10 = !{%dx.types.LinAlgMatrixC8M1025N16U0S2 undef, i32 8, i32 1025, i32 16, i32 0, i32 2}
!11 = !{%dx.types.LinAlgMatrixC8M16N1025U0S2 undef, i32 8, i32 16, i32 1025, i32 0, i32 2}
!12 = !{%dx.types.LinAlgMatrixC8M16N0U0S2 undef, i32 8, i32 16, i32 0, i32 0, i32 2}
!13 = !{%dx.types.LinAlgMatrixC8M3N16U0S2 undef, i32 8, i32 3, i32 16, i32 0, i32 2}
!14 = !{%dx.types.LinAlgMatrixC8M1025N129U1S2 undef, i32 8, i32 1025, i32 129, i32 1, i32 2}
!15 = !{%dx.types.LinAlgMatrixC8M129N1025U1S2 undef, i32 8, i32 129, i32 1025, i32 1, i32 2}
!16 = !{%dx.types.LinAlgMatrixC8M0N129U1S2 undef, i32 8, i32 0, i32 129, i32 1, i32 2}
!17 = !{%dx.types.LinAlgMatrixC8M129N3U1S2 undef, i32 8, i32 129, i32 3, i32 1, i32 2}
!18 = !{%dx.types.LinAlgMatrixC8M128N128U0S0 undef, i32 8, i32 128, i32 128, i32 0, i32 0}
!19 = !{%dx.types.LinAlgMatrixC8M128N128U1S0 undef, i32 8, i32 128, i32 128, i32 1, i32 0}
!20 = !{%dx.types.LinAlgMatrixC8M1024N1024U0S2 undef, i32 8, i32 1024, i32 1024, i32 0, i32 2}
!21 = !{%dx.types.LinAlgMatrixC8M1024N1024U1S2 undef, i32 8, i32 1024, i32 1024, i32 1, i32 2}
!22 = !{!"dxc(private) 1.9.0.15416 (main, 27579abe5)"}
!23 = !{i32 1, i32 10}
!24 = !{!"cs", i32 6, i32 10}
!25 = !{!26, null, null, null}
!26 = !{!27}
!27 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!28 = !{void ()* @main, !"main", null, !25, !29}
!29 = !{i32 0, i64 8388624, i32 4, !30}
!30 = !{i32 1, i32 1, i32 1}
