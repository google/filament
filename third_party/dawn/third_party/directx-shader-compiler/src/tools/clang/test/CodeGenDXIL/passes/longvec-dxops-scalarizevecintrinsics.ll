; RUN: %dxopt %s -hlsl-passes-resume -hlsl-dxil-scalarize-vector-intrinsics -S | FileCheck %s

; Verify that scalarize vector pass will convert dxil vector operations
; into the equivalent collection of scalar operations.

; Compiled with this source -lib_6_9 and altered metadata version info
; StructuredBuffer< vector<float, 4> > buf;
; ByteAddressBuffer rbuf;
; // CHECK-LABEL: define void @main()
; [shader("pixel")]
; float4 main(uint i : SV_PrimitiveID, uint4 m : M) : SV_Target {
;   vector<float, 4> vec1 = rbuf.Load< vector<float, 4> >(i++*32);
;   vector<float, 4> vec2 = rbuf.Load< vector<float, 4> >(i++*32);
;   vector<float, 4> vec3 = rbuf.Load< vector<float, 4> >(i++*32);
;   vector<bool, 4> bvec = rbuf.Load< vector<bool, 4> >(i++*32);
;   vector<uint, 4> ivec1 = rbuf.Load< vector<uint, 4> >(i++*32);
;   vector<uint, 4> ivec2 = rbuf.Load< vector<uint, 4> >(i++*32);
;   vector<float, 4> res = 0;
;   res += sin(vec1);
;   res += max(vec1, vec2);
;   res += countbits(ivec2);
;   res += QuadReadLaneAt(vec2, i - 4);
;   res += WaveActiveMin(vec1);
;   bvec ^= isinf(vec3);
;   return select(bvec, res, vec3);
; }


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.v4f32 = type { <4 x float>, i32 }
%dx.types.ResRet.v4i32 = type { <4 x i32>, i32 }
%struct.ByteAddressBuffer = type { i32 }

@"\01?rbuf@@3UByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4

