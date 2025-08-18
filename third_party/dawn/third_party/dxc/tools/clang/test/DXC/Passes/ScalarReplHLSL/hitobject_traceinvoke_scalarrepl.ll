; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Based on tools/clang/test/CodeGenDXIL/hlsl/objects/HitObject/hitobject_traceinvoke.hlsl

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%"class.RWStructuredBuffer<float>" = type { float }
%ConstantBuffer = type opaque
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%struct.Payload = type { <3 x float> }
%dx.types.HitObject = type { i8* }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.dx::HitObject" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4
@"\01?UAV@@3V?$RWStructuredBuffer@M@@A" = external global %"class.RWStructuredBuffer<float>", align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %rayDesc = alloca %struct.RayDesc, align 4
  %pld = alloca %struct.Payload, align 4

  ; CHECK: %[[HITOBJ:[^ ,]+]] = alloca %dx.types.HitObject, align 4

  %hit = alloca %dx.types.HitObject, align 4

  %0 = bitcast %struct.RayDesc* %rayDesc to i8*, !dbg !37 ; line:82 col:3
  call void @llvm.lifetime.start(i64 32, i8* %0) #0, !dbg !37 ; line:82 col:3

  ; Init RayDesc.
  ; CHECK-DAG: store <3 x float> <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00>, <3 x float>* %[[ORIGIN_P0:[^ ,]+]], align 4
  ; CHECK-DAG: store float 3.000000e+00, float* %[[TMIN_P0:[^ ,]+]], align 4
  ; CHECK-DAG: store <3 x float> <float 4.000000e+00, float 5.000000e+00, float 6.000000e+00>, <3 x float>* %[[DIRECTION_P0:[^ ,]+]], align 4
  ; CHECK-DAG: store float 7.000000e+00, float* %[[TMAX_P0:[^ ,]+]], align 4

  %Origin = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %rayDesc, i32 0, i32 0, !dbg !41 ; line:83 col:11
  store <3 x float> <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00>, <3 x float>* %Origin, align 4, !dbg !42, !tbaa !43 ; line:83 col:18
  %TMin = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %rayDesc, i32 0, i32 1, !dbg !46 ; line:84 col:11
  store float 3.000000e+00, float* %TMin, align 4, !dbg !47, !tbaa !48 ; line:84 col:16
  %Direction = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %rayDesc, i32 0, i32 2, !dbg !50 ; line:85 col:11
  store <3 x float> <float 4.000000e+00, float 5.000000e+00, float 6.000000e+00>, <3 x float>* %Direction, align 4, !dbg !51, !tbaa !43 ; line:85 col:21
  %TMax = getelementptr inbounds %struct.RayDesc, %struct.RayDesc* %rayDesc, i32 0, i32 3, !dbg !52 ; line:86 col:11
  store float 7.000000e+00, float* %TMax, align 4, !dbg !53, !tbaa !48 ; line:86 col:16

  %1 = bitcast %struct.Payload* %pld to i8*, !dbg !54 ; line:88 col:3
  call void @llvm.lifetime.start(i64 12, i8* %1) #0, !dbg !54 ; line:88 col:3
  %dummy = getelementptr inbounds %struct.Payload, %struct.Payload* %pld, i32 0, i32 0, !dbg !55 ; line:89 col:7
  store <3 x float> <float 7.000000e+00, float 8.000000e+00, float 9.000000e+00>, <3 x float>* %dummy, align 4, !dbg !56, !tbaa !43 ; line:89 col:13
  %2 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !57 ; line:91 col:3
  call void @llvm.lifetime.start(i64 4, i8* %2) #0, !dbg !57 ; line:91 col:3
  %3 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !58 ; line:91 col:23
  %4 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %3), !dbg !58 ; line:91 col:23

  ; CHECK-DAG: %[[RTAS:[^ ,]+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %{{[^ ,]+}}, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef)

  %5 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure undef), !dbg !58 ; line:91 col:23

  ; Copy RayDesc.
  ; CHECK-DAG: %[[ORIGIN_L0:[^ ,]+]] = load <3 x float>, <3 x float>* %[[ORIGIN_P0]]
  ; CHECK-DAG: store <3 x float> %[[ORIGIN_L0]], <3 x float>* %[[ORIGIN_P1:[^ ,]+]]
  ; CHECK-DAG: %[[TMIN_L0:[^ ,]+]] = load float, float* %[[TMIN_P0]]
  ; CHECK-DAG: store float %[[TMIN_L0]], float* %[[TMIN_P1:[^ ,]+]]
  ; CHECK-DAG: %[[DIRECTION_L0:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIRECTION_P0]]
  ; CHECK-DAG: store <3 x float> %[[DIRECTION_L0]], <3 x float>* %[[DIRECTION_P1:[^ ,]+]]
  ; CHECK-DAG: %[[TMAX_L0:[^ ,]+]] = load float, float* %[[TMAX_P0]]
  ; CHECK-DAG: store float %[[TMAX_L0]], float* %[[TMAX_P1:[^ ,]+]]

  ; Load RayDesc.
  ; CHECK-DAG: %[[ORIGIN_L1:[^ ,]+]] = load <3 x float>, <3 x float>* %[[ORIGIN_P1]]
  ; CHECK-DAG: %[[TMIN_L1:[^ ,]+]] = load float, float* %[[TMIN_P1]]
  ; CHECK-DAG: %[[DIRECTION_L1:[^ ,]+]] = load <3 x float>, <3 x float>* %[[DIRECTION_P1]]
  ; CHECK-DAG: %[[TMAX_L1:[^ ,]+]] = load float, float* %[[TMAX_P1]]

  ; RayDesc is scalar replaced in HL op for dx::HitObject::TraceRay.
  ; CHECK: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32 389, %dx.types.HitObject* %[[HITOBJ]], %dx.types.Handle %[[RTAS]], i32 513, i32 1, i32 2, i32 4, i32 0, <3 x float> %[[ORIGIN_L1]], float %[[TMIN_L1]], <3 x float> %[[DIRECTION_L1]], float %[[TMAX_L1]], %struct.Payload* %[[PLD_P0:[^ ,]+]])

  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*)"(i32 389, %dx.types.HitObject* %hit, %dx.types.Handle %5, i32 513, i32 1, i32 2, i32 4, i32 0, %struct.RayDesc* %rayDesc, %struct.Payload* %pld), !dbg !58 ; line:91 col:23

  ; Copy payload.
  ; CHECK: %[[GEP_PLD_P0:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %[[PLD_P0]], i32 0, i32 0
  ; CHECK: %[[PLD_L0:[^ ,]+]] = load <3 x float>, <3 x float>* %[[GEP_PLD_P0]]
  ; CHECK: store <3 x float> %[[PLD_L0]], <3 x float>* %[[PLD_M0_P0:[^ ,]+]]
  ; CHECK: %[[GEP_PLD_P1:[^ ,]+]] = getelementptr inbounds %struct.Payload, %struct.Payload* %[[PLD_P1:[^ ,]+]], i32 0, i32 0
  ; CHECK: [[PLD_L1:[^ ,]+]] = load <3 x float>, <3 x float>* %[[PLD_M0_P0]]
  ; CHECK: store <3 x float> [[PLD_L1]], <3 x float>* %[[GEP_PLD_P1]]

  ; dx::HitObject::Invoke
  ; CHECK: call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32 382, %dx.types.HitObject* %[[HITOBJ]], %struct.Payload* %[[PLD_P1]])

  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32 382, %dx.types.HitObject* %hit, %struct.Payload* %pld), !dbg !59 ; line:101 col:3

  %6 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !60 ; line:102 col:1
  call void @llvm.lifetime.end(i64 4, i8* %6) #0, !dbg !60 ; line:102 col:1
  %7 = bitcast %struct.Payload* %pld to i8*, !dbg !60 ; line:102 col:1
  call void @llvm.lifetime.end(i64 12, i8* %7) #0, !dbg !60 ; line:102 col:1
  %8 = bitcast %struct.RayDesc* %rayDesc to i8*, !dbg !60 ; line:102 col:1
  call void @llvm.lifetime.end(i64 32, i8* %8) #0, !dbg !60 ; line:102 col:1
  ret void, !dbg !60 ; line:102 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*)"(i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, %struct.RayDesc*, %struct.Payload*) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32, %dx.types.HitObject*, %struct.Payload*) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !20}
