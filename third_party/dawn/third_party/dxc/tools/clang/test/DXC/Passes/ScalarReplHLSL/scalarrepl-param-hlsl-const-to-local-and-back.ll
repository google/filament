; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; The first memcpy, from arr1 to arr1_copy.i, should be replaced by a series of 10 loads and stores,
; while the second memcpy, from arr1_copy.i back to arr1, should be removed:
;   %19 = bitcast [10 x i32]* %arr1_copy.i to i8*, !dbg !33 ; line:25 col:23
;   call void @llvm.memcpy.p0i8.p0i8.i64(i8* %19, i8* bitcast ([10 x i32]* @arr1 to i8*), i64 40, i32 1, i1 false) #0, !dbg !33 ; line:25 col:23
;   %20 = bitcast [10 x i32]* %arr1_copy.i to i8*, !dbg !34 ; line:26 col:10
;   call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([10 x i32]* @arr1 to i8*), i8* %20, i64 40, i32 1, i1 false) #0, !dbg !34 ; line:26 col:10
;   store i32 0, i32* %i.i.1.i, align 4, !dbg !35, !tbaa !12 ; line:7 col:7

; CHECK:  [[DEST0:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 0
; CHECK-NEXT:  [[SRC0:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 0)
; CHECK-NEXT:  store i32 [[SRC0:%[a-z0-9\.]+]], i32* [[DEST0:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST1:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 1
; CHECK-NEXT:  [[SRC1:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 1)
; CHECK-NEXT:  store i32 [[SRC1:%[a-z0-9\.]+]], i32* [[DEST1:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST2:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 2
; CHECK-NEXT:  [[SRC2:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 2)
; CHECK-NEXT:  store i32 [[SRC2:%[a-z0-9\.]+]], i32* [[DEST2:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST3:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 3
; CHECK-NEXT:  [[SRC3:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 3)
; CHECK-NEXT:  store i32 [[SRC3:%[a-z0-9\.]+]], i32* [[DEST3:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST4:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 4
; CHECK-NEXT:  [[SRC4:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 4)
; CHECK-NEXT:  store i32 [[SRC4:%[a-z0-9\.]+]], i32* [[DEST4:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST5:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 5
; CHECK-NEXT:  [[SRC5:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 5)
; CHECK-NEXT:  store i32 [[SRC5:%[a-z0-9\.]+]], i32* [[DEST5:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST6:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 6
; CHECK-NEXT:  [[SRC6:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 6)
; CHECK-NEXT:  store i32 [[SRC6:%[a-z0-9\.]+]], i32* [[DEST6:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST7:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 7
; CHECK-NEXT:  [[SRC7:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 7)
; CHECK-NEXT:  store i32 [[SRC7:%[a-z0-9\.]+]], i32* [[DEST7:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST8:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 8
; CHECK-NEXT:  [[SRC8:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 8)
; CHECK-NEXT:  store i32 [[SRC8:%[a-z0-9\.]+]], i32* [[DEST8:%[a-z0-9\.]+]]
; CHECK-NEXT:  [[DEST9:%[a-z0-9\.]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %arr1_copy.i, i32 0, i32 9
; CHECK-NEXT:  [[SRC9:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 9)
; CHECK-NEXT:  store i32 [[SRC9:%[a-z0-9\.]+]], i32* [[DEST9:%[a-z0-9\.]+]]

;
; Buffer Definitions:
;
; cbuffer $Globals
; {
;
;   [0 x i8] (type annotation not present)
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0   cb4294967295     1
; buff                              texture    byte         r/o      T0             t0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.ByteAddressBuffer = type { i32 }
%ConstantBuffer = type opaque
%struct.tint_symbol = type { <4 x float> }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?buff@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@arr1 = internal global [10 x i32] zeroinitializer, align 4
@arr2 = internal global [10 x i32] zeroinitializer, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main(%struct.tint_symbol* noalias sret %agg.result) #0 {
  %1 = alloca float
  store float 0.000000e+00, float* %1
  %i.i.1.i = alloca i32, align 4
  %i.i.i = alloca i32, align 4
  %cond.i = alloca i32, align 4
  %arr1_copy.i = alloca [10 x i32], align 4
  %inner_result = alloca float, align 4
  %wrapper_result = alloca %struct.tint_symbol, align 4
  store i32 0, i32* %i.i.i, align 4, !dbg !23, !tbaa !31 ; line:7 col:7
  %2 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?buff@@3UByteAddressBuffer@@A", !dbg !35 ; line:8 col:7
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %2) #0, !dbg !35 ; line:8 col:7
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef) #0, !dbg !35 ; line:8 col:7
  %5 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %4, i32 0) #0, !dbg !35 ; line:8 col:7
  %6 = icmp ne i32 %5, 0, !dbg !35 ; line:8 col:7
  br i1 %6, label %"\01?foo@@YAXXZ.exit.i", label %7, !dbg !35 ; line:8 col:7

; <label>:7                                       ; preds = %0
  %8 = load i32, i32* %i.i.i, align 4, !dbg !36, !tbaa !31 ; line:11 col:18
  %9 = getelementptr inbounds [10 x i32], [10 x i32]* @arr1, i32 0, i32 %8, !dbg !37 ; line:11 col:13
  %10 = load i32, i32* %9, align 4, !dbg !37, !tbaa !31 ; line:11 col:13
  %11 = load i32, i32* %i.i.i, align 4, !dbg !38, !tbaa !31 ; line:11 col:8
  %12 = getelementptr inbounds [10 x i32], [10 x i32]* @arr2, i32 0, i32 %11, !dbg !39 ; line:11 col:3
  store i32 %10, i32* %12, align 4, !dbg !40, !tbaa !31 ; line:11 col:11
  %13 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 0), align 4, !dbg !41, !tbaa !31 ; line:12 col:18
  %14 = sitofp i32 %13 to float, !dbg !41 ; line:12 col:18
  store float %14, float* %1, align 4, !dbg !42, !tbaa !43 ; line:12 col:10
  br label %"\01?foo@@YAXXZ.exit.i", !dbg !45 ; line:13 col:1

"\01?foo@@YAXXZ.exit.i":                          ; preds = %7, %0
  store i32 0, i32* %cond.i, align 4, !dbg !46, !tbaa !47 ; line:21 col:8
  br label %15, !dbg !49 ; line:22 col:3

; <label>:15                                      ; preds = %15, %"\01?foo@@YAXXZ.exit.i"
  %16 = load i32, i32* %cond.i, align 4, !dbg !50, !tbaa !47, !range !51 ; line:23 col:9
  %17 = icmp ne i32 %16, 0, !dbg !50 ; line:23 col:9
  br i1 %17, label %18, label %15, !dbg !50 ; line:23 col:9

; <label>:18                                      ; preds = %15
  %19 = bitcast [10 x i32]* %arr1_copy.i to i8*, !dbg !52 ; line:25 col:23
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %19, i8* bitcast ([10 x i32]* @arr1 to i8*), i64 40, i32 1, i1 false) #0, !dbg !52 ; line:25 col:23
  %20 = bitcast [10 x i32]* %arr1_copy.i to i8*, !dbg !53 ; line:26 col:10
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* bitcast ([10 x i32]* @arr1 to i8*), i8* %20, i64 40, i32 1, i1 false) #0, !dbg !53 ; line:26 col:10
  store i32 0, i32* %i.i.1.i, align 4, !dbg !54, !tbaa !31 ; line:7 col:7
  %21 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?buff@@3UByteAddressBuffer@@A", !dbg !56 ; line:8 col:7
  %22 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %21) #0, !dbg !56 ; line:8 col:7
  %23 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %22, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef) #0, !dbg !56 ; line:8 col:7
  %24 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %23, i32 0) #0, !dbg !56 ; line:8 col:7
  %25 = icmp ne i32 %24, 0, !dbg !56 ; line:8 col:7
  br i1 %25, label %"\01?main_inner@@YAMXZ.exit", label %26, !dbg !56 ; line:8 col:7

; <label>:26                                      ; preds = %18
  %27 = load i32, i32* %i.i.1.i, align 4, !dbg !57, !tbaa !31 ; line:11 col:18
  %28 = getelementptr inbounds [10 x i32], [10 x i32]* @arr1, i32 0, i32 %27, !dbg !58 ; line:11 col:13
  %29 = load i32, i32* %28, align 4, !dbg !58, !tbaa !31 ; line:11 col:13
  %30 = load i32, i32* %i.i.1.i, align 4, !dbg !59, !tbaa !31 ; line:11 col:8
  %31 = getelementptr inbounds [10 x i32], [10 x i32]* @arr2, i32 0, i32 %30, !dbg !60 ; line:11 col:3
  store i32 %29, i32* %31, align 4, !dbg !61, !tbaa !31 ; line:11 col:11
  %32 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr1, i32 0, i32 0), align 4, !dbg !62, !tbaa !31 ; line:12 col:18
  %33 = sitofp i32 %32 to float, !dbg !62 ; line:12 col:18
  store float %33, float* %1, align 4, !dbg !63, !tbaa !43 ; line:12 col:10
  br label %"\01?main_inner@@YAMXZ.exit", !dbg !64 ; line:13 col:1

"\01?main_inner@@YAMXZ.exit":                     ; preds = %18, %26
  %34 = load float, float* %1, align 4, !dbg !65, !tbaa !43 ; line:28 col:10
  store float %34, float* %inner_result, align 4, !dbg !66, !tbaa !43 ; line:32 col:9
  %35 = getelementptr inbounds %struct.tint_symbol, %struct.tint_symbol* %wrapper_result, i32 0, i32 0, !dbg !67 ; line:33 col:45
  store <4 x float> zeroinitializer, <4 x float>* %35, !dbg !67 ; line:33 col:45
  %36 = load float, float* %inner_result, align 4, !dbg !68, !tbaa !43 ; line:34 col:28
  %37 = getelementptr inbounds %struct.tint_symbol, %struct.tint_symbol* %wrapper_result, i32 0, i32 0, !dbg !69 ; line:34 col:18
  %38 = load <4 x float>, <4 x float>* %37, align 4, !dbg !70 ; line:34 col:26
  %39 = getelementptr <4 x float>, <4 x float>* %37, i32 0, i32 0, !dbg !70 ; line:34 col:26
  store float %36, float* %39, !dbg !70 ; line:34 col:26
  %40 = bitcast %struct.tint_symbol* %agg.result to i8*, !dbg !71 ; line:35 col:10
  %41 = bitcast %struct.tint_symbol* %wrapper_result to i8*, !dbg !71 ; line:35 col:10
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %40, i8* %41, i64 16, i32 1, i1 false), !dbg !71 ; line:35 col:10
  ret void, !dbg !72 ; line:35 col:3
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind readonly
declare i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32, %struct.ByteAddressBuffer) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

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
!2 = !{!"dxc(private) 1.8.0.4547 (14ec4b49d)"}
!3 = !{i32 1, i32 2}
!4 = !{i32 1, i32 8}
!5 = !{!"ps", i32 6, i32 2}
!6 = !{i32 0, %struct.tint_symbol undef, !7}
!7 = !{i32 16, !8}
!8 = !{i32 6, !"value", i32 3, i32 0, i32 4, !"SV_Target0", i32 7, i32 9}
!9 = !{i32 1, void (%struct.tint_symbol*)* @main, !10}
!10 = !{!11, !13}
!11 = !{i32 0, !12, !12}
!12 = !{}
!13 = !{i32 1, !12, !12}
!14 = !{void (%struct.tint_symbol*)* @main, !"main", null, !15, null}
!15 = !{!16, null, !18, null}
!16 = !{!17}
!17 = !{i32 0, %struct.ByteAddressBuffer* @"\01?buff@@3UByteAddressBuffer@@A", !"buff", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!18 = !{!19}
!19 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!20 = !{void (%struct.tint_symbol*)* @main, i32 0, i1 false}
!21 = !{i32 144}
!22 = !{i32 -1}
!23 = !DILocation(line: 7, column: 7, scope: !24, inlinedAt: !27)
!24 = !DISubprogram(name: "foo", scope: !25, file: !25, line: 6, type: !26, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false)
!25 = !DIFile(filename: "333414294_simplifed.hlsl", directory: "")
!26 = !DISubroutineType(types: !12)
!27 = distinct !DILocation(line: 20, column: 3, scope: !28, inlinedAt: !29)
!28 = !DISubprogram(name: "main_inner", scope: !25, file: !25, line: 19, type: !26, isLocal: false, isDefinition: true, scopeLine: 19, flags: DIFlagPrototyped, isOptimized: false)
!29 = distinct !DILocation(line: 32, column: 24, scope: !30)
!30 = !DISubprogram(name: "main", scope: !25, file: !25, line: 31, type: !26, isLocal: false, isDefinition: true, scopeLine: 31, flags: DIFlagPrototyped, isOptimized: false, function: void (%struct.tint_symbol*)* @main)
!31 = !{!32, !32, i64 0}
!32 = !{!"int", !33, i64 0}
!33 = !{!"omnipotent char", !34, i64 0}
!34 = !{!"Simple C/C++ TBAA"}
!35 = !DILocation(line: 8, column: 7, scope: !24, inlinedAt: !27)
!36 = !DILocation(line: 11, column: 18, scope: !24, inlinedAt: !27)
!37 = !DILocation(line: 11, column: 13, scope: !24, inlinedAt: !27)
!38 = !DILocation(line: 11, column: 8, scope: !24, inlinedAt: !27)
!39 = !DILocation(line: 11, column: 3, scope: !24, inlinedAt: !27)
!40 = !DILocation(line: 11, column: 11, scope: !24, inlinedAt: !27)
!41 = !DILocation(line: 12, column: 18, scope: !24, inlinedAt: !27)
!42 = !DILocation(line: 12, column: 10, scope: !24, inlinedAt: !27)
!43 = !{!44, !44, i64 0}
!44 = !{!"float", !33, i64 0}
!45 = !DILocation(line: 13, column: 1, scope: !24, inlinedAt: !27)
!46 = !DILocation(line: 21, column: 8, scope: !28, inlinedAt: !29)
!47 = !{!48, !48, i64 0}
!48 = !{!"bool", !33, i64 0}
!49 = !DILocation(line: 22, column: 3, scope: !28, inlinedAt: !29)
!50 = !DILocation(line: 23, column: 9, scope: !28, inlinedAt: !29)
!51 = !{i32 0, i32 2}
!52 = !DILocation(line: 25, column: 23, scope: !28, inlinedAt: !29)
!53 = !DILocation(line: 26, column: 10, scope: !28, inlinedAt: !29)
!54 = !DILocation(line: 7, column: 7, scope: !24, inlinedAt: !55)
!55 = distinct !DILocation(line: 27, column: 3, scope: !28, inlinedAt: !29)
!56 = !DILocation(line: 8, column: 7, scope: !24, inlinedAt: !55)
!57 = !DILocation(line: 11, column: 18, scope: !24, inlinedAt: !55)
!58 = !DILocation(line: 11, column: 13, scope: !24, inlinedAt: !55)
!59 = !DILocation(line: 11, column: 8, scope: !24, inlinedAt: !55)
!60 = !DILocation(line: 11, column: 3, scope: !24, inlinedAt: !55)
!61 = !DILocation(line: 11, column: 11, scope: !24, inlinedAt: !55)
!62 = !DILocation(line: 12, column: 18, scope: !24, inlinedAt: !55)
!63 = !DILocation(line: 12, column: 10, scope: !24, inlinedAt: !55)
!64 = !DILocation(line: 13, column: 1, scope: !24, inlinedAt: !55)
!65 = !DILocation(line: 28, column: 10, scope: !28, inlinedAt: !29)
!66 = !DILocation(line: 32, column: 9, scope: !30)
!67 = !DILocation(line: 33, column: 45, scope: !30)
!68 = !DILocation(line: 34, column: 28, scope: !30)
!69 = !DILocation(line: 34, column: 18, scope: !30)
!70 = !DILocation(line: 34, column: 26, scope: !30)
!71 = !DILocation(line: 35, column: 10, scope: !30)
!72 = !DILocation(line: 35, column: 3, scope: !30)
