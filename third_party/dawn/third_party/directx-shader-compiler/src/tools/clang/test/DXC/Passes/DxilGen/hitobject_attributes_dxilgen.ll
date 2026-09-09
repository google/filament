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
%struct.CustomAttrs = type { <4 x float>, i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.dx::HitObject" = type { i32 }

@"\01?outbuf@@3URWByteAddressBuffer@@A" = external global %struct.RWByteAddressBuffer, align 4

; CHECK: %[[ATTRA:[^ ]+]] = alloca %struct.CustomAttrs, align 4
; CHECK: call void @dx.op.hitObject_Attributes.struct.CustomAttrs(i32 289, %dx.types.HitObject %{{[^ ]+}}, %struct.CustomAttrs* %[[ATTRA]])

; Function Attrs: nounwind
define void @"\01?main@@YAXXZ"() #0 {
entry:
  %hit = alloca %dx.types.HitObject, align 4
  %attrs = alloca %struct.CustomAttrs, align 4
  %0 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !21 ; line:29 col:3
  call void @llvm.lifetime.start(i64 4, i8* %0) #0, !dbg !21 ; line:29 col:3
  %1 = call %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32 358, %dx.types.HitObject* %hit), !dbg !25 ; line:29 col:17
  %2 = bitcast %struct.CustomAttrs* %attrs to i8*, !dbg !26 ; line:30 col:3
  call void @llvm.lifetime.start(i64 20, i8* %2) #0, !dbg !26 ; line:30 col:3
  call void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.CustomAttrs*)"(i32 364, %dx.types.HitObject* %hit, %struct.CustomAttrs* %attrs), !dbg !27 ; line:31 col:3
  %v = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !28 ; line:32 col:21
  %3 = load <4 x float>, <4 x float>* %v, align 4, !dbg !29 ; line:32 col:15
  %4 = extractelement <4 x float> %3, i32 0, !dbg !29 ; line:32 col:15
  %v1 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !30 ; line:32 col:33
  %5 = load <4 x float>, <4 x float>* %v1, align 4, !dbg !31 ; line:32 col:27
  %6 = extractelement <4 x float> %5, i32 1, !dbg !31 ; line:32 col:27
  %add = fadd float %4, %6, !dbg !32 ; line:32 col:25
  %v2 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !33 ; line:32 col:45
  %7 = load <4 x float>, <4 x float>* %v2, align 4, !dbg !34 ; line:32 col:39
  %8 = extractelement <4 x float> %7, i32 2, !dbg !34 ; line:32 col:39
  %add3 = fadd float %add, %8, !dbg !35 ; line:32 col:37
  %v4 = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 0, !dbg !36 ; line:32 col:57
  %9 = load <4 x float>, <4 x float>* %v4, align 4, !dbg !37 ; line:32 col:51
  %10 = extractelement <4 x float> %9, i32 3, !dbg !37 ; line:32 col:51
  %add5 = fadd float %add3, %10, !dbg !38 ; line:32 col:49
  %y = getelementptr inbounds %struct.CustomAttrs, %struct.CustomAttrs* %attrs, i32 0, i32 1, !dbg !39 ; line:32 col:69
  %11 = load i32, i32* %y, align 4, !dbg !39, !tbaa !40 ; line:32 col:69
  %conv = sitofp i32 %11 to float, !dbg !44 ; line:32 col:63
  %add6 = fadd float %add5, %conv, !dbg !45 ; line:32 col:61
  %12 = load %struct.RWByteAddressBuffer, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !dbg !46 ; line:33 col:3
  %13 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer %12), !dbg !46 ; line:33 col:3
  %14 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle %13, %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer zeroinitializer), !dbg !46 ; line:33 col:3
  call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32 277, %dx.types.Handle %14, i32 0, float %add6), !dbg !46 ; line:33 col:3
  %15 = bitcast %struct.CustomAttrs* %attrs to i8*, !dbg !47 ; line:34 col:1
  call void @llvm.lifetime.end(i64 20, i8* %15) #0, !dbg !47 ; line:34 col:1
  %16 = bitcast %dx.types.HitObject* %hit to i8*, !dbg !47 ; line:34 col:1
  call void @llvm.lifetime.end(i64 4, i8* %16) #0, !dbg !47 ; line:34 col:1
  ret void, !dbg !47 ; line:34 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare %dx.types.HitObject* @"dx.hl.op..%dx.types.HitObject* (i32, %dx.types.HitObject*)"(i32, %dx.types.HitObject*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.HitObject*, %struct.CustomAttrs*)"(i32, %dx.types.HitObject*, %struct.CustomAttrs*) #0