define void @main() {
bb:
  %tmp = load %dx.types.Handle, %dx.types.Handle* @"\01?rbuf@@3UByteAddressBuffer@@A", align 4
  %tmp1 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %tmp2 = shl i32 %tmp1, 5
  %tmp3 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp)
  %tmp4 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp3, %dx.types.ResourceProperties { i32 11, i32 0 })
  %tmp5 = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %tmp4, i32 %tmp2, i32 undef, i32 4)
  %tmp6 = extractvalue %dx.types.ResRet.v4f32 %tmp5, 0
  %tmp7 = add i32 %tmp2, 32
  %tmp8 = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %tmp4, i32 %tmp7, i32 undef, i32 4)
  %tmp9 = extractvalue %dx.types.ResRet.v4f32 %tmp8, 0
  %tmp10 = add i32 %tmp2, 64
  %tmp11 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp)
  %tmp12 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp11, %dx.types.ResourceProperties { i32 11, i32 0 })
  %tmp13 = call %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32 303, %dx.types.Handle %tmp12, i32 %tmp10, i32 undef, i32 4)
  %tmp14 = extractvalue %dx.types.ResRet.v4f32 %tmp13, 0
  %tmp15 = add i32 %tmp2, 96
  %tmp16 = call %dx.types.ResRet.v4i32 @dx.op.rawBufferVectorLoad.v4i32(i32 303, %dx.types.Handle %tmp12, i32 %tmp15, i32 undef, i32 4)
  %tmp17 = extractvalue %dx.types.ResRet.v4i32 %tmp16, 0
  %tmp18 = icmp ne <4 x i32> %tmp17, zeroinitializer
  %tmp19 = zext <4 x i1> %tmp18 to <4 x i32>
  %tmp20 = add i32 %tmp2, 160
  %tmp21 = call %dx.types.ResRet.v4i32 @dx.op.rawBufferVectorLoad.v4i32(i32 303, %dx.types.Handle %tmp12, i32 %tmp20, i32 undef, i32 4)
  %tmp22 = extractvalue %dx.types.ResRet.v4i32 %tmp21, 0

  ; Test dx.op.unary scalarization using sin().
  ; CHECK: call float @dx.op.unary.f32(i32 13,
  ; CHECK: call float @dx.op.unary.f32(i32 13,
  ; CHECK: call float @dx.op.unary.f32(i32 13,
  ; CHECK: call float @dx.op.unary.f32(i32 13,
  %tmp23 = call <4 x float> @dx.op.unary.v4f32(i32 13, <4 x float> %tmp6)

  ; Test @dx.op.binary.f32 scalarization using max().
  ; CHECK: call float @dx.op.binary.f32(i32 35
  ; CHECK: call float @dx.op.binary.f32(i32 35
  ; CHECK: call float @dx.op.binary.f32(i32 35
  ; CHECK: call float @dx.op.binary.f32(i32 35
  %tmp24 = call <4 x float> @dx.op.binary.v4f32(i32 35, <4 x float> %tmp6, <4 x float> %tmp9)
  %tmp25 = fadd fast <4 x float> %tmp24, %tmp23

  ; Test dx.op.unaryBits using countbits().
  ; CHECK: call i32 @dx.op.unaryBits.i32(i32 31
  ; CHECK: call i32 @dx.op.unaryBits.i32(i32 31
  ; CHECK: call i32 @dx.op.unaryBits.i32(i32 31
  ; CHECK: call i32 @dx.op.unaryBits.i32(i32 31
  %tmp26 = call <4 x i32> @dx.op.unaryBits.v4i32(i32 31, <4 x i32> %tmp22)
  %tmp27 = uitofp <4 x i32> %tmp26 to <4 x float>
  %tmp28 = fadd fast <4 x float> %tmp25, %tmp27
  %tmp29 = add i32 %tmp1, 2

  ; Test dx.op.quadReadLaneAt using . . . QuadReadLaneAt().
  ; CHECK: call float @dx.op.quadReadLaneAt.f32(i32 122
  ; CHECK: call float @dx.op.quadReadLaneAt.f32(i32 122
  ; CHECK: call float @dx.op.quadReadLaneAt.f32(i32 122
  ; CHECK: call float @dx.op.quadReadLaneAt.f32(i32 122
  %tmp30 = call <4 x float> @dx.op.quadReadLaneAt.v4f32(i32 122, <4 x float> %tmp9, i32 %tmp29)
  %tmp31 = fadd fast <4 x float> %tmp28, %tmp30

  ; Test dx.op.waveActiveOp using WaveActiveMin().
  ; CHECK: call float @dx.op.waveActiveOp.f32(i32 119
  ; CHECK: call float @dx.op.waveActiveOp.f32(i32 119
  ; CHECK: call float @dx.op.waveActiveOp.f32(i32 119
  ; CHECK: call float @dx.op.waveActiveOp.f32(i32 119
  %tmp32 = call <4 x float> @dx.op.waveActiveOp.v4f32(i32 119, <4 x float> %tmp6, i8 2, i8 0)
  %tmp33 = fadd fast <4 x float> %tmp31, %tmp32

  ; Test dx.op.isSpecialFloat using isinf().
  ; CHECK: call i1 @dx.op.isSpecialFloat.f32(i32 9
  ; CHECK: call i1 @dx.op.isSpecialFloat.f32(i32 9
  ; CHECK: call i1 @dx.op.isSpecialFloat.f32(i32 9
  ; CHECK: call i1 @dx.op.isSpecialFloat.f32(i32 9
  %tmp36 = call <4 x i1> @dx.op.isSpecialFloat.v4f32(i32 9, <4 x float> %tmp14)
  %tmp37 = icmp ne <4 x i32> %tmp19, zeroinitializer
  %tmp38 = xor <4 x i1> %tmp37, %tmp36
  %tmp39 = zext <4 x i1> %tmp38 to <4 x i32>
  %tmp40 = icmp ne <4 x i32> %tmp39, zeroinitializer
  %tmp41 = select <4 x i1> %tmp40, <4 x float> %tmp33, <4 x float> %tmp14
  %tmp42 = extractelement <4 x float> %tmp41, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %tmp42)
  %tmp43 = extractelement <4 x float> %tmp41, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %tmp43)
  %tmp44 = extractelement <4 x float> %tmp41, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %tmp44)
  %tmp45 = extractelement <4 x float> %tmp41, i64 3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %tmp45)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.v4f32 @dx.op.rawBufferVectorLoad.v4f32(i32, %dx.types.Handle, i32, i32, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.v4i32 @dx.op.rawBufferVectorLoad.v4i32(i32, %dx.types.Handle, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare <4 x float> @dx.op.unary.v4f32(i32, <4 x float>) #0

; Function Attrs: nounwind readnone
declare <4 x float> @dx.op.binary.v4f32(i32, <4 x float>, <4 x float>) #0

; Function Attrs: nounwind readnone
declare <4 x i32> @dx.op.unaryBits.v4i32(i32, <4 x i32>) #0

; Function Attrs: nounwind
declare <4 x float> @dx.op.quadReadLaneAt.v4f32(i32, <4 x float>, i32) #1

; Function Attrs: nounwind
declare <4 x float> @dx.op.waveActiveOp.v4f32(i32, <4 x float>, i8, i8) #1

; Function Attrs: nounwind readnone
declare <4 x i1> @dx.op.isSpecialFloat.v4f32(i32, <4 x float>) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10, !12}

!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{!4, null, null, null}
!4 = !{!5}
!5 = !{i32 0, %struct.ByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?rbuf@@3UByteAddressBuffer@@A" to %struct.ByteAddressBuffer*), !"rbuf", i32 -1, i32 -1, i32 1, i32 11, i32 0, null}
!6 = !{i32 1, void ()* @main, !7}
!7 = !{!8}
!8 = !{i32 0, !9, !9}
!9 = !{}
!10 = !{null, !"", null, !3, !11}
!11 = !{i32 0, i64 524304}
!12 = !{void ()* @main, !"main", !13, null, !20}
!13 = !{!14, !18, null}
!14 = !{!15, !17}
!15 = !{i32 0, !"SV_PrimitiveID", i8 5, i8 10, !16, i8 1, i32 1, i8 1, i32 0, i8 0, null}
!16 = !{i32 0}
!17 = !{i32 1, !"M", i8 5, i8 0, !16, i8 1, i32 1, i8 4, i32 1, i8 0, null}
!18 = !{!19}
!19 = !{i32 0, !"SV_Target", i8 9, i8 16, !16, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!20 = !{i32 8, i32 0, i32 5, !16}
