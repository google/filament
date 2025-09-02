; REQUIRES: dxil-1-9
; RUN: %dxv %s 2>&1 | FileCheck %s

;Original Source: \tools\clang\test\CodeGenHLSL\linalg\outer-product-accumulate-matrix-layout.hlsl

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.v8f16 = type { <8 x half>, i32 }
%struct.ByteAddressBuffer = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

;CHECK: Validation succeeded.

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %5 = call %dx.types.ResRet.v8f16 @dx.op.rawBufferVectorLoad.v8f16(i32 303, %dx.types.Handle %4, i32 0, i32 undef, i32 2)  ; RawBufferVectorLoad(buf,index,elementOffset,alignment)
  %6 = extractvalue %dx.types.ResRet.v8f16 %5, 0
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %8 = call %dx.types.ResRet.v8f16 @dx.op.rawBufferVectorLoad.v8f16(i32 303, %dx.types.Handle %7, i32 0, i32 undef, i32 2)  ; RawBufferVectorLoad(buf,index,elementOffset,alignment)
  %9 = extractvalue %dx.types.ResRet.v8f16 %8, 0
  %10 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.outerProductAccumulate.v8f16.v8f16(i32 307, <8 x half> %6, <8 x half> %9, %dx.types.Handle %10, i32 0, i32 8, i32 3, i32 0)  ; OuterProductAccumulate(inputVector1,inputVector2,matrixBuffer,matrixOffset,matrixIntepretation,matrixLayout,matrixStride)
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.v8f16 @dx.op.rawBufferVectorLoad.v8f16(i32, %dx.types.Handle, i32, i32, i32) #0

; Function Attrs: nounwind
declare void @dx.op.outerProductAccumulate.v8f16.v8f16(i32, <8 x half>, <8 x half>, %dx.types.Handle, i32, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }

!dx.version = !{!0}
!dx.valver = !{!0}
!dx.shaderModel = !{!1}
!dx.resources = !{!2}
!dx.entryPoints = !{!8}

!0 = !{i32 1, i32 9}
!1 = !{!"cs", i32 6, i32 9}
!2 = !{!3, !6, null, null}
!3 = !{!4, !5}
!4 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!5 = !{i32 1, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 1, i32 1, i32 11, i32 0, null}
!6 = !{!7}
!7 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!8 = !{void ()* @main, !"main", null, !2, !9}
!9 = !{i32 0, i64 8598323216, i32 4, !10}
!10 = !{i32 1, i32 1, i32 1}
