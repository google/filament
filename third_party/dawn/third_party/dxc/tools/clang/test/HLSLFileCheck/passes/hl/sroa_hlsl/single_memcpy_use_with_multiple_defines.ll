; RUN: %opt %s -scalarrepl-param-hlsl -S | FileCheck %s

; Make sure memcpy which dst only defined once in the memcpy, src only used once in the memcpy got removed by replacement.
; As a result, one of the alloca will be removed.
; Based on tools\clang\test\HLSLFileCheck\shader_targets\library\lib_skip_copy_in.hlsl

; CHECK: alloca [2 x %class.matrix.float
; CHECK: alloca [2 x %class.matrix.float
; CHECK-NOT: alloca [2 x %class.matrix.float

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { i32 }
%class.matrix.float.2.2.Col = type { [2 x <2 x float>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%class.matrix.float.2.2.Row = type { [2 x <2 x float>] }

@"\01?idx@@3HB" = external constant i32, align 4
@"$Globals" = external constant %"$Globals"


; Function Attrs: nounwind readonly
declare %class.matrix.float.2.2.Col @"dx.hl.matldst.colLoad.%class.matrix.float.2.2.Col (i32, %class.matrix.float.2.2.Col*)"(i32, %class.matrix.float.2.2.Col*) #1

; Function Attrs: nounwind
declare %class.matrix.float.2.2.Col @"dx.hl.matldst.colStore.%class.matrix.float.2.2.Col (i32, %class.matrix.float.2.2.Col*, %class.matrix.float.2.2.Col)"(i32, %class.matrix.float.2.2.Col*, %class.matrix.float.2.2.Col) #2

; Function Attrs: nounwind
define %class.matrix.float.2.2.Col @main() #2 {
entry:

  %tmp_mat_array = alloca [2 x %class.matrix.float.2.2.Col]
  %ID = alloca i32
  %retval.i = alloca %class.matrix.float.2.2.Col, align 4, !dx.temp !13
  %arr = alloca [2 x %class.matrix.float.2.2.Row], align 4

  %agg.tmp = alloca [2 x %class.matrix.float.2.2.Col], align 4


  %arr_0_gep = getelementptr inbounds [2 x %class.matrix.float.2.2.Row], [2 x %class.matrix.float.2.2.Row]* %arr, i32 0, i32 0
  %arr_0_x_ptr = call float* @"dx.hl.subscript.rowMajor_m.float* (i32, %class.matrix.float.2.2.Row*, <1 x i32>)"(i32 4, %class.matrix.float.2.2.Row* %arr_0_gep, <1 x i32> zeroinitializer)

  %arr_1_gep = getelementptr inbounds [2 x %class.matrix.float.2.2.Row], [2 x %class.matrix.float.2.2.Row]* %arr, i32 0, i32 1
  %arr_1_x_ptr = call float* @"dx.hl.subscript.rowMajor_m.float* (i32, %class.matrix.float.2.2.Row*, <1 x i32>)"(i32 4, %class.matrix.float.2.2.Row* %arr_1_gep, <1 x i32> zeroinitializer)

  %arr_0_x = load float, float* %arr_0_x_ptr
  %arr_1_x = load float, float* %arr_1_x_ptr
  
  %agg_tmp_0_gep = getelementptr inbounds [2 x %class.matrix.float.2.2.Col], [2 x %class.matrix.float.2.2.Col]* %agg.tmp, i32 0, i32 0
  %agg_tmp_0_x_ptr = call float* @"dx.hl.subscript.colMajor_m.float* (i32, %class.matrix.float.2.2.Col*, <1 x i32>)"(i32 3, %class.matrix.float.2.2.Col* %agg_tmp_0_gep, <1 x i32> zeroinitializer)
  %agg_tmp_1_gep = getelementptr inbounds [2 x %class.matrix.float.2.2.Col], [2 x %class.matrix.float.2.2.Col]* %agg.tmp, i32 0, i32 1
  %agg_tmp_1_x_ptr = call float* @"dx.hl.subscript.colMajor_m.float* (i32, %class.matrix.float.2.2.Col*, <1 x i32>)"(i32 3, %class.matrix.float.2.2.Col* %agg_tmp_1_gep, <1 x i32> zeroinitializer)
  store float %arr_0_x, float* %agg_tmp_0_x_ptr
  
  store float %arr_1_x, float* %agg_tmp_1_x_ptr
  %memcpy_dst = bitcast [2 x %class.matrix.float.2.2.Col]* %tmp_mat_array to i8*
  %memcpy_src = bitcast [2 x %class.matrix.float.2.2.Col]* %agg.tmp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %memcpy_dst, i8* %memcpy_src, i64 32, i32 1, i1 false)
  %index_ld = load i32, i32* %ID, align 4

  %arrayidx.i = getelementptr inbounds [2 x %class.matrix.float.2.2.Col], [2 x %class.matrix.float.2.2.Col]* %tmp_mat_array, i32 0, i32 %index_ld
  %mat_array_elt_ld = call %class.matrix.float.2.2.Col @"dx.hl.matldst.colLoad.%class.matrix.float.2.2.Col (i32, %class.matrix.float.2.2.Col*)"(i32 0, %class.matrix.float.2.2.Col* %arrayidx.i) #2
  ret %class.matrix.float.2.2.Col %mat_array_elt_ld
}

; Function Attrs: nounwind readnone
declare %class.matrix.float.2.2.Col @"dx.hl.cast.default.%class.matrix.float.2.2.Col (i32, float)"(i32, float) #3

; Function Attrs: nounwind readnone
declare %class.matrix.float.2.2.Row @"dx.hl.cast.colMatToRowMat.%class.matrix.float.2.2.Row (i32, %class.matrix.float.2.2.Col)"(i32, %class.matrix.float.2.2.Col) #3

; Function Attrs: nounwind
declare %class.matrix.float.2.2.Row @"dx.hl.matldst.rowStore.%class.matrix.float.2.2.Row (i32, %class.matrix.float.2.2.Row*, %class.matrix.float.2.2.Row)"(i32, %class.matrix.float.2.2.Row*, %class.matrix.float.2.2.Row) #2

; Function Attrs: nounwind readnone
declare float* @"dx.hl.subscript.rowMajor_m.float* (i32, %class.matrix.float.2.2.Row*, <1 x i32>)"(i32, %class.matrix.float.2.2.Row*, <1 x i32>) #3

; Function Attrs: nounwind readnone
declare float* @"dx.hl.subscript.colMajor_m.float* (i32, %class.matrix.float.2.2.Col*, <1 x i32>)"(i32, %class.matrix.float.2.2.Col*, <1 x i32>) #3

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #2


; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #3

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #3

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"$Globals") #3

attributes #0 = { alwaysinline nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !8}
!dx.entryPoints = !{!14}
!dx.fnprops = !{!18}
!dx.options = !{!19, !20}

!0 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!1 = !{!"dxc(private) 1.7.0.3875 (mat_orientation, b71d96c0b)"}
!2 = !{i32 1, i32 3}
!3 = !{i32 1, i32 7}
!4 = !{!"lib", i32 6, i32 3}
!5 = !{i32 0, %"$Globals" undef, !6}
!6 = !{i32 4, !7}
!7 = !{i32 6, !"idx", i32 3, i32 0, i32 7, i32 4}
!8 = !{i32 1, %class.matrix.float.2.2.Col ()* @main, !11}

!9 = !{i32 2, i32 2, i32 2}
!10 = !{}

!11 = !{!12}
!12 = !{i32 1, !13, !10}
!13 = !{i32 2, !9, i32 4, !"OUT", i32 7, i32 9}
!14 = !{null, !"", null, !15, null}
!15 = !{null, null, !16, null}
!16 = !{!17}
!17 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 4, null}
!18 = !{%class.matrix.float.2.2.Col ()* @main, i32 1}
!19 = !{i32 152}
!20 = !{i32 -1}
