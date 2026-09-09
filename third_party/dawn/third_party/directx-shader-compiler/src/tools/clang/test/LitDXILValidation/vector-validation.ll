; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Confirm that 6.9 specific LLVM operations and DXIL intrinsics fail in 6.8

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.v4f32 = type { <4 x float>, i32 }
%"class.RWStructuredBuffer<vector<float, 4> >" = type { <4 x float> }

; CHECK: Function: main: error: Instructions must be of an allowed type.
; CHECK: note: at '%6 = insertelement <4 x float> undef, float %2, i32 0
; CHECK: Function: main: error: Instructions must be of an allowed type.
; CHECK: note: at '%7 = shufflevector <4 x float> %6, <4 x float> undef, <4 x i32> zeroinitializer
; CHECK: Function: main: error: Instructions must be of an allowed type.
; CHECK: note: at '%8 = extractelement <4 x float> %5, i32 2
; CHECK: Function: main: error: Opcode RawBufferVectorLoad not valid in shader model vs_6_8.
; CHECK: note: at '%4 = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %3, i32 1, i32 0, i32 8)'
; CHECK: Function: main: error: Opcode RawBufferVectorStore not valid in shader model vs_6_8.
; CHECK: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %3, i32 0, i32 0, <4 x float> %7, i32 4)'
; CHECK: Function: main: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK: Function: main: error: Function uses features incompatible with the shader model.
define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4108, i32 16 })
  %4 = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %3, i32 1, i32 0, i32 8)
  %5 = extractvalue %dx.types.ResRet.v4f32 %4, 0
  %6 = insertelement <4 x float> undef, float %2, i32 0
  %7 = shufflevector <4 x float> %6, <4 x float> undef, <4 x i32> zeroinitializer
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %3, i32 0, i32 0, <4 x float> %7, i32 4)
  %8 = extractelement <4 x float> %5, i32 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %8)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %8)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %8)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %8)
  ret void
}

declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1
declare %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32, %dx.types.Handle, i32, i32, i32) #2
declare void @dx.op.rawBufferVectorStore.v4f32(i32, %dx.types.Handle, i32, i32, <4 x float>, i32) #1
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!7}
!dx.entryPoints = !{!8}

!1 = !{i32 1, i32 8}
!2 = !{!"vs", i32 6, i32 8}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %"class.RWStructuredBuffer<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !6}
!6 = !{i32 1, i32 16}
!7 = !{[3 x i32] [i32 1, i32 4, i32 0]}
!8 = !{void ()* @main, !"main", !9, !3, !17}
!9 = !{!10, !14, null}
!10 = !{!11}
!11 = !{i32 0, !"VAL", i8 9, i8 0, !12, i8 0, i32 1, i8 1, i32 0, i8 0, !13}
!12 = !{i32 0}
!13 = !{i32 3, i32 1}
!14 = !{!15}
!15 = !{i32 0, !"SV_Position", i8 9, i8 3, !12, i8 4, i32 1, i8 4, i32 0, i8 0, !16}
!16 = !{i32 3, i32 15}
!17 = !{i32 0, i64 8590000144}

