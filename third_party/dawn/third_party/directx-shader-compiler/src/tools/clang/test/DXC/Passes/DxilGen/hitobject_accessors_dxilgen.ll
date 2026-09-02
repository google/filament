; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-9

;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; outbuf                                UAV    byte         r/w      U0u4294967295,space4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RWByteAddressBuffer = type { i32 }
%dx.types.HitObject = type { i8* }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.dx::HitObject" = type { i32 }

@"\01?outbuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4

; CHECK:  %{{[^ ]+}} = call %dx.types.HitObject @dx.op.hitObject_SetShaderTableIndex(i32 287, %dx.types.HitObject %{{[^ ]+}}, i32 1)
; CHECK:  %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 270, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 269, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i1 @dx.op.hitObject_StateScalar.i1(i32 271, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 281, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 285, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 282, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 283, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 284, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_StateScalar.i32(i32 286, %dx.types.HitObject %{{[^ ]+}})
; CHECK:  %{{[^ ]+}} = call i32 @dx.op.hitObject_LoadLocalRootTableConstant(i32 288, %dx.types.HitObject %{{[^ ]+}}, i32 42)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %{{[^ ]+}}, i32 0)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %{{[^ ]+}}, i32 1)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 275, %dx.types.HitObject %{{[^ ]+}}, i32 2)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %{{[^ ]+}}, i32 0)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %{{[^ ]+}}, i32 1)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 276, %dx.types.HitObject %{{[^ ]+}}, i32 2)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %{{[^ ]+}}, i32 0)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %{{[^ ]+}}, i32 1)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 277, %dx.types.HitObject %{{[^ ]+}}, i32 2)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %{{[^ ]+}}, i32 0)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %{{[^ ]+}}, i32 1)
; CHECK:  %{{[^ ]+}} = call float @dx.op.hitObject_StateVector.f32(i32 278, %dx.types.HitObject %{{[^ ]+}}, i32 2)

