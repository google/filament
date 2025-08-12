; REQUIRES: dxil-1-9
; RUN: not %dxv %s 2>&1 | FileCheck %s

; Ensure proper validation errors are produced for invalid parameters to load and store operations.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%dx.types.ResRet.v4f32 = type { <4 x float>, i32 }
%"class.Texture1D<vector<float, 4> >" = type { <4 x float>, %"class.Texture1D<vector<float, 4> >::mips_type" }
%"class.Texture1D<vector<float, 4> >::mips_type" = type { i32 }
%"class.StructuredBuffer<vector<float, 4> >" = type { <4 x float> }
%"class.StructuredBuffer<float>" = type { float }
%struct.ByteAddressBuffer = type { i32 }
%"class.RWStructuredBuffer<vector<float, 4> >" = type { <4 x float> }
%"class.RWStructuredBuffer<float>" = type { float }
%struct.RWByteAddressBuffer = type { i32 }
%struct.SamplerState = type { i32 }

; Unfortunately, the validation errors come in weird orders.
; Inlining them isn't helpful, so we'll just dump them all here.
; Inline comments, variable names, and notes should help find the corresponding source.

; CHECK: error: raw/typed buffer offset must be undef.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp44, i32 0, i32 0, float %badBabOff, float undef, float undef, float undef, i8 1, i32 4)'
; CHECK: error: Assignment of undefined values to UAV.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp42, i32 4, i32 0, float undef, float undef, float undef, float undef, i8 1, i32 4)
; CHECK: error: structured buffer requires defined index and offset coordinates.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp41, i32 3, i32 undef, float %badStrOff, float undef, float undef, float undef, i8 1, i32 4)
; CHECK: error: Raw Buffer alignment value must be a constant.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp40, i32 2, i32 0, float %badAln, float undef, float undef, float undef, i8 1, i32 %ix)'
; CHECK: error: buffer load/store only works on Raw/Typed/StructuredBuffer.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %rwTex, i32 1, i32 0, float %badRK, float undef, float undef, float undef, i8 1, i32 4)'
; CHECK: error: store should be on uav resource.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %scalBuf, i32 0, i32 0, float %badRC, float undef, float undef, float undef, i8 1, i32 4)'

; CHECK: error: raw/typed buffer offset must be undef.
; CHECK-NEXT: note: at '%badBabOffLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %baBuf, i32 0, i32 0, i8 1, i32 4)'
; CHECK: error: structured buffer requires defined index and offset coordinates.
; CHECK-NEXT: note: at '%badStrOffLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %scalBuf, i32 3, i32 undef, i8 1, i32 4)'
; CHECK: error: Raw Buffer alignment value must be a constant.
; CHECK-NEXT: note: at '%badAlnLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %scalBuf, i32 2, i32 0, i8 1, i32 %ix)'
; CHECK: error: buffer load/store only works on Raw/Typed/StructuredBuffer
; CHECK-NEXT: note: at '%badRKLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tex, i32 1, i32 0, i8 1, i32 4)'
; CHECK: error: load can only run on UAV/SRV resource.
; CHECK-NEXT: note: at '%badRCLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %samp, i32 0, i32 0, i8 1, i32 4)'
; CHECK-NEXT: error: buffer load/store only works on Raw/Typed/StructuredBuffer.
; CHECK-NEXT: note: at '%badRCLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %samp, i32 0, i32 0, i8 1, i32 4)'

; CHECK: error: raw/typed buffer offset must be undef.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp51, i32 4, i32 0, <4 x float> %badBabOffVc, i32 4)'
; CHECK: error: Assignment of undefined values to UAV.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp49, i32 4, i32 0, <4 x float> undef, i32 4)'
; CHECK: error: structured buffer requires defined index and offset coordinates.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp48, i32 3, i32 undef, <4 x float> %badStrOffVc, i32 4)'
; CHECK: error: Raw Buffer alignment value must be a constant.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp47, i32 2, i32 0, <4 x float> %badAlnVc, i32 %ix)'
; CHECK: error: buffer load/store only works on Raw/Typed/StructuredBuffer.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %rwTex, i32 1, i32 0, <4 x float> %badRKVc, i32 4)'
; CHECK: error: store should be on uav resource.
; CHECK-NEXT: note: at 'call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %vecBuf, i32 0, i32 0, <4 x float> %badRCVc, i32 4)'

; CHECK: error: raw/typed buffer offset must be undef.
; CHECK-NEXT: note: at '%badBabOffVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %baBuf, i32 4, i32 0, i32 4)'
; CHECK: error: structured buffer requires defined index and offset coordinates.
; CHECK-NEXT: note: at '%badStrOffVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %vecBuf, i32 3, i32 undef, i32 4)'
; CHECK: error: Raw Buffer alignment value must be a constant.
; CHECK-NEXT: note: at '%badAlnVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %vecBuf, i32 2, i32 0, i32 %ix)'
; CHECK: error: buffer load/store only works on Raw/Typed/StructuredBuffer
; CHECK-NEXT: note: at '%badRKVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %tex, i32 1, i32 0, i32 4)'
; CHECK: error: load can only run on UAV/SRV resource.
; CHECK-NEXT: note: at '%badRCVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %samp, i32 0, i32 0, i32 4)'
; CHECK-NEXT: error: buffer load/store only works on Raw/Typed/StructuredBuffer.
; CHECK-NEXT: note: at '%badRCVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %samp, i32 0, i32 0, i32 4)'

