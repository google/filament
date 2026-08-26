; RUN: %dxv %s | FileCheck %s

; CHECK: DXIL intrinsic overload must be valid
; Change dx.op.loadInput.i32(i32 4 to dx.op.loadInput.i32(i32 3

;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; A                        0   xyzw        0     NONE     int
;
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET     int   xyzw
;
;
; Pipeline Runtime Information:
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Input signature:
;
; Name                 Index             InterpMode
; -------------------- ----- ----------------------
; A                        0        nointerpolation
;
; Output signature:
;
; Name                 Index             InterpMode
; -------------------- ----- ----------------------
; SV_Target                0
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
;
target datalayout = "e-m:e-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define void @"\01?main@@YA?AV?$vector@H$03@@V1@@Z.flat"(<4 x i32>, <4 x i32>* nocapture readnone) #0 {
entry:
  %2 = tail call i32 @dx.op.loadInput.i32(i32 3, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %3 = tail call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = tail call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = tail call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %.i0 = xor i32 %3, -2147483648
  %.i1 = xor i32 %2, -2147483648
  %add.i0 = add i32 %.i0, %2
  %add.i1 = add i32 %3, %.i1
  %add.i2 = add i32 %4, %.i1
  %add.i3 = add i32 %5, %.i1
  tail call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %add.i0)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  tail call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 %add.i1)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  tail call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 %add.i2)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  tail call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 %add.i3)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.storeOutput.i32(i32, i32, i32, i8, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!12}

!0 = !{!"clang version 3.7.0 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{!"ps", i32 6, i32 0}
!3 = !{i32 1, void (<4 x i32>, <4 x i32>*)* @"\01?main@@YA?AV?$vector@H$03@@V1@@Z.flat", !4}
!4 = !{!5, !7, !10}
!5 = !{i32 0, !6, !6}
!6 = !{}
!7 = !{i32 0, !8, !9}
!8 = !{i32 4, !"A", i32 7, i32 4}
!9 = !{i32 0}
!10 = !{i32 1, !11, !9}
!11 = !{i32 4, !"SV_TARGET", i32 7, i32 4}
!12 = !{void (<4 x i32>, <4 x i32>*)* @"\01?main@@YA?AV?$vector@H$03@@V1@@Z.flat", !"", !13, null, null}
!13 = !{!14, !16, null}
!14 = !{!15}
!15 = !{i32 0, !"A", i8 4, i8 0, !9, i8 1, i32 1, i8 4, i32 0, i8 0, null}
!16 = !{!17}
!17 = !{i32 0, !"SV_Target", i8 4, i8 16, !9, i8 0, i32 1, i8 4, i32 0, i8 0, null}

