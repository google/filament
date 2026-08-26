; RUN: %opt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; This test ensures that NodeIO types don't get broken down to i32s by SROA.
; SROA woulda have reduced the %inputData variable to an i32, but this pass should now keep the type the same.
; Specifically, before the change associated with this new test, the CHECKs below would fail because
; SROA would replace struct.DispatchNodeInputRecord<loadStressRecord> with i32.
; 
; CHECK: alloca %"struct.DispatchNodeInputRecord<loadStressRecord>"
; CHECK: load %"struct.DispatchNodeInputRecord<loadStressRecord>", %"struct.DispatchNodeInputRecord<loadStressRecord>"*

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque
%"struct.DispatchNodeInputRecord<loadStressRecord>" = type { i32 }
%"struct.NodeOutput<loadStressRecord>" = type { i32 }
%dx.types.NodeHandle = type { i8* }
%dx.types.NodeInfo = type { i32, i32 }
%dx.types.NodeRecordHandle = type { i8* }
%dx.types.NodeRecordInfo = type { i32, i32 }
%"struct.GroupNodeOutputRecords<loadStressRecord>" = type { i32 }
%struct.loadStressRecord = type { [29 x i32], <3 x i32> }

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind
define void @loadStress_16(%"struct.DispatchNodeInputRecord<loadStressRecord>"* %inputData, %"struct.NodeOutput<loadStressRecord>"* %loadStressChild) #0 {
entry:
  %val.i = alloca i32, align 4
  %0 = call %dx.types.NodeHandle @"dx.hl.createnodeoutputhandle..%dx.types.NodeHandle (i32, i32)"(i32 11, i32 0)
  %1 = call %dx.types.NodeHandle @"dx.hl.annotatenodehandle..%dx.types.NodeHandle (i32, %dx.types.NodeHandle, %dx.types.NodeInfo)"(i32 16, %dx.types.NodeHandle %0, %dx.types.NodeInfo { i32 6, i32 128 })
  %2 = call %"struct.NodeOutput<loadStressRecord>" @"dx.hl.cast..%\22struct.NodeOutput<loadStressRecord>\22 (i32, %dx.types.NodeHandle)"(i32 9, %dx.types.NodeHandle %1)
  store %"struct.NodeOutput<loadStressRecord>" %2, %"struct.NodeOutput<loadStressRecord>"* %loadStressChild
  %3 = call %dx.types.NodeRecordHandle @"dx.hl.createnodeinputrecordhandle..%dx.types.NodeRecordHandle (i32, i32)"(i32 13, i32 0)
  %4 = call %dx.types.NodeRecordHandle @"dx.hl.annotatenoderecordhandle..%dx.types.NodeRecordHandle (i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo)"(i32 17, %dx.types.NodeRecordHandle %3, %dx.types.NodeRecordInfo { i32 97, i32 128 })
  %5 = call %"struct.DispatchNodeInputRecord<loadStressRecord>" @"dx.hl.cast..%\22struct.DispatchNodeInputRecord<loadStressRecord>\22 (i32, %dx.types.NodeRecordHandle)"(i32 11, %dx.types.NodeRecordHandle %4)
  store %"struct.DispatchNodeInputRecord<loadStressRecord>" %5, %"struct.DispatchNodeInputRecord<loadStressRecord>"* %inputData
  %6 = alloca %"struct.DispatchNodeInputRecord<loadStressRecord>"
  %agg.tmp = alloca %"struct.GroupNodeOutputRecords<loadStressRecord>", align 4
  %7 = bitcast %"struct.DispatchNodeInputRecord<loadStressRecord>"* %inputData to i8*, !dbg !23 ; line:28 col:5
  call void @llvm.lifetime.start(i64 4, i8* %7) #0, !dbg !23 ; line:28 col:5
  %8 = load %"struct.NodeOutput<loadStressRecord>", %"struct.NodeOutput<loadStressRecord>"* %loadStressChild, !dbg !27 ; line:28 col:33
  %9 = call %dx.types.NodeHandle @"dx.hl.cast..%dx.types.NodeHandle (i32, %\22struct.NodeOutput<loadStressRecord>\22)"(i32 10, %"struct.NodeOutput<loadStressRecord>" %8), !dbg !27 ; line:28 col:33
  %10 = call %dx.types.NodeRecordHandle @"dx.hl.op..%dx.types.NodeRecordHandle (i32, %dx.types.NodeHandle, i32)"(i32 335, %dx.types.NodeHandle %9, i32 1), !dbg !27 ; line:28 col:33
  %11 = call %dx.types.NodeRecordHandle @"dx.hl.annotatenoderecordhandle..%dx.types.NodeRecordHandle (i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo)"(i32 17, %dx.types.NodeRecordHandle %10, %dx.types.NodeRecordInfo { i32 70, i32 128 }), !dbg !27 ; line:28 col:33
  %12 = call %"struct.GroupNodeOutputRecords<loadStressRecord>" @"dx.hl.cast..%\22struct.GroupNodeOutputRecords<loadStressRecord>\22 (i32, %dx.types.NodeRecordHandle)"(i32 11, %dx.types.NodeRecordHandle %11), !dbg !27 ; line:28 col:33
  store %"struct.GroupNodeOutputRecords<loadStressRecord>" %12, %"struct.GroupNodeOutputRecords<loadStressRecord>"* %agg.tmp, !dbg !27 ; line:28 col:33
  %13 = bitcast i32* %val.i to i8*, !dbg !28 ; line:17 col:5
  call void @llvm.lifetime.start(i64 4, i8* %13) #0, !dbg !28, !noalias !31 ; line:17 col:5
  %14 = load %"struct.DispatchNodeInputRecord<loadStressRecord>", %"struct.DispatchNodeInputRecord<loadStressRecord>"* %inputData, !dbg !34, !alias.scope !31 ; line:17 col:17
  %15 = call %dx.types.NodeRecordHandle @"dx.hl.cast..%dx.types.NodeRecordHandle (i32, %\22struct.DispatchNodeInputRecord<loadStressRecord>\22)"(i32 12, %"struct.DispatchNodeInputRecord<loadStressRecord>" %14) #0, !dbg !34 ; line:17 col:17
  %16 = call %struct.loadStressRecord* @"dx.hl.subscript.[].rn.%struct.loadStressRecord* (i32, %dx.types.NodeRecordHandle)"(i32 0, %dx.types.NodeRecordHandle %15) #0, !dbg !34 ; line:17 col:17
  %data.i = getelementptr inbounds %struct.loadStressRecord, %struct.loadStressRecord* %16, i32 0, i32 0, !dbg !35 ; line:17 col:33
  %arrayidx.i = getelementptr inbounds [29 x i32], [29 x i32]* %data.i, i32 0, i32 0, !dbg !34 ; line:17 col:17
  %17 = load i32, i32* %arrayidx.i, align 4, !dbg !34, !tbaa !36, !noalias !31 ; line:17 col:17
  store i32 %17, i32* %val.i, align 4, !dbg !40, !tbaa !36, !noalias !31 ; line:17 col:10
  %18 = load i32, i32* %val.i, align 4, !dbg !41, !tbaa !36, !noalias !31 ; line:19 col:28
  %add.i = add i32 %18, 61, !dbg !42 ; line:19 col:32
  %19 = load %"struct.GroupNodeOutputRecords<loadStressRecord>", %"struct.GroupNodeOutputRecords<loadStressRecord>"* %agg.tmp, !dbg !43, !noalias !31 ; line:19 col:5
  %20 = call %dx.types.NodeRecordHandle @"dx.hl.cast..%dx.types.NodeRecordHandle (i32, %\22struct.GroupNodeOutputRecords<loadStressRecord>\22)"(i32 12, %"struct.GroupNodeOutputRecords<loadStressRecord>" %19) #0, !dbg !43 ; line:19 col:5
  %21 = call %struct.loadStressRecord* @"dx.hl.subscript.[].rn.%struct.loadStressRecord* (i32, %dx.types.NodeRecordHandle, i32)"(i32 0, %dx.types.NodeRecordHandle %20, i32 0) #0, !dbg !43 ; line:19 col:5
  %data2.i = getelementptr inbounds %struct.loadStressRecord, %struct.loadStressRecord* %21, i32 0, i32 0, !dbg !44 ; line:19 col:18
  %arrayidx3.i = getelementptr inbounds [29 x i32], [29 x i32]* %data2.i, i32 0, i32 0, !dbg !43 ; line:19 col:5
  store i32 %add.i, i32* %arrayidx3.i, align 4, !dbg !45, !tbaa !36, !noalias !31 ; line:19 col:26
  %22 = bitcast i32* %val.i to i8*, !dbg !46 ; line:20 col:1
  call void @llvm.lifetime.end(i64 4, i8* %22) #0, !dbg !46, !noalias !31 ; line:20 col:1
  %23 = bitcast %"struct.DispatchNodeInputRecord<loadStressRecord>"* %6 to i8*, !dbg !23 ; line:28 col:5
  call void @llvm.lifetime.end(i64 4, i8* %23) #0, !dbg !23 ; line:28 col:5
  ret void, !dbg !47 ; line:29 col:1
}

