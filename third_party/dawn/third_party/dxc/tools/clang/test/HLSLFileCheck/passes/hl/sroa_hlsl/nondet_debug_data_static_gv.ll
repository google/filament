; RUN: %opt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s -check-prefix=IRCHECK

; Ensure no global variable SROA element debug info is lost.

; This test is meant to ensure that the output of scalarrepl-param-hlsl does not
; drop debug information for any scalarized global variables.  It also ensures
; that they appear in a deterministic order.

; This should include debug info for all of the global variables, with
; scalarized variables in the current order produced by scalarrepl-param-hlsl.

; If a change to scalarrepl-param-hlsl purposely changes the ordering produced,
; this test should be updated to reflect the current order, so it may catch
; non-deterministic regressions

; Capture some metadata indices
; IRCHECK: !llvm.dbg.cu = !{![[DICompileUnit:[0-9]+]]}
; IRCHECK: ![[DICompileUnit]] = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: ![[DIFile:[0-9]+]],

; IRCHECK: !DIGlobalVariable(name: "UnusedCbufferFloat0", linkageName: "\01?UnusedCbufferFloat0@cb0@@3MB", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 27, type: ![[ConstFloatTy:[0-9]+]], isLocal: false, isDefinition: true, variable: float* @"\01?UnusedCbufferFloat0@cb0@@3MB")
; IRCHECK: !DIGlobalVariable(name: "UnusedCbufferFloat1", linkageName: "\01?UnusedCbufferFloat1@cb0@@3MB", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 28, type: ![[ConstFloatTy]], isLocal: false, isDefinition: true, variable: float* @"\01?UnusedCbufferFloat1@cb0@@3MB")
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 1, type: ![[ConstIntTy:[0-9]+]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 2, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant2", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 3, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant3", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 4, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant4", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 5, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant5", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 6, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant6", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 7, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant7", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 8, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant8", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 9, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant9", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 10, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant10", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 11, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant11", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 12, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant12", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 13, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedIntConstant13", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 14, type: ![[ConstIntTy]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedFloatArrayConstant0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 16, type: ![[Arr2ConstFloat2Ty:[0-9]+]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UnusedFloatArrayConstant1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 17, type: ![[Arr2ConstFloat2Ty]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "g_xAxis", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 19, type: ![[ConstUInt3Ty:[0-9]+]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "g_yAxis", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 20, type: ![[ConstUInt3Ty]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "g_zAxis", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 21, type: ![[ConstUInt3Ty]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "g_gridResolution", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 23, type: ![[ConstUInt3Ty]], isLocal: true, isDefinition: true)
; IRCHECK: !DIGlobalVariable(name: "UsedTexture", linkageName: "\01?UsedTexture@@3V?$Texture2D@M@@A", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 31, type: !{{[0-9]+}}, isLocal: false, isDefinition: true, variable: %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A")
; IRCHECK: !DIGlobalVariable(name: "Unused3dTexture0", linkageName: "\01?Unused3dTexture0@@3V?$Texture3D@M@@A", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 32, type: !{{[0-9]+}}, isLocal: false, isDefinition: true, variable: %"class.Texture3D<float>"* @"\01?Unused3dTexture0@@3V?$Texture3D@M@@A")
; IRCHECK: !DIGlobalVariable(name: "Unused3dTexture1", linkageName: "\01?Unused3dTexture1@@3V?$Texture3D@M@@A", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 33, type: !{{[0-9]+}}, isLocal: false, isDefinition: true, variable: %"class.Texture3D<float>"* @"\01?Unused3dTexture1@@3V?$Texture3D@M@@A")
; IRCHECK: !DIGlobalVariable(name: "g_gridResolution.0", linkageName: "g_gridResolution.0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 23, type: ![[ConstUInt3Member0:[0-9]+]], isLocal: false, isDefinition: true, variable: i32* @g_gridResolution.0)
; IRCHECK: !DIGlobalVariable(name: "g_gridResolution.1", linkageName: "g_gridResolution.1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 23, type: ![[ConstUInt3Member1:[0-9]+]], isLocal: false, isDefinition: true, variable: i32* @g_gridResolution.1)
; IRCHECK: !DIGlobalVariable(name: "g_gridResolution.2", linkageName: "g_gridResolution.2", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 23, type: ![[ConstUInt3Member2:[0-9]+]], isLocal: false, isDefinition: true, variable: i32* @g_gridResolution.2)
; IRCHECK: !DIGlobalVariable(name: "g_xAxis.0", linkageName: "g_xAxis.0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 19, type: ![[ConstUInt3Member0]], isLocal: false, isDefinition: true, variable: i32* @g_xAxis.0)
; IRCHECK: !DIGlobalVariable(name: "g_xAxis.1", linkageName: "g_xAxis.1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 19, type: ![[ConstUInt3Member1]], isLocal: false, isDefinition: true, variable: i32* @g_xAxis.1)
; IRCHECK: !DIGlobalVariable(name: "g_xAxis.2", linkageName: "g_xAxis.2", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 19, type: ![[ConstUInt3Member2]], isLocal: false, isDefinition: true, variable: i32* @g_xAxis.2)
; IRCHECK: !DIGlobalVariable(name: "g_yAxis.0", linkageName: "g_yAxis.0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 20, type: ![[ConstUInt3Member0]], isLocal: false, isDefinition: true, variable: i32* @g_yAxis.0)
; IRCHECK: !DIGlobalVariable(name: "g_yAxis.1", linkageName: "g_yAxis.1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 20, type: ![[ConstUInt3Member1]], isLocal: false, isDefinition: true, variable: i32* @g_yAxis.1)
; IRCHECK: !DIGlobalVariable(name: "g_yAxis.2", linkageName: "g_yAxis.2", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 20, type: ![[ConstUInt3Member2]], isLocal: false, isDefinition: true, variable: i32* @g_yAxis.2)
; IRCHECK: !DIGlobalVariable(name: "g_zAxis.0", linkageName: "g_zAxis.0", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 21, type: ![[ConstUInt3Member0]], isLocal: false, isDefinition: true, variable: i32* @g_zAxis.0)
; IRCHECK: !DIGlobalVariable(name: "g_zAxis.1", linkageName: "g_zAxis.1", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 21, type: ![[ConstUInt3Member1]], isLocal: false, isDefinition: true, variable: i32* @g_zAxis.1)
; IRCHECK: !DIGlobalVariable(name: "g_zAxis.2", linkageName: "g_zAxis.2", scope: ![[DICompileUnit]], file: ![[DIFile]], line: 21, type: ![[ConstUInt3Member2]], isLocal: false, isDefinition: true, variable: i32* @g_zAxis.2)
; No more global variables from here:
; IRCHECK-NOT: = !DIGlobalVariable
; Stop at source contents with CHECK line, since that will include
; DIGlobalVariable checks from HLSL test.
; IRCHECK-LABEL: = !{!"nondet_debug_data_static_gv.hlsl",

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.Texture2D<float>" = type { float, %"class.Texture2D<float>::mips_type" }
%"class.Texture2D<float>::mips_type" = type { i32 }
%"class.Texture3D<float>" = type { float, %"class.Texture3D<float>::mips_type" }
%"class.Texture3D<float>::mips_type" = type { i32 }
%ConstantBuffer = type opaque
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@UnusedIntConstant0 = internal constant i32 0, align 4
@UnusedIntConstant1 = internal constant i32 0, align 4
@UnusedIntConstant2 = internal constant i32 0, align 4
@UnusedIntConstant3 = internal constant i32 0, align 4
@UnusedIntConstant4 = internal constant i32 0, align 4
@UnusedIntConstant5 = internal constant i32 0, align 4
@UnusedIntConstant6 = internal constant i32 0, align 4
@UnusedIntConstant7 = internal constant i32 0, align 4
@UnusedIntConstant8 = internal constant i32 0, align 4
@UnusedIntConstant9 = internal constant i32 0, align 4
@UnusedIntConstant10 = internal constant i32 0, align 4
@UnusedIntConstant11 = internal constant i32 0, align 4
@UnusedIntConstant12 = internal constant i32 0, align 4
@UnusedIntConstant13 = internal constant i32 0, align 4
@UnusedFloatArrayConstant0 = internal constant [2 x <2 x float>] zeroinitializer, align 4
@UnusedFloatArrayConstant1 = internal constant [2 x <2 x float>] zeroinitializer, align 4
@g_xAxis = internal constant <3 x i32> <i32 1, i32 0, i32 0>, align 4
@g_yAxis = internal constant <3 x i32> <i32 0, i32 1, i32 0>, align 4
@g_zAxis = internal constant <3 x i32> <i32 0, i32 0, i32 1>, align 4
@g_gridResolution = internal constant <3 x i32> <i32 1, i32 1, i32 1>, align 4
@"\01?UnusedCbufferFloat0@cb0@@3MB" = external constant float, align 4
@"\01?UnusedCbufferFloat1@cb0@@3MB" = external constant float, align 4
@"\01?UsedTexture@@3V?$Texture2D@M@@A" = external global %"class.Texture2D<float>", align 4
@"\01?Unused3dTexture0@@3V?$Texture3D@M@@A" = external global %"class.Texture3D<float>", align 4
@"\01?Unused3dTexture1@@3V?$Texture3D@M@@A" = external global %"class.Texture3D<float>", align 4
@"$Globals" = external constant %ConstantBuffer
@cb0 = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main(<3 x i32> %dID) #0 {
entry:
  %gridResolution.addr.i.i.34 = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i.35 = alloca i32, align 4, !dx.temp !2
  %retval.i.36 = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.37 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i.i.26 = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i.27 = alloca i32, align 4, !dx.temp !2
  %retval.i.28 = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.29 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i.i.18 = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i.19 = alloca i32, align 4, !dx.temp !2
  %retval.i.20 = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.21 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i.i.10 = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i.11 = alloca i32, align 4, !dx.temp !2
  %retval.i.12 = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.13 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i.i.2 = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i.3 = alloca i32, align 4, !dx.temp !2
  %retval.i.4 = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.5 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i.i = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i.i = alloca i32, align 4, !dx.temp !2
  %retval.i = alloca float, align 4, !dx.temp !2
  %gridCoords.addr.i.1 = alloca i32, align 4, !dx.temp !2
  %gridResolution.addr.i = alloca i32, align 4, !dx.temp !2
  %gridCoords.addr.i = alloca i32, align 4, !dx.temp !2
  %dID.addr = alloca <3 x i32>, align 4, !dx.temp !2
  %valueX = alloca float, align 4
  %valueY = alloca float, align 4
  %valueZ = alloca float, align 4
  store <3 x i32> %dID, <3 x i32>* %dID.addr, align 4, !tbaa !99
  call void @llvm.dbg.declare(metadata <3 x i32>* %dID.addr, metadata !102, metadata !103), !dbg !104 ; var:"dID" !DIExpression() func:"main"
  %0 = load <3 x i32>, <3 x i32>* @g_gridResolution, align 4, !dbg !105 ; line:51 col:26
  %1 = extractelement <3 x i32> %0, i32 0, !dbg !105 ; line:51 col:26
  %2 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !107 ; line:51 col:19
  %3 = extractelement <3 x i32> %2, i32 0, !dbg !107 ; line:51 col:19
  store i32 %1, i32* %gridResolution.addr.i, align 4, !dbg !108, !tbaa !109 ; line:51 col:10
  store i32 %3, i32* %gridCoords.addr.i, align 4, !dbg !108, !tbaa !109 ; line:51 col:10
  %4 = load i32, i32* %gridCoords.addr.i, align 4, !dbg !111, !tbaa !109 ; line:37 col:12
  %5 = load i32, i32* %gridResolution.addr.i, align 4, !dbg !113, !tbaa !109 ; line:37 col:25
  %cmp.i = icmp ult i32 %4, %5, !dbg !114 ; line:37 col:23
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i, metadata !115, metadata !103), !dbg !116 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i, metadata !117, metadata !103), !dbg !118 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i, metadata !115, metadata !103), !dbg !119 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i, metadata !117, metadata !103), !dbg !123 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.1, metadata !124, metadata !103), !dbg !125 ; var:"gridCoords" !DIExpression() func:"sample"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i.2, metadata !115, metadata !103), !dbg !126 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i.3, metadata !117, metadata !103), !dbg !129 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.5, metadata !124, metadata !103), !dbg !130 ; var:"gridCoords" !DIExpression() func:"sample"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i.10, metadata !115, metadata !103), !dbg !131 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i.11, metadata !117, metadata !103), !dbg !134 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.13, metadata !124, metadata !103), !dbg !135 ; var:"gridCoords" !DIExpression() func:"sample"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i.18, metadata !115, metadata !103), !dbg !136 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i.19, metadata !117, metadata !103), !dbg !139 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.21, metadata !124, metadata !103), !dbg !140 ; var:"gridCoords" !DIExpression() func:"sample"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i.26, metadata !115, metadata !103), !dbg !141 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i.27, metadata !117, metadata !103), !dbg !144 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.29, metadata !124, metadata !103), !dbg !145 ; var:"gridCoords" !DIExpression() func:"sample"
  call void @llvm.dbg.declare(metadata i32* %gridResolution.addr.i.i.34, metadata !115, metadata !103), !dbg !146 ; var:"gridResolution" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.i.35, metadata !117, metadata !103), !dbg !149 ; var:"gridCoords" !DIExpression() func:"IsInside"
  call void @llvm.dbg.declare(metadata i32* %gridCoords.addr.i.37, metadata !124, metadata !103), !dbg !150 ; var:"gridCoords" !DIExpression() func:"sample"
  br i1 %cmp.i, label %if.end, label %return, !dbg !151 ; line:51 col:9

