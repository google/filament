; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"


%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.v8f64 = type { <8 x double>, i32 }
%struct.RWByteAddressBuffer = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  %3 = call %dx.types.ResRet.v8f64 @dx.op.rawBufferVectorLoad.v8f64(i32 303, %dx.types.Handle %2, i32 0, i32 undef, i32 4)  ; RawBufferVectorLoad(buf,index,elementOffset,alignment)
  %4 = extractvalue %dx.types.ResRet.v8f64 %3, 0
  %5 = call %dx.types.ResRet.v8f64 @dx.op.rawBufferVectorLoad.v8f64(i32 303, %dx.types.Handle %2, i32 32, i32 undef, i32 4)  ; RawBufferVectorLoad(buf,index,elementOffset,alignment)
  %6 = extractvalue %dx.types.ResRet.v8f64 %5, 0


; CHECK: Function: main: error: DXIL intrinsic overload must be valid.
; CHECK: note: at '%7 = call double @dx.op.dot.v8f64(i32 311, <8 x double> %4, <8 x double> %6)' in block '#0' of function 'main'.
  %7 = call double @dx.op.dot.v8f64(i32 311, <8 x double> %4, <8 x double> %6)  ; FDot(a,b)


  %8 = extractelement <8 x double> %6, i32 0
  %9 = fadd fast double %8, %7
  %10 = insertelement <8 x double> %6, double %9, i32 0
  call void @dx.op.rawBufferVectorStore.v8f64(i32 304, %dx.types.Handle %2, i32 0, i32 undef, <8 x double> %10, i32 4)  ; RawBufferVectorStore(uav,index,elementOffset,value0,alignment)
  ret void
}

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.v8f64 @dx.op.rawBufferVectorLoad.v8f64(i32, %dx.types.Handle, i32, i32, i32) #0

; Function Attrs: nounwind readnone
declare double @dx.op.dot.v8f64(i32, <8 x double>, <8 x double>) #1

; Function Attrs: nounwind
declare void @dx.op.rawBufferVectorStore.v8f64(i32, %dx.types.Handle, i32, i32, <8 x double>, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #1

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.entryPoints = !{!6}

!0 = !{!"dxc(private) 1.8.0.15017 (main, 4e0f5364a-dirty)"}
!1 = !{i32 1, i32 9}
!2 = !{!"cs", i32 6, i32 9}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 0, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!6 = !{void ()* @main, !"main", null, !3, !7}
!7 = !{i32 0, i64 8598323220, i32 4, !8}
!8 = !{i32 4, i32 1, i32 1}

