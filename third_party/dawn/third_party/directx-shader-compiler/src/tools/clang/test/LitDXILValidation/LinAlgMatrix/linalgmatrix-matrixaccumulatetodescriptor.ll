; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M4N4U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U2S1 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U0S0 = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %4 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32 -2147483634, %dx.types.Handle %3, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %5 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %6 = call %dx.types.LinAlgMatrixC8M4N4U2S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S1(i32 -2147483634, %dx.types.Handle %5, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %8 = call %dx.types.LinAlgMatrixC8M4N4U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U0S0(i32 -2147483634, %dx.types.Handle %7, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %9 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %10 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %9, i32 0, i32 undef, i8 1, i32 4)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0

  ; CHECK: Function: main: error: Layout of LinAlgMatrixAccumulateToDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %12 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %12, i32 0, i32 0, i32 %11, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix with scope 'Thread' requires layout OuterProductOptimal or OuterProductOptimalTranspose for LinAlgMatrixAccumulateToDescriptor.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %13 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %13, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Stride of LinAlgMatrixAccumulateToDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %14 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %14, i32 0, i32 %11, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: LinAlgMatrixAccumulateToDescriptor requires RWByteAddressBuffer.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %15 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %15, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Align of LinAlgMatrixAccumulateToDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %16 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %16, i32 0, i32 0, i32 4, i32 %11)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be greater than 0, got 0
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %17 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %17, i32 0, i32 0, i32 4, i32 0)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be a multiple of 128, got 215
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %18 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %18, i32 0, i32 0, i32 4, i32 215)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: LinAlgMatrixAccumulateToDescriptor with layout 'OuterProductOptimal' requires stride 0.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0
  %19 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S0 %4, %dx.types.Handle %19, i32 0, i32 1, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix with scope 'Wave' requires layout RowMajor or ColumnMajor for LinAlgMatrixAccumulateToDescriptor.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S1
  %20 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S1(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U2S1 %6, %dx.types.Handle %20, i32 0, i32 0, i32 3, i32 256)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix use 'A' does not match expected use Accumulator.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U0S0
  %21 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U0S0(i32 -2147483621, %dx.types.LinAlgMatrixC8M4N4U0S0 %8, %dx.types.Handle %21, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S1(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32, %dx.types.Handle, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U2S1(i32, %dx.types.LinAlgMatrixC8M4N4U2S1, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToDescriptor.mC8M4N4U0S0(i32, %dx.types.LinAlgMatrixC8M4N4U0S0, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2}
!llvm.ident = !{!3}
!dx.version = !{!4}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.entryPoints = !{!11}

!0 = !{%dx.types.LinAlgMatrixC8M4N4U2S0 undef, i32 8, i32 4, i32 4, i32 2, i32 0}
!1 = !{%dx.types.LinAlgMatrixC8M4N4U2S1 undef, i32 8, i32 4, i32 4, i32 2, i32 1}
!2 = !{%dx.types.LinAlgMatrixC8M4N4U0S0 undef, i32 8, i32 4, i32 4, i32 0, i32 0}
!3 = !{!"dxc(private) 1.9.0.5433 (linalg-vali-matrixaccumtodescriptor, 60016e894-dirty)"}
!4 = !{i32 1, i32 10}
!5 = !{!"cs", i32 6, i32 10}
!6 = !{!7, !9, null, null}
!7 = !{!8}
!8 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!9 = !{!10}
!10 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!11 = !{void ()* @main, !"main", null, !6, !12}
!12 = !{i32 0, i64 8598323216, i32 4, !13}
!13 = !{i32 1, i32 1, i32 1}