if.end:                                           ; preds = %entry
  call void @llvm.dbg.declare(metadata float* %valueX, metadata !152, metadata !103), !dbg !153 ; var:"valueX" !DIExpression() func:"main"
  %6 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !154 ; line:54 col:27
  %7 = extractelement <3 x i32> %6, i32 0, !dbg !154 ; line:54 col:27
  %8 = load <3 x i32>, <3 x i32>* @g_xAxis, align 4, !dbg !155 ; line:54 col:35
  %9 = extractelement <3 x i32> %8, i32 0, !dbg !155 ; line:54 col:35
  %add = add i32 %7, %9, !dbg !156 ; line:54 col:33
  store i32 %add, i32* %gridCoords.addr.i.5, align 4, !dbg !157, !tbaa !109 ; line:54 col:20
  %10 = load i32, i32* %gridCoords.addr.i.5, align 4, !dbg !158, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i.2, align 4, !dbg !159, !tbaa !109 ; line:42 col:9
  store i32 %10, i32* %gridCoords.addr.i.i.3, align 4, !dbg !159, !tbaa !109 ; line:42 col:9
  %11 = load i32, i32* %gridCoords.addr.i.i.3, align 4, !dbg !160, !tbaa !109 ; line:37 col:12
  %12 = load i32, i32* %gridResolution.addr.i.i.2, align 4, !dbg !161, !tbaa !109 ; line:37 col:25
  %cmp.i.i.6 = icmp ult i32 %11, %12, !dbg !162 ; line:37 col:23
  br i1 %cmp.i.i.6, label %if.then.i.7, label %if.else.i.8, !dbg !163 ; line:42 col:9