; Function Attrs: nounwind readnone
declare %struct.loadStressRecord* @"dx.hl.subscript.[].rn.%struct.loadStressRecord* (i32, %dx.types.NodeRecordHandle)"(i32, %dx.types.NodeRecordHandle) #1

; Function Attrs: nounwind readnone
declare %dx.types.NodeRecordHandle @"dx.hl.cast..%dx.types.NodeRecordHandle (i32, %\22struct.DispatchNodeInputRecord<loadStressRecord>\22)"(i32, %"struct.DispatchNodeInputRecord<loadStressRecord>") #1

; Function Attrs: nounwind readnone
declare %struct.loadStressRecord* @"dx.hl.subscript.[].rn.%struct.loadStressRecord* (i32, %dx.types.NodeRecordHandle, i32)"(i32, %dx.types.NodeRecordHandle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.NodeRecordHandle @"dx.hl.cast..%dx.types.NodeRecordHandle (i32, %\22struct.GroupNodeOutputRecords<loadStressRecord>\22)"(i32, %"struct.GroupNodeOutputRecords<loadStressRecord>") #1

; Function Attrs: nounwind
declare %dx.types.NodeRecordHandle @"dx.hl.op..%dx.types.NodeRecordHandle (i32, %dx.types.NodeHandle, i32)"(i32, %dx.types.NodeHandle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.NodeHandle @"dx.hl.cast..%dx.types.NodeHandle (i32, %\22struct.NodeOutput<loadStressRecord>\22)"(i32, %"struct.NodeOutput<loadStressRecord>") #1

; Function Attrs: nounwind
declare %dx.types.NodeRecordHandle @"dx.hl.annotatenoderecordhandle..%dx.types.NodeRecordHandle (i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo)"(i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo) #0

; Function Attrs: nounwind readnone
declare %"struct.GroupNodeOutputRecords<loadStressRecord>" @"dx.hl.cast..%\22struct.GroupNodeOutputRecords<loadStressRecord>\22 (i32, %dx.types.NodeRecordHandle)"(i32, %dx.types.NodeRecordHandle) #1

; Function Attrs: nounwind
declare %dx.types.NodeRecordHandle @"dx.hl.createnodeinputrecordhandle..%dx.types.NodeRecordHandle (i32, i32)"(i32, i32) #0

; Function Attrs: nounwind readnone
declare %"struct.DispatchNodeInputRecord<loadStressRecord>" @"dx.hl.cast..%\22struct.DispatchNodeInputRecord<loadStressRecord>\22 (i32, %dx.types.NodeRecordHandle)"(i32, %dx.types.NodeRecordHandle) #1

; Function Attrs: nounwind
declare %dx.types.NodeHandle @"dx.hl.createnodeoutputhandle..%dx.types.NodeHandle (i32, i32)"(i32, i32) #0

; Function Attrs: nounwind
declare %dx.types.NodeHandle @"dx.hl.annotatenodehandle..%dx.types.NodeHandle (i32, %dx.types.NodeHandle, %dx.types.NodeInfo)"(i32, %dx.types.NodeHandle, %dx.types.NodeInfo) #0

; Function Attrs: nounwind readnone
declare %"struct.NodeOutput<loadStressRecord>" @"dx.hl.cast..%\22struct.NodeOutput<loadStressRecord>\22 (i32, %dx.types.NodeHandle)"(i32, %dx.types.NodeHandle) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !11}
!dx.entryPoints = !{!16}
!dx.fnprops = !{!20}
!dx.options = !{!21, !22}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.7.0.4181 (recursive_functions_in_shader, 387025815-dirty)"}
!3 = !{i32 1, i32 8}
!4 = !{!"lib", i32 6, i32 8}
!5 = !{i32 0, %"struct.DispatchNodeInputRecord<loadStressRecord>" undef, !6, %"struct.GroupNodeOutputRecords<loadStressRecord>" undef, !6, %"struct.NodeOutput<loadStressRecord>" undef, !6}
!6 = !{i32 4, !7, !8}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!8 = !{i32 0, !9}
!9 = !{!10}
!10 = !{i32 0, %struct.loadStressRecord undef}
!11 = !{i32 1, void (%"struct.DispatchNodeInputRecord<loadStressRecord>"*, %"struct.NodeOutput<loadStressRecord>"*)* @loadStress_16, !12}
!12 = !{!13, !15, !15}
!13 = !{i32 1, !14, !14}
!14 = !{}
!15 = !{i32 14, !14, !14}
!16 = !{null, !"", null, !17, null}
!17 = !{null, null, !18, null}
!18 = !{!19}
!19 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!20 = !{void (%"struct.DispatchNodeInputRecord<loadStressRecord>"*, %"struct.NodeOutput<loadStressRecord>"*)* @loadStress_16, i32 15, i32 16, i32 1, i32 1, i32 1, i1 false, !"loadStress_16", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 3, i32 1, i32 1, i32 0}
!21 = !{i32 144}
!22 = !{i32 -1}
!23 = !DILocation(line: 28, column: 5, scope: !24)
!24 = !DISubprogram(name: "loadStress_16", scope: !25, file: !25, line: 25, type: !26, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagPrototyped, isOptimized: false, function: void (%"struct.DispatchNodeInputRecord<loadStressRecord>"*, %"struct.NodeOutput<loadStressRecord>"*)* @loadStress_16)
!25 = !DIFile(filename: "D:\5CDXC\5Ctools\5Cclang\5Ctest\5CHLSLFileCheck\5Chlsl\5Cworkgraph\5Ccalled_function_arg_record_object.hlsl", directory: "")
!26 = !DISubroutineType(types: !14)
!27 = !DILocation(line: 28, column: 33, scope: !24)
!28 = !DILocation(line: 17, column: 5, scope: !29, inlinedAt: !30)
!29 = !DISubprogram(name: "loadStressWorker", scope: !25, file: !25, line: 12, type: !26, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false)
!30 = distinct !DILocation(line: 28, column: 5, scope: !24)
!31 = !{!32}
!32 = distinct !{!32, !33, !"\01?loadStressWorker@@YAXU?$DispatchNodeInputRecord@UloadStressRecord@@@@U?$GroupNodeOutputRecords@UloadStressRecord@@@@@Z: %inputData"}
!33 = distinct !{!33, !"\01?loadStressWorker@@YAXU?$DispatchNodeInputRecord@UloadStressRecord@@@@U?$GroupNodeOutputRecords@UloadStressRecord@@@@@Z"}
!34 = !DILocation(line: 17, column: 17, scope: !29, inlinedAt: !30)
!35 = !DILocation(line: 17, column: 33, scope: !29, inlinedAt: !30)
!36 = !{!37, !37, i64 0}
!37 = !{!"int", !38, i64 0}
!38 = !{!"omnipotent char", !39, i64 0}
!39 = !{!"Simple C/C++ TBAA"}
!40 = !DILocation(line: 17, column: 10, scope: !29, inlinedAt: !30)
!41 = !DILocation(line: 19, column: 28, scope: !29, inlinedAt: !30)
!42 = !DILocation(line: 19, column: 32, scope: !29, inlinedAt: !30)
!43 = !DILocation(line: 19, column: 5, scope: !29, inlinedAt: !30)
!44 = !DILocation(line: 19, column: 18, scope: !29, inlinedAt: !30)
!45 = !DILocation(line: 19, column: 26, scope: !29, inlinedAt: !30)
!46 = !DILocation(line: 20, column: 1, scope: !29, inlinedAt: !30)
!47 = !DILocation(line: 29, column: 1, scope: !24)

; The test was generated using the pass test generation script:
; python3 ExtractIRForPassTest.py -p scalarrepl-param-hlsl -o outtest_without_nodeiotypecheck.ll %HLSL_SRC_DIR%\tools\clang\test\HLSLFileCheck\hlsl\workgraph\called_function_arg_record_object.hlsl -- -T lib_6_8
