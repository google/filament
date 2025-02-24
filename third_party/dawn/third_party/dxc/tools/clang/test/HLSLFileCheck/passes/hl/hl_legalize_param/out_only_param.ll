; RUN: %opt %s -hl-legalize-parameter -S | FileCheck %s

; Make sure no memcpy generated.
; CHECK-NOT:memcpy

;struct ST {
;  float4 a;
;};
;
;ST CreateST(float a) {
;  ST st;
;  st.a = a;
;  return st;
;}
;
;float a;
;ST GetST() { return CreateST(a); }
;
;float4 main() : SV_Target {
;
;  return GetST().a;
;}

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { float }
%struct.ST = type { <4 x float> }
%dx.types.Handle = type { i8* }

@"\01?a@@3MB" = external constant float, align 4
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define <4 x float> @main() #0 {
entry:
  %tmp = alloca %struct.ST, align 4
  call void @"\01?GetST@@YA?AUST@@XZ"(%struct.ST* sret %tmp)
  %a = getelementptr inbounds %struct.ST, %struct.ST* %tmp, i32 0, i32 0
  %0 = load <4 x float>, <4 x float>* %a, align 4, !tbaa !27
  ret <4 x float> %0
}

; Function Attrs: alwaysinline nounwind
define internal void @"\01?GetST@@YA?AUST@@XZ"(%struct.ST* noalias sret %agg.result) #1 {
entry:
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0)
  %1 = call %"$Globals"* @"dx.hl.subscript.cb.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %0, i32 0)
  %2 = getelementptr inbounds %"$Globals", %"$Globals"* %1, i32 0, i32 0
  %3 = load float, float* %2, align 4, !tbaa !30
  call void @"\01?CreateST@@YA?AUST@@M@Z"(%struct.ST* sret %agg.result, float %3)
  ret void
}

; Function Attrs: alwaysinline nounwind
define internal void @"\01?CreateST@@YA?AUST@@M@Z"(%struct.ST* noalias sret %agg.result, float %a) #1 {
entry:
  %a.addr = alloca float, align 4, !dx.temp !13
  store float %a, float* %a.addr, align 4, !tbaa !30
  %0 = load float, float* %a.addr, align 4, !tbaa !30
  %splat.splatinsert = insertelement <4 x float> undef, float %0, i32 0
  %splat.splat = shufflevector <4 x float> %splat.splatinsert, <4 x float> undef, <4 x i32> zeroinitializer
  %a1 = getelementptr inbounds %struct.ST, %struct.ST* %agg.result, i32 0, i32 0
  store <4 x float> %splat.splat, <4 x float>* %a1, align 4, !tbaa !27
  ret void
}

; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !9}
!dx.entryPoints = !{!20}
!dx.fnprops = !{!24}
!dx.options = !{!25, !26}

!0 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!1 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 6}
!4 = !{!"ps", i32 6, i32 0}
!5 = !{i32 0, %struct.ST undef, !6, %"$Globals" undef, !8}
!6 = !{i32 16, !7}
!7 = !{i32 6, !"a", i32 3, i32 0, i32 7, i32 9}
!8 = !{i32 4, !7}
!9 = !{i32 1, <4 x float> ()* @main, !10, void (%struct.ST*)* @"\01?GetST@@YA?AUST@@XZ", !14, void (%struct.ST*, float)* @"\01?CreateST@@YA?AUST@@M@Z", !17}
!10 = !{!11}
!11 = !{i32 1, !12, !13}
!12 = !{i32 4, !"SV_Target", i32 7, i32 9}
!13 = !{}
!14 = !{!15, !16}
!15 = !{i32 0, !13, !13}
!16 = !{i32 1, !13, !13}
!17 = !{!15, !16, !18}
!18 = !{i32 0, !19, !13}
!19 = !{i32 7, i32 9}
!20 = !{<4 x float> ()* @main, !"main", null, !21, null}
!21 = !{null, null, !22, null}
!22 = !{!23}
!23 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 4, null}
!24 = !{<4 x float> ()* @main, i32 0, i1 false}
!25 = !{i32 144}
!26 = !{i32 -1}
!27 = !{!28, !28, i64 0}
!28 = !{!"omnipotent char", !29, i64 0}
!29 = !{!"Simple C/C++ TBAA"}
!30 = !{!31, !31, i64 0}
!31 = !{!"float", !28, i64 0}