if.then.i.7:                                      ; preds = %if.end
  %13 = bitcast i32* %gridCoords.addr.i.5 to <1 x i32>*, !dbg !164 ; line:43 col:28
  %14 = load i32, i32* %gridCoords.addr.i.5, !dbg !164 ; line:43 col:28
  %15 = insertelement <1 x i32> undef, i32 %14, i64 0, !dbg !164 ; line:43 col:28
  %16 = shufflevector <1 x i32> %15, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !164 ; line:43 col:28
  %17 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !165 ; line:43 col:16
  %18 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %17) #0, !dbg !165 ; line:43 col:16
  %19 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %18, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !165 ; line:43 col:16
  %20 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %19, <2 x i32> %16) #0, !dbg !165 ; line:43 col:16
  %21 = load float, float* %20, !dbg !165, !tbaa !166 ; line:43 col:16
  store float %21, float* %retval.i.4, !dbg !168 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit.9", !dbg !168 ; line:43 col:9

if.else.i.8:                                      ; preds = %if.end
  store float 0.000000e+00, float* %retval.i.4, !dbg !169 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit.9", !dbg !169 ; line:45 col:9

"\01?sample@@YAMI@Z.exit.9":                      ; preds = %if.then.i.7, %if.else.i.8
  %22 = load float, float* %retval.i.4, !dbg !170 ; line:46 col:1
  %23 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !171 ; line:54 col:55
  %24 = extractelement <3 x i32> %23, i32 0, !dbg !171 ; line:54 col:55
  %25 = load <3 x i32>, <3 x i32>* @g_xAxis, align 4, !dbg !172 ; line:54 col:63
  %26 = extractelement <3 x i32> %25, i32 0, !dbg !172 ; line:54 col:63
  %sub = sub i32 %24, %26, !dbg !173 ; line:54 col:61
  store i32 %sub, i32* %gridCoords.addr.i.13, align 4, !dbg !174, !tbaa !109 ; line:54 col:48
  %27 = load i32, i32* %gridCoords.addr.i.13, align 4, !dbg !175, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i.10, align 4, !dbg !176, !tbaa !109 ; line:42 col:9
  store i32 %27, i32* %gridCoords.addr.i.i.11, align 4, !dbg !176, !tbaa !109 ; line:42 col:9
  %28 = load i32, i32* %gridCoords.addr.i.i.11, align 4, !dbg !177, !tbaa !109 ; line:37 col:12
  %29 = load i32, i32* %gridResolution.addr.i.i.10, align 4, !dbg !178, !tbaa !109 ; line:37 col:25
  %cmp.i.i.14 = icmp ult i32 %28, %29, !dbg !179 ; line:37 col:23
  br i1 %cmp.i.i.14, label %if.then.i.15, label %if.else.i.16, !dbg !180 ; line:42 col:9

if.then.i.15:                                     ; preds = %"\01?sample@@YAMI@Z.exit.9"
  %30 = bitcast i32* %gridCoords.addr.i.13 to <1 x i32>*, !dbg !181 ; line:43 col:28
  %31 = load i32, i32* %gridCoords.addr.i.13, !dbg !181 ; line:43 col:28
  %32 = insertelement <1 x i32> undef, i32 %31, i64 0, !dbg !181 ; line:43 col:28
  %33 = shufflevector <1 x i32> %32, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !181 ; line:43 col:28
  %34 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !182 ; line:43 col:16
  %35 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %34) #0, !dbg !182 ; line:43 col:16
  %36 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %35, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !182 ; line:43 col:16
  %37 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %36, <2 x i32> %33) #0, !dbg !182 ; line:43 col:16
  %38 = load float, float* %37, !dbg !182, !tbaa !166 ; line:43 col:16
  store float %38, float* %retval.i.12, !dbg !183 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit.17", !dbg !183 ; line:43 col:9

if.else.i.16:                                     ; preds = %"\01?sample@@YAMI@Z.exit.9"
  store float 0.000000e+00, float* %retval.i.12, !dbg !184 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit.17", !dbg !184 ; line:45 col:9

"\01?sample@@YAMI@Z.exit.17":                     ; preds = %if.then.i.15, %if.else.i.16
  %39 = load float, float* %retval.i.12, !dbg !185 ; line:46 col:1
  %sub3 = fsub float %22, %39, !dbg !186 ; line:54 col:46
  store float %sub3, float* %valueX, align 4, !dbg !153, !tbaa !166 ; line:54 col:11
  call void @llvm.dbg.declare(metadata float* %valueY, metadata !187, metadata !103), !dbg !188 ; var:"valueY" !DIExpression() func:"main"
  %40 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !189 ; line:55 col:27
  %41 = extractelement <3 x i32> %40, i32 0, !dbg !189 ; line:55 col:27
  %42 = load <3 x i32>, <3 x i32>* @g_yAxis, align 4, !dbg !190 ; line:55 col:35
  %43 = extractelement <3 x i32> %42, i32 0, !dbg !190 ; line:55 col:35
  %add4 = add i32 %41, %43, !dbg !191 ; line:55 col:33
  store i32 %add4, i32* %gridCoords.addr.i.21, align 4, !dbg !192, !tbaa !109 ; line:55 col:20
  %44 = load i32, i32* %gridCoords.addr.i.21, align 4, !dbg !193, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i.18, align 4, !dbg !194, !tbaa !109 ; line:42 col:9
  store i32 %44, i32* %gridCoords.addr.i.i.19, align 4, !dbg !194, !tbaa !109 ; line:42 col:9
  %45 = load i32, i32* %gridCoords.addr.i.i.19, align 4, !dbg !195, !tbaa !109 ; line:37 col:12
  %46 = load i32, i32* %gridResolution.addr.i.i.18, align 4, !dbg !196, !tbaa !109 ; line:37 col:25
  %cmp.i.i.22 = icmp ult i32 %45, %46, !dbg !197 ; line:37 col:23
  br i1 %cmp.i.i.22, label %if.then.i.23, label %if.else.i.24, !dbg !198 ; line:42 col:9

