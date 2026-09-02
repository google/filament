; RUN: %opt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Check that dynamic indexing results in dynamic indexing of cbuffer VectorArray
; and contained vector, instead of using a local copy.

; Generated using:
; ExtractIRForPassTest.py -p scalarrepl-param-hlsl -o array-from-cbvec-3.ll array-from-cbvec-3.hlsl -- -T vs_6_0
; uint4 VectorArray[2];
; static const uint ScalarArray[8] = (uint[8])VectorArray;
;
; uint main(int i : IN) : OUT {
;     return ScalarArray[i];
; }

; CHECK:define void @main(i32* noalias, i32)
; CHECK:  %[[iaddr:.*]] = alloca i32, align 4
; CHECK:  %[[VectorArray:.*]] = getelementptr inbounds %"$Globals", %"$Globals"* %{{.*}}, i32 0, i32 0
; CHECK:  store i32 %1, i32* %[[iaddr]], align 4
; CHECK:  %[[ld_i:.*]] = load i32, i32* %[[iaddr]], align 4
; CHECK:  %[[add_0_i:.*]] = add i32 0, %[[ld_i]]
; CHECK:  %[[lshr_i:.*]] = lshr i32 %[[add_0_i]], 2
; CHECK:  %[[gep_VA_i:.*]] = getelementptr [2 x <4 x i32>], [2 x <4 x i32>]* %[[VectorArray]], i32 0, i32 %[[lshr_i]]
; CHECK:  %[[and_i_3:.*]] = and i32 %[[add_0_i]], 3
; CHECK:  %[[gep_VA_i_and_i_3:.*]] = getelementptr <4 x i32>, <4 x i32>* %[[gep_VA_i]], i32 0, i32 %[[and_i_3]]
; CHECK:  %[[ld_VA_i_and_i_3:.*]] = load i32, i32* %[[gep_VA_i_and_i_3]], align 4
; CHECK:  store i32 %[[ld_VA_i_and_i_3]], i32* %0
; CHECK:  ret void

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { [2 x <4 x i32>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?VectorArray@@3QBV?$vector@I$03@@B" = external constant [2 x <4 x i32>], align 4
@ScalarArray = internal global [8 x i32] undef, align 4
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define i32 @main(i32 %i) #0 {
entry:
  %0 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0) #0, !dbg !23 ; line:2 col:45
  %1 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %0, %dx.types.ResourceProperties { i32 13, i32 32 }, %"$Globals" undef) #0, !dbg !23 ; line:2 col:45
  %2 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %1, i32 0) #0, !dbg !23 ; line:2 col:45
  %3 = getelementptr inbounds %"$Globals", %"$Globals"* %2, i32 0, i32 0, !dbg !23 ; line:2 col:45
  %4 = bitcast [2 x <4 x i32>]* %3 to i8*, !dbg !23 ; line:2 col:45
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([8 x i32]* @ScalarArray to i8*), i8* %4, i64 32, i32 1, i1 false) #0, !dbg !23 ; line:2 col:45
  %i.addr = alloca i32, align 4, !dx.temp !13
  store i32 %i, i32* %i.addr, align 4, !tbaa !29
  %5 = load i32, i32* %i.addr, align 4, !dbg !33, !tbaa !29 ; line:5 col:24
  %arrayidx = getelementptr inbounds [8 x i32], [8 x i32]* @ScalarArray, i32 0, i32 %5, !dbg !34 ; line:5 col:12
  %6 = load i32, i32* %arrayidx, align 4, !dbg !34, !tbaa !29 ; line:5 col:12
  ret i32 %6, !dbg !35 ; line:5 col:5
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind readnone
declare %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32, %"$Globals"*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"$Globals") #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !9}
!dx.entryPoints = !{!16}
!dx.fnprops = !{!20}
!dx.options = !{!21, !22}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.14508 (main, 263a77335-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 6}
!5 = !{!"vs", i32 6, i32 0}
!6 = !{i32 0, %"$Globals" undef, !7}
!7 = !{i32 32, !8}
!8 = !{i32 6, !"VectorArray", i32 3, i32 0, i32 7, i32 5}
!9 = !{i32 1, i32 (i32)* @main, !10}
!10 = !{!11, !14}
!11 = !{i32 1, !12, !13}
!12 = !{i32 4, !"OUT", i32 7, i32 5}
!13 = !{}
!14 = !{i32 0, !15, !13}
!15 = !{i32 4, !"IN", i32 7, i32 4}
!16 = !{i32 (i32)* @main, !"main", null, !17, null}
!17 = !{null, null, !18, null}
!18 = !{!19}
!19 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 32, null}
!20 = !{i32 (i32)* @main, i32 1}
!21 = !{i32 -2147483584}
!22 = !{i32 -1}
!23 = !DILocation(line: 2, column: 45, scope: !24, inlinedAt: !27)
!24 = !DISubprogram(name: "??__EScalarArray@@YAXXZ", scope: !25, file: !25, line: 2, type: !26, isLocal: true, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false)
!25 = !DIFile(filename: "t:\5Carray-mapping.hlsl", directory: "")
!26 = !DISubroutineType(types: !13)
!27 = distinct !DILocation(line: 4, scope: !28)
!28 = !DISubprogram(name: "main", scope: !25, file: !25, line: 4, type: !26, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, function: i32 (i32)* @main)
!29 = !{!30, !30, i64 0}
!30 = !{!"int", !31, i64 0}
!31 = !{!"omnipotent char", !32, i64 0}
!32 = !{!"Simple C/C++ TBAA"}
!33 = !DILocation(line: 5, column: 24, scope: !28)
!34 = !DILocation(line: 5, column: 12, scope: !28)
!35 = !DILocation(line: 5, column: 5, scope: !28)
