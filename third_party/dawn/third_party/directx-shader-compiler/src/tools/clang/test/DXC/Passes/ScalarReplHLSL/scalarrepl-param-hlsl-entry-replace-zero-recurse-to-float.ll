; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; The pass replaces a memcpy from a zero-initialized global that does not have an
; intervening store in the entry block.
; When tracing through geps and bitcasts of uses of that global, the algorithm might
; bottom out at replacing a load of a scalar float.  Verify this works.

; In the following code, %1 should be replaced by int 0
;    %1 = load float, float* %arrayidx,...
; It only has one use: being stored to one of the elements of @arr_var

; CHECK-LABEL: entry:
; CHECK:        [[DEST:%[a-z0-9\.]+]] = getelementptr inbounds [4 x float], [4 x float]* %zero_arr, i32 0, i32 0
; CHECK-NEXT:   store float 0.000000e+00, float* [[DEST]]

; Generated from compiling the following HLSL:
; static int arr_var[4] = (int[4])0;
; 
; [numthreads(1, 1, 1)]
; void main() {
;   int i32_var = 0;
;   int f32_var = arr_var[i32_var];
;   int zero_arr[4] = (int[4])0;
;   arr_var = zero_arr;
; }

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
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque

@arr_var = internal global [4 x float] zeroinitializer, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %i32_var = alloca i32, align 4
  %f32_var = alloca float, align 4
  %zero_arr = alloca [4 x float], align 4
  store i32 0, i32* %i32_var, align 4, !dbg !17, !tbaa !21 ; line:5 col:7
  %0 = load i32, i32* %i32_var, align 4, !dbg !25, !tbaa !21 ; line:6 col:27
  %arrayidx = getelementptr inbounds [4 x float], [4 x float]* @arr_var, i32 0, i32 %0, !dbg !26 ; line:6 col:19
  %1 = load float, float* %arrayidx, align 4, !dbg !26, !tbaa !27 ; line:6 col:19
  store float %1, float* %f32_var, align 4, !dbg !29, !tbaa !27 ; line:6 col:9
  %2 = getelementptr inbounds [4 x float], [4 x float]* %zero_arr, i32 0, i32 0, !dbg !30 ; line:7 col:33
  store float 0.000000e+00, float* %2, !dbg !30 ; line:7 col:33
  %3 = getelementptr inbounds [4 x float], [4 x float]* %zero_arr, i32 0, i32 1, !dbg !30 ; line:7 col:33
  store float 0.000000e+00, float* %3, !dbg !30 ; line:7 col:33
  %4 = getelementptr inbounds [4 x float], [4 x float]* %zero_arr, i32 0, i32 2, !dbg !30 ; line:7 col:33
  store float 0.000000e+00, float* %4, !dbg !30 ; line:7 col:33
  %5 = getelementptr inbounds [4 x float], [4 x float]* %zero_arr, i32 0, i32 3, !dbg !30 ; line:7 col:33
  store float 0.000000e+00, float* %5, !dbg !30 ; line:7 col:33
  %6 = bitcast [4 x float]* @arr_var to i8*, !dbg !31 ; line:8 col:13
  %7 = bitcast [4 x float]* %zero_arr to i8*, !dbg !31 ; line:8 col:13
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %6, i8* %7, i64 16, i32 1, i1 false), !dbg !31 ; line:8 col:13
  ret void, !dbg !32 ; line:9 col:1
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
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10}
!dx.fnprops = !{!14}
!dx.options = !{!15, !16}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4666 (759e9e1da-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 1, void ()* @main, !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{void ()* @main, !"main", null, !11, null}
!11 = !{null, null, !12, null}
!12 = !{!13}
!13 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!14 = !{void ()* @main, i32 5, i32 1, i32 1, i32 1}
!15 = !{i32 64}
!16 = !{i32 -1}
!17 = !DILocation(line: 5, column: 7, scope: !18)
!18 = !DISubprogram(name: "main", scope: !19, file: !19, line: 4, type: !20, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!19 = !DIFile(filename: "/mnt/c/Users/amaiorano/Downloads/356423093/356423093_reduced.hlsl", directory: "")
!20 = !DISubroutineType(types: !9)
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
!25 = !DILocation(line: 6, column: 27, scope: !18)
!26 = !DILocation(line: 6, column: 19, scope: !18)
!27 = !{!28, !28, i64 0}
!28 = !{!"float", !23, i64 0}
!29 = !DILocation(line: 6, column: 9, scope: !18)
!30 = !DILocation(line: 7, column: 33, scope: !18)
!31 = !DILocation(line: 8, column: 13, scope: !18)
!32 = !DILocation(line: 9, column: 1, scope: !18)
