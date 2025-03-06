; RUN: %dxopt %s -hlsl-passes-resume -scalarrepl-param-hlsl -S | FileCheck %s

; Produced from the following HLSL:
;   static int4 g[4] = (int4[4])0;
;   static int4 h[4] = (int4[4])0;
;   
;   [numthreads(1, 1, 1)]
;   void main() {
;     int a = 0;
;     int b = h[0][a];
;     h = g;
;   }
;
; This was crashing in scalarrepl-param-hlsl because it was attempting to flatten
; global variable 'h' even though it is dynamically indexed. This was not detected
; because the resulting IR was a dynamically indexed GEP of a constant-indexed GEP,
; and the code was only checking the immediate users of 'h':
;
;  %1 = getelementptr <4 x i32>, <4 x i32>* getelementptr inbounds ([4 x <4 x i32>], [4 x <4 x i32>]* @h, i32 0, i32 0), i32 0, i32 %0, !dbg !26 ; line:7 col:11
;
; Verify that it does not get flattened
; CHECK: %1 = getelementptr <4 x i32>, <4 x i32>* getelementptr inbounds ([4 x <4 x i32>], [4 x <4 x i32>]* @g, i32 0, i32 0), i32 0, i32 %0

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

@h = internal global [4 x <4 x i32>] zeroinitializer, align 4
@g = internal global [4 x <4 x i32>] zeroinitializer, align 4
@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 0, i32* %a, align 4, !dbg !17, !tbaa !21 ; line:6 col:7
  %0 = load i32, i32* %a, align 4, !dbg !25, !tbaa !21 ; line:7 col:16
  %1 = getelementptr <4 x i32>, <4 x i32>* getelementptr inbounds ([4 x <4 x i32>], [4 x <4 x i32>]* @h, i32 0, i32 0), i32 0, i32 %0, !dbg !26 ; line:7 col:11
  %2 = load i32, i32* %1, !dbg !26, !tbaa !21 ; line:7 col:11
  store i32 %2, i32* %b, align 4, !dbg !27, !tbaa !21 ; line:7 col:7
  %3 = bitcast [4 x <4 x i32>]* @h to i8*, !dbg !28 ; line:8 col:7
  %4 = bitcast [4 x <4 x i32>]* @g to i8*, !dbg !28 ; line:8 col:7
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 64, i32 1, i1 false), !dbg !28 ; line:8 col:7
  ret void, !dbg !29 ; line:9 col:1
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
!2 = !{!"dxc(private) 1.8.0.4514 (d9bd2a706-dirty)"}
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
!17 = !DILocation(line: 6, column: 7, scope: !18)
!18 = !DISubprogram(name: "main", scope: !19, file: !19, line: 5, type: !20, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!19 = !DIFile(filename: "/mnt/c/Users/amaiorano/Downloads/342428008/standalone_reduced.hlsl", directory: "")
!20 = !DISubroutineType(types: !9)
!21 = !{!22, !22, i64 0}
!22 = !{!"int", !23, i64 0}
!23 = !{!"omnipotent char", !24, i64 0}
!24 = !{!"Simple C/C++ TBAA"}
!25 = !DILocation(line: 7, column: 16, scope: !18)
!26 = !DILocation(line: 7, column: 11, scope: !18)
!27 = !DILocation(line: 7, column: 7, scope: !18)
!28 = !DILocation(line: 8, column: 7, scope: !18)
!29 = !DILocation(line: 9, column: 1, scope: !18)
