; RUN: %dxv %s

; Regression test for validator, it should pass validation.
; See val-dx-type-minprec.hlsl for explanation.

; Note: shader requires additional functionality:
;       Tiled resources
;       Use native low precision
;
;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; OUT                      0   x           0     NONE    fp16   x   
;
; shader hash: 59565e73b1c4f8f3c2db612c5f639ded
;
; Pipeline Runtime Information: 
;
; Vertex Shader
; OutputPositionPresent=0
;
;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; OUT                      0                 linear       
;
; Buffer Definitions:
;
; cbuffer $Globals
; {
;
;   struct $Globals
;   {
;
;       int16_t4 i1;                                  ; Offset:    0
;       int16_t4 i2;                                  ; Offset:    8
;   
;   } $Globals;                                       ; Offset:    0 Size:    16
;
; }
;
; Resource bind info for SB
; {
;
;   struct struct.MyStruct
;   {
;
;       half f;                                       ; Offset:    0
;   
;   } $Element;                                       ; Offset:    0 Size:     2
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0            cb0     1
; SB                                texture  struct         r/o      T0             t0     1
;
;
; ViewId state:
;
; Number of inputs: 0, outputs: 1
; Outputs dependent on ViewId: {  }
; Inputs contributing to computation of Outputs:
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i16.8 = type { i16, i16, i16, i16, i16, i16, i16, i16 }
%dx.types.ResRet.f16 = type { half, half, half, half, i32 }
%"class.StructuredBuffer<MyStruct>" = type { %struct.MyStruct }
%struct.MyStruct = type { half }
%"$Globals" = type { <4 x i16>, <4 x i16> }

; Function Attrs: nounwind readonly
declare i1 @dx.op.checkAccessFullyMapped.i32(i32, i32) #1

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %3 = call %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32 59, %dx.types.Handle %2, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  %4 = extractvalue %dx.types.CBufRet.i16.8 %3, 5
  %5 = sext i16 %4 to i32
  %6 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %1, i32 %5, i32 0, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
  %7 = extractvalue %dx.types.ResRet.f16 %6, 0
  %8 = extractvalue %dx.types.ResRet.f16 %6, 4
  %9 = call i1 @dx.op.checkAccessFullyMapped.i32(i32 71, i32 %8)  ; CheckAccessFullyMapped(status)
  %10 = select i1 %9, half %7, half 0xHBC00
  call void @dx.op.storeOutput.f16(i32 5, i32 0, i32 0, i8 0, half %10)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f16(i32, i32, i32, i8, half) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32, %dx.types.Handle, i32, i32, i8, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i16.8 @dx.op.cbufferLoadLegacy.i16(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.viewIdState = !{!10}
!dx.entryPoints = !{!11}

!0 = !{!"dxc(private) 1.7.0.4154 (dxil-op-cache-init, c8642b4f6-dirty)"}
!1 = !{i32 1, i32 3}
!2 = !{i32 1, i32 7}
!3 = !{!"vs", i32 6, i32 3}
!4 = !{!5, null, !8, null}
!5 = !{!6}
!6 = !{i32 0, %"class.StructuredBuffer<MyStruct>"* undef, !"", i32 0, i32 0, i32 1, i32 12, i32 0, !7}
!7 = !{i32 1, i32 2}
!8 = !{!9}
!9 = !{i32 0, %"$Globals"* undef, !"", i32 0, i32 0, i32 1, i32 16, null}
!10 = !{[2 x i32] [i32 0, i32 1]}
!11 = !{void ()* @main, !"main", !12, !4, !17}
!12 = !{null, !13, null}
!13 = !{!14}
!14 = !{i32 0, !"OUT", i8 8, i8 0, !15, i8 2, i32 1, i8 1, i32 0, i8 0, !16}
!15 = !{i32 0}
!16 = !{i32 3, i32 1}
!17 = !{i32 0, i64 8392752}
