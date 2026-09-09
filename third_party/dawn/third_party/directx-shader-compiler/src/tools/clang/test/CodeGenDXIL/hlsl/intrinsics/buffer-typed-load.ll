; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWBuffer<vector<bool, 2> >" = type { <2 x i32> }
%"class.Texture2DMS<vector<bool, 2>, 0>" = type { <2 x i32>, %"class.Texture2DMS<vector<bool, 2>, 0>::sample_type" }
%"class.Texture2DMS<vector<bool, 2>, 0>::sample_type" = type { i32 }
%"class.Texture1D<vector<float, 2> >" = type { <2 x float>, %"class.Texture1D<vector<float, 2> >::mips_type" }
%"class.Texture1D<vector<float, 2> >::mips_type" = type { i32 }
%"class.Texture2D<vector<float, 2> >" = type { <2 x float>, %"class.Texture2D<vector<float, 2> >::mips_type" }
%"class.Texture2D<vector<float, 2> >::mips_type" = type { i32 }
%"class.Texture3D<vector<float, 2> >" = type { <2 x float>, %"class.Texture3D<vector<float, 2> >::mips_type" }
%"class.Texture3D<vector<float, 2> >::mips_type" = type { i32 }
%"class.Texture2DArray<vector<float, 2> >" = type { <2 x float>, %"class.Texture2DArray<vector<float, 2> >::mips_type" }
%"class.Texture2DArray<vector<float, 2> >::mips_type" = type { i32 }
%"class.RWBuffer<vector<float, 2> >" = type { <2 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?TyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A" = external global %"class.RWBuffer<vector<bool, 2> >", align 4
@"\01?Tex2dMs@@3V?$Texture2DMS@V?$vector@_N$01@@$0A@@@A" = external global %"class.Texture2DMS<vector<bool, 2>, 0>", align 4
@"\01?Tex1d@@3V?$Texture1D@V?$vector@M$01@@@@A" = external global %"class.Texture1D<vector<float, 2> >", align 4
@"\01?Tex2d@@3V?$Texture2D@V?$vector@M$01@@@@A" = external global %"class.Texture2D<vector<float, 2> >", align 4
@"\01?Tex3d@@3V?$Texture3D@V?$vector@M$01@@@@A" = external global %"class.Texture3D<vector<float, 2> >", align 4
@"\01?Tex2dArr@@3V?$Texture2DArray@V?$vector@M$01@@@@A" = external global %"class.Texture2DArray<vector<float, 2> >", align 4
@"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A" = external global %"class.RWBuffer<vector<float, 2> >", align 4

; Function Attrs: nounwind
define void @main(i32 %ix1, <2 x i32> %ix2, <3 x i32> %ix3, <4 x i32> %ix4) #0 {
  ; CHECK: [[PIX:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0,
  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 1
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<bool, 2> >"(i32 160, %"class.RWBuffer<vector<bool, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[LD:%.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef)
  ; CHECK-DAG: [[V0:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 0
  ; CHECK-DAG: [[V1:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 1
  ; CHECK-DAG: [[VEC0:%.*]] = insertelement <2 x i32> undef, i32 [[V0]], i64 0
  ; CHECK-DAG: [[VEC1:%.*]] = insertelement <2 x i32> [[VEC0]], i32 [[V1]], i64 1
  ; CHECK: icmp ne <2 x i32> [[VEC1]], zeroinitializer
  %1 = add i32 %ix1, 1
  %2 = load %"class.RWBuffer<vector<bool, 2> >", %"class.RWBuffer<vector<bool, 2> >"* @"\01?TyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A"
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >" %2)
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" zeroinitializer)
  %5 = call <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %4, i32 %1)

  %6 = zext <2 x i1> %5 to <2 x i32>

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 2
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<bool, 2> >"(i32 160, %"class.RWBuffer<vector<bool, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 517 })
  ; CHECK: [[LD:%.*]] = call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef)
  ; CHECK-DAG: [[V0:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 0
  ; CHECK-DAG: [[V1:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 1
  ; CHECK-DAG: [[VEC0:%.*]] = insertelement <2 x i32> undef, i32 [[V0]], i64 0
  ; CHECK-DAG: [[VEC1:%.*]] = insertelement <2 x i32> [[VEC0]], i32 [[V1]], i64 1
  ; CHECK: icmp ne <2 x i32> [[VEC1]], zeroinitializer
  %7 = add i32 %ix1, 2
  %8 = load %"class.RWBuffer<vector<bool, 2> >", %"class.RWBuffer<vector<bool, 2> >"* @"\01?TyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A"
  %9 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >" %8)
  %10 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle %9, %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" zeroinitializer)
  %11 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %10, i32 %7)
  %12 = load <2 x i32>, <2 x i32>* %11

  %13 = icmp ne <2 x i32> %12, zeroinitializer
  %14 = zext <2 x i1> %13 to <2 x i32>

  ; CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 3, i32 3>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.Texture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 3, i32 517 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <2 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <2 x i32> [[IX]], i64 1
  ; CHECK: [[LD:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[ANHDL]], i32 [[PIX]], i32 [[IX0]], i32 [[IX1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK-DAG: [[V0:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 0
  ; CHECK-DAG: [[V1:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 1
  ; CHECK-DAG: [[VEC0:%.*]] = insertelement <2 x i32> undef, i32 [[V0]], i64 0
  ; CHECK-DAG: [[VEC1:%.*]] = insertelement <2 x i32> [[VEC0]], i32 [[V1]], i64 1
  ; CHECK: icmp ne <2 x i32> [[VEC1]], zeroinitializer
  %15 = add <2 x i32> %ix2, <i32 3, i32 3>
  %16 = load %"class.Texture2DMS<vector<bool, 2>, 0>", %"class.Texture2DMS<vector<bool, 2>, 0>"* @"\01?Tex2dMs@@3V?$Texture2DMS@V?$vector@_N$01@@$0A@@@A"
  %17 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.Texture2DMS<vector<bool, 2>, 0>" %16)
  %18 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %17, %dx.types.ResourceProperties { i32 3, i32 517 }, %"class.Texture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %19 = call <2 x i1> @"dx.hl.op..<2 x i1> (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 231, %dx.types.Handle %18, <2 x i32> %15, i32 %ix1)
  %20 = zext <2 x i1> %19 to <2 x i32>

  ; CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 4, i32 4>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2DMS<vector<bool, 2>, 0>"(i32 160, %"class.Texture2DMS<vector<bool, 2>, 0>"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 3, i32 517 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <2 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <2 x i32> [[IX]], i64 1
  ; CHECK: [[LD:%.*]] = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle [[ANHDL]], i32 0, i32 [[IX0]], i32 [[IX1]], i32 undef, i32 undef, i32 undef, i32 undef)
  ; CHECK-DAG: [[V0:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 0
  ; CHECK-DAG: [[V1:%.*]] = extractvalue %dx.types.ResRet.i32 [[LD]], 1
  ; CHECK-DAG: [[VEC0:%.*]] = insertelement <2 x i32> undef, i32 [[V0]], i64 0
  ; CHECK-DAG: [[VEC1:%.*]] = insertelement <2 x i32> [[VEC0]], i32 [[V1]], i64 1
  ; CHECK: icmp ne <2 x i32> [[VEC1]], zeroinitializer
  %21 = add <2 x i32> %ix2, <i32 4, i32 4>
  %22 = load %"class.Texture2DMS<vector<bool, 2>, 0>", %"class.Texture2DMS<vector<bool, 2>, 0>"* @"\01?Tex2dMs@@3V?$Texture2DMS@V?$vector@_N$01@@$0A@@@A"
  %23 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.Texture2DMS<vector<bool, 2>, 0>" %22)
  %24 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle %23, %dx.types.ResourceProperties { i32 3, i32 517 }, %"class.Texture2DMS<vector<bool, 2>, 0>" zeroinitializer)
  %25 = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %24, <2 x i32> %21)
  %26 = load <2 x i32>, <2 x i32>* %25

  %27 = icmp ne <2 x i32> %26, zeroinitializer
  %28 = zext <2 x i1> %27 to <2 x i32>

  ; CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 5, i32 5>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture1D<vector<float, 2> >"(i32 160, %"class.Texture1D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 1, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <2 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <2 x i32> [[IX]], i64 1
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 [[IX1]], i32 [[IX0]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  %29 = add <2 x i32> %ix2, <i32 5, i32 5>
  %30 = load %"class.Texture1D<vector<float, 2> >", %"class.Texture1D<vector<float, 2> >"* @"\01?Tex1d@@3V?$Texture1D@V?$vector@M$01@@@@A"
  %31 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture1D<vector<float, 2> >\22)"(i32 0, %"class.Texture1D<vector<float, 2> >" %30)
  %32 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture1D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %31, %dx.types.ResourceProperties { i32 1, i32 521 }, %"class.Texture1D<vector<float, 2> >" zeroinitializer)
  %33 = call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <2 x i32>)"(i32 231, %dx.types.Handle %32, <2 x i32> %29)

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 6
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture1D<vector<float, 2> >"(i32 160, %"class.Texture1D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 1, i32 521 })
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 0, i32 [[IX]], i32 undef, i32 undef, i32 undef, i32 undef, i32 undef)
  %34 = add i32 %ix1, 6
  %35 = load %"class.Texture1D<vector<float, 2> >", %"class.Texture1D<vector<float, 2> >"* @"\01?Tex1d@@3V?$Texture1D@V?$vector@M$01@@@@A"
  %36 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture1D<vector<float, 2> >\22)"(i32 0, %"class.Texture1D<vector<float, 2> >" %35)
  %37 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture1D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %36, %dx.types.ResourceProperties { i32 1, i32 521 }, %"class.Texture1D<vector<float, 2> >" zeroinitializer)
  %38 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %37, i32 %34)
  %39 = load <2 x float>, <2 x float>* %38

  ; CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 7, i32 7, i32 7>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2D<vector<float, 2> >"(i32 160, %"class.Texture2D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 2, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <3 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <3 x i32> [[IX]], i64 1
  ; CHECK-DAG: [[IX2:%.*]] = extractelement <3 x i32> [[IX]], i64 2
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 [[IX2]], i32 [[IX0]], i32 [[IX1]], i32 undef, i32 undef, i32 undef, i32 undef)
  %40 = add <3 x i32> %ix3, <i32 7, i32 7, i32 7>
  %41 = load %"class.Texture2D<vector<float, 2> >", %"class.Texture2D<vector<float, 2> >"* @"\01?Tex2d@@3V?$Texture2D@V?$vector@M$01@@@@A"
  %42 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 2> >\22)"(i32 0, %"class.Texture2D<vector<float, 2> >" %41)
  %43 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %42, %dx.types.ResourceProperties { i32 2, i32 521 }, %"class.Texture2D<vector<float, 2> >" zeroinitializer)
  %44 = call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32 231, %dx.types.Handle %43, <3 x i32> %40)

  ; CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 8, i32 8>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2D<vector<float, 2> >"(i32 160, %"class.Texture2D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 2, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <2 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <2 x i32> [[IX]], i64 1
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 0, i32 [[IX0]], i32 [[IX1]], i32 undef, i32 undef, i32 undef, i32 undef)
  %45 = add <2 x i32> %ix2, <i32 8, i32 8>
  %46 = load %"class.Texture2D<vector<float, 2> >", %"class.Texture2D<vector<float, 2> >"* @"\01?Tex2d@@3V?$Texture2D@V?$vector@M$01@@@@A"
  %47 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 2> >\22)"(i32 0, %"class.Texture2D<vector<float, 2> >" %46)
  %48 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %47, %dx.types.ResourceProperties { i32 2, i32 521 }, %"class.Texture2D<vector<float, 2> >" zeroinitializer)
  %49 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %48, <2 x i32> %45)
  %50 = load <2 x float>, <2 x float>* %49

  ; CHECK: [[IX:%.*]] = add <4 x i32> {{%.*}}, <i32 9, i32 9, i32 9, i32 9>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture3D<vector<float, 2> >"(i32 160, %"class.Texture3D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <4 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <4 x i32> [[IX]], i64 1
  ; CHECK-DAG: [[IX2:%.*]] = extractelement <4 x i32> [[IX]], i64 2
  ; CHECK-DAG: [[IX3:%.*]] = extractelement <4 x i32> [[IX]], i64 3
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 [[IX3]], i32 [[IX0]], i32 [[IX1]], i32 [[IX2]], i32 undef, i32 undef, i32 undef)
  %51 = add <4 x i32> %ix4, <i32 9, i32 9, i32 9, i32 9>
  %52 = load %"class.Texture3D<vector<float, 2> >", %"class.Texture3D<vector<float, 2> >"* @"\01?Tex3d@@3V?$Texture3D@V?$vector@M$01@@@@A"
  %53 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<vector<float, 2> >\22)"(i32 0, %"class.Texture3D<vector<float, 2> >" %52)
  %54 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %53, %dx.types.ResourceProperties { i32 4, i32 521 }, %"class.Texture3D<vector<float, 2> >" zeroinitializer)
  %55 = call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <4 x i32>)"(i32 231, %dx.types.Handle %54, <4 x i32> %51)

  ; CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 10, i32 10, i32 10>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture3D<vector<float, 2> >"(i32 160, %"class.Texture3D<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <3 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <3 x i32> [[IX]], i64 1
  ; CHECK-DAG: [[IX2:%.*]] = extractelement <3 x i32> [[IX]], i64 2
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 0, i32 [[IX0]], i32 [[IX1]], i32 [[IX2]], i32 undef, i32 undef, i32 undef)
  %56 = add <3 x i32> %ix3, <i32 10, i32 10, i32 10>
  %57 = load %"class.Texture3D<vector<float, 2> >", %"class.Texture3D<vector<float, 2> >"* @"\01?Tex3d@@3V?$Texture3D@V?$vector@M$01@@@@A"
  %58 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<vector<float, 2> >\22)"(i32 0, %"class.Texture3D<vector<float, 2> >" %57)
  %59 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %58, %dx.types.ResourceProperties { i32 4, i32 521 }, %"class.Texture3D<vector<float, 2> >" zeroinitializer)
  %60 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %59, <3 x i32> %56)
  %61 = load <2 x float>, <2 x float>* %60

  ; CHECK: [[IX:%.*]] = add <4 x i32> {{%.*}}, <i32 11, i32 11, i32 11, i32 11>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2DArray<vector<float, 2> >"(i32 160, %"class.Texture2DArray<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 7, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <4 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <4 x i32> [[IX]], i64 1
  ; CHECK-DAG: [[IX2:%.*]] = extractelement <4 x i32> [[IX]], i64 2
  ; CHECK-DAG: [[IX3:%.*]] = extractelement <4 x i32> [[IX]], i64 3
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 [[IX3]], i32 [[IX0]], i32 [[IX1]], i32 [[IX2]], i32 undef, i32 undef, i32 undef)
  %62 = add <4 x i32> %ix4, <i32 11, i32 11, i32 11, i32 11>
  %63 = load %"class.Texture2DArray<vector<float, 2> >", %"class.Texture2DArray<vector<float, 2> >"* @"\01?Tex2dArr@@3V?$Texture2DArray@V?$vector@M$01@@@@A"
  %64 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 0, %"class.Texture2DArray<vector<float, 2> >" %63)
  %65 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %64, %dx.types.ResourceProperties { i32 7, i32 521 }, %"class.Texture2DArray<vector<float, 2> >" zeroinitializer)
  %66 = call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <4 x i32>)"(i32 231, %dx.types.Handle %65, <4 x i32> %62)

  ; CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 12, i32 12, i32 12>
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2DArray<vector<float, 2> >"(i32 160, %"class.Texture2DArray<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 7, i32 521 })
  ; CHECK-DAG: [[IX0:%.*]] = extractelement <3 x i32> [[IX]], i64 0
  ; CHECK-DAG: [[IX1:%.*]] = extractelement <3 x i32> [[IX]], i64 1
  ; CHECK-DAG: [[IX2:%.*]] = extractelement <3 x i32> [[IX]], i64 2
  ; CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32(i32 66, %dx.types.Handle [[ANHDL]], i32 0, i32 [[IX0]], i32 [[IX1]], i32 [[IX2]], i32 undef, i32 undef, i32 undef)
  %67 = add <3 x i32> %ix3, <i32 12, i32 12, i32 12>
  %68 = load %"class.Texture2DArray<vector<float, 2> >", %"class.Texture2DArray<vector<float, 2> >"* @"\01?Tex2dArr@@3V?$Texture2DArray@V?$vector@M$01@@@@A"
  %69 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 0, %"class.Texture2DArray<vector<float, 2> >" %68)
  %70 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %69, %dx.types.ResourceProperties { i32 7, i32 521 }, %"class.Texture2DArray<vector<float, 2> >" zeroinitializer)
  %71 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle %70, <3 x i32> %67)
  %72 = load <2 x float>, <2 x float>* %71

  %73 = icmp ne <2 x i32> %6, zeroinitializer
  %74 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x i1>, <2 x float>, <2 x float>)"(i32 184, <2 x i1> %73, <2 x float> %33, <2 x float> %39)

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 13
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 2> >"(i32 160, %"class.RWBuffer<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 })
  ; CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef,
  %75 = add i32 %ix1, 13
  %76 = load %"class.RWBuffer<vector<float, 2> >", %"class.RWBuffer<vector<float, 2> >"* @"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A"
  %77 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >" %76)
  %78 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %77, %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" zeroinitializer)
  %79 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %78, i32 %75)
  store <2 x float> %74, <2 x float>* %79

  %80 = icmp ne <2 x i32> %14, zeroinitializer
  %81 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x i1>, <2 x float>, <2 x float>)"(i32 184, <2 x i1> %80, <2 x float> %44, <2 x float> %50)

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 14
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 2> >"(i32 160, %"class.RWBuffer<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 })
  ; CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef
  %82 = add i32 %ix1, 14
  %83 = load %"class.RWBuffer<vector<float, 2> >", %"class.RWBuffer<vector<float, 2> >"* @"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A"
  %84 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >" %83)
  %85 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %84, %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" zeroinitializer)
  %86 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %85, i32 %82)
  store <2 x float> %81, <2 x float>* %86

  %87 = icmp ne <2 x i32> %20, zeroinitializer
  %88 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x i1>, <2 x float>, <2 x float>)"(i32 184, <2 x i1> %87, <2 x float> %55, <2 x float> %61)

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 15
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 2> >"(i32 160, %"class.RWBuffer<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 })
  ; CHECK:  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef
  %89 = add i32 %ix1, 15
  %90 = load %"class.RWBuffer<vector<float, 2> >", %"class.RWBuffer<vector<float, 2> >"* @"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A"
  %91 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >" %90)
  %92 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %91, %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" zeroinitializer)
  %93 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %92, i32 %89)
  store <2 x float> %88, <2 x float>* %93

  %94 = icmp ne <2 x i32> %28, zeroinitializer
  %95 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x i1>, <2 x float>, <2 x float>)"(i32 184, <2 x i1> %94, <2 x float> %66, <2 x float> %72)

  ; CHECK: [[IX:%.*]] = add i32 [[PIX]], 16
  ; CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<vector<float, 2> >"(i32 160, %"class.RWBuffer<vector<float, 2> >"
  ; CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 })
  ; CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle [[ANHDL]], i32 [[IX]], i32 undef
  %96 = add i32 %ix1, 16
  %97 = load %"class.RWBuffer<vector<float, 2> >", %"class.RWBuffer<vector<float, 2> >"* @"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A"
  %98 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >" %97)
  %99 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle %98, %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" zeroinitializer)
  %100 = call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %99, i32 %96)
  store <2 x float> %95, <2 x float>* %100

  ret void
}