if.then.i.23:                                     ; preds = %"\01?sample@@YAMI@Z.exit.17"
  %47 = bitcast i32* %gridCoords.addr.i.21 to <1 x i32>*, !dbg !199 ; line:43 col:28
  %48 = load i32, i32* %gridCoords.addr.i.21, !dbg !199 ; line:43 col:28
  %49 = insertelement <1 x i32> undef, i32 %48, i64 0, !dbg !199 ; line:43 col:28
  %50 = shufflevector <1 x i32> %49, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !199 ; line:43 col:28
  %51 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !200 ; line:43 col:16
  %52 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %51) #0, !dbg !200 ; line:43 col:16
  %53 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %52, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !200 ; line:43 col:16
  %54 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %53, <2 x i32> %50) #0, !dbg !200 ; line:43 col:16
  %55 = load float, float* %54, !dbg !200, !tbaa !166 ; line:43 col:16
  store float %55, float* %retval.i.20, !dbg !201 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit.25", !dbg !201 ; line:43 col:9

if.else.i.24:                                     ; preds = %"\01?sample@@YAMI@Z.exit.17"
  store float 0.000000e+00, float* %retval.i.20, !dbg !202 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit.25", !dbg !202 ; line:45 col:9

"\01?sample@@YAMI@Z.exit.25":                     ; preds = %if.then.i.23, %if.else.i.24
  %56 = load float, float* %retval.i.20, !dbg !203 ; line:46 col:1
  %57 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !204 ; line:55 col:55
  %58 = extractelement <3 x i32> %57, i32 0, !dbg !204 ; line:55 col:55
  %59 = load <3 x i32>, <3 x i32>* @g_yAxis, align 4, !dbg !205 ; line:55 col:63
  %60 = extractelement <3 x i32> %59, i32 0, !dbg !205 ; line:55 col:63
  %sub6 = sub i32 %58, %60, !dbg !206 ; line:55 col:61
  store i32 %sub6, i32* %gridCoords.addr.i.29, align 4, !dbg !207, !tbaa !109 ; line:55 col:48
  %61 = load i32, i32* %gridCoords.addr.i.29, align 4, !dbg !208, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i.26, align 4, !dbg !209, !tbaa !109 ; line:42 col:9
  store i32 %61, i32* %gridCoords.addr.i.i.27, align 4, !dbg !209, !tbaa !109 ; line:42 col:9
  %62 = load i32, i32* %gridCoords.addr.i.i.27, align 4, !dbg !210, !tbaa !109 ; line:37 col:12
  %63 = load i32, i32* %gridResolution.addr.i.i.26, align 4, !dbg !211, !tbaa !109 ; line:37 col:25
  %cmp.i.i.30 = icmp ult i32 %62, %63, !dbg !212 ; line:37 col:23
  br i1 %cmp.i.i.30, label %if.then.i.31, label %if.else.i.32, !dbg !213 ; line:42 col:9

if.then.i.31:                                     ; preds = %"\01?sample@@YAMI@Z.exit.25"
  %64 = bitcast i32* %gridCoords.addr.i.29 to <1 x i32>*, !dbg !214 ; line:43 col:28
  %65 = load i32, i32* %gridCoords.addr.i.29, !dbg !214 ; line:43 col:28
  %66 = insertelement <1 x i32> undef, i32 %65, i64 0, !dbg !214 ; line:43 col:28
  %67 = shufflevector <1 x i32> %66, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !214 ; line:43 col:28
  %68 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !215 ; line:43 col:16
  %69 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %68) #0, !dbg !215 ; line:43 col:16
  %70 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %69, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !215 ; line:43 col:16
  %71 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %70, <2 x i32> %67) #0, !dbg !215 ; line:43 col:16
  %72 = load float, float* %71, !dbg !215, !tbaa !166 ; line:43 col:16
  store float %72, float* %retval.i.28, !dbg !216 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit.33", !dbg !216 ; line:43 col:9

if.else.i.32:                                     ; preds = %"\01?sample@@YAMI@Z.exit.25"
  store float 0.000000e+00, float* %retval.i.28, !dbg !217 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit.33", !dbg !217 ; line:45 col:9

"\01?sample@@YAMI@Z.exit.33":                     ; preds = %if.then.i.31, %if.else.i.32
  %73 = load float, float* %retval.i.28, !dbg !218 ; line:46 col:1
  %sub8 = fsub float %56, %73, !dbg !219 ; line:55 col:46
  store float %sub8, float* %valueY, align 4, !dbg !188, !tbaa !166 ; line:55 col:11
  call void @llvm.dbg.declare(metadata float* %valueZ, metadata !220, metadata !103), !dbg !221 ; var:"valueZ" !DIExpression() func:"main"
  %74 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !222 ; line:56 col:27
  %75 = extractelement <3 x i32> %74, i32 0, !dbg !222 ; line:56 col:27
  %76 = load <3 x i32>, <3 x i32>* @g_zAxis, align 4, !dbg !223 ; line:56 col:35
  %77 = extractelement <3 x i32> %76, i32 0, !dbg !223 ; line:56 col:35
  %add9 = add i32 %75, %77, !dbg !224 ; line:56 col:33
  store i32 %add9, i32* %gridCoords.addr.i.37, align 4, !dbg !225, !tbaa !109 ; line:56 col:20
  %78 = load i32, i32* %gridCoords.addr.i.37, align 4, !dbg !226, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i.34, align 4, !dbg !227, !tbaa !109 ; line:42 col:9
  store i32 %78, i32* %gridCoords.addr.i.i.35, align 4, !dbg !227, !tbaa !109 ; line:42 col:9
  %79 = load i32, i32* %gridCoords.addr.i.i.35, align 4, !dbg !228, !tbaa !109 ; line:37 col:12
  %80 = load i32, i32* %gridResolution.addr.i.i.34, align 4, !dbg !229, !tbaa !109 ; line:37 col:25
  %cmp.i.i.38 = icmp ult i32 %79, %80, !dbg !230 ; line:37 col:23
  br i1 %cmp.i.i.38, label %if.then.i.39, label %if.else.i.40, !dbg !231 ; line:42 col:9

if.then.i.39:                                     ; preds = %"\01?sample@@YAMI@Z.exit.33"
  %81 = bitcast i32* %gridCoords.addr.i.37 to <1 x i32>*, !dbg !232 ; line:43 col:28
  %82 = load i32, i32* %gridCoords.addr.i.37, !dbg !232 ; line:43 col:28
  %83 = insertelement <1 x i32> undef, i32 %82, i64 0, !dbg !232 ; line:43 col:28
  %84 = shufflevector <1 x i32> %83, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !232 ; line:43 col:28
  %85 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !233 ; line:43 col:16
  %86 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %85) #0, !dbg !233 ; line:43 col:16
  %87 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %86, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !233 ; line:43 col:16
  %88 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %87, <2 x i32> %84) #0, !dbg !233 ; line:43 col:16
  %89 = load float, float* %88, !dbg !233, !tbaa !166 ; line:43 col:16
  store float %89, float* %retval.i.36, !dbg !234 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit.41", !dbg !234 ; line:43 col:9

