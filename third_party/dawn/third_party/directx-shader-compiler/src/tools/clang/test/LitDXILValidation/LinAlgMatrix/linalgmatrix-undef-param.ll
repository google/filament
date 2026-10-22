; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.LinAlgMatrixC4M16N16U0S2 = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %handle = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer

  ; CHECK: Function: main: error: Instructions should not read uninitialized value.
  ; CHECK-NEXT: note: at {{.*}}@dx.op.linAlgMatrixAccumulateToDescriptor
  call void @dx.op.linAlgMatrixAccumulateToDescriptor.mC4M16N16U0S2(i32 -2147483621, %dx.types.LinAlgMatrixC4M16N16U0S2 undef, %dx.types.Handle %handle, i32 1, i32 2, i32 3, i32 4)  ; LinAlgMatrixAccumulateToDescriptor(matrix,handle,offset,stride,layout,align)

  ; CHECK: Function: main: error: Instructions should not read uninitialized value.
  ; CHECK-NEXT: note: at {{.*}}@dx.op.linAlgMatrixLength
  %v2 = call i32 @dx.op.linAlgMatrixLength.mC4M16N16U0S2(i32 -2147483632, %dx.types.LinAlgMatrixC4M16N16U0S2 undef)  ; LinAlgMatrixLength(matrix)

  ; CHECK-NEXT: Validation failed.

  ret void
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

; Function Attrs: nounwind
declare void @dx.op.linAlgMatrixAccumulateToDescriptor.mC4M16N16U0S2(i32, %dx.types.LinAlgMatrixC4M16N16U0S2, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare i32 @dx.op.linAlgMatrixLength.mC4M16N16U0S2(i32, %dx.types.LinAlgMatrixC4M16N16U0S2) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!dx.targetTypes = !{!0}
!llvm.ident = !{!17}
!dx.version = !{!18}
!dx.valver = !{!18}
!dx.shaderModel = !{!19}
!dx.resources = !{!20}
!dx.entryPoints = !{!23}

!0 = !{%dx.types.LinAlgMatrixC4M16N16U0S2 undef, i32 4, i32 16, i32 16, i32 0, i32 2}
!17 = !{!"dxc(private) 1.9.0.15241 (main, 1f63535ae)"}
!18 = !{i32 1, i32 10}
!19 = !{!"cs", i32 6, i32 10}
!20 = !{null, !21, null, null}
!21 = !{!22}
!22 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!23 = !{void ()* @main, !"main", null, !20, !24}
!24 = !{i32 0, i64 8589934608, i32 4, !25}
!25 = !{i32 4, i32 4, i32 4}