!dx.entryPoints = !{!24}
!dx.fnprops = !{!34}
!dx.options = !{!35, !36}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4928 (ser_hlslattributes_patch, 937c16cc6)"}
!3 = !{i32 1, i32 9}
!4 = !{!"lib", i32 6, i32 9}
!5 = !{i32 0, %"class.RWStructuredBuffer<float>" undef, !6, %struct.RayDesc undef, !11, %struct.Payload undef, !16, %"class.dx::HitObject" undef, !18}
!6 = !{i32 4, !7, !8}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!8 = !{i32 0, !9}
!9 = !{!10}
!10 = !{i32 0, float undef}
!11 = !{i32 32, !12, !13, !14, !15}
!12 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!13 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!14 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9, i32 13, i32 3}
!15 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!16 = !{i32 12, !17}
!17 = !{i32 6, !"dummy", i32 3, i32 0, i32 7, i32 9, i32 13, i32 3}
!18 = !{i32 4, !19}
!19 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!20 = !{i32 1, void ()* @"\01?main@@YAXXZ", !21}
!21 = !{!22}
!22 = !{i32 1, !23, !23}
!23 = !{}
!24 = !{null, !"", null, !25, null}
!25 = !{!26, !29, !32, null}
!26 = !{!27}
!27 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !28}
!28 = !{i32 0, i32 4}
!29 = !{!30}
!30 = !{i32 0, %"class.RWStructuredBuffer<float>"* @"\01?UAV@@3V?$RWStructuredBuffer@M@@A", !"UAV", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !31}
!31 = !{i32 1, i32 4}
!32 = !{!33}
!33 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!34 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!35 = !{i32 -2147483584}
!36 = !{i32 -1}
!37 = !DILocation(line: 82, column: 3, scope: !38)
!38 = !DISubprogram(name: "main", scope: !39, file: !39, line: 81, type: !40, isLocal: false, isDefinition: true, scopeLine: 81, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!39 = !DIFile(filename: "D:\5Cgit\5Cdxc\5Cmain\5Ctools\5Cclang\5Ctest\5CCodeGenDXIL\5Chlsl\5Cobjects\5CHitObject\5Chitobject_traceinvoke.hlsl", directory: "")
!40 = !DISubroutineType(types: !23)
!41 = !DILocation(line: 83, column: 11, scope: !38)
!42 = !DILocation(line: 83, column: 18, scope: !38)
!43 = !{!44, !44, i64 0}
!44 = !{!"omnipotent char", !45, i64 0}
!45 = !{!"Simple C/C++ TBAA"}
!46 = !DILocation(line: 84, column: 11, scope: !38)
!47 = !DILocation(line: 84, column: 16, scope: !38)
!48 = !{!49, !49, i64 0}
!49 = !{!"float", !44, i64 0}
!50 = !DILocation(line: 85, column: 11, scope: !38)
!51 = !DILocation(line: 85, column: 21, scope: !38)
!52 = !DILocation(line: 86, column: 11, scope: !38)
!53 = !DILocation(line: 86, column: 16, scope: !38)
!54 = !DILocation(line: 88, column: 3, scope: !38)
!55 = !DILocation(line: 89, column: 7, scope: !38)
!56 = !DILocation(line: 89, column: 13, scope: !38)
!57 = !DILocation(line: 91, column: 3, scope: !38)
!58 = !DILocation(line: 91, column: 23, scope: !38)
!59 = !DILocation(line: 101, column: 3, scope: !38)
!60 = !DILocation(line: 102, column: 1, scope: !38)