if.else.i.40:                                     ; preds = %"\01?sample@@YAMI@Z.exit.33"
  store float 0.000000e+00, float* %retval.i.36, !dbg !235 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit.41", !dbg !235 ; line:45 col:9

"\01?sample@@YAMI@Z.exit.41":                     ; preds = %if.then.i.39, %if.else.i.40
  %90 = load float, float* %retval.i.36, !dbg !236 ; line:46 col:1
  %91 = load <3 x i32>, <3 x i32>* %dID.addr, align 4, !dbg !237 ; line:56 col:55
  %92 = extractelement <3 x i32> %91, i32 0, !dbg !237 ; line:56 col:55
  %93 = load <3 x i32>, <3 x i32>* @g_zAxis, align 4, !dbg !238 ; line:56 col:63
  %94 = extractelement <3 x i32> %93, i32 0, !dbg !238 ; line:56 col:63
  %sub11 = sub i32 %92, %94, !dbg !239 ; line:56 col:61
  store i32 %sub11, i32* %gridCoords.addr.i.1, align 4, !dbg !240, !tbaa !109 ; line:56 col:48
  %95 = load i32, i32* %gridCoords.addr.i.1, align 4, !dbg !241, !tbaa !109 ; line:42 col:18
  store i32 1, i32* %gridResolution.addr.i.i, align 4, !dbg !242, !tbaa !109 ; line:42 col:9
  store i32 %95, i32* %gridCoords.addr.i.i, align 4, !dbg !242, !tbaa !109 ; line:42 col:9
  %96 = load i32, i32* %gridCoords.addr.i.i, align 4, !dbg !243, !tbaa !109 ; line:37 col:12
  %97 = load i32, i32* %gridResolution.addr.i.i, align 4, !dbg !244, !tbaa !109 ; line:37 col:25
  %cmp.i.i = icmp ult i32 %96, %97, !dbg !245 ; line:37 col:23
  br i1 %cmp.i.i, label %if.then.i, label %if.else.i, !dbg !246 ; line:42 col:9

if.then.i:                                        ; preds = %"\01?sample@@YAMI@Z.exit.41"
  %98 = bitcast i32* %gridCoords.addr.i.1 to <1 x i32>*, !dbg !247 ; line:43 col:28
  %99 = load i32, i32* %gridCoords.addr.i.1, !dbg !247 ; line:43 col:28
  %100 = insertelement <1 x i32> undef, i32 %99, i64 0, !dbg !247 ; line:43 col:28
  %101 = shufflevector <1 x i32> %100, <1 x i32> undef, <2 x i32> zeroinitializer, !dbg !247 ; line:43 col:28
  %102 = load %"class.Texture2D<float>", %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !dbg !248 ; line:43 col:16
  %103 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32 0, %"class.Texture2D<float>" %102) #0, !dbg !248 ; line:43 col:16
  %104 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32 11, %dx.types.Handle %103, %dx.types.ResourceProperties { i32 2, i32 265 }, %"class.Texture2D<float>" undef) #0, !dbg !248 ; line:43 col:16
  %105 = call float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle %104, <2 x i32> %101) #0, !dbg !248 ; line:43 col:16
  %106 = load float, float* %105, !dbg !248, !tbaa !166 ; line:43 col:16
  store float %106, float* %retval.i, !dbg !249 ; line:43 col:9
  br label %"\01?sample@@YAMI@Z.exit", !dbg !249 ; line:43 col:9

if.else.i:                                        ; preds = %"\01?sample@@YAMI@Z.exit.41"
  store float 0.000000e+00, float* %retval.i, !dbg !250 ; line:45 col:9
  br label %"\01?sample@@YAMI@Z.exit", !dbg !250 ; line:45 col:9

"\01?sample@@YAMI@Z.exit":                        ; preds = %if.then.i, %if.else.i
  %107 = load float, float* %retval.i, !dbg !251 ; line:46 col:1
  %sub13 = fsub float %90, %107, !dbg !252 ; line:56 col:46
  store float %sub13, float* %valueZ, align 4, !dbg !221, !tbaa !166 ; line:56 col:11
  br label %return, !dbg !253 ; line:57 col:1

