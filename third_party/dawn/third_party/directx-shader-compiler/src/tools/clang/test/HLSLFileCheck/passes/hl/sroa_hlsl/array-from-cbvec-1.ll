; RUN: %opt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; This case should fail to do memcpy replacement because init block does not
; dominate init.end block.
; However, it should not produce invalid IR, as it would have with #6510 due
; to attempting to replace constant dest with instruction source, while
; assuming the source was not constant.

; This makes sure memcpy was split and elements are properly copied to the
; scalar array, then that the scalar array is used for the result.

; Generated using:
; ExtractIRForPassTest.py -p scalarrepl-param-hlsl -o array-to-cbvec-1.ll array-to-cbvec-1.hlsl -- -T vs_6_0
; uint4 VectorArray[2];
;
; uint2 main(int i : IN) : OUT {
;     static const uint ScalarArray[8] = (uint[8])VectorArray;
;     return uint2(ScalarArray[1], ScalarArray[6]);
; }

; CHECK-NOT: badref
; CHECK-NOT: store <4 x float> zeroinitializer

; Copy array elements from constant to scalar array
; CHECK:  %[[VectorArray:.*]] = getelementptr inbounds %"$Globals", %"$Globals"* %{{.*}}, i32 0, i32 0
; CHECK:  %[[gep_VA0:.*]] = getelementptr [2 x <4 x i32>], [2 x <4 x i32>]* %[[VectorArray]], i32 0, i32 0
; CHECK:  %[[ld_VA0:.*]] = load <4 x i32>, <4 x i32>* %[[gep_VA0]]
; CHECK:  %[[ea_VA0_1:.*]] = extractelement <4 x i32> %[[ld_VA0]], i64 1
; CHECK:  store i32 %[[ea_VA0_1]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 1)
; CHECK:  %[[gep_VA1:.*]] = getelementptr [2 x <4 x i32>], [2 x <4 x i32>]* %[[VectorArray]], i32 0, i32 1
; CHECK:  %[[ld_VA1:.*]] = load <4 x i32>, <4 x i32>* %[[gep_VA1]]
; CHECK:  %[[ea_VA1_2:.*]] = extractelement <4 x i32> %[[ld_VA1]], i64 2
; CHECK:  store i32 %[[ea_VA1_2]], i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 6)

; Load from scalar array and return it
; CHECK:  %[[ld_SA1:.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 1), align 4
; CHECK:  %[[ld_SA6:.*]] = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 6), align 4
; CHECK:  %[[ie_SA1:.*]] = insertelement <2 x i32> undef, i32 %[[ld_SA1]], i64 0
; CHECK:  %[[ie_SA6:.*]] = insertelement <2 x i32> %[[ie_SA1]], i32 %[[ld_SA6]], i64 1
; CHECK:  store <2 x i32> %[[ie_SA6]], <2 x i32>* %0

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"$Globals" = type { [2 x <4 x i32>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?VectorArray@@3QBV?$vector@I$03@@B" = external constant [2 x <4 x i32>], align 4
@"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB" = internal global [8 x i32] zeroinitializer, align 4
@"$Globals" = external constant %"$Globals"

; Function Attrs: nounwind
define <2 x i32> @main(i32 %i) #0 {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22$Globals\22*, i32)"(i32 0, %"$Globals"* @"$Globals", i32 0)
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22$Globals\22)"(i32 14, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 13, i32 32 }, %"$Globals" undef)
  %3 = call %"$Globals"* @"dx.hl.subscript.cb.rn.%\22$Globals\22* (i32, %dx.types.Handle, i32)"(i32 6, %dx.types.Handle %2, i32 0)
  %4 = getelementptr inbounds %"$Globals", %"$Globals"* %3, i32 0, i32 0
  %i.addr = alloca i32, align 4, !dx.temp !13
  store i32 %i, i32* %i.addr, align 4, !tbaa !23
  %5 = load i32, i32* %0, !dbg !27 ; line:4 col:5
  %6 = and i32 %5, 1, !dbg !27 ; line:4 col:5
  %7 = icmp ne i32 %6, 0, !dbg !27 ; line:4 col:5
  br i1 %7, label %init.end, label %init, !dbg !27 ; line:4 col:5

init:                                             ; preds = %entry
  %8 = or i32 %5, 1, !dbg !27 ; line:4 col:5
  store i32 %8, i32* %0, !dbg !27 ; line:4 col:5
  %9 = bitcast [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB" to i8*, !dbg !31 ; line:4 col:49
  %10 = bitcast [2 x <4 x i32>]* %4 to i8*, !dbg !31 ; line:4 col:49
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %10, i64 32, i32 1, i1 false), !dbg !31 ; line:4 col:49
  br label %init.end, !dbg !27 ; line:4 col:5

init.end:                                         ; preds = %init, %entry
  %11 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 1), align 4, !dbg !32, !tbaa !23 ; line:5 col:18
  %12 = load i32, i32* getelementptr inbounds ([8 x i32], [8 x i32]* @"\01?ScalarArray@?1??main@@YA?AV?$vector@I$01@@H@Z@4QBIB", i32 0, i32 6), align 4, !dbg !33, !tbaa !23 ; line:5 col:34
  %13 = insertelement <2 x i32> undef, i32 %11, i64 0, !dbg !34 ; line:5 col:17
  %14 = insertelement <2 x i32> %13, i32 %12, i64 1, !dbg !34 ; line:5 col:17
  ret <2 x i32> %14, !dbg !35 ; line:5 col:5
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
!4 = !{i32 1, i32 7}
!5 = !{!"vs", i32 6, i32 0}
!6 = !{i32 0, %"$Globals" undef, !7}
!7 = !{i32 32, !8}
!8 = !{i32 6, !"VectorArray", i32 3, i32 0, i32 7, i32 5}
!9 = !{i32 1, <2 x i32> (i32)* @main, !10}
!10 = !{!11, !14}
!11 = !{i32 1, !12, !13}
!12 = !{i32 4, !"OUT", i32 7, i32 5}
!13 = !{}
!14 = !{i32 0, !15, !13}
!15 = !{i32 4, !"IN", i32 7, i32 4}
!16 = !{<2 x i32> (i32)* @main, !"main", null, !17, null}
!17 = !{null, null, !18, null}
!18 = !{!19}
!19 = !{i32 0, %"$Globals"* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 32, null}
!20 = !{<2 x i32> (i32)* @main, i32 1}
!21 = !{i32 -2147483584}
!22 = !{i32 -1}
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
!27 = !DILocation(line: 4, column: 5, scope: !28)
!28 = !DISubprogram(name: "main", scope: !29, file: !29, line: 3, type: !30, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, function: <2 x i32> (i32)* @main)
!29 = !DIFile(filename: "t:\5Carray-mapping.hlsl", directory: "")
!30 = !DISubroutineType(types: !13)
!31 = !DILocation(line: 4, column: 49, scope: !28)
!32 = !DILocation(line: 5, column: 18, scope: !28)
!33 = !DILocation(line: 5, column: 34, scope: !28)
!34 = !DILocation(line: 5, column: 17, scope: !28)
!35 = !DILocation(line: 5, column: 5, scope: !28)
