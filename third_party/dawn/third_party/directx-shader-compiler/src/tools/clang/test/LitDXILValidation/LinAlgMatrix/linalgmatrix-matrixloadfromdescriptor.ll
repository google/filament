; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.LinAlgMatrixC8M4N4U2S2 = type { i8* }
%dx.types.LinAlgMatrixC8M4N4U2S0 = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%dx.types.LinAlgMatrixC8M8N8U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M4N8U2S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N16U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M8N8U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M8N4U0S0 = type { i8* }
%dx.types.LinAlgMatrixC8M16N8U0S0 = type { i8* }
%struct.ByteAddressBuffer = type { i32 }
%"class.StructuredBuffer<unsigned int>" = type { i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK: Function: main: error: Return matrix with scope 'ThreadGroup' requires layout RowMajor or ColumnMajor for LinAlgMatrixLoadFromDescriptor.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S2
  %5 = call %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S2(i32 -2147483634, %dx.types.Handle %4, i32 0, i32 0, i32 4, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: LinAlgMatrixLoadFromDescriptor with layout 'OuterProductOptimal' requires stride 0.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0
  %7 = call %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32 -2147483634, %dx.types.Handle %6, i32 0, i32 4, i32 4, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 12, i32 4 })  ; AnnotateHandle(res,props)  resource: StructuredBuffer<stride=4>
  %9 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %8, i32 0, i32 0, i8 1, i32 4)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
  %10 = extractvalue %dx.types.ResRet.i32 %9, 0
  %11 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: Layout of LinAlgMatrixLoadFromDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U2S0
  %12 = call %dx.types.LinAlgMatrixC8M8N8U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U2S0(i32 -2147483634, %dx.types.Handle %11, i32 0, i32 0, i32 %10, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %13 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: Stride of LinAlgMatrixLoadFromDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S0
  %14 = call %dx.types.LinAlgMatrixC8M16N16U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S0(i32 -2147483634, %dx.types.Handle %13, i32 0, i32 %10, i32 4, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %15 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; No error expected for non-imm arg stride on row/col layout
  %16 = call %dx.types.LinAlgMatrixC8M4N8U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N8U2S0(i32 -2147483634, %dx.types.Handle %15, i32 0, i32 %10, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %17 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be a multiple of 128, got 215
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N8U2S0
  %18 = call %dx.types.LinAlgMatrixC8M4N8U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N8U2S0(i32 -2147483634, %dx.types.Handle %17, i32 0, i32 0, i32 0, i32 215)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %19 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: Loading matrix with Thread scope requires ByteAddressBuffer.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0
  %20 = call %dx.types.LinAlgMatrixC8M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0(i32 -2147483634, %dx.types.Handle %19, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %21 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: Align of LinAlgMatrixLoadFromDescriptor must be an immediate constant.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S0
  %22 = call %dx.types.LinAlgMatrixC8M8N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S0(i32 -2147483634, %dx.types.Handle %21, i32 0, i32 0, i32 0, i32 %10)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %23 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 12, i32 4 })  ; AnnotateHandle(res,props)  resource: StructuredBuffer<stride=4>


  ; CHECK-NEXT: Function: main: error: Loading matrix with Thread scope requires ByteAddressBuffer.
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N4U0S0
  %24 = call %dx.types.LinAlgMatrixC8M8N4U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N4U0S0(i32 -2147483634, %dx.types.Handle %23, i32 0, i32 0, i32 0, i32 128)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)
  %25 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 })  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer


  ; CHECK-NEXT: Function: main: error: parameter 'Align' must be greater than 0, got 0
  ; CHECK-NEXT: note: at {{.*}} @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N8U0S0
  %26 = call %dx.types.LinAlgMatrixC8M16N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N8U0S0(i32 -2147483634, %dx.types.Handle %25, i32 0, i32 0, i32 0, i32 0)  ; LinAlgMatrixLoadFromDescriptor(handle,offset,stride,layout,align)

  ; CHECK-NEXT: Validation failed.
  ret void
}

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S2 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S2(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N4U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N4U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32, %dx.types.Handle, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M4N8U2S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M4N8U2S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N16U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N16U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N8U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M8N4U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M8N4U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.LinAlgMatrixC8M16N8U0S0 @dx.op.linAlgMatrixLoadFromDescriptor.mC8M16N8U0S0(i32, %dx.types.Handle, i32, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.targetTypes = !{!0, !1, !2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}
!dx.version = !{!10}
!dx.valver = !{!10}
!dx.shaderModel = !{!11}
!dx.resources = !{!12}
!dx.entryPoints = !{!19}

!0 = !{%dx.types.LinAlgMatrixC8M4N4U2S2 undef, i32 8, i32 4, i32 4, i32 2, i32 2}
!1 = !{%dx.types.LinAlgMatrixC8M4N4U2S0 undef, i32 8, i32 4, i32 4, i32 2, i32 0}
!2 = !{%dx.types.LinAlgMatrixC8M8N8U2S0 undef, i32 8, i32 8, i32 8, i32 2, i32 0}
!3 = !{%dx.types.LinAlgMatrixC8M16N16U2S0 undef, i32 8, i32 16, i32 16, i32 2, i32 0}
!4 = !{%dx.types.LinAlgMatrixC8M4N8U2S0 undef, i32 8, i32 4, i32 8, i32 2, i32 0}
!5 = !{%dx.types.LinAlgMatrixC8M16N16U0S0 undef, i32 8, i32 16, i32 16, i32 0, i32 0}
!6 = !{%dx.types.LinAlgMatrixC8M8N8U0S0 undef, i32 8, i32 8, i32 8, i32 0, i32 0}
!7 = !{%dx.types.LinAlgMatrixC8M8N4U0S0 undef, i32 8, i32 8, i32 4, i32 0, i32 0}
!8 = !{%dx.types.LinAlgMatrixC8M16N8U0S0 undef, i32 8, i32 16, i32 8, i32 0, i32 0}
!9 = !{!"dxc(private) 1.9.0.5417 (linalg-vali-matrixloadfromdescriptor, 31c54f018-dirty)"}
!10 = !{i32 1, i32 10}
!11 = !{!"cs", i32 6, i32 10}
!12 = !{!13, !17, null, null}
!13 = !{!14, !15}
!14 = !{i32 0, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!15 = !{i32 1, %"class.StructuredBuffer<unsigned int>"* undef, !"", i32 0, i32 1, i32 1, i32 12, i32 0, !16}
!16 = !{i32 1, i32 4}
!17 = !{!18}
!18 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!19 = !{void ()* @main, !"main", null, !12, !20}
!20 = !{i32 0, i64 8598323216, i32 4, !21}
!21 = !{i32 1, i32 1, i32 1}