return:                                           ; preds = %entry, %"\01?sample@@YAMI@Z.exit"
  ret void, !dbg !253 ; line:57 col:1
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare float* @"dx.hl.subscript.[].float* (i32, %dx.types.Handle, <2 x i32>)"(i32, %dx.types.Handle, <2 x i32>) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<float>\22)"(i32, %"class.Texture2D<float>") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<float>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture2D<float>") #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!71, !72}
!pauseresume = !{!73}
!llvm.ident = !{!74}
!dx.source.contents = !{!75}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!76}
!dx.source.args = !{!77}
!dx.version = !{!78}
!dx.valver = !{!79}
!dx.shaderModel = !{!80}
!dx.typeAnnotations = !{!81}
!dx.entryPoints = !{!86}
!dx.fnprops = !{!96}
!dx.options = !{!97, !98}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "dxc(private) 1.7.0.3914 (fix-nondet-sroa-staticgv, cd2b7cea1)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !27)
!1 = !DIFile(filename: "nondet_debug_data_static_gv.hlsl", directory: "")
!2 = !{}
!3 = !{!4, !18, !23}
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 49, type: !5, isLocal: false, isDefinition: true, scopeLine: 50, flags: DIFlagPrototyped, isOptimized: false, function: void (<3 x i32>)* @main)
!5 = !DISubroutineType(types: !6)
!6 = !{null, !7}
!7 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint3", file: !1, line: 28, baseType: !8)
!8 = !DICompositeType(tag: DW_TAG_class_type, name: "vector<unsigned int, 3>", file: !1, line: 28, size: 96, align: 32, elements: !9, templateParams: !14)
!9 = !{!10, !12, !13}
!10 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !8, file: !1, line: 28, baseType: !11, size: 32, align: 32, flags: DIFlagPublic)
!11 = !DIBasicType(name: "unsigned int", size: 32, align: 32, encoding: DW_ATE_unsigned)
!12 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !8, file: !1, line: 28, baseType: !11, size: 32, align: 32, offset: 32, flags: DIFlagPublic)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !8, file: !1, line: 28, baseType: !11, size: 32, align: 32, offset: 64, flags: DIFlagPublic)
!14 = !{!15, !16}
!15 = !DITemplateTypeParameter(name: "element", type: !11)
!16 = !DITemplateValueParameter(name: "element_count", type: !17, value: i32 3)
!17 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!18 = !DISubprogram(name: "IsInside", linkageName: "\01?IsInside@@YA_NII@Z", scope: !1, file: !1, line: 35, type: !19, isLocal: false, isDefinition: true, scopeLine: 36, flags: DIFlagPrototyped, isOptimized: false)
!19 = !DISubroutineType(types: !20)
!20 = !{!21, !22, !22}
!21 = !DIBasicType(name: "bool", size: 32, align: 32, encoding: DW_ATE_boolean)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !1, line: 33, baseType: !11)
!23 = !DISubprogram(name: "sample", linkageName: "\01?sample@@YAMI@Z", scope: !1, file: !1, line: 40, type: !24, isLocal: false, isDefinition: true, scopeLine: 41, flags: DIFlagPrototyped, isOptimized: false)
!24 = !DISubroutineType(types: !25)
!25 = !{!26, !22}
!26 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!27 = !{!28, !30, !31, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !59, !60, !62, !63, !64, !65, !68, !70}
!28 = !DIGlobalVariable(name: "UnusedCbufferFloat0", linkageName: "\01?UnusedCbufferFloat0@cb0@@3MB", scope: !0, file: !1, line: 27, type: !29, isLocal: false, isDefinition: true, variable: float* @"\01?UnusedCbufferFloat0@cb0@@3MB")
!29 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !26)
!30 = !DIGlobalVariable(name: "UnusedCbufferFloat1", linkageName: "\01?UnusedCbufferFloat1@cb0@@3MB", scope: !0, file: !1, line: 28, type: !29, isLocal: false, isDefinition: true, variable: float* @"\01?UnusedCbufferFloat1@cb0@@3MB")
!31 = !DIGlobalVariable(name: "UnusedIntConstant0", scope: !0, file: !1, line: 1, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant0)
!32 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !17)
!33 = !DIGlobalVariable(name: "UnusedIntConstant1", scope: !0, file: !1, line: 2, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant1)
!34 = !DIGlobalVariable(name: "UnusedIntConstant2", scope: !0, file: !1, line: 3, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant2)
!35 = !DIGlobalVariable(name: "UnusedIntConstant3", scope: !0, file: !1, line: 4, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant3)
!36 = !DIGlobalVariable(name: "UnusedIntConstant4", scope: !0, file: !1, line: 5, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant4)
!37 = !DIGlobalVariable(name: "UnusedIntConstant5", scope: !0, file: !1, line: 6, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant5)
!38 = !DIGlobalVariable(name: "UnusedIntConstant6", scope: !0, file: !1, line: 7, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant6)
!39 = !DIGlobalVariable(name: "UnusedIntConstant7", scope: !0, file: !1, line: 8, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant7)
!40 = !DIGlobalVariable(name: "UnusedIntConstant8", scope: !0, file: !1, line: 9, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant8)
!41 = !DIGlobalVariable(name: "UnusedIntConstant9", scope: !0, file: !1, line: 10, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant9)
!42 = !DIGlobalVariable(name: "UnusedIntConstant10", scope: !0, file: !1, line: 11, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant10)
!43 = !DIGlobalVariable(name: "UnusedIntConstant11", scope: !0, file: !1, line: 12, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant11)
!44 = !DIGlobalVariable(name: "UnusedIntConstant12", scope: !0, file: !1, line: 13, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant12)
!45 = !DIGlobalVariable(name: "UnusedIntConstant13", scope: !0, file: !1, line: 14, type: !32, isLocal: true, isDefinition: true, variable: i32* @UnusedIntConstant13)
!46 = !DIGlobalVariable(name: "UnusedFloatArrayConstant0", scope: !0, file: !1, line: 16, type: !47, isLocal: true, isDefinition: true, variable: [2 x <2 x float>]* @UnusedFloatArrayConstant0)
!47 = !DICompositeType(tag: DW_TAG_array_type, baseType: !48, size: 128, align: 32, elements: !57)
!48 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !49)
!49 = !DIDerivedType(tag: DW_TAG_typedef, name: "float2", file: !1, line: 16, baseType: !50)
!50 = !DICompositeType(tag: DW_TAG_class_type, name: "vector<float, 2>", file: !1, line: 16, size: 64, align: 32, elements: !51, templateParams: !54)
!51 = !{!52, !53}
!52 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !50, file: !1, line: 16, baseType: !26, size: 32, align: 32, flags: DIFlagPublic)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !50, file: !1, line: 16, baseType: !26, size: 32, align: 32, offset: 32, flags: DIFlagPublic)
!54 = !{!55, !56}
!55 = !DITemplateTypeParameter(name: "element", type: !26)
!56 = !DITemplateValueParameter(name: "element_count", type: !17, value: i32 2)
!57 = !{!58}
!58 = !DISubrange(count: 2)
!59 = !DIGlobalVariable(name: "UnusedFloatArrayConstant1", scope: !0, file: !1, line: 17, type: !47, isLocal: true, isDefinition: true, variable: [2 x <2 x float>]* @UnusedFloatArrayConstant1)
!60 = !DIGlobalVariable(name: "g_xAxis", scope: !0, file: !1, line: 19, type: !61, isLocal: true, isDefinition: true, variable: <3 x i32>* @g_xAxis)
!61 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!62 = !DIGlobalVariable(name: "g_yAxis", scope: !0, file: !1, line: 20, type: !61, isLocal: true, isDefinition: true, variable: <3 x i32>* @g_yAxis)
!63 = !DIGlobalVariable(name: "g_zAxis", scope: !0, file: !1, line: 21, type: !61, isLocal: true, isDefinition: true, variable: <3 x i32>* @g_zAxis)
!64 = !DIGlobalVariable(name: "g_gridResolution", scope: !0, file: !1, line: 23, type: !61, isLocal: true, isDefinition: true, variable: <3 x i32>* @g_gridResolution)
!65 = !DIGlobalVariable(name: "UsedTexture", linkageName: "\01?UsedTexture@@3V?$Texture2D@M@@A", scope: !0, file: !1, line: 31, type: !66, isLocal: false, isDefinition: true, variable: %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A")
!66 = !DICompositeType(tag: DW_TAG_class_type, name: "Texture2D<float>", file: !1, line: 31, size: 64, align: 32, elements: !2, templateParams: !67)
!67 = !{!55}
!68 = !DIGlobalVariable(name: "Unused3dTexture0", linkageName: "\01?Unused3dTexture0@@3V?$Texture3D@M@@A", scope: !0, file: !1, line: 32, type: !69, isLocal: false, isDefinition: true, variable: %"class.Texture3D<float>"* @"\01?Unused3dTexture0@@3V?$Texture3D@M@@A")
!69 = !DICompositeType(tag: DW_TAG_class_type, name: "Texture3D<float>", file: !1, line: 32, size: 64, align: 32, elements: !2, templateParams: !67)
!70 = !DIGlobalVariable(name: "Unused3dTexture1", linkageName: "\01?Unused3dTexture1@@3V?$Texture3D@M@@A", scope: !0, file: !1, line: 33, type: !69, isLocal: false, isDefinition: true, variable: %"class.Texture3D<float>"* @"\01?Unused3dTexture1@@3V?$Texture3D@M@@A")
!71 = !{i32 2, !"Dwarf Version", i32 4}
!72 = !{i32 2, !"Debug Info Version", i32 3}
!73 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!74 = !{!"dxc(private) 1.7.0.3914 (fix-nondet-sroa-staticgv, cd2b7cea1)"}
!75 = !{!"nondet_debug_data_static_gv.hlsl", !"static const int UnusedIntConstant0 = 0;\0Astatic const int UnusedIntConstant1 = 0;\0Astatic const int UnusedIntConstant2 = 0;\0Astatic const int UnusedIntConstant3 = 0;\0Astatic const int UnusedIntConstant4 = 0;\0Astatic const int UnusedIntConstant5 = 0;\0Astatic const int UnusedIntConstant6 = 0;\0Astatic const int UnusedIntConstant7 = 0;\0Astatic const int UnusedIntConstant8 = 0;\0Astatic const int UnusedIntConstant9 = 0;\0Astatic const int UnusedIntConstant10 = 0;\0Astatic const int UnusedIntConstant11 = 0;\0Astatic const int UnusedIntConstant12 = 0;\0Astatic const int UnusedIntConstant13 = 0;\0A\0Astatic const float2 UnusedFloatArrayConstant0[2] = {float2(0.0f, 0.0f), float2(0.0f, 0.0f)};\0Astatic const float2 UnusedFloatArrayConstant1[2] = {float2(0.0f, 0.0f), float2(0.0f, 0.0f)};\0A\0Astatic const uint3 g_xAxis = uint3(1, 0, 0);\0Astatic const uint3 g_yAxis = uint3(0, 1, 0);\0Astatic const uint3 g_zAxis = uint3(0, 0, 1);\0A\0Astatic const uint3 g_gridResolution = uint3(1, 1, 1);\0A\0Acbuffer cb0 : register(b0)\0A{\0A    float UnusedCbufferFloat0;\0A    float UnusedCbufferFloat1;\0A}\0A\0ATexture2D<float> UsedTexture : register(t0);\0ATexture3D<float> Unused3dTexture0 : register(t1);\0ATexture3D<float> Unused3dTexture1 : register(t2);\0A\0Abool IsInside(uint gridCoords, uint gridResolution)\0A{\0A    return gridCoords < gridResolution;\0A}\0A\0Afloat sample(uint gridCoords)\0A{\0A    if (IsInside(gridCoords.x, g_gridResolution.x))\0A        return UsedTexture[gridCoords.xx];\0A    else\0A        return 0.0f;\0A}\0A\0A[numthreads(4, 4, 4)]\0Avoid main(uint3 dID : SV_DispatchThreadID)\0A{\0A    if (!IsInside(dID.x, g_gridResolution.x))\0A        return;\0A\0A    float valueX = sample(dID.x + g_xAxis.x) - sample(dID.x - g_xAxis.x);\0A    float valueY = sample(dID.x + g_yAxis.x) - sample(dID.x - g_yAxis.x);\0A    float valueZ = sample(dID.x + g_zAxis.x) - sample(dID.x - g_zAxis.x);\0A}\0A"}
!76 = !{!"nondet_debug_data_static_gv.hlsl"}
!77 = !{!"-E", !"main", !"-T", !"cs_6_0", !"-fcgl", !"-Zi", !"-Qembed_debug"}
!78 = !{i32 1, i32 0}
!79 = !{i32 1, i32 7}
!80 = !{!"cs", i32 6, i32 0}
!81 = !{i32 1, void (<3 x i32>)* @main, !82}
!82 = !{!83, !84}
!83 = !{i32 1, !2, !2}
!84 = !{i32 0, !85, !2}
!85 = !{i32 4, !"SV_DispatchThreadID", i32 7, i32 5}
!86 = !{void (<3 x i32>)* @main, !"main", null, !87, null}
!87 = !{!88, null, !93, null}
!88 = !{!89, !91, !92}
!89 = !{i32 0, %"class.Texture2D<float>"* @"\01?UsedTexture@@3V?$Texture2D@M@@A", !"UsedTexture", i32 0, i32 0, i32 1, i32 2, i32 0, !90}
!90 = !{i32 0, i32 9}
!91 = !{i32 1, %"class.Texture3D<float>"* @"\01?Unused3dTexture0@@3V?$Texture3D@M@@A", !"Unused3dTexture0", i32 0, i32 1, i32 1, i32 4, i32 0, !90}
!92 = !{i32 2, %"class.Texture3D<float>"* @"\01?Unused3dTexture1@@3V?$Texture3D@M@@A", !"Unused3dTexture1", i32 0, i32 2, i32 1, i32 4, i32 0, !90}
!93 = !{!94, !95}
!94 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!95 = !{i32 1, %ConstantBuffer* @cb0, !"cb0", i32 0, i32 0, i32 1, i32 8, null}
!96 = !{void (<3 x i32>)* @main, i32 5, i32 4, i32 4, i32 4}
!97 = !{i32 144}
!98 = !{i32 -1}
!99 = !{!100, !100, i64 0}
!100 = !{!"omnipotent char", !101, i64 0}
!101 = !{!"Simple C/C++ TBAA"}
!102 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "dID", arg: 1, scope: !4, file: !1, line: 49, type: !7)
!103 = !DIExpression()
!104 = !DILocation(line: 49, column: 17, scope: !4)
!105 = !DILocation(line: 51, column: 26, scope: !106)
!106 = distinct !DILexicalBlock(scope: !4, file: !1, line: 51, column: 9)
!107 = !DILocation(line: 51, column: 19, scope: !106)
!108 = !DILocation(line: 51, column: 10, scope: !106)
!109 = !{!110, !110, i64 0}
!110 = !{!"int", !100, i64 0}
!111 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !112)
!112 = distinct !DILocation(line: 51, column: 10, scope: !106)
!113 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !112)
!114 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !112)
!115 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "gridResolution", arg: 2, scope: !18, file: !1, line: 35, type: !22)
!116 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !112)
!117 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "gridCoords", arg: 1, scope: !18, file: !1, line: 35, type: !22)
!118 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !112)
!119 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !120)
!120 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !122)
!121 = distinct !DILexicalBlock(scope: !23, file: !1, line: 42, column: 9)
!122 = distinct !DILocation(line: 56, column: 48, scope: !4)
!123 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !120)
!124 = !DILocalVariable(tag: DW_TAG_arg_variable, name: "gridCoords", arg: 1, scope: !23, file: !1, line: 40, type: !22)
!125 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !122)
!126 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !127)
!127 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !128)
!128 = distinct !DILocation(line: 54, column: 20, scope: !4)
!129 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !127)
!130 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !128)
!131 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !132)
!132 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !133)
!133 = distinct !DILocation(line: 54, column: 48, scope: !4)
!134 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !132)
!135 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !133)
!136 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !137)
!137 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !138)
!138 = distinct !DILocation(line: 55, column: 20, scope: !4)
!139 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !137)
!140 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !138)
!141 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !142)
!142 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !143)
!143 = distinct !DILocation(line: 55, column: 48, scope: !4)
!144 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !142)
!145 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !143)
!146 = !DILocation(line: 35, column: 37, scope: !18, inlinedAt: !147)
!147 = distinct !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !148)
!148 = distinct !DILocation(line: 56, column: 20, scope: !4)
!149 = !DILocation(line: 35, column: 20, scope: !18, inlinedAt: !147)
!150 = !DILocation(line: 40, column: 19, scope: !23, inlinedAt: !148)
!151 = !DILocation(line: 51, column: 9, scope: !4)
!152 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "valueX", scope: !4, file: !1, line: 54, type: !26)
!153 = !DILocation(line: 54, column: 11, scope: !4)
!154 = !DILocation(line: 54, column: 27, scope: !4)
!155 = !DILocation(line: 54, column: 35, scope: !4)
!156 = !DILocation(line: 54, column: 33, scope: !4)
!157 = !DILocation(line: 54, column: 20, scope: !4)
!158 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !128)
!159 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !128)
!160 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !127)
!161 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !127)
!162 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !127)
!163 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !128)
!164 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !128)
!165 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !128)
!166 = !{!167, !167, i64 0}
!167 = !{!"float", !100, i64 0}
!168 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !128)
!169 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !128)
!170 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !128)
!171 = !DILocation(line: 54, column: 55, scope: !4)
!172 = !DILocation(line: 54, column: 63, scope: !4)
!173 = !DILocation(line: 54, column: 61, scope: !4)
!174 = !DILocation(line: 54, column: 48, scope: !4)
!175 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !133)
!176 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !133)
!177 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !132)
!178 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !132)
!179 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !132)
!180 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !133)
!181 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !133)
!182 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !133)
!183 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !133)
!184 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !133)
!185 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !133)
!186 = !DILocation(line: 54, column: 46, scope: !4)
!187 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "valueY", scope: !4, file: !1, line: 55, type: !26)
!188 = !DILocation(line: 55, column: 11, scope: !4)
!189 = !DILocation(line: 55, column: 27, scope: !4)
!190 = !DILocation(line: 55, column: 35, scope: !4)
!191 = !DILocation(line: 55, column: 33, scope: !4)
!192 = !DILocation(line: 55, column: 20, scope: !4)
!193 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !138)
!194 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !138)
!195 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !137)
!196 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !137)
!197 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !137)
!198 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !138)
!199 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !138)
!200 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !138)
!201 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !138)
!202 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !138)
!203 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !138)
!204 = !DILocation(line: 55, column: 55, scope: !4)
!205 = !DILocation(line: 55, column: 63, scope: !4)
!206 = !DILocation(line: 55, column: 61, scope: !4)
!207 = !DILocation(line: 55, column: 48, scope: !4)
!208 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !143)
!209 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !143)
!210 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !142)
!211 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !142)
!212 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !142)
!213 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !143)
!214 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !143)
!215 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !143)
!216 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !143)
!217 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !143)
!218 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !143)
!219 = !DILocation(line: 55, column: 46, scope: !4)
!220 = !DILocalVariable(tag: DW_TAG_auto_variable, name: "valueZ", scope: !4, file: !1, line: 56, type: !26)
!221 = !DILocation(line: 56, column: 11, scope: !4)
!222 = !DILocation(line: 56, column: 27, scope: !4)
!223 = !DILocation(line: 56, column: 35, scope: !4)
!224 = !DILocation(line: 56, column: 33, scope: !4)
!225 = !DILocation(line: 56, column: 20, scope: !4)
!226 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !148)
!227 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !148)
!228 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !147)
!229 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !147)
!230 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !147)
!231 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !148)
!232 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !148)
!233 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !148)
!234 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !148)
!235 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !148)
!236 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !148)
!237 = !DILocation(line: 56, column: 55, scope: !4)
!238 = !DILocation(line: 56, column: 63, scope: !4)
!239 = !DILocation(line: 56, column: 61, scope: !4)
!240 = !DILocation(line: 56, column: 48, scope: !4)
!241 = !DILocation(line: 42, column: 18, scope: !121, inlinedAt: !122)
!242 = !DILocation(line: 42, column: 9, scope: !121, inlinedAt: !122)
!243 = !DILocation(line: 37, column: 12, scope: !18, inlinedAt: !120)
!244 = !DILocation(line: 37, column: 25, scope: !18, inlinedAt: !120)
!245 = !DILocation(line: 37, column: 23, scope: !18, inlinedAt: !120)
!246 = !DILocation(line: 42, column: 9, scope: !23, inlinedAt: !122)
!247 = !DILocation(line: 43, column: 28, scope: !121, inlinedAt: !122)
!248 = !DILocation(line: 43, column: 16, scope: !121, inlinedAt: !122)
!249 = !DILocation(line: 43, column: 9, scope: !121, inlinedAt: !122)
!250 = !DILocation(line: 45, column: 9, scope: !121, inlinedAt: !122)
!251 = !DILocation(line: 46, column: 1, scope: !23, inlinedAt: !122)
!252 = !DILocation(line: 56, column: 46, scope: !4)
!253 = !DILocation(line: 57, column: 1, scope: !4)

