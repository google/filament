; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M4N4U0S1 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U2S0 = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %4 = call %dx.types.LinAlgMatrixC8M4N4U0S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U0S1(i32 -2147483634, %dx.types.Handle %3, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK: Function: main: error: LinAlgMatrixStoreToDescriptor requires layout RowMajor or ColumnMajor.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %5 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %5, i32 0, i32 0, i32 4, i32 256)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: LinAlgMatrixStoreToDescriptor requires RWByteAddressBuffer.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %6, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be a multiple of 128, got 296
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %7, i32 0, i32 0, i32 0, i32 296)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be greater than 0, got 0
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %8, i32 0, i32 0, i32 0, i32 0)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  %9 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %10 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %9, i32 0, i32 undef, i8 1, i32 4)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
  %11 = extractvalue %dx.types.ResRet.i32 %10, 0

  ; CHECK-NEXT: Function: main: error: Align of LinAlgMatrixStoreToDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %12 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %12, i32 0, i32 0, i32 0, i32 %11)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Layout of LinAlgMatrixStoreToDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1
  %13 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U0S1 %4, %dx.types.Handle %13, i32 0, i32 0, i32 %11, i32 256)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Function: main: error: Input matrix scope 'Thread' does not match expected scope Wave or ThreadGroup.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U2S0
  %14 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %15 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32 -2147483634, %dx.types.Handle %14, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %16 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U2S0(i32 -2147483628, %dx.types.LinAlgMatrixC8M4N4U2S0 %15, %dx.types.Handle %16, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixStoreToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U0S1 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U0S1(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U0S1(i32, %dx.types.LinAlgMatrixC8M4N4U0S1, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixStoreToDescriptor.mC8M4N4U2S0(i32, %dx.types.LinAlgMatrixC8M4N4U2S0, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32, %dx.types.Handle, i32, i32, i8, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone }

!dx.targetTypes = !{!0, !1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.resources = !{!5}
!dx.entryPoints = !{!10}

!0 = !{%dx.types.LinAlgMatrixC8M4N4U0S1 undef, i32 8, i32 4, i32 4, i32 0, i32 1}
!1 = !{%dx.types.LinAlgMatrixC8M4N4U2S0 undef, i32 8, i32 4, i32 4, i32 2, i32 0}
!2 = !{!"dxc(private) 1.9.0.5436 (linalg-vali-matrixstoretodescriptor, 83eca680d-dirty)"}
!3 = !{i32 1, i32 10}
!4 = !{!"cs", i32 6, i32 10}
!5 = !{!6, !8, null, null}
!6 = !{!7}
!7 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!8 = !{!9}
!9 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!10 = !{void ()* @main, !"main", null, !5, !11}
!11 = !{i32 0, i64 8598323216, i32 4, !12}
!12 = !{i32 1, i32 1, i32 1}

