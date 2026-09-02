; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; Verify that Sample called from vertex shader is not allowed.
; test generated from deriv-in-nested-fn-vs.hlsl

; CHECK: Function: main: error: Entry function calls one or more functions using incompatible features.  See other errors for details.
; CHECK: Function: {{.*}}fn_sample{{.*}}: error: Function uses features incompatible with the shader stage (vs) of the entry function.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%"class.RWBuffer<vector<float, 4> >" = type { <4 x float> }
%struct.SamplerState = type { i32 }

@"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A" = external constant %dx.types.Handle, align 4
@"\01?S@@3USamplerState@@A" = external constant %dx.types.Handle, align 4
@"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A" = external constant %dx.types.Handle, align 4

; Function Attrs: noinline nounwind
define void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> %value) #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A", align 4
  %2 = extractelement <4 x float> %value, i32 0
  %3 = fptoui float %2 to i32
  %4 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %5 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 4106, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<4xF32>
  %6 = extractelement <4 x float> %value, i64 0
  %7 = extractelement <4 x float> %value, i64 1
  %8 = extractelement <4 x float> %value, i64 2
  %9 = extractelement <4 x float> %value, i64 3
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %5, i32 %3, i32 undef, float %6, float %7, float %8, float %9, i8 15)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  ret void
}

; Function Attrs: noinline nounwind
define void @"\01?fn_sample@@YAXXZ"() #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?S@@3USamplerState@@A", align 4
  %2 = load %dx.types.Handle, %dx.types.Handle* @"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A", align 4
  %3 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %2)  ; CreateHandleForLib(Resource)
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
  %5 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %6 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 14, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerState
  %7 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %4, %dx.types.Handle %6, float 0.000000e+00, float 0.000000e+00, float undef, float undef, i32 0, i32 0, i32 undef, float undef)  ; Sample(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,clamp)
  %8 = extractvalue %dx.types.ResRet.f32 %7, 0
  %9 = insertelement <4 x float> undef, float %8, i64 0
  %10 = extractvalue %dx.types.ResRet.f32 %7, 1
  %11 = insertelement <4 x float> %9, float %10, i64 1
  %12 = extractvalue %dx.types.ResRet.f32 %7, 2
  %13 = insertelement <4 x float> %11, float %12, i64 2
  %14 = extractvalue %dx.types.ResRet.f32 %7, 3
  %15 = insertelement <4 x float> %13, float %14, i64 3
  call void @"\01?write_value@@YAXV?$vector@M$03@@@Z"(<4 x float> %15)
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

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #3

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!11}
!dx.entryPoints = !{!18, !20}

!0 = !{!"dxc(private) 1.8.0.4482 (val-compat-calls, 055b660e4)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{!4, !7, null, !9}
!4 = !{!5}
!5 = !{i32 0, %"class.Texture2D<vector<float, 4> >"* bitcast (%dx.types.Handle* @"\01?T@@3V?$Texture2D@V?$vector@M$03@@@@A" to %"class.Texture2D<vector<float, 4> >"*), !"T", i32 0, i32 0, i32 1, i32 2, i32 0, !6}
!6 = !{i32 0, i32 9}
!7 = !{!8}
!8 = !{i32 0, %"class.RWBuffer<vector<float, 4> >"* bitcast (%dx.types.Handle* @"\01?Buf@@3V?$RWBuffer@V?$vector@M$03@@@@A" to %"class.RWBuffer<vector<float, 4> >"*), !"Buf", i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, !6}
!9 = !{!10}
!10 = !{i32 0, %struct.SamplerState* bitcast (%dx.types.Handle* @"\01?S@@3USamplerState@@A" to %struct.SamplerState*), !"S", i32 0, i32 0, i32 1, i32 0, null}
!11 = !{i32 1, void (<4 x float>)* @"\01?write_value@@YAXV?$vector@M$03@@@Z", !12, void ()* @"\01?fn_sample@@YAXXZ", !17, void ()* @"\01?intermediate@@YAXXZ", !17, void ()* @main, !17}
!12 = !{!13, !15}
!13 = !{i32 1, !14, !14}
!14 = !{}
!15 = !{i32 0, !16, !14}
!16 = !{i32 7, i32 9, i32 13, i32 4}
!17 = !{!13}
!18 = !{null, !"", null, !3, !19}
!19 = !{i32 0, i64 8589934592}
!20 = !{void ()* @main, !"main", null, null, !21}
!21 = !{i32 8, i32 1, i32 5, !22}
!22 = !{i32 0}
