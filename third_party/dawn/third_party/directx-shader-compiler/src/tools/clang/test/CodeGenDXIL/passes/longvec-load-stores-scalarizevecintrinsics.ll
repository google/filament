; RUN: %dxopt %s -hlsl-passes-resume -hlsl-dxil-scalarize-vector-intrinsics -S | FileCheck %s

; Verify that scalarize vector load stores pass will convert raw buffer vector operations
; into the equivalent collection of scalar load store calls.
; Sourced from buffer-load-stors-sm69.hlsl.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.v17f32 = type { <17 x float>, i32 }
%struct.ByteAddressBuffer = type { i32 }
%"class.StructuredBuffer<vector<float, 17> >" = type { <17 x float> }
%struct.RWByteAddressBuffer = type { i32 }
%"class.RWStructuredBuffer<vector<float, 17> >" = type { <17 x float> }
%"class.ConsumeStructuredBuffer<vector<float, 17> >" = type { <17 x float> }
%"class.AppendStructuredBuffer<vector<float, 17> >" = type { <17 x float> }

@"\01?RoByBuf@@3UByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4
@"\01?RwByBuf@@3URWByteAddressBuffer@@A" = external constant %dx.types.Handle, align 4
@"\01?RoStBuf@@3V?$StructuredBuffer@V?$vector@M$0BB@@@@@A" = external constant %dx.types.Handle, align 4
@"\01?RwStBuf@@3V?$RWStructuredBuffer@V?$vector@M$0BB@@@@@A" = external constant %dx.types.Handle, align 4
@"\01?CnStBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$0BB@@@@@A" = external constant %dx.types.Handle, align 4
@"\01?ApStBuf@@3V?$AppendStructuredBuffer@V?$vector@M$0BB@@@@@A" = external constant %dx.types.Handle, align 4