define void @main() {
bb:
  %tmp = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 1 }, i32 2, i1 false)
  %tmp1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 1 }, i32 1, i1 false)
  %tmp2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)
  %tmp3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 0 }, i32 3, i1 false)
  %tmp4 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
  %tmp5 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
  %tmp6 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)
  %tmp7 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)
  %tmp8 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 1 }, i32 0, i1 false)
  %ix = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %texIx = sitofp i32 %ix to float
  %tex = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp6, %dx.types.ResourceProperties { i32 1, i32 1033 })
  %samp = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp7, %dx.types.ResourceProperties { i32 14, i32 0 })
  %tmp10 = call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, %dx.types.Handle %tex, %dx.types.Handle %samp, float %texIx, float undef, float undef, float undef, i32 0, i32 undef, i32 undef, float undef)
  %tmp11 = extractvalue %dx.types.ResRet.f32 %tmp10, 0
  %tmp12 = extractvalue %dx.types.ResRet.f32 %tmp10, 1
  %tmp13 = extractvalue %dx.types.ResRet.f32 %tmp10, 2
  %tmp14 = extractvalue %dx.types.ResRet.f32 %tmp10, 3
  %rwTex = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp8, %dx.types.ResourceProperties { i32 4097, i32 1033 })
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %rwTex, i32 0, i32 undef, i32 undef, float %tmp11, float %tmp12, float %tmp13, float %tmp14, i8 15)
  %scalBuf = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp4, %dx.types.ResourceProperties { i32 12, i32 4 })
  ; Invalid RC on Load (and inevitably invalid RK).
  %badRCLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %samp, i32 0, i32 0, i8 1, i32 4)
  %badRC = extractvalue %dx.types.ResRet.f32 %badRCLd, 0
  ; Invalid RK on Load.
  %badRKLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tex, i32 1, i32 0, i8 1, i32 4)
  %badRK = extractvalue %dx.types.ResRet.f32 %badRKLd, 0
  ; Non-constant alignment on Load.
  %badAlnLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %scalBuf, i32 2, i32 0, i8 1, i32 %ix)
  %badAln = extractvalue %dx.types.ResRet.f32 %badAlnLd, 0
  ; Undefined offset on Structured Buffer Load.
  %badStrOffLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %scalBuf, i32 3, i32 undef, i8 1, i32 4)
  %badStrOff = extractvalue %dx.types.ResRet.f32 %badStrOffLd, 0
  %baBuf = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp3, %dx.types.ResourceProperties { i32 11, i32 0 })
  ; Defined (and therefore invalid) offset on Byte Address Buffer Load.
  %badBabOffLd = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %baBuf, i32 0, i32 0, i8 1, i32 4)
  %badBabOff = extractvalue %dx.types.ResRet.f32 %badBabOffLd, 0

  %vecBuf = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp5, %dx.types.ResourceProperties { i32 12, i32 16 })
  ; Invalid RC on Vector Load (and inevitably invalid RK).
  %badRCVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %samp, i32 0, i32 0, i32 4)
  %badRCVc = extractvalue %dx.types.ResRet.v4f32 %badRCVcLd, 0
  ; Invalid RK on Vector Load.
  %badRKVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %tex, i32 1, i32 0, i32 4)
  %badRKVc = extractvalue %dx.types.ResRet.v4f32 %badRKVcLd, 0
  ; Non-constant alignment on Vector Load.
  %badAlnVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %vecBuf, i32 2, i32 0, i32 %ix)
  %badAlnVc = extractvalue %dx.types.ResRet.v4f32 %badAlnVcLd, 0
  ; Undefined offset on Structured Buffer Vector Load.
  %badStrOffVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %vecBuf, i32 3, i32 undef, i32 4)
  %badStrOffVc = extractvalue %dx.types.ResRet.v4f32 %badStrOffVcLd, 0
  ; Defined (and therefore invalid) offset on Byte Address Buffer Vector Load.
  %badBabOffVcLd = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %baBuf, i32 4, i32 0, i32 4)
  %badBabOffVc = extractvalue %dx.types.ResRet.v4f32 %badBabOffVcLd, 0

  ; Store to non-UAV.
  %tmp38 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %scalBuf, i32 0, i32 0, float %badRC, float undef, float undef, float undef, i8 1, i32 4)
  ; Invalid RK on Store.
  %tmp39 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %rwTex, i32 1, i32 0, float %badRK, float undef, float undef, float undef, i8 1, i32 4)
  ; Non-constant alignment on Store.
  %tmp40 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp40, i32 2, i32 0, float %badAln, float undef, float undef, float undef, i8 1, i32 %ix)
  ; Undefined offset on Structured Buffer Store.
  %tmp41 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp41, i32 3, i32 undef, float %badStrOff, float undef, float undef, float undef, i8 1, i32 4)
  ; Undefined value Store.
  %tmp42 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp1, %dx.types.ResourceProperties { i32 4108, i32 4 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp42, i32 4, i32 0, float undef, float undef, float undef, float undef, i8 1, i32 4)
  ; Defined (and therefore invalid) offset on Byte Address Buffer Store.
  %tmp44 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp, %dx.types.ResourceProperties { i32 4107, i32 0 })
  call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp44, i32 0, i32 0, float %badBabOff, float undef, float undef, float undef, i8 1, i32 4)

  ; Vector Store to non-UAV.
  %tmp45 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %rwTex, %dx.types.ResourceProperties { i32 4108, i32 16 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %vecBuf, i32 0, i32 0, <4 x float> %badRCVc, i32 4)
  ; Invalid RK on Vector Store.
  %tmp46 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp2, %dx.types.ResourceProperties { i32 4108, i32 16 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %rwTex, i32 1, i32 0, <4 x float> %badRKVc, i32 4)
  ; Non-constant alignment on Vector Store.
  %tmp47 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp2, %dx.types.ResourceProperties { i32 4108, i32 16 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp47, i32 2, i32 0, <4 x float> %badAlnVc, i32 %ix)
  ; Undefined offset on Structured Buffer Vector Store.
  %tmp48 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp2, %dx.types.ResourceProperties { i32 4108, i32 16 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp48, i32 3, i32 undef, <4 x float> %badStrOffVc, i32 4)
  ; Undefinded value Vector Store.
  %tmp49 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp2, %dx.types.ResourceProperties { i32 4108, i32 16 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp49, i32 4, i32 0, <4 x float> undef, i32 4)
  ; Defined (and therefore invalid) offset on Byte Address Buffer Vector Store.
  %tmp51 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp, %dx.types.ResourceProperties { i32 4107, i32 0 })
  call void @dx.op.rawBufferVectorStore.v4f32(i32 304, %dx.types.Handle %tmp51, i32 4, i32 0, <4 x float> %badBabOffVc, i32 4)

  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %tmp11)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %tmp12)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %tmp13)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %tmp14)
  ret void
}

declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #2
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0
declare %dx.types.ResRet.f32 @dx.op.sample.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float) #1
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #0
declare void @dx.op.rawBufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8, i32) #0
declare %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32, %dx.types.Handle, i32, i32, i8, i32) #1
declare void @dx.op.rawBufferVectorStore.v4f32(i32, %dx.types.Handle, i32, i32, <4 x float>, i32) #0
declare %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32, %dx.types.Handle, i32, i32, i32) #1
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!18}
!dx.entryPoints = !{!19}

!1 = !{i32 1, i32 9}
!2 = !{!"ps", i32 6, i32 9}
!3 = !{!4, !12, null, !16}
!4 = !{!5, !7, !9, !11}
!5 = !{i32 0, %"class.Texture1D<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 1, i32 0, !6}
!6 = !{i32 0, i32 9}
!7 = !{i32 1, %"class.StructuredBuffer<vector<float, 4> >"* undef, !"", i32 0, i32 1, i32 1, i32 12, i32 0, !8}
!8 = !{i32 1, i32 16}
!9 = !{i32 2, %"class.StructuredBuffer<float>"* undef, !"", i32 0, i32 2, i32 1, i32 12, i32 0, !10}
!10 = !{i32 1, i32 4}
!11 = !{i32 3, %struct.ByteAddressBuffer* undef, !"", i32 0, i32 3, i32 1, i32 11, i32 0, null}
!12 = !{!13, !14, !15}
!13 = !{i32 0, %"class.RWStructuredBuffer<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !8}
!14 = !{i32 1, %"class.RWStructuredBuffer<float>"* undef, !"", i32 0, i32 1, i32 1, i32 12, i1 false, i1 false, i1 false, !10}
!15 = !{i32 2, %struct.RWByteAddressBuffer* undef, !"", i32 0, i32 2, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!16 = !{!17}
!17 = !{i32 0, %struct.SamplerState* undef, !"", i32 0, i32 0, i32 1, i32 0, null}
!18 = !{[3 x i32] [i32 1, i32 4, i32 0]}
!19 = !{void ()* @main, !"main", !20, !3, !27}
!20 = !{!21, !24, null}
!21 = !{!22}
!22 = !{i32 0, !"IX", i8 4, i8 0, !23, i8 1, i32 1, i8 1, i32 0, i8 0, null}
!23 = !{i32 0}
!24 = !{!25}
!25 = !{i32 0, !"SV_Target", i8 9, i8 16, !23, i8 0, i32 1, i8 4, i32 0, i8 0, !26}
!26 = !{i32 3, i32 15}
!27 = !{i32 0, i64 8589934608}
