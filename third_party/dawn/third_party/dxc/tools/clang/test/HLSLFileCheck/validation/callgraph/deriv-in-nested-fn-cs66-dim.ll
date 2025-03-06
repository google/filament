; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; Verify that Sample called from compute shader is not allowed when thread
; group size is not compatible with derivatives.

; CHECK: Function: main: error: Entry function calls one or more functions using incompatible features.  See other errors for details.
; CHECK: Function: {{.*}}fn_sample{{.*}}: error: Function uses derivatives in compute-model shader with NumThreads (8, 7, 1); derivatives require NumThreads to be 1D and a multiple of 4, or 2D/3D with X and Y both being a multiple of 2.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%"class.RWBuffer<vector<float, 4> >" = type { <4 x float> }
%struct.SamplerState = type { i32 }

define void @main() {
  call fastcc void @"\01?intermediate@@YAXXZ"()
  ret void
}

; Function Attrs: noinline nounwind
define internal fastcc void @"\01?intermediate@@YAXXZ"() #0 {
  call fastcc void @"\01?write_value@@YAXM@Z"(float 1.000000e+00)
  call fastcc void @"\01?fn_sample@@YAXXZ"()
  call fastcc void @"\01?write_value@@YAXM@Z"(float 2.000000e+00)
  ret void
}

; Function Attrs: noinline nounwind
define internal fastcc void @"\01?write_value@@YAXM@Z"(float %value) #0 {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = fptoui float %value to i32
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4106, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<4xF32>
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %3, i32 %2, i32 undef, float %value, float %value, float %value, float %value, i8 15)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  ret void
}

; Function Attrs: noinline nounwind
define internal fastcc void @"\01?fn_sample@@YAXXZ"() #0 {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
  %4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 14, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerState
  %5 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %3, %dx.types.Handle %4, float 0.000000e+00, float 0.000000e+00, float undef, float undef, i32 0, i32 0, i32 undef, float undef)  ; Sample(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,clamp)
  %6 = extractvalue %dx.types.ResRet.f32 %5, 0
  call fastcc void @"\01?write_value@@YAXM@Z"(float %6)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sample.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #3

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #3

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.entryPoints = !{!12}

!0 = !{!"dxc(private) 1.8.0.4484 (val-compat-calls, 9e4ee71a0)"}
!1 = !{i32 1, i32 6}
!2 = !{i32 1, i32 8}
!3 = !{!"cs", i32 6, i32 6}
!4 = !{!5, !8, null, !10}
!5 = !{!6}
!6 = !{i32 0, %"class.Texture2D<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 2, i32 0, !7}
!7 = !{i32 0, i32 9}
!8 = !{!9}
!9 = !{i32 0, %"class.RWBuffer<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, !7}
!10 = !{!11}
!11 = !{i32 0, %struct.SamplerState* undef, !"", i32 0, i32 0, i32 1, i32 0, null}
!12 = !{void ()* @main, !"main", null, !4, !13}
!13 = !{i32 4, !14}
!14 = !{i32 8, i32 7, i32 1}
