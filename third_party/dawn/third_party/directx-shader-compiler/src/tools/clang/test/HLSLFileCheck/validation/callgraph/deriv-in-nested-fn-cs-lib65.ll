; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; Verify that Sample called from compute shader is not allowed when shader
; model is less than 6.6 when compiled to library.
; test generated from deriv-in-nested-fn-cs65.hlsl

; CHECK: Function: main: error: Entry function calls one or more functions using incompatible features.  See other errors for details.
; CHECK: Function: {{.*}}fn_sample{{.*}}: error: Function uses derivatives in compute-model shader, which is only supported in shader model 6.6 and above.

; This next error is also emitted because when the derivative usage is combined
; with the shader stage at the entry point, it results in setting the minimum
; shader model to 6.6.  It is seen as an independent conflict in a way.
; It would be difficult to mask it off without missing other conflicts that
; could have independently caused the minimum shader model to be set higher.
; CHECK: Function: main: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK: Function: main: error: Function uses features incompatible with the shader model.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%struct.SamplerState = type { i32 }
%"class.RWBuffer<vector<float, 4> >" = type { <4 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }

@"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A" = external constant %"class.Texture2D<vector<float, 4> >", align 4
@"\01?S@@3USamplerState@@A" = external constant %struct.SamplerState, align 4
@"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A" = external constant %"class.RWBuffer<vector<float, 4> >", align 4

; Function Attrs: noinline nounwind
define void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> %value) #0 {
  %1 = load %"class.RWBuffer<vector<float, 4> >", %"class.RWBuffer<vector<float, 4> >"* @"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A", align 4
  %2 = extractelement <4 x float> %value, i32 0
  %3 = fptoui float %2 to i32
  %4 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 4> >"(i32 160, %"class.RWBuffer<vector<float, 4> >" %1)  ; CreateHandleForLib(Resource)
  %5 = extractelement <4 x float> %value, i64 0
  %6 = extractelement <4 x float> %value, i64 1
  %7 = extractelement <4 x float> %value, i64 2
  %8 = extractelement <4 x float> %value, i64 3
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %4, i32 %3, i32 undef, float %5, float %6, float %7, float %8, i8 15)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  ret void
}

; Function Attrs: noinline nounwind
define void @"\01?fn_sample@@YAXXZ"() #0 {
  %1 = load %struct.SamplerState, %struct.SamplerState* @"\01?S@@3USamplerState@@A", align 4
  %2 = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* @"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A", align 4
  %3 = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %2)  ; CreateHandleForLib(Resource)
  %4 = call %dx.types.Handle @dx.op.createHandleForLib.struct.SamplerState(i32 160, %struct.SamplerState %1)  ; CreateHandleForLib(Resource)
  %5 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %3, %dx.types.Handle %4, float 0.000000e+00, float 0.000000e+00, float undef, float undef, i32 0, i32 0, i32 undef, float undef)  ; Sample(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,clamp)
  %6 = extractvalue %dx.types.ResRet.f32 %5, 0
  %7 = insertelement <4 x float> undef, float %6, i64 0
  %8 = extractvalue %dx.types.ResRet.f32 %5, 1
  %9 = insertelement <4 x float> %7, float %8, i64 1
  %10 = extractvalue %dx.types.ResRet.f32 %5, 2
  %11 = insertelement <4 x float> %9, float %10, i64 2
  %12 = extractvalue %dx.types.ResRet.f32 %5, 3
  %13 = insertelement <4 x float> %11, float %12, i64 3
  call void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> %13)
  ret void
}

; Function Attrs: noinline nounwind
define void @"\01?intermediate@@YAXXZ"() #0 {
  call void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>)
  call void @"\01?fn_sample@@YAXXZ"()
  call void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>)
  ret void
}

; Function Attrs: nounwind
define void @main() #1 {
  call void @"\01?intermediate@@YAXXZ"()
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sample.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 4> >"(i32, %"class.RWBuffer<vector<float, 4> >") #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2D<vector<float, 4> >"(i32, %"class.Texture2D<vector<float, 4> >") #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.struct.SamplerState(i32, %struct.SamplerState) #2

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.typeAnnotations = !{!12}
!dx.entryPoints = !{!19, !20}

!0 = !{!"dxc(private) 1.8.0.4482 (val-compat-calls, 055b660e4)"}
!1 = !{i32 1, i32 5}
!2 = !{i32 1, i32 8}
!3 = !{!"lib", i32 6, i32 5}
!4 = !{!5, !8, null, !10}
!5 = !{!6}
!6 = !{i32 0, %"class.Texture2D<vector<float, 4> >"* @"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A", !"T", i32 0, i32 0, i32 1, i32 2, i32 0, !7}
!7 = !{i32 0, i32 9}
!8 = !{!9}
!9 = !{i32 0, %"class.RWBuffer<vector<float, 4> >"* @"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A", !"Buf", i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, !7}
!10 = !{!11}
!11 = !{i32 0, %struct.SamplerState* @"\01?S@@3USamplerState@@A", !"S", i32 0, i32 0, i32 1, i32 0, null}
!12 = !{i32 1, void (<4 x float>)* @"\01?write_value@@YAXV?$vector@M$03@@@Z", !13, void ()* @"\01?fn_sample@@YAXXZ", !18, void ()* @"\01?intermediate@@YAXXZ", !18, void ()* @main, !18}
!13 = !{!14, !16}
!14 = !{i32 1, !15, !15}
!15 = !{}
!16 = !{i32 0, !17, !15}
!17 = !{i32 7, i32 9}
!18 = !{!14}
!19 = !{null, !"", null, !4, null}
!20 = !{void ()* @main, !"main", null, null, !21}
!21 = !{i32 8, i32 5, i32 4, !22, i32 5, !23}
!22 = !{i32 8, i32 8, i32 1}
!23 = !{i32 0}
