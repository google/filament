; RUN: %dxilver 1.6 | %D3DReflect %s | FileCheck %s -check-prefixes=RDAT,RDAT16

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%struct.RWByteAddressBuffer = type { i32 }

@"\01?BAB@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4

; Ensure min shader target incorporates optional features used

; RDAT: FunctionTable[{{.*}}] = {

; SM 6.6+

;/////////////////////////////////////////////////////////////////////////////
; ShaderFeatureInfo_DerivativesInMeshAndAmpShaders (0x1000000) = 16777216
; OptFeatureInfo_UsesDerivatives (0x0000010000000000) = FeatureInfo2: 256

; OptFeatureInfo_UsesDerivatives Flag used to indicate derivative use in
; functions, then fixed up for entry functions.
; Val. ver. 1.8 required to recursively check called functions.

; RDAT-LABEL: UnmangledName: "deriv_in_func"
; RDAT:   FeatureInfo1: 0
; RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
; Old: deriv use not tracked
; RDAT16: FeatureInfo2: 0
; RDAT17: FeatureInfo2: 0
; RDAT18: ShaderStageFlag: (Pixel | Compute | Library | Mesh | Amplification | Node)
; Old would not report Compute, Mesh, Amplification, or Node compatibility.
; RDAT16: ShaderStageFlag: (Pixel | Library)
; RDAT17: ShaderStageFlag: (Pixel | Library)
; RDAT17: MinShaderTarget: 0x60060
; RDAT18: MinShaderTarget: 0x60060
; Old: Didn't set min target properly for lib function
; RDAT16: MinShaderTarget: 0x60066

; Function Attrs: noinline nounwind
define void @"\01?deriv_in_func@@YAXV?$vector@M$01@@@Z"(<2 x float> %uv) #0 {
  %1 = load %dx.types.Handle, %dx.types.Handle* @"\01?BAB@@3URWByteAddressBuffer@@A", align 4
  %2 = extractelement <2 x float> %uv, i64 0
  %3 = extractelement <2 x float> %uv, i64 1
  %4 = call float @dx.op.unary.f32(i32 83, float %2)  ; DerivCoarseX(value)
  %5 = call float @dx.op.unary.f32(i32 83, float %3)  ; DerivCoarseX(value)
  %6 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %1)  ; CreateHandleForLib(Resource)
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 4107, i32 0 })  ; AnnotateHandle(res,props)  resource: RWByteAddressBuffer
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %7, i32 0, i32 undef, float %4, float %5, float undef, float undef, i8 3, i32 4)  ; RawBufferStore(uav,index,elementOffset,value0,value1,value2,value3,mask,alignment)
  ret void
}

; RDAT-LABEL: UnmangledName: "deriv_in_mesh"
; RDAT18: FeatureInfo1: (DerivativesInMeshAndAmpShaders)
; Old: missed called function
; RDAT16: FeatureInfo1: 0
; RDAT17: FeatureInfo1: 0
; RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
; Old: deriv use not tracked
; RDAT16: FeatureInfo2: 0
; RDAT17: FeatureInfo2: 0
; RDAT:   ShaderStageFlag: (Mesh)
; RDAT18: MinShaderTarget: 0xd0066
; Old: 6.0
; RDAT16: MinShaderTarget: 0xd0060
; RDAT17: MinShaderTarget: 0xd0060

define void @deriv_in_mesh() {
  %1 = call i32 @dx.op.threadId.i32(i32 93, i32 0)  ; ThreadId(component)
  %2 = call i32 @dx.op.threadId.i32(i32 93, i32 1)  ; ThreadId(component)
  %3 = uitofp i32 %1 to float
  %4 = uitofp i32 %2 to float
  %5 = fmul fast float %3, 1.250000e-01
  %6 = fmul fast float %4, 1.250000e-01
  %7 = insertelement <2 x float> undef, float %5, i32 0
  %8 = insertelement <2 x float> %7, float %6, i32 1
  call void @"\01?deriv_in_func@@YAXV?$vector@M$01@@@Z"(<2 x float> %8)
  ret void
}

; RDAT-LABEL: UnmangledName: "deriv_in_compute"
; RDAT:   FeatureInfo1: 0
; RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
; Old: deriv use not tracked
; RDAT16: FeatureInfo2: 0
; RDAT17: FeatureInfo2: 0
; RDAT:   ShaderStageFlag: (Compute)
; RDAT18: MinShaderTarget: 0x50066
; Old: 6.0
; RDAT16: MinShaderTarget: 0x50060
; RDAT17: MinShaderTarget: 0x50060

