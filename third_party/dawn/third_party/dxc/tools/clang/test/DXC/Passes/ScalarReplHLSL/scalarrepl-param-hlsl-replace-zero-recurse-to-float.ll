; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; The pass replaces a memcpy from a zero-initialized global that does not have an intervening store.
; When tracing through geps and bitcasts of uses of that global, the algorithm might
; bottom out at replacing a load of a scalar float.  Verify this works.

; In the following code, %2 should be replaced by float 0.0
;    %2 = load float, float* %src_in_g,...
; It only has one use: being stored to one of the elements of @g_1

; CHECK: for.body.i:
; CHECK: [[DEST:%[a-z0-9\.]+]] = getelementptr inbounds [10 x float], [10 x float]* @g_1, i32 0
; CHECK: store float 0.000000e+00, float* [[DEST]]
; CHECK: end.block:


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.ByteAddressBuffer = type { i32 }
%ConstantBuffer = type opaque
%struct.PSOut = type { <4 x float> }

@"\01?g_2@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@g = internal global [10 x float] zeroinitializer, align 4
@g_1 = internal global [10 x float] zeroinitializer, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @frag_main(%struct.PSOut* noalias sret %agg.result) #0 {
entry:
  %i.i = alloca i32, align 4
  %copy.i = alloca [10 x float], align 4
  %wrapper_result = alloca %struct.PSOut, align 4
  store i32 0, i32* %i.i, align 4, !dbg !23, !tbaa !29 ; line:10 col:12
  br label %for.cond.i, !dbg !33 ; line:10 col:8

for.cond.i:                                       ; preds = %for.body.i, %entry
  %0 = load i32, i32* %i.i, align 4, !dbg !34, !tbaa !29 ; line:10 col:19
  %cmp.i = icmp slt i32 %0, 10, !dbg !35 ; line:10 col:21
  br i1 %cmp.i, label %for.body.i, label %end.block, !dbg !36 ; line:10 col:3

for.body.i:                                       ; preds = %for.cond.i
  %1 = load i32, i32* %i.i, align 4, !dbg !37, !tbaa !29 ; line:11 col:16
  %src_in_g = getelementptr inbounds [10 x float], [10 x float]* @g, i32 0, i32 %1, !dbg !38 ; line:11 col:14
  %2 = load float, float* %src_in_g, align 4, !dbg !38, !tbaa !39 ; line:11 col:14
  %3 = load i32, i32* %i.i, align 4, !dbg !41, !tbaa !29 ; line:11 col:9
  %dest = getelementptr inbounds [10 x float], [10 x float]* @g_1, i32 0, i32 %3, !dbg !42 ; line:11 col:5
  store float %2, float* %dest, align 4, !dbg !43, !tbaa !39 ; line:11 col:12
  %4 = load i32, i32* %i.i, align 4, !dbg !44, !tbaa !29 ; line:10 col:28
  %inc.i = add nsw i32 %4, 1, !dbg !44 ; line:10 col:28
  store i32 %inc.i, i32* %i.i, align 4, !dbg !44, !tbaa !29 ; line:10 col:28
  br label %for.cond.i, !dbg !36 ; line:10 col:3