define void @main() {
bb:
  %tmp = load %dx.types.Handle, %dx.types.Handle* @"\01?RoStBuf@@3V?$StructuredBuffer@V?$vector@M$0BB@@@@@A", align 4
  %tmp1 = load %dx.types.Handle, %dx.types.Handle* @"\01?RoByBuf@@3UByteAddressBuffer@@A", align 4
  %tmp2 = load %dx.types.Handle, %dx.types.Handle* @"\01?ApStBuf@@3V?$AppendStructuredBuffer@V?$vector@M$0BB@@@@@A", align 4
  %tmp3 = load %dx.types.Handle, %dx.types.Handle* @"\01?CnStBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$0BB@@@@@A", align 4
  %tmp4 = load %dx.types.Handle, %dx.types.Handle* @"\01?RwStBuf@@3V?$RWStructuredBuffer@V?$vector@M$0BB@@@@@A", align 4
  %tmp5 = load %dx.types.Handle, %dx.types.Handle* @"\01?RwByBuf@@3URWByteAddressBuffer@@A", align 4
  %tmp6 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  %tmp7 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp5)
  %tmp8 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp7, %dx.types.ResourceProperties { i32 4107, i32 0 })

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp8, i32 %tmp6, i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix1:%.*]] = add i32 %tmp6, 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp8, i32 [[ix1]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix2:%.*]] = add i32 [[ix1]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp8, i32 [[ix2]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix3:%.*]] = add i32 [[ix2]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp8, i32 [[ix3]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix4:%.*]] = add i32 [[ix3]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp8, i32 [[ix4]], i32 undef, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp9 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp8, i32 %tmp6, i32 undef, i32 4)
  %tmp10 = extractvalue %dx.types.ResRet.v17f32 %tmp9, 0
  %tmp11 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp1)
  %tmp12 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp11, %dx.types.ResourceProperties { i32 11, i32 0 })

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp12, i32 %tmp6, i32 undef, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix1:%.*]] = add i32 %tmp6, 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp12, i32 [[ix1]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix2:%.*]] = add i32 [[ix1]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp12, i32 [[ix2]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix3:%.*]] = add i32 [[ix2]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp12, i32 [[ix3]], i32 undef, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ix4:%.*]] = add i32 [[ix3]], 16
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp12, i32 [[ix4]], i32 undef, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp13 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp12, i32 %tmp6, i32 undef, i32 4)
  %tmp14 = extractvalue %dx.types.ResRet.v17f32 %tmp13, 0
  %tmp15 = fadd fast <17 x float> %tmp14, %tmp10

  ; CHECK: [[val0:%.*]] = extractelement <17 x float> %tmp15, i64 0
  ; CHECK: [[val1:%.*]] = extractelement <17 x float> %tmp15, i64 1
  ; CHECK: [[val2:%.*]] = extractelement <17 x float> %tmp15, i64 2
  ; CHECK: [[val3:%.*]] = extractelement <17 x float> %tmp15, i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp8, i32 %tmp6, i32 undef, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15, i32 4)
  ; CHECK: [[ix1:%.*]] = add i32 %tmp6, 16
  ; CHECK: [[val4:%.*]] = extractelement <17 x float> %tmp15, i64 4
  ; CHECK: [[val5:%.*]] = extractelement <17 x float> %tmp15, i64 5
  ; CHECK: [[val6:%.*]] = extractelement <17 x float> %tmp15, i64 6
  ; CHECK: [[val7:%.*]] = extractelement <17 x float> %tmp15, i64 7
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp8, i32 [[ix1]], i32 undef, float [[val4]], float [[val5]], float [[val6]], float [[val7]], i8 15, i32 4)
  ; CHECK: [[ix2:%.*]] = add i32 %80, 16
  ; CHECK: [[val8:%.*]] = extractelement <17 x float> %tmp15, i64 8
  ; CHECK: [[val9:%.*]] = extractelement <17 x float> %tmp15, i64 9
  ; CHECK: [[val10:%.*]] = extractelement <17 x float> %tmp15, i64 10
  ; CHECK: [[val11:%.*]] = extractelement <17 x float> %tmp15, i64 11
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp8, i32 [[ix2]], i32 undef, float [[val8]], float [[val9]], float [[val10]], float [[val11]], i8 15, i32 4)
  ; CHECK: [[ix3:%.*]] = add i32 %85, 16
  ; CHECK: [[val12:%.*]] = extractelement <17 x float> %tmp15, i64 12
  ; CHECK: [[val13:%.*]] = extractelement <17 x float> %tmp15, i64 13
  ; CHECK: [[val14:%.*]] = extractelement <17 x float> %tmp15, i64 14
  ; CHECK: [[val15:%.*]] = extractelement <17 x float> %tmp15, i64 15
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp8, i32 [[ix3]], i32 undef, float [[val12]], float [[val13]], float [[val14]], float [[val15]], i8 15, i32 4)
  ; CHECK: [[ix4:%.*]] = add i32 %90, 16
  ; CHECK: [[val16:%.*]] = extractelement <17 x float> %tmp15, i64 16
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp8, i32 [[ix4]], i32 undef, float [[val16]], float undef, float undef, float undef, i8 1, i32 4)
  call void @dx.op.rawBufferVectorStore.v17f32(i32 304, %dx.types.Handle %tmp8, i32 %tmp6, i32 undef, <17 x float> %tmp15, i32 4)
  %tmp16 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp4)
  %tmp17 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp16, %dx.types.ResourceProperties { i32 4108, i32 68 })

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp6, i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp6, i32 16, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp6, i32 32, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp6, i32 48, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp6, i32 64, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp18 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp17, i32 %tmp6, i32 0, i32 4)
  %tmp19 = extractvalue %dx.types.ResRet.v17f32 %tmp18, 0
  %tmp20 = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 1, i8 0, i32 undef)

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp20, i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp20, i32 16, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp20, i32 32, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp20, i32 48, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp17, i32 %tmp20, i32 64, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp21 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp17, i32 %tmp20, i32 0, i32 4)
  %tmp22 = extractvalue %dx.types.ResRet.v17f32 %tmp21, 0
  %tmp23 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp)
  %tmp24 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp23, %dx.types.ResourceProperties { i32 12, i32 68 })

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp6, i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp6, i32 16, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp6, i32 32, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp6, i32 48, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp6, i32 64, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp25 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp24, i32 %tmp6, i32 0, i32 4)
  %tmp26 = extractvalue %dx.types.ResRet.v17f32 %tmp25, 0

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp20, i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp20, i32 16, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp20, i32 32, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp20, i32 48, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp24, i32 %tmp20, i32 64, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp27 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp24, i32 %tmp20, i32 0, i32 4)
  %tmp28 = extractvalue %dx.types.ResRet.v17f32 %tmp27, 0
  %tmp29 = fadd fast <17 x float> %tmp22, %tmp19
  %tmp30 = fadd fast <17 x float> %tmp29, %tmp26
  %tmp31 = fadd fast <17 x float> %tmp30, %tmp28

  ; CHECK: [[val0:%.*]] = extractelement <17 x float> %tmp31, i64 0
  ; CHECK: [[val1:%.*]] = extractelement <17 x float> %tmp31, i64 1
  ; CHECK: [[val2:%.*]] = extractelement <17 x float> %tmp31, i64 2
  ; CHECK: [[val3:%.*]] = extractelement <17 x float> %tmp31, i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp17, i32 %tmp6, i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractelement <17 x float> %tmp31, i64 4
  ; CHECK: [[val5:%.*]] = extractelement <17 x float> %tmp31, i64 5
  ; CHECK: [[val6:%.*]] = extractelement <17 x float> %tmp31, i64 6
  ; CHECK: [[val7:%.*]] = extractelement <17 x float> %tmp31, i64 7
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp17, i32 %tmp6, i32 16, float [[val4]], float [[val5]], float [[val6]], float [[val7]], i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractelement <17 x float> %tmp31, i64 8
  ; CHECK: [[val9:%.*]] = extractelement <17 x float> %tmp31, i64 9
  ; CHECK: [[val10:%.*]] = extractelement <17 x float> %tmp31, i64 10
  ; CHECK: [[val11:%.*]] = extractelement <17 x float> %tmp31, i64 11
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp17, i32 %tmp6, i32 32, float [[val8]], float [[val9]], float [[val10]], float [[val11]], i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractelement <17 x float> %tmp31, i64 12
  ; CHECK: [[val13:%.*]] = extractelement <17 x float> %tmp31, i64 13
  ; CHECK: [[val14:%.*]] = extractelement <17 x float> %tmp31, i64 14
  ; CHECK: [[val15:%.*]] = extractelement <17 x float> %tmp31, i64 15
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp17, i32 %tmp6, i32 48, float [[val12]], float [[val13]], float [[val14]], float [[val15]], i8 15, i32 4)
  ; CHECK: [[val16:%.*]] = extractelement <17 x float> %tmp31, i64 16
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp17, i32 %tmp6, i32 64, float [[val16]], float undef, float undef, float undef, i8 1, i32 4)
  call void @dx.op.rawBufferVectorStore.v17f32(i32 304, %dx.types.Handle %tmp17, i32 %tmp6, i32 0, <17 x float> %tmp31, i32 4)
  %tmp32 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp3)
  %tmp33 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp32, %dx.types.ResourceProperties { i32 36876, i32 68 })
  %tmp34 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle %tmp33, i8 -1)

  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp33, i32 %tmp34, i32 0, i8 15, i32 4)
  ; CHECK: [[val0:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val1:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val2:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val3:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp33, i32 %tmp34, i32 16, i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val5:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val6:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val7:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp33, i32 %tmp34, i32 32, i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val9:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val10:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val11:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp33, i32 %tmp34, i32 48, i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[val13:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 1
  ; CHECK: [[val14:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 2
  ; CHECK: [[val15:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 3
  ; CHECK: [[ld:%.*]] = call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %tmp33, i32 %tmp34, i32 64, i8 1, i32 4)
  ; CHECK: [[val16:%.*]] = extractvalue %dx.types.ResRet.f32 [[ld]], 0
  ; CHECK: [[vec0:%.*]] = insertelement <17 x float> undef, float [[val0]], i64 0
  ; CHECK: [[vec1:%.*]] = insertelement <17 x float> [[vec0]], float [[val1]], i64 1
  ; CHECK: [[vec2:%.*]] = insertelement <17 x float> [[vec1]], float [[val2]], i64 2
  ; CHECK: [[vec3:%.*]] = insertelement <17 x float> [[vec2]], float [[val3]], i64 3
  ; CHECK: [[vec4:%.*]] = insertelement <17 x float> [[vec3]], float [[val4]], i64 4
  ; CHECK: [[vec5:%.*]] = insertelement <17 x float> [[vec4]], float [[val5]], i64 5
  ; CHECK: [[vec6:%.*]] = insertelement <17 x float> [[vec5]], float [[val6]], i64 6
  ; CHECK: [[vec7:%.*]] = insertelement <17 x float> [[vec6]], float [[val7]], i64 7
  ; CHECK: [[vec8:%.*]] = insertelement <17 x float> [[vec7]], float [[val8]], i64 8
  ; CHECK: [[vec9:%.*]] = insertelement <17 x float> [[vec8]], float [[val9]], i64 9
  ; CHECK: [[vec10:%.*]] = insertelement <17 x float> [[vec9]], float [[val10]], i64 10
  ; CHECK: [[vec11:%.*]] = insertelement <17 x float> [[vec10]], float [[val11]], i64 11
  ; CHECK: [[vec12:%.*]] = insertelement <17 x float> [[vec11]], float [[val12]], i64 12
  ; CHECK: [[vec13:%.*]] = insertelement <17 x float> [[vec12]], float [[val13]], i64 13
  ; CHECK: [[vec14:%.*]] = insertelement <17 x float> [[vec13]], float [[val14]], i64 14
  ; CHECK: [[vec15:%.*]] = insertelement <17 x float> [[vec14]], float [[val15]], i64 15
  ; CHECK: [[vec16:%.*]] = insertelement <17 x float> [[vec15]], float [[val16]], i64 16
  %tmp35 = call %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32 303, %dx.types.Handle %tmp33, i32 %tmp34, i32 0, i32 4)
  %tmp36 = extractvalue %dx.types.ResRet.v17f32 %tmp35, 0
  %tmp37 = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle %tmp2)
  %tmp38 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %tmp37, %dx.types.ResourceProperties { i32 36876, i32 68 })
  %tmp39 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle %tmp38, i8 1)

  ; CHECK: [[val0:%.*]] = extractelement <17 x float> [[vec16]], i64 0
  ; CHECK: [[val1:%.*]] = extractelement <17 x float> [[vec16]], i64 1
  ; CHECK: [[val2:%.*]] = extractelement <17 x float> [[vec16]], i64 2
  ; CHECK: [[val3:%.*]] = extractelement <17 x float> [[vec16]], i64 3
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp38, i32 %tmp39, i32 0, float [[val0]], float [[val1]], float [[val2]], float [[val3]], i8 15, i32 4)
  ; CHECK: [[val4:%.*]] = extractelement <17 x float> [[vec16]], i64 4
  ; CHECK: [[val5:%.*]] = extractelement <17 x float> [[vec16]], i64 5
  ; CHECK: [[val6:%.*]] = extractelement <17 x float> [[vec16]], i64 6
  ; CHECK: [[val7:%.*]] = extractelement <17 x float> [[vec16]], i64 7
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp38, i32 %tmp39, i32 16, float [[val4]], float [[val5]], float [[val6]], float [[val7]], i8 15, i32 4)
  ; CHECK: [[val8:%.*]] = extractelement <17 x float> [[vec16]], i64 8
  ; CHECK: [[val9:%.*]] = extractelement <17 x float> [[vec16]], i64 9
  ; CHECK: [[val10:%.*]] = extractelement <17 x float> [[vec16]], i64 10
  ; CHECK: [[val11:%.*]] = extractelement <17 x float> [[vec16]], i64 11
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp38, i32 %tmp39, i32 32, float [[val8]], float [[val9]], float [[val10]], float [[val11]], i8 15, i32 4)
  ; CHECK: [[val12:%.*]] = extractelement <17 x float> [[vec16]], i64 12
  ; CHECK: [[val13:%.*]] = extractelement <17 x float> [[vec16]], i64 13
  ; CHECK: [[val14:%.*]] = extractelement <17 x float> [[vec16]], i64 14
  ; CHECK: [[val15:%.*]] = extractelement <17 x float> [[vec16]], i64 15
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp38, i32 %tmp39, i32 48, float [[val12]], float [[val13]], float [[val14]], float [[val15]], i8 15, i32 4)
  ; CHECK: [[val16:%.*]] = extractelement <17 x float> [[vec16]], i64 16
  ; CHECK: call void @dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %tmp38, i32 %tmp39, i32 64, float [[val16]], float undef, float undef, float undef, i8 1, i32 4)
  call void @dx.op.rawBufferVectorStore.v17f32(i32 304, %dx.types.Handle %tmp38, i32 %tmp39, i32 0, <17 x float> %tmp36, i32 4)
  ret void
}

declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0
declare %dx.types.ResRet.v17f32 @dx.op.rawBufferVectorLoad.v17f32(i32, %dx.types.Handle, i32, i32, i32) #1
declare void @dx.op.rawBufferVectorStore.v17f32(i32, %dx.types.Handle, i32, i32, <17 x float>, i32) #2
declare i32 @dx.op.bufferUpdateCounter(i32, %dx.types.Handle, i8) #2
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0
declare %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32, %dx.types.Handle) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind }

!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.typeAnnotations = !{!13}
!dx.entryPoints = !{!17, !19}

!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{!4, !8, null, null}
!4 = !{!5, !6}
!5 = !{i32 0, %struct.ByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?RoByBuf@@3UByteAddressBuffer@@A" to %struct.ByteAddressBuffer*), !"RoByBuf", i32 0, i32 1, i32 1, i32 11, i32 0, null}
!6 = !{i32 1, %"class.StructuredBuffer<vector<float, 17> >"* bitcast (%dx.types.Handle* @"\01?RoStBuf@@3V?$StructuredBuffer@V?$vector@M$0BB@@@@@A" to %"class.StructuredBuffer<vector<float, 17> >"*), !"RoStBuf", i32 0, i32 2, i32 1, i32 12, i32 0, !7}
!7 = !{i32 1, i32 68}
!8 = !{!9, !10, !11, !12}
!9 = !{i32 0, %struct.RWByteAddressBuffer* bitcast (%dx.types.Handle* @"\01?RwByBuf@@3URWByteAddressBuffer@@A" to %struct.RWByteAddressBuffer*), !"RwByBuf", i32 0, i32 1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!10 = !{i32 1, %"class.RWStructuredBuffer<vector<float, 17> >"* bitcast (%dx.types.Handle* @"\01?RwStBuf@@3V?$RWStructuredBuffer@V?$vector@M$0BB@@@@@A" to %"class.RWStructuredBuffer<vector<float, 17> >"*), !"RwStBuf", i32 0, i32 2, i32 1, i32 12, i1 false, i1 false, i1 false, !7}
!11 = !{i32 2, %"class.ConsumeStructuredBuffer<vector<float, 17> >"* bitcast (%dx.types.Handle* @"\01?CnStBuf@@3V?$ConsumeStructuredBuffer@V?$vector@M$0BB@@@@@A" to %"class.ConsumeStructuredBuffer<vector<float, 17> >"*), !"CnStBuf", i32 0, i32 4, i32 1, i32 12, i1 false, i1 true, i1 false, !7}
!12 = !{i32 3, %"class.AppendStructuredBuffer<vector<float, 17> >"* bitcast (%dx.types.Handle* @"\01?ApStBuf@@3V?$AppendStructuredBuffer@V?$vector@M$0BB@@@@@A" to %"class.AppendStructuredBuffer<vector<float, 17> >"*), !"ApStBuf", i32 0, i32 5, i32 1, i32 12, i1 false, i1 true, i1 false, !7}
!13 = !{i32 1, void ()* @main, !14}
!14 = !{!15}
!15 = !{i32 0, !16, !16}
!16 = !{}
!17 = !{null, !"", null, !3, !18}
!18 = !{i32 0, i64 8589934608}
!19 = !{void ()* @main, !"main", !20, null, !24}
!20 = !{!21, null, null}
!21 = !{!22}
!22 = !{i32 0, !"IX", i8 5, i8 0, !23, i8 0, i32 2, i8 1, i32 0, i8 0, null}
!23 = !{i32 0, i32 1}
!24 = !{i32 8, i32 1, i32 5, !25}
!25 = !{i32 0}