; CHECK:  %[[M34OW00:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO:[^ ]+]], i32 0, i32 0)
; CHECK-NEXT:  %[[M34VOW0:[^ ]+]] = insertelement <12 x float> undef, float %[[M34OW00]], i64 0
; CHECK-NEXT:  %[[M34OW01:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 0, i32 1)
; CHECK-NEXT:  %[[M34VOW1:[^ ]+]] = insertelement <12 x float> %[[M34VOW0]], float %[[M34OW01]], i64 1
; CHECK-NEXT:  %[[M34OW02:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 0, i32 2)
; CHECK-NEXT:  %[[M34VOW2:[^ ]+]] = insertelement <12 x float> %[[M34VOW1]], float %[[M34OW02]], i64 2
; CHECK-NEXT:  %[[M34OW03:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 0, i32 3)
; CHECK-NEXT:  %[[M34VOW3:[^ ]+]] = insertelement <12 x float> %[[M34VOW2]], float %[[M34OW03]], i64 3
; CHECK-NEXT:  %[[M34OW10:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 1, i32 0)
; CHECK-NEXT:  %[[M34VOW4:[^ ]+]] = insertelement <12 x float> %[[M34VOW3]], float %[[M34OW10]], i64 4
; CHECK-NEXT:  %[[M34OW11:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 1, i32 1)
; CHECK-NEXT:  %[[M34VOW5:[^ ]+]] = insertelement <12 x float> %[[M34VOW4]], float %[[M34OW11]], i64 5
; CHECK-NEXT:  %[[M34OW12:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 1, i32 2)
; CHECK-NEXT:  %[[M34VOW6:[^ ]+]] = insertelement <12 x float> %[[M34VOW5]], float %[[M34OW12]], i64 6
; CHECK-NEXT:  %[[M34OW13:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 1, i32 3)
; CHECK-NEXT:  %[[M34VOW7:[^ ]+]] = insertelement <12 x float> %[[M34VOW6]], float %[[M34OW13]], i64 7
; CHECK-NEXT:  %[[M34OW20:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 2, i32 0)
; CHECK-NEXT:  %[[M34VOW8:[^ ]+]] = insertelement <12 x float> %[[M34VOW7]], float %[[M34OW20]], i64 8
; CHECK-NEXT:  %[[M34OW21:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 2, i32 1)
; CHECK-NEXT:  %[[M34VOW9:[^ ]+]] = insertelement <12 x float> %[[M34VOW8]], float %[[M34OW21]], i64 9
; CHECK-NEXT:  %[[M34OW22:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 2, i32 2)
; CHECK-NEXT:  %[[M34VOW10:[^ ]+]] = insertelement <12 x float> %[[M34VOW9]], float %[[M34OW22]], i64 10
; CHECK-NEXT:  %[[M34OW23:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M34OWHO]], i32 2, i32 3)
; CHECK-NEXT:  %{{[^ ]+}} = insertelement <12 x float> %[[M34VOW10]], float %[[M34OW23]], i64 11

; CHECK:  %[[M43OW00:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO:[^ ]+]], i32 0, i32 0)
; CHECK-NEXT:  %[[M43VOW0:[^ ]+]] = insertelement <12 x float> undef, float %[[M43OW00]], i64 0
; CHECK-NEXT:  %[[M43OW10:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 1, i32 0)
; CHECK-NEXT:  %[[M43VOW1:[^ ]+]] = insertelement <12 x float> %[[M43VOW0]], float %[[M43OW10]], i64 1
; CHECK-NEXT:  %[[M43OW20:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 2, i32 0)
; CHECK-NEXT:  %[[M43VOW2:[^ ]+]] = insertelement <12 x float> %[[M43VOW1]], float %[[M43OW20]], i64 2
; CHECK-NEXT:  %[[M43OW01:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 0, i32 1)
; CHECK-NEXT:  %[[M43VOW3:[^ ]+]] = insertelement <12 x float> %[[M43VOW2]], float %[[M43OW01]], i64 3
; CHECK-NEXT:  %[[M43OW11:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 1, i32 1)
; CHECK-NEXT:  %[[M43VOW4:[^ ]+]] = insertelement <12 x float> %[[M43VOW3]], float %[[M43OW11]], i64 4
; CHECK-NEXT:  %[[M43OW21:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 2, i32 1)
; CHECK-NEXT:  %[[M43VOW5:[^ ]+]] = insertelement <12 x float> %[[M43VOW4]], float %[[M43OW21]], i64 5
; CHECK-NEXT:  %[[M43OW02:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 0, i32 2)
; CHECK-NEXT:  %[[M43VOW6:[^ ]+]] = insertelement <12 x float> %[[M43VOW5]], float %[[M43OW02]], i64 6
; CHECK-NEXT:  %[[M43OW12:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 1, i32 2)
; CHECK-NEXT:  %[[M43VOW7:[^ ]+]] = insertelement <12 x float> %[[M43VOW6]], float %[[M43OW12]], i64 7
; CHECK-NEXT:  %[[M43OW22:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 2, i32 2)
; CHECK-NEXT:  %[[M43VOW8:[^ ]+]] = insertelement <12 x float> %[[M43VOW7]], float %[[M43OW22]], i64 8
; CHECK-NEXT:  %[[M43OW03:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 0, i32 3)
; CHECK-NEXT:  %[[M43VOW9:[^ ]+]] = insertelement <12 x float> %[[M43VOW8]], float %[[M43OW03]], i64 9
; CHECK-NEXT:  %[[M43OW13:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 1, i32 3)
; CHECK-NEXT:  %[[M43VOW10:[^ ]+]] = insertelement <12 x float> %[[M43VOW9]], float %[[M43OW13]], i64 10
; CHECK-NEXT:  %[[M43OW23:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 279, %dx.types.HitObject %[[M43OWHO]], i32 2, i32 3)
; CHECK-NEXT:  %{{[^ ]+}} = insertelement <12 x float> %[[M43VOW10]], float %[[M43OW23]], i64 11

; CHECK:  %[[M34WO00:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO:[^ ]+]], i32 0, i32 0)
; CHECK-NEXT:  %[[M34VWO0:[^ ]+]] = insertelement <12 x float> undef, float %[[M34WO00]], i64 0
; CHECK-NEXT:  %[[M34WO01:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 0, i32 1)
; CHECK-NEXT:  %[[M34VWO1:[^ ]+]] = insertelement <12 x float> %[[M34VWO0]], float %[[M34WO01]], i64 1
; CHECK-NEXT:  %[[M34WO02:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 0, i32 2)
; CHECK-NEXT:  %[[M34VWO2:[^ ]+]] = insertelement <12 x float> %[[M34VWO1]], float %[[M34WO02]], i64 2
; CHECK-NEXT:  %[[M34WO03:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 0, i32 3)
; CHECK-NEXT:  %[[M34VWO3:[^ ]+]] = insertelement <12 x float> %[[M34VWO2]], float %[[M34WO03]], i64 3
; CHECK-NEXT:  %[[M34WO10:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 1, i32 0)
; CHECK-NEXT:  %[[M34VWO4:[^ ]+]] = insertelement <12 x float> %[[M34VWO3]], float %[[M34WO10]], i64 4
; CHECK-NEXT:  %[[M34WO11:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 1, i32 1)
; CHECK-NEXT:  %[[M34VWO5:[^ ]+]] = insertelement <12 x float> %[[M34VWO4]], float %[[M34WO11]], i64 5
; CHECK-NEXT:  %[[M34WO12:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 1, i32 2)
; CHECK-NEXT:  %[[M34VWO6:[^ ]+]] = insertelement <12 x float> %[[M34VWO5]], float %[[M34WO12]], i64 6
; CHECK-NEXT:  %[[M34WO13:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 1, i32 3)
; CHECK-NEXT:  %[[M34VWO7:[^ ]+]] = insertelement <12 x float> %[[M34VWO6]], float %[[M34WO13]], i64 7
; CHECK-NEXT:  %[[M34WO20:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 2, i32 0)
; CHECK-NEXT:  %[[M34VWO8:[^ ]+]] = insertelement <12 x float> %[[M34VWO7]], float %[[M34WO20]], i64 8
; CHECK-NEXT:  %[[M34WO21:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 2, i32 1)
; CHECK-NEXT:  %[[M34VWO9:[^ ]+]] = insertelement <12 x float> %[[M34VWO8]], float %[[M34WO21]], i64 9
; CHECK-NEXT:  %[[M34WO22:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 2, i32 2)
; CHECK-NEXT:  %[[M34VWO10:[^ ]+]] = insertelement <12 x float> %[[M34VWO9]], float %[[M34WO22]], i64 10
; CHECK-NEXT:  %[[M34WO23:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M34WOHO]], i32 2, i32 3)
; CHECK-NEXT:  %{{[^ ]+}} = insertelement <12 x float> %[[M34VWO10]], float %[[M34WO23]], i64 11

; CHECK:  %[[M43WO00:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO:[^ ]+]], i32 0, i32 0)
; CHECK-NEXT:  %[[M43VWO0:[^ ]+]] = insertelement <12 x float> undef, float %[[M43WO00]], i64 0
; CHECK-NEXT:  %[[M43WO10:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 1, i32 0)
; CHECK-NEXT:  %[[M43VWO1:[^ ]+]] = insertelement <12 x float> %[[M43VWO0]], float %[[M43WO10]], i64 1
; CHECK-NEXT:  %[[M43WO20:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 2, i32 0)
; CHECK-NEXT:  %[[M43VWO2:[^ ]+]] = insertelement <12 x float> %[[M43VWO1]], float %[[M43WO20]], i64 2
; CHECK-NEXT:  %[[M43WO01:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 0, i32 1)
; CHECK-NEXT:  %[[M43VWO3:[^ ]+]] = insertelement <12 x float> %[[M43VWO2]], float %[[M43WO01]], i64 3
; CHECK-NEXT:  %[[M43WO11:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 1, i32 1)
; CHECK-NEXT:  %[[M43VWO4:[^ ]+]] = insertelement <12 x float> %[[M43VWO3]], float %[[M43WO11]], i64 4
; CHECK-NEXT:  %[[M43WO21:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 2, i32 1)
; CHECK-NEXT:  %[[M43VWO5:[^ ]+]] = insertelement <12 x float> %[[M43VWO4]], float %[[M43WO21]], i64 5
; CHECK-NEXT:  %[[M43WO02:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 0, i32 2)
; CHECK-NEXT:  %[[M43VWO6:[^ ]+]] = insertelement <12 x float> %[[M43VWO5]], float %[[M43WO02]], i64 6
; CHECK-NEXT:  %[[M43WO12:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 1, i32 2)
; CHECK-NEXT:  %[[M43VWO7:[^ ]+]] = insertelement <12 x float> %[[M43VWO6]], float %[[M43WO12]], i64 7
; CHECK-NEXT:  %[[M43WO22:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 2, i32 2)
; CHECK-NEXT:  %[[M43VWO8:[^ ]+]] = insertelement <12 x float> %[[M43VWO7]], float %[[M43WO22]], i64 8
; CHECK-NEXT:  %[[M43WO03:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 0, i32 3)
; CHECK-NEXT:  %[[M43VWO9:[^ ]+]] = insertelement <12 x float> %[[M43VWO8]], float %[[M43WO03]], i64 9
; CHECK-NEXT:  %[[M43WO13:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 1, i32 3)
; CHECK-NEXT:  %[[M43VWO10:[^ ]+]] = insertelement <12 x float> %[[M43VWO9]], float %[[M43WO13]], i64 10
; CHECK-NEXT:  %[[M43WO23:[^ ]+]] = call float @dx.op.hitObject_StateMatrix.f32(i32 280, %dx.types.HitObject %[[M43WOHO]], i32 2, i32 3)
; CHECK-NEXT:  %{{[^ ]+}} = insertelement <12 x float> %[[M43VWO10]], float %[[M43WO23]], i64 11

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
for.body.i.lr.ph:
  %0 = alloca [12 x float]
  %1 = alloca [3 x i32]
  %2 = alloca [12 x float]
  %3 = alloca [4 x i32]
  %4 = alloca [12 x float]
  %5 = alloca [3 x i32]
  %6 = alloca [12 x float]
  %7 = alloca [4 x i32]
  %hit = alloca %dx.types.HitObject, align 4
  %8 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !19 ; line:69 col:3
  call void @llvm.lifetime.start(i64 4, i8* %8) #0, !dbg !19 ; line:69 col:3
  %9 = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %hit), !dbg !23 ; line:69 col:17
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32 388, %dx.types.HitObject* %hit, i32 1), !dbg !24 ; line:75 col:3
  %10 = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 383, %dx.types.HitObject* %hit), !dbg !25 ; line:80 col:11
  %conv = zext i1 %10 to i32, !dbg !25 ; line:80 col:11
  %11 = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 384, %dx.types.HitObject* %hit), !dbg !26 ; line:81 col:11
  %conv3 = zext i1 %11 to i32, !dbg !26 ; line:81 col:11
  %add4 = add nsw i32 %conv, %conv3, !dbg !27 ; line:81 col:8
  %12 = call i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32 385, %dx.types.HitObject* %hit), !dbg !28 ; line:82 col:11
  %conv6 = zext i1 %12 to i32, !dbg !28 ; line:82 col:11
  %add7 = add nsw i32 %add4, %conv6, !dbg !29 ; line:82 col:8
  %13 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 365, %dx.types.HitObject* %hit), !dbg !30 ; line:85 col:11
  %add9 = add i32 %add7, %13, !dbg !31 ; line:85 col:8
  %14 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 366, %dx.types.HitObject* %hit), !dbg !32 ; line:86 col:11
  %add11 = add i32 %add9, %14, !dbg !33 ; line:86 col:8
  %15 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 368, %dx.types.HitObject* %hit), !dbg !34 ; line:87 col:11
  %add13 = add i32 %add11, %15, !dbg !35 ; line:87 col:8
  %16 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 367, %dx.types.HitObject* %hit), !dbg !36 ; line:88 col:11
  %add15 = add i32 %add13, %16, !dbg !37 ; line:88 col:8
  %17 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 373, %dx.types.HitObject* %hit), !dbg !38 ; line:89 col:11
  %add17 = add i32 %add15, %17, !dbg !39 ; line:89 col:8
  %18 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 377, %dx.types.HitObject* %hit), !dbg !40 ; line:90 col:11
  %add19 = add i32 %add17, %18, !dbg !41 ; line:90 col:8
  %19 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.HitObject*, i32)"(i32 386, %dx.types.HitObject* %hit, i32 42), !dbg !42 ; line:91 col:11
  %add21 = add i32 %add19, %19, !dbg !43 ; line:91 col:8
  %20 = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 379, %dx.types.HitObject* %hit), !dbg !44 ; line:94 col:11
  %add23 = fadd <3 x float> zeroinitializer, %20, !dbg !45 ; line:94 col:8
  %21 = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 378, %dx.types.HitObject* %hit), !dbg !46 ; line:95 col:11
  %add25 = fadd <3 x float> %add23, %21, !dbg !47 ; line:95 col:8
  %22 = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 370, %dx.types.HitObject* %hit), !dbg !48 ; line:96 col:11
  %add27 = fadd <3 x float> %add25, %22, !dbg !49 ; line:96 col:8
  %23 = call <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32 369, %dx.types.HitObject* %hit), !dbg !50 ; line:97 col:11
  %add29 = fadd <3 x float> %add27, %23, !dbg !51 ; line:97 col:8
  %vsum.0.vec.extract = extractelement <3 x float> %add29, i32 0, !dbg !52 ; line:98 col:11
  %vsum.4.vec.extract = extractelement <3 x float> %add29, i32 1, !dbg !53 ; line:98 col:21
  %add30 = fadd float %vsum.0.vec.extract, %vsum.4.vec.extract, !dbg !54 ; line:98 col:19
  %vsum.8.vec.extract = extractelement <3 x float> %add29, i32 2, !dbg !55 ; line:98 col:31
  %add31 = fadd float %add30, %vsum.8.vec.extract, !dbg !56 ; line:98 col:29
  %add32 = fadd float 0.000000e+00, %add31, !dbg !57 ; line:98 col:8
  %24 = call <12 x float> @"dx.hl.op.rn.<12 x float> (i32, %dx.types.HitObject*)"(i32 371, %dx.types.HitObject* %hit), !dbg !58 ; line:101 col:23
  %row2col = shufflevector <12 x float> %24, <12 x float> %24, <12 x i32> <i32 0, i32 4, i32 8, i32 1, i32 5, i32 9, i32 2, i32 6, i32 10, i32 3, i32 7, i32 11>, !dbg !59 ; line:101 col:11
  br label %for.body.7.i.lr.ph, !dbg !60 ; line:61 col:3

for.body.7.i.lr.ph:                               ; preds = %for.cond.cleanup.6.i, %for.body.i.lr.ph
  %i.i.0 = phi i32 [ 0, %for.body.i.lr.ph ], [ %inc9.i, %for.cond.cleanup.6.i ]
  %h.i.0 = phi float [ 0.000000e+00, %for.body.i.lr.ph ], [ %add.i, %for.cond.cleanup.6.i ]
  br label %for.body.7.i, !dbg !63 ; line:62 col:5

for.cond.cleanup.6.i:                             ; preds = %for.body.7.i
  %inc9.i = add nsw i32 %i.i.0, 1, !dbg !64 ; line:61 col:26
  %cmp.i = icmp slt i32 %inc9.i, 3, !dbg !65 ; line:61 col:21
  br i1 %cmp.i, label %for.body.7.i.lr.ph, label %for.body.i.8.lr.ph, !dbg !60 ; line:61 col:3

for.body.7.i:                                     ; preds = %for.body.7.i.lr.ph, %for.body.7.i
  %h.i.263 = phi float [ %h.i.0, %for.body.7.i.lr.ph ], [ %add.i, %for.body.7.i ]
  %j.i.0 = phi i32 [ 0, %for.body.7.i.lr.ph ], [ %inc.i, %for.body.7.i ]
  %25 = add i32 3, %i.i.0, !dbg !66 ; line:63 col:12
  %26 = add i32 6, %i.i.0, !dbg !66 ; line:63 col:12
  %27 = add i32 9, %i.i.0, !dbg !66 ; line:63 col:12
  %28 = getelementptr [4 x i32], [4 x i32]* %7, i32 0, i32 0, !dbg !66 ; line:63 col:12
  store i32 %i.i.0, i32* %28, !dbg !66 ; line:63 col:12
  %29 = getelementptr [4 x i32], [4 x i32]* %7, i32 0, i32 1, !dbg !66 ; line:63 col:12
  store i32 %25, i32* %29, !dbg !66 ; line:63 col:12
  %30 = getelementptr [4 x i32], [4 x i32]* %7, i32 0, i32 2, !dbg !66 ; line:63 col:12
  store i32 %26, i32* %30, !dbg !66 ; line:63 col:12
  %31 = getelementptr [4 x i32], [4 x i32]* %7, i32 0, i32 3, !dbg !66 ; line:63 col:12
  store i32 %27, i32* %31, !dbg !66 ; line:63 col:12
  %32 = getelementptr [4 x i32], [4 x i32]* %7, i32 0, i32 %j.i.0, !dbg !66 ; line:63 col:12
  %33 = load i32, i32* %32, !dbg !66 ; line:63 col:12
  %34 = extractelement <12 x float> %row2col, i64 0, !dbg !66 ; line:63 col:12
  %35 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 0, !dbg !66 ; line:63 col:12
  store float %34, float* %35, !dbg !66 ; line:63 col:12
  %36 = extractelement <12 x float> %row2col, i64 1, !dbg !66 ; line:63 col:12
  %37 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 1, !dbg !66 ; line:63 col:12
  store float %36, float* %37, !dbg !66 ; line:63 col:12
  %38 = extractelement <12 x float> %row2col, i64 2, !dbg !66 ; line:63 col:12
  %39 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 2, !dbg !66 ; line:63 col:12
  store float %38, float* %39, !dbg !66 ; line:63 col:12
  %40 = extractelement <12 x float> %row2col, i64 3, !dbg !66 ; line:63 col:12
  %41 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 3, !dbg !66 ; line:63 col:12
  store float %40, float* %41, !dbg !66 ; line:63 col:12
  %42 = extractelement <12 x float> %row2col, i64 4, !dbg !66 ; line:63 col:12
  %43 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 4, !dbg !66 ; line:63 col:12
  store float %42, float* %43, !dbg !66 ; line:63 col:12
  %44 = extractelement <12 x float> %row2col, i64 5, !dbg !66 ; line:63 col:12
  %45 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 5, !dbg !66 ; line:63 col:12
  store float %44, float* %45, !dbg !66 ; line:63 col:12
  %46 = extractelement <12 x float> %row2col, i64 6, !dbg !66 ; line:63 col:12
  %47 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 6, !dbg !66 ; line:63 col:12
  store float %46, float* %47, !dbg !66 ; line:63 col:12
  %48 = extractelement <12 x float> %row2col, i64 7, !dbg !66 ; line:63 col:12
  %49 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 7, !dbg !66 ; line:63 col:12
  store float %48, float* %49, !dbg !66 ; line:63 col:12
  %50 = extractelement <12 x float> %row2col, i64 8, !dbg !66 ; line:63 col:12
  %51 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 8, !dbg !66 ; line:63 col:12
  store float %50, float* %51, !dbg !66 ; line:63 col:12
  %52 = extractelement <12 x float> %row2col, i64 9, !dbg !66 ; line:63 col:12
  %53 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 9, !dbg !66 ; line:63 col:12
  store float %52, float* %53, !dbg !66 ; line:63 col:12
  %54 = extractelement <12 x float> %row2col, i64 10, !dbg !66 ; line:63 col:12
  %55 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 10, !dbg !66 ; line:63 col:12
  store float %54, float* %55, !dbg !66 ; line:63 col:12
  %56 = extractelement <12 x float> %row2col, i64 11, !dbg !66 ; line:63 col:12
  %57 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 11, !dbg !66 ; line:63 col:12
  store float %56, float* %57, !dbg !66 ; line:63 col:12
  %58 = getelementptr [12 x float], [12 x float]* %6, i32 0, i32 %33, !dbg !66 ; line:63 col:12
  %59 = load float, float* %58, !dbg !66 ; line:63 col:12
  %add.i = fadd float %h.i.263, %59, !dbg !67 ; line:63 col:9
  %inc.i = add nsw i32 %j.i.0, 1, !dbg !68 ; line:62 col:28
  %cmp3.i = icmp slt i32 %inc.i, 4, !dbg !69 ; line:62 col:23
  br i1 %cmp3.i, label %for.body.7.i, label %for.cond.cleanup.6.i, !dbg !63 ; line:62 col:5

for.body.i.8.lr.ph:                               ; preds = %for.cond.cleanup.6.i
  %add35 = fadd float %add32, %add.i, !dbg !70 ; line:101 col:8
  %60 = call <12 x float> @"dx.hl.op.rn.<12 x float> (i32, %dx.types.HitObject*)"(i32 372, %dx.types.HitObject* %hit), !dbg !71 ; line:102 col:23
  %row2col52 = shufflevector <12 x float> %60, <12 x float> %60, <12 x i32> <i32 0, i32 3, i32 6, i32 9, i32 1, i32 4, i32 7, i32 10, i32 2, i32 5, i32 8, i32 11>, !dbg !72 ; line:102 col:11
  br label %for.body.7.i.15.lr.ph, !dbg !73 ; line:61 col:3

for.body.7.i.15.lr.ph:                            ; preds = %for.cond.cleanup.6.i.12, %for.body.i.8.lr.ph
  %i.i.3.0 = phi i32 [ 0, %for.body.i.8.lr.ph ], [ %inc9.i.11, %for.cond.cleanup.6.i.12 ]
  %h.i.2.0 = phi float [ 0.000000e+00, %for.body.i.8.lr.ph ], [ %add.i.13, %for.cond.cleanup.6.i.12 ]
  br label %for.body.7.i.15, !dbg !76 ; line:62 col:5

for.cond.cleanup.6.i.12:                          ; preds = %for.body.7.i.15
  %inc9.i.11 = add nsw i32 %i.i.3.0, 1, !dbg !77 ; line:61 col:26
  %cmp.i.6 = icmp slt i32 %inc9.i.11, 4, !dbg !78 ; line:61 col:21
  br i1 %cmp.i.6, label %for.body.7.i.15.lr.ph, label %for.body.i.23.lr.ph, !dbg !73 ; line:61 col:3

for.body.7.i.15:                                  ; preds = %for.body.7.i.15.lr.ph, %for.body.7.i.15
  %j.i.5.0 = phi i32 [ 0, %for.body.7.i.15.lr.ph ], [ %inc.i.14, %for.body.7.i.15 ]
  %h.i.2.2 = phi float [ %h.i.2.0, %for.body.7.i.15.lr.ph ], [ %add.i.13, %for.body.7.i.15 ]
  %61 = add i32 4, %i.i.3.0, !dbg !79 ; line:63 col:12
  %62 = add i32 8, %i.i.3.0, !dbg !79 ; line:63 col:12
  %63 = getelementptr [3 x i32], [3 x i32]* %5, i32 0, i32 0, !dbg !79 ; line:63 col:12
  store i32 %i.i.3.0, i32* %63, !dbg !79 ; line:63 col:12
  %64 = getelementptr [3 x i32], [3 x i32]* %5, i32 0, i32 1, !dbg !79 ; line:63 col:12
  store i32 %61, i32* %64, !dbg !79 ; line:63 col:12
  %65 = getelementptr [3 x i32], [3 x i32]* %5, i32 0, i32 2, !dbg !79 ; line:63 col:12
  store i32 %62, i32* %65, !dbg !79 ; line:63 col:12
  %66 = getelementptr [3 x i32], [3 x i32]* %5, i32 0, i32 %j.i.5.0, !dbg !79 ; line:63 col:12
  %67 = load i32, i32* %66, !dbg !79 ; line:63 col:12
  %68 = extractelement <12 x float> %row2col52, i64 0, !dbg !79 ; line:63 col:12
  %69 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 0, !dbg !79 ; line:63 col:12
  store float %68, float* %69, !dbg !79 ; line:63 col:12
  %70 = extractelement <12 x float> %row2col52, i64 1, !dbg !79 ; line:63 col:12
  %71 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 1, !dbg !79 ; line:63 col:12
  store float %70, float* %71, !dbg !79 ; line:63 col:12
  %72 = extractelement <12 x float> %row2col52, i64 2, !dbg !79 ; line:63 col:12
  %73 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 2, !dbg !79 ; line:63 col:12
  store float %72, float* %73, !dbg !79 ; line:63 col:12
  %74 = extractelement <12 x float> %row2col52, i64 3, !dbg !79 ; line:63 col:12
  %75 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 3, !dbg !79 ; line:63 col:12
  store float %74, float* %75, !dbg !79 ; line:63 col:12
  %76 = extractelement <12 x float> %row2col52, i64 4, !dbg !79 ; line:63 col:12
  %77 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 4, !dbg !79 ; line:63 col:12
  store float %76, float* %77, !dbg !79 ; line:63 col:12
  %78 = extractelement <12 x float> %row2col52, i64 5, !dbg !79 ; line:63 col:12
  %79 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 5, !dbg !79 ; line:63 col:12
  store float %78, float* %79, !dbg !79 ; line:63 col:12
  %80 = extractelement <12 x float> %row2col52, i64 6, !dbg !79 ; line:63 col:12
  %81 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 6, !dbg !79 ; line:63 col:12
  store float %80, float* %81, !dbg !79 ; line:63 col:12
  %82 = extractelement <12 x float> %row2col52, i64 7, !dbg !79 ; line:63 col:12
  %83 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 7, !dbg !79 ; line:63 col:12
  store float %82, float* %83, !dbg !79 ; line:63 col:12
  %84 = extractelement <12 x float> %row2col52, i64 8, !dbg !79 ; line:63 col:12
  %85 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 8, !dbg !79 ; line:63 col:12
  store float %84, float* %85, !dbg !79 ; line:63 col:12
  %86 = extractelement <12 x float> %row2col52, i64 9, !dbg !79 ; line:63 col:12
  %87 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 9, !dbg !79 ; line:63 col:12
  store float %86, float* %87, !dbg !79 ; line:63 col:12
  %88 = extractelement <12 x float> %row2col52, i64 10, !dbg !79 ; line:63 col:12
  %89 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 10, !dbg !79 ; line:63 col:12
  store float %88, float* %89, !dbg !79 ; line:63 col:12
  %90 = extractelement <12 x float> %row2col52, i64 11, !dbg !79 ; line:63 col:12
  %91 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 11, !dbg !79 ; line:63 col:12
  store float %90, float* %91, !dbg !79 ; line:63 col:12
  %92 = getelementptr [12 x float], [12 x float]* %4, i32 0, i32 %67, !dbg !79 ; line:63 col:12
  %93 = load float, float* %92, !dbg !79 ; line:63 col:12
  %add.i.13 = fadd float %h.i.2.2, %93, !dbg !80 ; line:63 col:9
  %inc.i.14 = add nsw i32 %j.i.5.0, 1, !dbg !81 ; line:62 col:28
  %cmp3.i.9 = icmp slt i32 %inc.i.14, 3, !dbg !82 ; line:62 col:23
  br i1 %cmp3.i.9, label %for.body.7.i.15, label %for.cond.cleanup.6.i.12, !dbg !76 ; line:62 col:5

for.body.i.23.lr.ph:                              ; preds = %for.cond.cleanup.6.i.12
  %add38 = fadd float %add35, %add.i.13, !dbg !83 ; line:102 col:8
  %94 = call <12 x float> @"dx.hl.op.rn.<12 x float> (i32, %dx.types.HitObject*)"(i32 380, %dx.types.HitObject* %hit), !dbg !84 ; line:103 col:23
  %row2col53 = shufflevector <12 x float> %94, <12 x float> %94, <12 x i32> <i32 0, i32 4, i32 8, i32 1, i32 5, i32 9, i32 2, i32 6, i32 10, i32 3, i32 7, i32 11>, !dbg !85 ; line:103 col:11
  br label %for.body.7.i.30.lr.ph, !dbg !86 ; line:61 col:3

for.body.7.i.30.lr.ph:                            ; preds = %for.cond.cleanup.6.i.27, %for.body.i.23.lr.ph
  %i.i.18.0 = phi i32 [ 0, %for.body.i.23.lr.ph ], [ %inc9.i.26, %for.cond.cleanup.6.i.27 ]
  %h.i.17.0 = phi float [ 0.000000e+00, %for.body.i.23.lr.ph ], [ %add.i.28, %for.cond.cleanup.6.i.27 ]
  br label %for.body.7.i.30, !dbg !88 ; line:62 col:5

for.cond.cleanup.6.i.27:                          ; preds = %for.body.7.i.30
  %inc9.i.26 = add nsw i32 %i.i.18.0, 1, !dbg !89 ; line:61 col:26
  %cmp.i.21 = icmp slt i32 %inc9.i.26, 3, !dbg !90 ; line:61 col:21
  br i1 %cmp.i.21, label %for.body.7.i.30.lr.ph, label %for.body.i.39.lr.ph, !dbg !86 ; line:61 col:3

for.body.7.i.30:                                  ; preds = %for.body.7.i.30.lr.ph, %for.body.7.i.30
  %j.i.20.0 = phi i32 [ 0, %for.body.7.i.30.lr.ph ], [ %inc.i.29, %for.body.7.i.30 ]
  %h.i.17.2 = phi float [ %h.i.17.0, %for.body.7.i.30.lr.ph ], [ %add.i.28, %for.body.7.i.30 ]
  %95 = add i32 3, %i.i.18.0, !dbg !91 ; line:63 col:12
  %96 = add i32 6, %i.i.18.0, !dbg !91 ; line:63 col:12
  %97 = add i32 9, %i.i.18.0, !dbg !91 ; line:63 col:12
  %98 = getelementptr [4 x i32], [4 x i32]* %3, i32 0, i32 0, !dbg !91 ; line:63 col:12
  store i32 %i.i.18.0, i32* %98, !dbg !91 ; line:63 col:12
  %99 = getelementptr [4 x i32], [4 x i32]* %3, i32 0, i32 1, !dbg !91 ; line:63 col:12
  store i32 %95, i32* %99, !dbg !91 ; line:63 col:12
  %100 = getelementptr [4 x i32], [4 x i32]* %3, i32 0, i32 2, !dbg !91 ; line:63 col:12
  store i32 %96, i32* %100, !dbg !91 ; line:63 col:12
  %101 = getelementptr [4 x i32], [4 x i32]* %3, i32 0, i32 3, !dbg !91 ; line:63 col:12
  store i32 %97, i32* %101, !dbg !91 ; line:63 col:12
  %102 = getelementptr [4 x i32], [4 x i32]* %3, i32 0, i32 %j.i.20.0, !dbg !91 ; line:63 col:12
  %103 = load i32, i32* %102, !dbg !91 ; line:63 col:12
  %104 = extractelement <12 x float> %row2col53, i64 0, !dbg !91 ; line:63 col:12
  %105 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 0, !dbg !91 ; line:63 col:12
  store float %104, float* %105, !dbg !91 ; line:63 col:12
  %106 = extractelement <12 x float> %row2col53, i64 1, !dbg !91 ; line:63 col:12
  %107 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 1, !dbg !91 ; line:63 col:12
  store float %106, float* %107, !dbg !91 ; line:63 col:12
  %108 = extractelement <12 x float> %row2col53, i64 2, !dbg !91 ; line:63 col:12
  %109 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 2, !dbg !91 ; line:63 col:12
  store float %108, float* %109, !dbg !91 ; line:63 col:12
  %110 = extractelement <12 x float> %row2col53, i64 3, !dbg !91 ; line:63 col:12
  %111 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 3, !dbg !91 ; line:63 col:12
  store float %110, float* %111, !dbg !91 ; line:63 col:12
  %112 = extractelement <12 x float> %row2col53, i64 4, !dbg !91 ; line:63 col:12
  %113 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 4, !dbg !91 ; line:63 col:12
  store float %112, float* %113, !dbg !91 ; line:63 col:12
  %114 = extractelement <12 x float> %row2col53, i64 5, !dbg !91 ; line:63 col:12
  %115 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 5, !dbg !91 ; line:63 col:12
  store float %114, float* %115, !dbg !91 ; line:63 col:12
  %116 = extractelement <12 x float> %row2col53, i64 6, !dbg !91 ; line:63 col:12
  %117 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 6, !dbg !91 ; line:63 col:12
  store float %116, float* %117, !dbg !91 ; line:63 col:12
  %118 = extractelement <12 x float> %row2col53, i64 7, !dbg !91 ; line:63 col:12
  %119 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 7, !dbg !91 ; line:63 col:12
  store float %118, float* %119, !dbg !91 ; line:63 col:12
  %120 = extractelement <12 x float> %row2col53, i64 8, !dbg !91 ; line:63 col:12
  %121 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 8, !dbg !91 ; line:63 col:12
  store float %120, float* %121, !dbg !91 ; line:63 col:12
  %122 = extractelement <12 x float> %row2col53, i64 9, !dbg !91 ; line:63 col:12
  %123 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 9, !dbg !91 ; line:63 col:12
  store float %122, float* %123, !dbg !91 ; line:63 col:12
  %124 = extractelement <12 x float> %row2col53, i64 10, !dbg !91 ; line:63 col:12
  %125 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 10, !dbg !91 ; line:63 col:12
  store float %124, float* %125, !dbg !91 ; line:63 col:12
  %126 = extractelement <12 x float> %row2col53, i64 11, !dbg !91 ; line:63 col:12
  %127 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 11, !dbg !91 ; line:63 col:12
  store float %126, float* %127, !dbg !91 ; line:63 col:12
  %128 = getelementptr [12 x float], [12 x float]* %2, i32 0, i32 %103, !dbg !91 ; line:63 col:12
  %129 = load float, float* %128, !dbg !91 ; line:63 col:12
  %add.i.28 = fadd float %h.i.17.2, %129, !dbg !92 ; line:63 col:9
  %inc.i.29 = add nsw i32 %j.i.20.0, 1, !dbg !93 ; line:62 col:28
  %cmp3.i.24 = icmp slt i32 %inc.i.29, 4, !dbg !94 ; line:62 col:23
  br i1 %cmp3.i.24, label %for.body.7.i.30, label %for.cond.cleanup.6.i.27, !dbg !88 ; line:62 col:5

for.body.i.39.lr.ph:                              ; preds = %for.cond.cleanup.6.i.27
  %add41 = fadd float %add38, %add.i.28, !dbg !95 ; line:103 col:8
  %130 = call <12 x float> @"dx.hl.op.rn.<12 x float> (i32, %dx.types.HitObject*)"(i32 381, %dx.types.HitObject* %hit), !dbg !96 ; line:104 col:23
  %row2col54 = shufflevector <12 x float> %130, <12 x float> %130, <12 x i32> <i32 0, i32 3, i32 6, i32 9, i32 1, i32 4, i32 7, i32 10, i32 2, i32 5, i32 8, i32 11>, !dbg !97 ; line:104 col:11
  br label %for.body.7.i.46.lr.ph, !dbg !98 ; line:61 col:3

for.body.7.i.46.lr.ph:                            ; preds = %for.cond.cleanup.6.i.43, %for.body.i.39.lr.ph
  %i.i.34.0 = phi i32 [ 0, %for.body.i.39.lr.ph ], [ %inc9.i.42, %for.cond.cleanup.6.i.43 ]
  %h.i.33.0 = phi float [ 0.000000e+00, %for.body.i.39.lr.ph ], [ %add.i.44, %for.cond.cleanup.6.i.43 ]
  br label %for.body.7.i.46, !dbg !100 ; line:62 col:5

for.cond.cleanup.6.i.43:                          ; preds = %for.body.7.i.46
  %inc9.i.42 = add nsw i32 %i.i.34.0, 1, !dbg !101 ; line:61 col:26
  %cmp.i.37 = icmp slt i32 %inc9.i.42, 4, !dbg !102 ; line:61 col:21
  br i1 %cmp.i.37, label %for.body.7.i.46.lr.ph, label %"\01??$hashM@$03$02@@YAMV?$matrix@M$03$02@@@Z.exit.47", !dbg !98 ; line:61 col:3

for.body.7.i.46:                                  ; preds = %for.body.7.i.46.lr.ph, %for.body.7.i.46
  %j.i.36.0 = phi i32 [ 0, %for.body.7.i.46.lr.ph ], [ %inc.i.45, %for.body.7.i.46 ]
  %h.i.33.2 = phi float [ %h.i.33.0, %for.body.7.i.46.lr.ph ], [ %add.i.44, %for.body.7.i.46 ]
  %131 = add i32 4, %i.i.34.0, !dbg !103 ; line:63 col:12
  %132 = add i32 8, %i.i.34.0, !dbg !103 ; line:63 col:12
  %133 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i32 0, !dbg !103 ; line:63 col:12
  store i32 %i.i.34.0, i32* %133, !dbg !103 ; line:63 col:12
  %134 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i32 1, !dbg !103 ; line:63 col:12
  store i32 %131, i32* %134, !dbg !103 ; line:63 col:12
  %135 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i32 2, !dbg !103 ; line:63 col:12
  store i32 %132, i32* %135, !dbg !103 ; line:63 col:12
  %136 = getelementptr [3 x i32], [3 x i32]* %1, i32 0, i32 %j.i.36.0, !dbg !103 ; line:63 col:12
  %137 = load i32, i32* %136, !dbg !103 ; line:63 col:12
  %138 = extractelement <12 x float> %row2col54, i64 0, !dbg !103 ; line:63 col:12
  %139 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 0, !dbg !103 ; line:63 col:12
  store float %138, float* %139, !dbg !103 ; line:63 col:12
  %140 = extractelement <12 x float> %row2col54, i64 1, !dbg !103 ; line:63 col:12
  %141 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 1, !dbg !103 ; line:63 col:12
  store float %140, float* %141, !dbg !103 ; line:63 col:12
  %142 = extractelement <12 x float> %row2col54, i64 2, !dbg !103 ; line:63 col:12
  %143 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 2, !dbg !103 ; line:63 col:12
  store float %142, float* %143, !dbg !103 ; line:63 col:12
  %144 = extractelement <12 x float> %row2col54, i64 3, !dbg !103 ; line:63 col:12
  %145 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 3, !dbg !103 ; line:63 col:12
  store float %144, float* %145, !dbg !103 ; line:63 col:12
  %146 = extractelement <12 x float> %row2col54, i64 4, !dbg !103 ; line:63 col:12
  %147 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 4, !dbg !103 ; line:63 col:12
  store float %146, float* %147, !dbg !103 ; line:63 col:12
  %148 = extractelement <12 x float> %row2col54, i64 5, !dbg !103 ; line:63 col:12
  %149 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 5, !dbg !103 ; line:63 col:12
  store float %148, float* %149, !dbg !103 ; line:63 col:12
  %150 = extractelement <12 x float> %row2col54, i64 6, !dbg !103 ; line:63 col:12
  %151 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 6, !dbg !103 ; line:63 col:12
  store float %150, float* %151, !dbg !103 ; line:63 col:12
  %152 = extractelement <12 x float> %row2col54, i64 7, !dbg !103 ; line:63 col:12
  %153 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 7, !dbg !103 ; line:63 col:12
  store float %152, float* %153, !dbg !103 ; line:63 col:12
  %154 = extractelement <12 x float> %row2col54, i64 8, !dbg !103 ; line:63 col:12
  %155 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 8, !dbg !103 ; line:63 col:12
  store float %154, float* %155, !dbg !103 ; line:63 col:12
  %156 = extractelement <12 x float> %row2col54, i64 9, !dbg !103 ; line:63 col:12
  %157 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 9, !dbg !103 ; line:63 col:12
  store float %156, float* %157, !dbg !103 ; line:63 col:12
  %158 = extractelement <12 x float> %row2col54, i64 10, !dbg !103 ; line:63 col:12
  %159 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 10, !dbg !103 ; line:63 col:12
  store float %158, float* %159, !dbg !103 ; line:63 col:12
  %160 = extractelement <12 x float> %row2col54, i64 11, !dbg !103 ; line:63 col:12
  %161 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 11, !dbg !103 ; line:63 col:12
  store float %160, float* %161, !dbg !103 ; line:63 col:12
  %162 = getelementptr [12 x float], [12 x float]* %0, i32 0, i32 %137, !dbg !103 ; line:63 col:12
  %163 = load float, float* %162, !dbg !103 ; line:63 col:12
  %add.i.44 = fadd float %h.i.33.2, %163, !dbg !104 ; line:63 col:9
  %inc.i.45 = add nsw i32 %j.i.36.0, 1, !dbg !105 ; line:62 col:28
  %cmp3.i.40 = icmp slt i32 %inc.i.45, 3, !dbg !106 ; line:62 col:23
  br i1 %cmp3.i.40, label %for.body.7.i.46, label %for.cond.cleanup.6.i.43, !dbg !100 ; line:62 col:5

"\01??$hashM@$03$02@@YAMV?$matrix@M$03$02@@@Z.exit.47": ; preds = %for.cond.cleanup.6.i.43
  %add44 = fadd float %add41, %add.i.44, !dbg !107 ; line:104 col:8
  %164 = call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32 374, %dx.types.HitObject* %hit), !dbg !108 ; line:107 col:11
  %add46 = add i32 %add21, %164, !dbg !109 ; line:107 col:8
  %165 = call float @"dx.hl.op.rn.float (i32, %dx.types.HitObject*)"(i32 376, %dx.types.HitObject* %hit), !dbg !110 ; line:108 col:11
  %add48 = fadd float %add44, %165, !dbg !111 ; line:108 col:8
  %166 = call float @"dx.hl.op.rn.float (i32, %dx.types.HitObject*)"(i32 375, %dx.types.HitObject* %hit), !dbg !112 ; line:109 col:11
  %add50 = fadd float %add48, %166, !dbg !113 ; line:109 col:8
  %167 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !114 ; line:111 col:3
  %168 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %167), !dbg !114 ; line:111 col:3
  %169 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %168, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !114 ; line:111 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %169, i32 0, float %add50), !dbg !114 ; line:111 col:3
  %170 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !115 ; line:112 col:3
  %171 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %170), !dbg !115 ; line:112 col:3
  %172 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %171, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !115 ; line:112 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32 277, %dx.types.Handle %172, i32 4, i32 %add46), !dbg !115 ; line:112 col:3
  %173 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !116 ; line:113 col:1
  call void @llvm.lifetime.end(i64 4, i8* %173) #0, !dbg !116 ; line:113 col:1
  ret void, !dbg !116 ; line:113 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, i32)"(i32, %dx.types.HitObject*, i32) #0

; Function Attrs: nounwind readnone
declare i1 @"dx.hl.op.rn.i1 (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind readnone
declare i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind readonly
declare i32 @"dx.hl.op.ro.i32 (i32, %dx.types.HitObject*, i32)"(i32, %dx.types.HitObject*, i32) #2

; Function Attrs: nounwind readnone
declare <3 x float> @"dx.hl.op.rn.<3 x float> (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind readnone
declare float @"dx.hl.op.rn.float (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32, %dx.types.Handle, i32, float) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32)"(i32, %dx.types.Handle, i32, i32) #0

; Function Attrs: nounwind readnone
declare <12 x float> @"dx.hl.op.rn.<12 x float> (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !8}
!dx.entryPoints = !{!12}
!dx.fnprops = !{!16}
!dx.options = !{!17, !18}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4891 (staging/ser_hlslaccessors_patch, 1ca27ee12)"}
!3 = !{i32 1, i32 9}
!4 = !{!"lib", i32 6, i32 9}
!5 = !{i32 0, %"class.dx::HitObject" undef, !6}
!6 = !{i32 4, !7}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!8 = !{i32 1, void ()* @"\01?main@@YAXXZ", !9}
!9 = !{!10}
!10 = !{i32 1, !11, !11}
!11 = !{}
!12 = !{null, !"", null, !13, null}
!13 = !{null, !14, null, null}
!14 = !{!15}
!15 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !"outbuf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!16 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!17 = !{i32 -2147483584}
!18 = !{i32 -1}
!19 = !DILocation(line: 69, column: 3, scope: !20)
!20 = !DISubprogram(name: "main", scope: !21, file: !21, line: 68, type: !22, isLocal: false, isDefinition: true, scopeLine: 68, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!21 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/HitObject/hitobject_accessors.hlsl", directory: "")
!22 = !DISubroutineType(types: !11)
!23 = !DILocation(line: 69, column: 17, scope: !20)
!24 = !DILocation(line: 75, column: 3, scope: !20)
!25 = !DILocation(line: 80, column: 11, scope: !20)
!26 = !DILocation(line: 81, column: 11, scope: !20)
!27 = !DILocation(line: 81, column: 8, scope: !20)
!28 = !DILocation(line: 82, column: 11, scope: !20)
!29 = !DILocation(line: 82, column: 8, scope: !20)
!30 = !DILocation(line: 85, column: 11, scope: !20)
!31 = !DILocation(line: 85, column: 8, scope: !20)
!32 = !DILocation(line: 86, column: 11, scope: !20)
!33 = !DILocation(line: 86, column: 8, scope: !20)
!34 = !DILocation(line: 87, column: 11, scope: !20)
!35 = !DILocation(line: 87, column: 8, scope: !20)
!36 = !DILocation(line: 88, column: 11, scope: !20)
!37 = !DILocation(line: 88, column: 8, scope: !20)
!38 = !DILocation(line: 89, column: 11, scope: !20)
!39 = !DILocation(line: 89, column: 8, scope: !20)
!40 = !DILocation(line: 90, column: 11, scope: !20)
!41 = !DILocation(line: 90, column: 8, scope: !20)
!42 = !DILocation(line: 91, column: 11, scope: !20)
!43 = !DILocation(line: 91, column: 8, scope: !20)
!44 = !DILocation(line: 94, column: 11, scope: !20)
!45 = !DILocation(line: 94, column: 8, scope: !20)
!46 = !DILocation(line: 95, column: 11, scope: !20)
!47 = !DILocation(line: 95, column: 8, scope: !20)
!48 = !DILocation(line: 96, column: 11, scope: !20)
!49 = !DILocation(line: 96, column: 8, scope: !20)
!50 = !DILocation(line: 97, column: 11, scope: !20)
!51 = !DILocation(line: 97, column: 8, scope: !20)
!52 = !DILocation(line: 98, column: 11, scope: !20)
!53 = !DILocation(line: 98, column: 21, scope: !20)
!54 = !DILocation(line: 98, column: 19, scope: !20)
!55 = !DILocation(line: 98, column: 31, scope: !20)
!56 = !DILocation(line: 98, column: 29, scope: !20)
!57 = !DILocation(line: 98, column: 8, scope: !20)
!58 = !DILocation(line: 101, column: 23, scope: !20)
!59 = !DILocation(line: 101, column: 11, scope: !20)
!60 = !DILocation(line: 61, column: 3, scope: !61, inlinedAt: !62)
!61 = !DISubprogram(name: "hashM<3, 4>", scope: !21, file: !21, line: 59, type: !22, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false)
!62 = distinct !DILocation(line: 101, column: 11, scope: !20)
!63 = !DILocation(line: 62, column: 5, scope: !61, inlinedAt: !62)
!64 = !DILocation(line: 61, column: 26, scope: !61, inlinedAt: !62)
!65 = !DILocation(line: 61, column: 21, scope: !61, inlinedAt: !62)
!66 = !DILocation(line: 63, column: 12, scope: !61, inlinedAt: !62)
!67 = !DILocation(line: 63, column: 9, scope: !61, inlinedAt: !62)
!68 = !DILocation(line: 62, column: 28, scope: !61, inlinedAt: !62)
!69 = !DILocation(line: 62, column: 23, scope: !61, inlinedAt: !62)
!70 = !DILocation(line: 101, column: 8, scope: !20)
!71 = !DILocation(line: 102, column: 23, scope: !20)
!72 = !DILocation(line: 102, column: 11, scope: !20)
!73 = !DILocation(line: 61, column: 3, scope: !74, inlinedAt: !75)
!74 = !DISubprogram(name: "hashM<4, 3>", scope: !21, file: !21, line: 59, type: !22, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: false)
!75 = distinct !DILocation(line: 102, column: 11, scope: !20)
!76 = !DILocation(line: 62, column: 5, scope: !74, inlinedAt: !75)
!77 = !DILocation(line: 61, column: 26, scope: !74, inlinedAt: !75)
!78 = !DILocation(line: 61, column: 21, scope: !74, inlinedAt: !75)
!79 = !DILocation(line: 63, column: 12, scope: !74, inlinedAt: !75)
!80 = !DILocation(line: 63, column: 9, scope: !74, inlinedAt: !75)
!81 = !DILocation(line: 62, column: 28, scope: !74, inlinedAt: !75)
!82 = !DILocation(line: 62, column: 23, scope: !74, inlinedAt: !75)
!83 = !DILocation(line: 102, column: 8, scope: !20)
!84 = !DILocation(line: 103, column: 23, scope: !20)
!85 = !DILocation(line: 103, column: 11, scope: !20)
!86 = !DILocation(line: 61, column: 3, scope: !61, inlinedAt: !87)
!87 = distinct !DILocation(line: 103, column: 11, scope: !20)
!88 = !DILocation(line: 62, column: 5, scope: !61, inlinedAt: !87)
!89 = !DILocation(line: 61, column: 26, scope: !61, inlinedAt: !87)
!90 = !DILocation(line: 61, column: 21, scope: !61, inlinedAt: !87)
!91 = !DILocation(line: 63, column: 12, scope: !61, inlinedAt: !87)
!92 = !DILocation(line: 63, column: 9, scope: !61, inlinedAt: !87)
!93 = !DILocation(line: 62, column: 28, scope: !61, inlinedAt: !87)
!94 = !DILocation(line: 62, column: 23, scope: !61, inlinedAt: !87)
!95 = !DILocation(line: 103, column: 8, scope: !20)
!96 = !DILocation(line: 104, column: 23, scope: !20)
!97 = !DILocation(line: 104, column: 11, scope: !20)
!98 = !DILocation(line: 61, column: 3, scope: !74, inlinedAt: !99)
!99 = distinct !DILocation(line: 104, column: 11, scope: !20)
!100 = !DILocation(line: 62, column: 5, scope: !74, inlinedAt: !99)
!101 = !DILocation(line: 61, column: 26, scope: !74, inlinedAt: !99)
!102 = !DILocation(line: 61, column: 21, scope: !74, inlinedAt: !99)
!103 = !DILocation(line: 63, column: 12, scope: !74, inlinedAt: !99)
!104 = !DILocation(line: 63, column: 9, scope: !74, inlinedAt: !99)
!105 = !DILocation(line: 62, column: 28, scope: !74, inlinedAt: !99)
!106 = !DILocation(line: 62, column: 23, scope: !74, inlinedAt: !99)
!107 = !DILocation(line: 104, column: 8, scope: !20)
!108 = !DILocation(line: 107, column: 11, scope: !20)
!109 = !DILocation(line: 107, column: 8, scope: !20)
!110 = !DILocation(line: 108, column: 11, scope: !20)
!111 = !DILocation(line: 108, column: 8, scope: !20)
!112 = !DILocation(line: 109, column: 11, scope: !20)
!113 = !DILocation(line: 109, column: 8, scope: !20)
!114 = !DILocation(line: 111, column: 3, scope: !20)
!115 = !DILocation(line: 112, column: 3, scope: !20)
!116 = !DILocation(line: 113, column: 1, scope: !20)
