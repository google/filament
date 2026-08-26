; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC0M16N16U0S0 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer

  ; CHECK: Function: main: error: Component type 'Invalid' from return matrix not allowed in LinAlg Matrix operations.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC0M16N16U0S0
  ; Matrix<Invalid, 16, 16, A, Thread>
  %3 = call %dx.types.LinAlgMatrixC0M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC0M16N16U0S0(i32 -2147483634, %dx.types.Handle %2, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Validation failed.

  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC0M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC0M16N16U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.entryPoints = !{!7}

!0 = !{%dx.types.LinAlgMatrixC0M16N16U0S0 undef, i32 0, i32 16, i32 16, i32 0, i32 0}
!1 = !{!"dxc(private) 1.9.0.15416 (main, 27579abe5)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{!5, null, null, null}
!5 = !{!6}
!6 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!7 = !{void ()* @main, !"main", null, !4, !8}
!8 = !{i32 0, i64 8388624, i32 4, !9}
!9 = !{i32 1, i32 1, i32 1}
