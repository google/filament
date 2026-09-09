; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s
; REQUIRES: dxil-1-9

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.RaytracingAccelerationStructure = type { i32 }
%struct.Payload = type { <3 x float> }
%dx.types.HitObject = type { i8* }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RWStructuredBuffer<float>" = type { float }
%struct.RayDesc = type { <3 x float>, float, <3 x float>, float }
%"class.dx::HitObject" = type { i32 }

@"\01?RTAS@@3URaytracingAccelerationStructure@@A" = external global %struct.RaytracingAccelerationStructure, align 4

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %pld_invoke = alloca %struct.Payload
  %pld_trace = alloca %struct.Payload
  %hit = alloca %dx.types.HitObject, align 4
  %0 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !32 ; line:91 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !32 ; line:91 col:3
  %1 = load %struct.RaytracingAccelerationStructure, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !dbg !36 ; line:91 col:23
  %rtas = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32 0, %struct.RaytracingAccelerationStructure %1), !dbg !36 ; line:91 col:23

  ; Capture the handle for the RTAS
  ; CHECK: %[[RTAS:[^ ]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{[^ ]+}}, %dx.types.ResourceProperties { i32 16, i32 0 })
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32 14, %dx.types.Handle %rtas, %dx.types.ResourceProperties { i32 16, i32 0 }, %struct.RaytracingAccelerationStructure zeroinitializer), !dbg !36 ; line:91 col:23

  %3 = getelementptr inbounds %struct.Payload, %struct.Payload* %pld_trace, i32 0, i32 0, !dbg !36 ; line:91 col:23
  store <3 x float> <float 7.000000e+00, float 8.000000e+00, float 9.000000e+00>, <3 x float>* %3, !dbg !36 ; line:91 col:23

  ; CHECK: %[[TRACEHO:[^ ]+]] = call %dx.types.HitObject @dx.op.hitObject_TraceRay.struct.Payload(i32 262, %dx.types.Handle %[[RTAS]], i32 513, i32 1, i32 2, i32 4, i32 0, float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, %struct.Payload* %pld_trace), !dbg !3 ; line:91 col:23
  ; CHECK: store %dx.types.HitObject %[[TRACEHO]], %dx.types.HitObject* %hit
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32 389, %dx.types.HitObject* %hit, %dx.types.Handle %2, i32 513, i32 1, i32 2, i32 4, i32 0, <3 x float> <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00>, float 3.000000e+00, <3 x float> <float 4.000000e+00, float 5.000000e+00, float 6.000000e+00>, float 7.000000e+00, %struct.Payload* %pld_trace), !dbg !36 ; line:91 col:23

  %4 = getelementptr inbounds %struct.Payload, %struct.Payload* %pld_trace, i32 0, i32 0, !dbg !37 ; line:101 col:3
  %5 = load <3 x float>, <3 x float>* %4, !dbg !37 ; line:101 col:3
  %6 = getelementptr inbounds %struct.Payload, %struct.Payload* %pld_invoke, i32 0, i32 0, !dbg !37 ; line:101 col:3
  store <3 x float> %5, <3 x float>* %6, !dbg !37 ; line:101 col:3

  ; CHECK: %[[INVOKEHO:[^ ]+]] = load %dx.types.HitObject, %dx.types.HitObject* %hit
  ; CHECK: call void @dx.op.hitObject_Invoke.struct.Payload(i32 267, %dx.types.HitObject %[[INVOKEHO]], %struct.Payload* %pld_invoke)
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32 382, %dx.types.HitObject* %hit, %struct.Payload* %pld_invoke), !dbg !37 ; line:101 col:3

  %7 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !38 ; line:102 col:1
  call void @llvm.lifetime.end(i64 4, i8* %7) #0, !dbg !38 ; line:102 col:1
  ret void, !dbg !38 ; line:102 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RaytracingAccelerationStructure)"(i32, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RaytracingAccelerationStructure) #1

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.Payload*)"(i32, %dx.types.HitObject*, %struct.Payload*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*)"(i32, %dx.types.HitObject*, %dx.types.Handle, i32, i32, i32, i32, i32, <3 x float>, float, <3 x float>, float, %struct.Payload*) #0

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
!dx.fnprops = !{!29}
!dx.options = !{!30, !31}

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
!25 = !{!26, null, null, null}
!26 = !{!27}
!27 = !{i32 0, %struct.RaytracingAccelerationStructure* @"\01?RTAS@@3URaytracingAccelerationStructure@@A", !"RTAS", i32 -1, i32 -1, i32 1, i32 16, i32 0, !28}
!28 = !{i32 0, i32 4}
!29 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!30 = !{i32 -2147483584}
!31 = !{i32 -1}
!32 = !DILocation(line: 91, column: 3, scope: !33)
!33 = !DISubprogram(name: "main", scope: !34, file: !34, line: 81, type: !35, isLocal: false, isDefinition: true, scopeLine: 81, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!34 = !DIFile(filename: "tools/clang/test/CodeGenDXIL/hlsl/objects/HitObject/hitobject_traceinvoke.hlsl", directory: "")
!35 = !DISubroutineType(types: !23)
!36 = !DILocation(line: 91, column: 23, scope: !33)
!37 = !DILocation(line: 101, column: 3, scope: !33)
!38 = !DILocation(line: 102, column: 1, scope: !33)