define void @deriv_in_compute() {
  %1 = call i32 @dx.op.threadId.i32(i32 93, i32 0)  ; ThreadId(component)
  %2 = call i32 @dx.op.threadId.i32(i32 93, i32 1)  ; ThreadId(component)
  %3 = uitofp i32 %1 to float
  %4 = uitofp i32 %2 to float
  %5 = fmul fast float %3, 1.250000e-01
  %6 = fmul fast float %4, 1.250000e-01
  %7 = insertelement <2 x float> undef, float %5, i32 0
  %8 = insertelement <2 x float> %7, float %6, i32 1
  call void @"\01?deriv_in_func@@YAXV?$vector@M$01@@@Z"(<2 x float> %8)
  ret void
}

; RDAT-LABEL: UnmangledName: "deriv_in_pixel"
; RDAT:   FeatureInfo1: 0
; RDAT18: FeatureInfo2: (Opt_UsesDerivatives)
; Old: deriv use not tracked
; RDAT16: FeatureInfo2: 0
; RDAT17: FeatureInfo2: 0
; RDAT:   ShaderStageFlag: (Pixel)
; RDAT:   MinShaderTarget: 0x60

define void @deriv_in_pixel() {
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %2 = insertelement <2 x float> undef, float %1, i64 0
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = insertelement <2 x float> %2, float %3, i64 1
  call void @"\01?deriv_in_func@@YAXV?$vector@M$01@@@Z"(<2 x float> %4)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadId.i32(i32, i32) #1

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.rawBufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8, i32) #2

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #3

attributes #0 = { noinline nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!14, !16, !20, !23}

!0 = !{!"dxc(private) 1.7.0.4429 (rdat-minsm-check-flags, 9d3b6ba57)"}
!1 = !{i32 1, i32 6}
!2 = !{!"lib", i32 6, i32 6}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?BAB@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"BAB", i32 0, i32 1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!6 = !{i32 1, void (<2 x float>)* @"\01?deriv_in_func@@YAXV?$vector@M$01@@@Z", !7, void ()* @deriv_in_mesh, !12, void ()* @deriv_in_compute, !12, void ()* @deriv_in_pixel, !12}
!7 = !{!8, !10}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{i32 0, !11, !9}
!11 = !{i32 7, i32 9}
!12 = !{!13}
!13 = !{i32 0, !9, !9}
!14 = !{null, !"", null, !3, !15}
!15 = !{i32 0, i64 65552}
!16 = !{void ()* @deriv_in_compute, !"deriv_in_compute", null, null, !17}
!17 = !{i32 8, i32 5, i32 4, !18, i32 5, !19}
!18 = !{i32 8, i32 8, i32 1}
!19 = !{i32 0}
!20 = !{void ()* @deriv_in_mesh, !"deriv_in_mesh", null, null, !21}
!21 = !{i32 8, i32 13, i32 9, !22, i32 5, !19}
!22 = !{!18, i32 0, i32 0, i32 2, i32 0}
!23 = !{void ()* @deriv_in_pixel, !"deriv_in_pixel", !24, null, !27}
!24 = !{!25, null, null}
!25 = !{!26}
!26 = !{i32 0, !"TEXCOORD", i8 9, i8 0, !19, i8 2, i32 1, i8 2, i32 0, i8 0, null}
!27 = !{i32 8, i32 0, i32 5, !19}

; Make sure function-level derivative flag isn't in RequiredFeatureFlags,
; and make sure mesh shader sets required flag.

; RDAT-LABEL: ID3D12LibraryReflection:

; RDAT-LABEL: D3D12_FUNCTION_DESC: Name:
; RDAT-SAME: deriv_in_func
; RDAT:   RequiredFeatureFlags: 0

; RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_compute
; RDAT:   RequiredFeatureFlags: 0

; RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_mesh
; ShaderFeatureInfo_DerivativesInMeshAndAmpShaders (0x1000000) = 16777216
; RDAT18: RequiredFeatureFlags: 0x1000000
; Old: missed called function
; RDAT17: RequiredFeatureFlags: 0

; RDAT-LABEL: D3D12_FUNCTION_DESC: Name: deriv_in_pixel
; RDAT:   RequiredFeatureFlags: 0
