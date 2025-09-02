; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%"$Globals" = type { i32, i32, i32, i32, i32 }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%struct.Payload = type { <2 x float>, <3 x i32> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?Acc@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"\01?RayFlags@@3IB" = external constant i32, align 4
@"\01?InstanceInclusionMask@@3IB" = external constant i32, align 4
@"\01?RayContributionToHitGroupIndex@@3IB" = external constant i32, align 4
@"\01?MultiplierForGeometryContributionToHitGroupIndex@@3IB" = external constant i32, align 4
@"\01?MissShaderIndex@@3IB" = external constant i32, align 4
@"$Globals" = external constant %"$Globals"

; CHECK: define <4 x float> @"
; CHECK-SAME: ?emit@@YA?AV?$vector@M$03@@AIAV?$vector@M$01@@URayDesc@@UPayload@@@Z"(<2 x float>* noalias dereferenceable(8) %f2, %struct.RayDesc* %Ray, %struct.Payload* noalias %p)

; Function Attrs: nounwind
define <4 x float> @"\01?emit@@YA?AV?$vector@M$03@@AIAV?$vector@M$01@@URayDesc@@UPayload@@@Z"(<2 x float>* noalias dereferenceable(8) %f2, %struct.RayDesc* %Ray, %struct.Payload* noalias %p) #0 {
entry:

  ; Copy Payload fields (PLD_F0, PLD_F1) to local allocas:
  ; CHECK: %[[GEP:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %p, i32 0, i32 0
  ; CHECK: %[[LOAD:[^ ,]+]] = load <2 x float>, <2 x float>* %[[GEP]]
  ; CHECK: store <2 x float> %[[LOAD]], <2 x float>* %[[PLD_F0:[^ ,]+]]
  ; CHECK: %[[GEP:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %p, i32 0, i32 1
  ; CHECK: %[[LOAD:[^ ,]+]] = load <3 x i32>, <3 x i32>* %[[GEP]]
  ; CHECK: store <3 x i32> %[[LOAD]], <3 x i32>* %[[PLD_F1:[^ ,]+]]

  %0 = alloca %struct.RayDesc, !dbg !39 ; line:22 col:61
  %1 = bitcast %struct.RayDesc* %0 to i8*, !dbg !39 ; line:22 col:61
  %2 = bitcast %struct.RayDesc* %Ray to i8*, !dbg !39 ; line:22 col:61
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* %2, i64 32, i32 1, i1 false), !dbg !39 ; line:22 col:61
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0), !dbg !39 ; line:22 col:61
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 13, i32 20 }, %"$Globals" undef), !dbg !39 ; line:22 col:61
  %5 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %4, i32 0), !dbg !39 ; line:22 col:61
  %6 = getelementptr inbounds %"$Globals", %"$Globals"* %5, i32 0, i32 0, !dbg !39 ; line:22 col:61
  %7 = getelementptr inbounds %"$Globals", %"$Globals"* %5, i32 0, i32 1, !dbg !39 ; line:22 col:61
  %8 = getelementptr inbounds %"$Globals", %"$Globals"* %5, i32 0, i32 2, !dbg !39 ; line:22 col:61
  %9 = getelementptr inbounds %"$Globals", %"$Globals"* %5, i32 0, i32 3, !dbg !39 ; line:22 col:61
  %10 = getelementptr inbounds %"$Globals", %"$Globals"* %5, i32 0, i32 4, !dbg !39 ; line:22 col:61
  %11 = load i32, i32* %10, align 4, !dbg !39, !tbaa !43 ; line:22 col:61
  %12 = load i32, i32* %9, align 4, !dbg !47, !tbaa !43 ; line:22 col:12
  %13 = load i32, i32* %8, align 4, !dbg !48, !tbaa !43 ; line:21 col:12
  %14 = load i32, i32* %7, align 4, !dbg !49, !tbaa !43 ; line:20 col:25
  %15 = load i32, i32* %6, align 4, !dbg !50, !tbaa !43 ; line:20 col:16
  %16 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?Acc@@3URaytracingAccelerationStructure@@A", !dbg !51 ; line:20 col:3
  %17 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %16), !dbg !51 ; line:20 col:3

  ; CHECK: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef)
  %18 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %17, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef), !dbg !51 ; line:20 col:3

  ; Copy RayDesc fields (Origin, TMin, Direction, TMax) to local allocas:
  ; CHECK: %[[RAY_ORIGIN_GEP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %Ray, i32 0, i32 0
  ; CHECK: %[[RAY_ORIGIN_LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RAY_ORIGIN_GEP]]
  ; CHECK: store <3 x float> %[[RAY_ORIGIN_LOAD]], <3 x float>* %[[RAY_ORIGIN_P0:[^ ,]+]]
  ; CHECK: %[[TMIN_GEP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %Ray, i32 0, i32 1
  ; CHECK: %[[TMIN_LOAD:[^ ,]+]] = load float, float* %[[TMIN_GEP]]
  ; CHECK: store float %[[TMIN_LOAD]], float* %[[TMIN_P0:[^ ,]+]]
  ; CHECK: %[[DIRECTION_GEP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %Ray, i32 0, i32 2
  ; CHECK: %[[DIRECTION_LOAD:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIRECTION_GEP]]
  ; CHECK: store <3 x float> %[[DIRECTION_LOAD]], <3 x float>* %[[DIRECTION_P0:[^ ,]+]]
  ; CHECK: %[[TMAX_GEP:[^ ,]+]] = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %Ray, i32 0, i32 3
  ; CHECK: %[[TMAX_LOAD:[^ ,]+]] = load float, float* %[[TMAX_GEP]]
  ; CHECK: store float %[[TMAX_LOAD]], float* %[[TMAX_P0:[^ ,]+]]

  ; Copy Payload fields into payload struct for call:
  ; CHECK: %[[PLD_F0_GEP:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %[[PLD_P0:[^ ,]+]], i32 0, i32 0
  ; CHECK: %[[PLD_F0_LOAD:[^ ,]+]] = load <2 x float>, <2 x float>* %[[PLD_F0]]
  ; CHECK: store <2 x float> %[[PLD_F0_LOAD]], <2 x float>* %[[PLD_F0_GEP]]
  ; CHECK: %[[PLD_F1_GEP:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %[[PLD_P0]], i32 0, i32 1
  ; CHECK: %[[PLD_F1_LOAD:[^ ,]+]] = load <3 x i32>, <3 x i32>* %[[PLD_F1]]
  ; CHECK: store <3 x i32> %[[PLD_F1_LOAD]], <3 x i32>* %[[PLD_F1_GEP]]

  ; Load RayDesc fields:
  ; CHECK: %[[RAY_ORIGIN_LOAD2:[^ ,]+]] = load <3 x float>, <3 x float>* %[[RAY_ORIGIN_P0]]
  ; CHECK: %[[TMIN_LOAD2:[^ ,]+]] = load float, float* %[[TMIN_P0]]
  ; CHECK: %[[DIRECTION_LOAD2:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIRECTION_P0]]
  ; CHECK: %[[TMAX_LOAD2:[^ ,]+]] = load float, float* %[[TMAX_P0]]

  ; call TraceRay with the local allocas:
  ; CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32 69, %dx.types.Handle %[[RTAS]], i32 %{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 %{{[^ ,]+}}, i32 %{{[^ ,]+}}, <3 x float> %[[RAY_ORIGIN_LOAD2]], float %[[TMIN_LOAD2]], <3 x float> %[[DIRECTION_LOAD2]], float %[[TMAX_LOAD2]], %struct.Payload* %[[PLD_P0]])

  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*)"(i32 69, %dx.types.Handle %18, i32 %15, i32 %14, i32 %13, i32 %12, i32 %11, %struct.RayDesc* %0, %struct.Payload* %p), !dbg !51 ; line:20 col:3

  ret <4 x float> <float 0x4004CCCCC0000000, float 0x4004CCCCC0000000, float 0x4004CCCCC0000000, float 0x4004CCCCC0000000>, !dbg !52 ; line:24 col:4
}

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*)"(i32, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"$Globals") #1

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !21}
!dx.entryPoints = !{!30}
!dx.fnprops = !{}
!dx.options = !{!37, !38}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4928 (ser_hlslattributes_patch, 937c16cc6)"}
!3 = !{i32 1, i32 3}
!4 = !{i32 1, i32 9}
!5 = !{!"lib", i32 6, i32 3}
!6 = !{i32 0, %struct.RayDesc undef, !7, %struct.Payload undef, !12, %"$Globals" undef, !15}
!7 = !{i32 32, !8, !9, !10, !11}
!8 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9}
!9 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!10 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9}
!11 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!12 = !{i32 28, !13, !14}
!13 = !{i32 6, !"t", i32 3, i32 0, i32 7, i32 9}
!14 = !{i32 6, !"t2", i32 3, i32 16, i32 7, i32 4}
!15 = !{i32 20, !16, !17, !18, !19, !20}
!16 = !{i32 6, !"RayFlags", i32 3, i32 0, i32 7, i32 5}
!17 = !{i32 6, !"InstanceInclusionMask", i32 3, i32 4, i32 7, i32 5}
!18 = !{i32 6, !"RayContributionToHitGroupIndex", i32 3, i32 8, i32 7, i32 5}
!19 = !{i32 6, !"MultiplierForGeometryContributionToHitGroupIndex", i32 3, i32 12, i32 7, i32 5}
!20 = !{i32 6, !"MissShaderIndex", i32 3, i32 16, i32 7, i32 5}
!21 = !{i32 1, <4 x float> (<2 x float>*, %struct.RayDesc*, %struct.Payload*)* @"\01?emit@@YA?AV?$vector@M$03@@AIAV?$vector@M$01@@URayDesc@@UPayload@@@Z", !22}
!22 = !{!23, !26, !27, !29}
!23 = !{i32 1, !24, !25}
!24 = !{i32 7, i32 9}
!25 = !{}
!26 = !{i32 2, !24, !25}
!27 = !{i32 0, !28, !25}
!28 = !{i32 4, !"R"}
!29 = !{i32 2, !25, !25}
!30 = !{null, !"", null, !31, null}
!31 = !{!32, null, !35, null}
!32 = !{!33}
!33 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?Acc@@3URaytracingAccelerationStructure@@A", !"Acc", i32 -1, i32 -1, i32 1, i32 16, i32 0, !34}
!34 = !{i32 0, i32 4}
!35 = !{!36}
!36 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 20, null}
!37 = !{i32 -2147483584}
!38 = !{i32 11}
!39 = !DILocation(line: 22, column: 61, scope: !40)
!40 = !DISubprogram(name: "emit", scope: !41, file: !41, line: 19, type: !42, isLocal: false, isDefinition: true, scopeLine: 19, flags: DIFlagPrototyped, isOptimized: false, function: <4 x float> (<2 x float>*, %struct.RayDesc*, %struct.Payload*)* @"\01?emit@@YA?AV?$vector@M$03@@AIAV?$vector@M$01@@URayDesc@@UPayload@@@Z")
!41 = !DIFile(filename: "D:\5Cgit\5Cdxc\5Cmain\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Cshader_targets\5Craytracing\5Craytracing_traceray.hlsl", directory: "")
!42 = !DISubroutineType(types: !25)
!43 = !{!44, !44, i64 0}
!44 = !{!"int", !45, i64 0}
!45 = !{!"omnipotent char", !46, i64 0}
!46 = !{!"Simple C/C++ TBAA"}
!47 = !DILocation(line: 22, column: 12, scope: !40)
!48 = !DILocation(line: 21, column: 12, scope: !40)
!49 = !DILocation(line: 20, column: 25, scope: !40)
!50 = !DILocation(line: 20, column: 16, scope: !40)
!51 = !DILocation(line: 20, column: 3, scope: !40)
!52 = !DILocation(line: 24, column: 4, scope: !40)
