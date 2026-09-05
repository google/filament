; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; Generated with this, and debug info manually stripped:
; utils/hct/ExtractIRForPassTest.py -p dxilgen -o tools/clang/test/DXC/Passes/DxilGen/cb-col-matrix-subscript-dxilgen.ll <source>.hlsl -- -T vs_6_0

; Source HLSL:
; float3x2 f3x2;
; float main(int row : ROW) : OUT {
;     return f3x2[row].y;
; }

; Check cbuffer column-major matrix subscript codegen uses correct alloca size.

; Column-major matrix subscript is lowered in HLOperationLower by:
; Loading one column into an alloca, dynamically indexing that by row,
; then doing the same for the next column.
; Each column is 3 floats for 3 rows. Thus alloca size should be 3 floats.

; Just check the chain for the returned component, skipping the first cbuffer load.
; alloca size should be number of rows (3), not number of columns (2)
; CHECK: %[[alloca:[^ ]+]] = alloca [3 x float]
; Get row index from input:
; CHECK: %[[row:[^ ]+]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
; Load of column 1, skipping column 0:
; CHECK: %[[cbld:[^ ]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{[^,]+}}, i32 1)
; Reconstruct vector from original HL op:
; CHECK-NEXT: %[[cbld_x:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[cbld]], 0
; CHECK-NEXT: %[[v3x:[^ ]+]] = insertelement <3 x float> undef, float %[[cbld_x]], i64 0
; CHECK-NEXT: %[[cbld_y:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[cbld]], 1
; CHECK-NEXT: %[[v3xy:[^ ]+]] = insertelement <3 x float> %[[v3x]], float %[[cbld_y]], i64 1
; CHECK-NEXT: %[[cbld_z:[^ ]+]] = extractvalue %dx.types.CBufRet.f32 %[[cbld]], 2
; CHECK-NEXT: %[[v3xyz:[^ ]+]] = insertelement <3 x float> %[[v3xy]], float %[[cbld_z]], i64 2
; Store y column to alloca:
; CHECK-NEXT: %[[ee:[^ ]+]] = extractelement <3 x float> %[[v3xyz]], i32 0
; CHECK-NEXT: %[[gep_alloca_0:[^ ]+]] = getelementptr inbounds [3 x float], [3 x float]* %[[alloca]], i32 0, i32 0
; CHECK-NEXT: store float %[[ee]], float* %[[gep_alloca_0]]
; CHECK-NEXT: %[[ee:[^ ]+]] = extractelement <3 x float> %[[v3xyz]], i32 1
; CHECK-NEXT: %[[gep_alloca_1:[^ ]+]] = getelementptr inbounds [3 x float], [3 x float]* %[[alloca]], i32 0, i32 1
; CHECK-NEXT: store float %[[ee]], float* %[[gep_alloca_1]]
; CHECK-NEXT: %[[ee:[^ ]+]] = extractelement <3 x float> %[[v3xyz]], i32 2
; CHECK-NEXT: %[[gep_alloca_2:[^ ]+]] = getelementptr inbounds [3 x float], [3 x float]* %[[alloca]], i32 0, i32 2
; CHECK-NEXT: store float %[[ee]], float* %[[gep_alloca_2]]
; Load [row].y from alloca:
; CHECK-NEXT: %[[gep_alloca_row:[^ ]+]] = getelementptr inbounds [3 x float], [3 x float]* %[[alloca]], i32 0, i32 %[[row]]
; CHECK-NEXT: %[[load_row:[^ ]+]] = load float, float* %[[gep_alloca_row]]
; Insert [row].x into vec2 (unused)
; CHECK-NEXT: %[[v2x:[^ ]+]] = insertelement <2 x float> undef, float %{{[^,]+}}, i64 0
; Insert [row].y into vec2
; CHECK-NEXT: %[[v2xy:[^ ]+]] = insertelement <2 x float> %[[v2x]], float %[[load_row]], i64 1
; Extract component for .y output:
; CHECK-NEXT: %[[ee:[^ ]+]] = extractelement <2 x float> %[[v2xy]], i32 1
; CHECK-NEXT: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %[[ee]])

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { %class.matrix.float.3.2 }
%class.matrix.float.3.2 = type { [3 x <2 x float>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind readnone
declare <2 x float>* @"dx.hl.subscript.colMajor[].rn.<2 x float>* (i32, %class.matrix.float.3.2*, i32, i32)"(i32, %class.matrix.float.3.2*, i32, i32) #0

; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"$Globals") #0

; Function Attrs: nounwind
define void @main(float* noalias, i32) #1 {
entry:
  %2 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0)
  %3 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 13, i32 28 }, %"$Globals" undef)
  %4 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %3, i32 0)
  %5 = getelementptr inbounds %"$Globals", %"$Globals"* %4, i32 0, i32 0
  %6 = add i32 3, %1
  %7 = call <2 x float>* @"dx.hl.subscript.colMajor[].rn.<2 x float>* (i32, %class.matrix.float.3.2*, i32, i32)"(i32 1, %class.matrix.float.3.2* %5, i32 %1, i32 %6)
  %8 = load <2 x float>, <2 x float>* %7
  %9 = extractelement <2 x float> %8, i32 1
  store float %9, float* %0
  ret void
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !10}
!dx.entryPoints = !{!19}
!dx.fnprops = !{!23}
!dx.options = !{!24, !25}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.9.0.5175 (dxc-version-print, 93ba0122d-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 10}
!5 = !{!"vs", i32 6, i32 0}
!6 = !{i32 0, %"$Globals" undef, !7}
!7 = !{i32 28, !8}
!8 = !{i32 6, !"f3x2", i32 2, !9, i32 3, i32 0, i32 7, i32 9}
!9 = !{i32 3, i32 2, i32 2}
!10 = !{i32 1, void (float*, i32)* @main, !11}
!11 = !{!12, !14, !17}
!12 = !{i32 0, !13, !13}
!13 = !{}
!14 = !{i32 1, !15, !16}
!15 = !{i32 4, !"OUT", i32 7, i32 9}
!16 = !{i32 0}
!17 = !{i32 0, !18, !16}
!18 = !{i32 4, !"ROW", i32 7, i32 4}
!19 = !{void (float*, i32)* @main, !"main", null, !20, null}
!20 = !{null, null, !21, null}
!21 = !{!22}
!22 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 28, null}
!23 = !{void (float*, i32)* @main, i32 1}
!24 = !{i32 64}
!25 = !{i32 -1}
