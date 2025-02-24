; RUN: %dxopt %s -dxil-annotate-with-virtual-regs -hlsl-dxil-debug-instrumentation -S | FileCheck %s

; Expect an i25 cast, or this test isn't testing anything:
; CHECK: [[CAST:%.*]] = trunc i32 %{{.*}} to i25
;
; Check that we correctly z-extended that i25 before trying to write it to i32
; [[ZEXT:%.*]] = zext i25 [[CAST]] to i32
; call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %PIX_DebugUAV_Handle, i32 %{{.*}}, i32 undef, i32 [[ZEXT]]

; GENERATED FROM:
; dxc -Emain -Tps_6_1

; uint param;
; 
; bool fn()
; {
;     switch (param)
;     {
;     case 0:
;     case 20:
;     case 24:
;         return false;
;     }
;     return true;
; }
; 
; float4 main() : SV_Target
; {
;     float4 ret = float4(0, 0, 0, 0);
;     if (fn())
;     {
;         ret = float4(1, 1, 1, 1);
;     }
;     return ret;
; }



target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%"$Globals" = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %2 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %1, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  %3 = extractvalue %dx.types.CBufRet.i32 %2, 0
  %4 = icmp ult i32 %3, 25
  br i1 %4, label %5, label %11

; <label>:5                                       ; preds = %0
  %6 = trunc i32 %3 to i25
  %7 = lshr i25 15728638, %6
  %8 = and i25 %7, 1
  %9 = icmp ne i25 %8, 0
  %10 = select i1 %9, float 1.000000e+00, float 0.000000e+00
  br label %11

; <label>:11                                      ; preds = %5, %0
  %12 = phi float [ %10, %5 ], [ 1.000000e+00, %0 ]
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %12)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %12)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %12)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %12)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.viewIdState = !{!7}
!dx.entryPoints = !{!8}

!0 = !{!"dxc(private) 1.8.0.4522 (PIX_InputSigValues, 313ba88f3)"}
!1 = !{i32 1, i32 1}
!2 = !{i32 1, i32 8}
!3 = !{!"ps", i32 6, i32 1}
!4 = !{null, null, !5, null}
!5 = !{!6}
!6 = !{i32 0, %"$Globals"* undef, !"", i32 0, i32 0, i32 1, i32 4, null}
!7 = !{[2 x i32] [i32 0, i32 4]}
!8 = !{void ()* @main, !"main", !9, !4, null}
!9 = !{null, !10, null}
!10 = !{!11}
!11 = !{i32 0, !"SV_Target", i8 9, i8 16, !12, i8 0, i32 1, i8 4, i32 0, i8 0, !13}
!12 = !{i32 0}
!13 = !{i32 3, i32 15}