end.block:         ; preds = %for.cond.i
  %5 = bitcast [10 x float]* %copy.i to i8*, !dbg !45 ; line:13 col:20
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %5, i8* bitcast ([10 x float]* @g to i8*), i64 40, i32 1, i1 false) #0, !dbg !45 ; line:13 col:20
  %6 = bitcast [10 x float]* %copy.i to i8*, !dbg !46 ; line:14 col:7
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([10 x float]* @g to i8*), i8* %6, i64 40, i32 1, i1 false) #0, !dbg !46 ; line:14 col:7
  %value = getelementptr inbounds %struct.PSOut, %struct.PSOut* %wrapper_result, i32 0, i32 0, !dbg !47 ; line:20 col:18
  store <4 x float> zeroinitializer, <4 x float>* %value, align 4, !dbg !48, !tbaa !49 ; line:20 col:24
  %7 = bitcast %struct.PSOut* %agg.result to i8*, !dbg !50 ; line:21 col:10
  %8 = bitcast %struct.PSOut* %wrapper_result to i8*, !dbg !50 ; line:21 col:10
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %7, i8* %8, i64 16, i32 1, i1 false), !dbg !50 ; line:21 col:10
  ret void, !dbg !51 ; line:21 col:3
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !9}
!dx.entryPoints = !{!14}
!dx.fnprops = !{!20}
!dx.options = !{!21, !22}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.14549 (main, 0781ded87-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"ps", i32 6, i32 0}
!6 = !{i32 0, %struct.PSOut undef, !7}
!7 = !{i32 16, !8}
!8 = !{i32 6, !"value", i32 3, i32 0, i32 4, !"SV_Target0", i32 7, i32 9}
!9 = !{i32 1, void (%struct.PSOut*)* @frag_main, !10}
!10 = !{!11, !13}
!11 = !{i32 0, !12, !12}
!12 = !{}
!13 = !{i32 1, !12, !12}
!14 = !{void (%struct.PSOut*)* @frag_main, !"frag_main", null, !15, null}
!15 = !{!16, null, !18, null}
!16 = !{!17}
!17 = !{i32 0, %struct.ByteAddressBuffer* @"\01?g_2@@3UByteAddressBuffer@@A", !"g_2", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!18 = !{!19}
!19 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!20 = !{void (%struct.PSOut*)* @frag_main, i32 0, i1 false}
!21 = !{i32 144}
!22 = !{i32 -1}
!23 = !DILocation(line: 10, column: 12, scope: !24, inlinedAt: !27)
!24 = !DISubprogram(name: "inner", scope: !25, file: !25, line: 9, type: !26, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: false)
!25 = !DIFile(filename: "float.hlsl", directory: "")
!26 = !DISubroutineType(types: !12)
!27 = distinct !DILocation(line: 20, column: 26, scope: !28)
!28 = !DISubprogram(name: "frag_main", scope: !25, file: !25, line: 18, type: !26, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, function: void (%struct.PSOut*)* @frag_main)
!29 = !{!30, !30, i64 0}
!30 = !{!"int", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C/C++ TBAA"}
!33 = !DILocation(line: 10, column: 8, scope: !24, inlinedAt: !27)
!34 = !DILocation(line: 10, column: 19, scope: !24, inlinedAt: !27)
!35 = !DILocation(line: 10, column: 21, scope: !24, inlinedAt: !27)
!36 = !DILocation(line: 10, column: 3, scope: !24, inlinedAt: !27)
!37 = !DILocation(line: 11, column: 16, scope: !24, inlinedAt: !27)
!38 = !DILocation(line: 11, column: 14, scope: !24, inlinedAt: !27)
!39 = !{!40, !40, i64 0}
!40 = !{!"float", !31, i64 0}
!41 = !DILocation(line: 11, column: 9, scope: !24, inlinedAt: !27)
!42 = !DILocation(line: 11, column: 5, scope: !24, inlinedAt: !27)
!43 = !DILocation(line: 11, column: 12, scope: !24, inlinedAt: !27)
!44 = !DILocation(line: 10, column: 28, scope: !24, inlinedAt: !27)
!45 = !DILocation(line: 13, column: 20, scope: !24, inlinedAt: !27)
!46 = !DILocation(line: 14, column: 7, scope: !24, inlinedAt: !27)
!47 = !DILocation(line: 20, column: 18, scope: !28)
!48 = !DILocation(line: 20, column: 24, scope: !28)
!49 = !{!31, !31, i64 0}
!50 = !DILocation(line: 21, column: 10, scope: !28)
!51 = !DILocation(line: 21, column: 3, scope: !28)