; Test IR generated with:
; ExtractIRForPassTest.py -p scalarrepl-param-hlsl -o nondet_debug_data_static_gv.ll nondet_debug_data_static_gv.hlsl -- -E main -T cs_6_0 -Zi
; Where the contents of nondet_debug_data_static_gv.hlsl are as follows:
; -------------------------------------
; static const int UnusedIntConstant0 = 0;
; static const int UnusedIntConstant1 = 0;
; static const int UnusedIntConstant2 = 0;
; static const int UnusedIntConstant3 = 0;
; static const int UnusedIntConstant4 = 0;
; static const int UnusedIntConstant5 = 0;
; static const int UnusedIntConstant6 = 0;
; static const int UnusedIntConstant7 = 0;
; static const int UnusedIntConstant8 = 0;
; static const int UnusedIntConstant9 = 0;
; static const int UnusedIntConstant10 = 0;
; static const int UnusedIntConstant11 = 0;
; static const int UnusedIntConstant12 = 0;
; static const int UnusedIntConstant13 = 0;
; 
; static const float2 UnusedFloatArrayConstant0[2] = {float2(0.0f, 0.0f), float2(0.0f, 0.0f)};
; static const float2 UnusedFloatArrayConstant1[2] = {float2(0.0f, 0.0f), float2(0.0f, 0.0f)};
; 
; static const uint3 g_xAxis = uint3(1, 0, 0);
; static const uint3 g_yAxis = uint3(0, 1, 0);
; static const uint3 g_zAxis = uint3(0, 0, 1);
; 
; static const uint3 g_gridResolution = uint3(1, 1, 1);
; 
; cbuffer cb0 : register(b0)
; {
;     float UnusedCbufferFloat0;
;     float UnusedCbufferFloat1;
; }
; 
; Texture2D<float> UsedTexture : register(t0);
; Texture3D<float> Unused3dTexture0 : register(t1);
; Texture3D<float> Unused3dTexture1 : register(t2);
; 
; bool IsInside(uint gridCoords, uint gridResolution)
; {
;     return gridCoords < gridResolution;
; }
; 
; float sample(uint gridCoords)
; {
;     if (IsInside(gridCoords.x, g_gridResolution.x))
;         return UsedTexture[gridCoords.xx];
;     else
;         return 0.0f;
; }
; 
; [numthreads(4, 4, 4)]
; void main(uint3 dID : SV_DispatchThreadID)
; {
;     if (!IsInside(dID.x, g_gridResolution.x))
;         return;
; 
;     float valueX = sample(dID.x + g_xAxis.x) - sample(dID.x - g_xAxis.x);
;     float valueY = sample(dID.x + g_yAxis.x) - sample(dID.x - g_yAxis.x);
;     float valueZ = sample(dID.x + g_zAxis.x) - sample(dID.x - g_zAxis.x);
; }
; -------------------------------------