; Function Attrs: nounwind
declare void @"dx.hl.op..void (i32, %dx.types.Handle, i32, float)"(i32, %dx.types.Handle, i32, float) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32, %struct.RWByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !10}
!dx.entryPoints = !{!14}
!dx.fnprops = !{!18}
!dx.options = !{!19, !20}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{i32 1, i32 9}
!3 = !{!"lib", i32 6, i32 9}
!4 = !{i32 0, %"class.dx::HitObject" undef, !5, %struct.CustomAttrs undef, !7}
!5 = !{i32 4, !6}
!6 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!7 = !{i32 20, !8, !9}
!8 = !{i32 6, !"v", i32 3, i32 0, i32 7, i32 9, i32 13, i32 4}
!9 = !{i32 6, !"y", i32 3, i32 16, i32 7, i32 4}
!10 = !{i32 1, void ()* @"\01?main@@YAXXZ", !11}
!11 = !{!12}
!12 = !{i32 1, !13, !13}
!13 = !{}
!14 = !{null, !"", null, !15, null}
!15 = !{null, !16, null, null}
!16 = !{!17}
!17 = !{i32 0, %struct.RWByteAddressBuffer* @"\01?outbuf@@3URWByteAddressBuffer@@A", !"outbuf", i32 -1, i32 -1, i32 1, i32 11, i1 false, i1 false, i1 false, null}
!18 = !{void ()* @"\01?main@@YAXXZ", i32 7}
!19 = !{i32 -2147483584}
!20 = !{i32 -1}
!21 = !DILocation(line: 29, column: 3, scope: !22)
!22 = !DISubprogram(name: "main", scope: !23, file: !23, line: 28, type: !24, isLocal: false, isDefinition: true, scopeLine: 28, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @"\01?main@@YAXXZ")
!23 = !DIFile(filename: "tools/clang/test/SemaHLSL/hlsl/objects/HitObject/hitobject_attributes.hlsl", directory: "")
!24 = !DISubroutineType(types: !13)
!25 = !DILocation(line: 29, column: 17, scope: !22)
!26 = !DILocation(line: 30, column: 3, scope: !22)
!27 = !DILocation(line: 31, column: 3, scope: !22)
!28 = !DILocation(line: 32, column: 21, scope: !22)
!29 = !DILocation(line: 32, column: 15, scope: !22)
!30 = !DILocation(line: 32, column: 33, scope: !22)
!31 = !DILocation(line: 32, column: 27, scope: !22)
!32 = !DILocation(line: 32, column: 25, scope: !22)
!33 = !DILocation(line: 32, column: 45, scope: !22)
!34 = !DILocation(line: 32, column: 39, scope: !22)
!35 = !DILocation(line: 32, column: 37, scope: !22)
!36 = !DILocation(line: 32, column: 57, scope: !22)
!37 = !DILocation(line: 32, column: 51, scope: !22)
!38 = !DILocation(line: 32, column: 49, scope: !22)
!39 = !DILocation(line: 32, column: 69, scope: !22)
!40 = !{!41, !41, i64 0}
!41 = !{!"int", !42, i64 0}
!42 = !{!"omnipotent char", !43, i64 0}
!43 = !{!"Simple C/C++ TBAA"}
!44 = !DILocation(line: 32, column: 63, scope: !22)
!45 = !DILocation(line: 32, column: 61, scope: !22)
!46 = !DILocation(line: 33, column: 3, scope: !22)
!47 = !DILocation(line: 34, column: 1, scope: !22)