declare <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32, %"class.RWBuffer<vector<bool, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<vector<bool, 2> >") #2
declare <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare <2 x i1> @"dx.hl.op..<2 x i1> (i32, %dx.types.Handle, <2 x i32>, i32)"(i32, %dx.types.Handle, <2 x i32>, i32) #0
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32, %"class.Texture2DMS<vector<bool, 2>, 0>") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture2DMS<vector<bool, 2>, 0>") #2
declare <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #2
declare <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture1D<vector<float, 2> >\22)"(i32, %"class.Texture1D<vector<float, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture1D<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture1D<vector<float, 2> >") #2
declare <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2
declare <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 2> >\22)"(i32, %"class.Texture2D<vector<float, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture2D<vector<float, 2> >") #2
declare <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #2
declare <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <4 x i32>)"(i32, %dx.types.Handle, <4 x i32>) #1
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<vector<float, 2> >\22)"(i32, %"class.Texture3D<vector<float, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture3D<vector<float, 2> >") #2
declare <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32, %dx.types.Handle, <3 x i32>) #2
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32, %"class.Texture2DArray<vector<float, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture2DArray<vector<float, 2> >") #2
declare <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x i1>, <2 x float>, <2 x float>)"(i32, <2 x i1>, <2 x float>, <2 x float>) #2
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32, %"class.RWBuffer<vector<float, 2> >") #2
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWBuffer<vector<float, 2> >") #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!22}
!dx.fnprops = !{!35}
!dx.options = !{!36, !37}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4807 (longvec_bab_ldst, 88cfe61c3-dirty)"}
!3 = !{i32 1, i32 6}
!4 = !{i32 1, i32 9}
!5 = !{!"vs", i32 6, i32 6}
!6 = !{i32 1, void (i32, <2 x i32>, <3 x i32>, <4 x i32>)* @main, !7}
!7 = !{!8, !10, !13, !16, !19}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{i32 0, !11, !12}
!11 = !{i32 4, !"IX1", i32 7, i32 5}
!12 = !{i32 1}
!13 = !{i32 0, !14, !15}
!14 = !{i32 4, !"IX2", i32 7, i32 5}
!15 = !{i32 2}
!16 = !{i32 0, !17, !18}
!17 = !{i32 4, !"IX3", i32 7, i32 5}
!18 = !{i32 3}
!19 = !{i32 0, !20, !21}
!20 = !{i32 4, !"IX4", i32 7, i32 5}
!21 = !{i32 4}
!22 = !{void (i32, <2 x i32>, <3 x i32>, <4 x i32>)* @main, !"main", null, !23, null}
!23 = !{!24, !32, null, null}
!24 = !{!25, !27, !29, !30, !31}
!25 = !{i32 0, %"class.Texture2DMS<vector<bool, 2>, 0>"* @"\01?Tex2dMs@@3V?$Texture2DMS@V?$vector@_N$01@@$0A@@@A", !"Tex2dMs", i32 0, i32 2, i32 1, i32 3, i32 0, !26}
!26 = !{i32 0, i32 5}
!27 = !{i32 1, %"class.Texture1D<vector<float, 2> >"* @"\01?Tex1d@@3V?$Texture1D@V?$vector@M$01@@@@A", !"Tex1d", i32 0, i32 3, i32 1, i32 1, i32 0, !28}
!28 = !{i32 0, i32 9}
!29 = !{i32 2, %"class.Texture2D<vector<float, 2> >"* @"\01?Tex2d@@3V?$Texture2D@V?$vector@M$01@@@@A", !"Tex2d", i32 0, i32 4, i32 1, i32 2, i32 0, !28}
!30 = !{i32 3, %"class.Texture3D<vector<float, 2> >"* @"\01?Tex3d@@3V?$Texture3D@V?$vector@M$01@@@@A", !"Tex3d", i32 0, i32 5, i32 1, i32 4, i32 0, !28}
!31 = !{i32 4, %"class.Texture2DArray<vector<float, 2> >"* @"\01?Tex2dArr@@3V?$Texture2DArray@V?$vector@M$01@@@@A", !"Tex2dArr", i32 0, i32 6, i32 1, i32 7, i32 0, !28}
!32 = !{!33, !34}
!33 = !{i32 0, %"class.RWBuffer<vector<bool, 2> >"* @"\01?TyBuf@@3V?$RWBuffer@V?$vector@_N$01@@@@A", !"TyBuf", i32 0, i32 1, i32 1, i32 10, i1 false, i1 false, i1 false, !26}
!34 = !{i32 1, %"class.RWBuffer<vector<float, 2> >"* @"\01?OutBuf@@3V?$RWBuffer@V?$vector@M$01@@@@A", !"OutBuf", i32 0, i32 7, i32 1, i32 10, i1 false, i1 false, i1 false, !28}
!35 = !{void (i32, <2 x i32>, <3 x i32>, <4 x i32>)* @main, i32 1}
!36 = !{i32 64}
!37 = !{i32 -1}
