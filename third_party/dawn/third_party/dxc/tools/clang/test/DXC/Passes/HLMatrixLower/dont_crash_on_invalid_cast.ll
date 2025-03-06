; RUN: not %dxopt %s -hlsl-passes-resume -hlmatrixlower -S | FileCheck %s


; The HL matrix lowering pass can sometimes throw an exception
; due to an invalid LLVM-level cast<Ty> call.  Make sure that
; propagates out to a user-level error.

; Note: There is still a bug in the compiler here.  Not all matrix
; lowerings are covered by the pass.

; TODO: Fix the underlying bug https://github.com/microsoft/DirectXShaderCompiler/issues/6723
; Once that is fixed, this test changes or should be deleted.


; CHECK: Operation failed - error code

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
%class.matrix.float.2.4 = type { [2 x <4 x float>] }
%struct.e = type { [1 x %struct.d], [1 x %struct.d] }
%struct.d = type { %struct.a }
%struct.a = type { float, %class.matrix.float.2.4 }

@"$Globals" = external constant %ConstantBuffer
@g.0.0.0 = internal global [1 x float] undef, align 4
@g.0.0.1 = internal global [1 x %class.matrix.float.2.4] undef, align 4
@g.1.0.0 = internal global [1 x float] undef, align 4
@g.1.0.1 = internal global [1 x %class.matrix.float.2.4] undef, align 4

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %h.0.1 = alloca %class.matrix.float.2.4, !dbg !26 ; line:13 col:17
  store float 0.000000e+00, float* getelementptr inbounds ([1 x float], [1 x float]* @g.0.0.0, i32 0, i32 0), !dbg !26 ; line:13 col:17
  %0 = call %class.matrix.float.2.4 @"dx.hl.init.rn.%class.matrix.float.2.4 (i32, <8 x float>)"(i32 0, <8 x float> zeroinitializer) #0, !dbg !26 ; line:13 col:17
  %1 = call %class.matrix.float.2.4 @"dx.hl.cast.rowMatToColMat.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4)"(i32 7, %class.matrix.float.2.4 %0) #0, !dbg !26 ; line:13 col:17
  %2 = call %class.matrix.float.2.4 @"dx.hl.matldst.colStore.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4*, %class.matrix.float.2.4)"(i32 1, %class.matrix.float.2.4* getelementptr inbounds ([1 x %class.matrix.float.2.4], [1 x %class.matrix.float.2.4]* @g.0.0.1, i32 0, i32 0), %class.matrix.float.2.4 %1) #0, !dbg !26 ; line:13 col:17
  store float 0.000000e+00, float* getelementptr inbounds ([1 x float], [1 x float]* @g.1.0.0, i32 0, i32 0), !dbg !26 ; line:13 col:17
  %3 = call %class.matrix.float.2.4 @"dx.hl.init.rn.%class.matrix.float.2.4 (i32, <8 x float>)"(i32 0, <8 x float> zeroinitializer) #0, !dbg !26 ; line:13 col:17
  %4 = call %class.matrix.float.2.4 @"dx.hl.cast.rowMatToColMat.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4)"(i32 7, %class.matrix.float.2.4 %3) #0, !dbg !26 ; line:13 col:17
  %5 = call %class.matrix.float.2.4 @"dx.hl.matldst.colStore.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4*, %class.matrix.float.2.4)"(i32 1, %class.matrix.float.2.4* getelementptr inbounds ([1 x %class.matrix.float.2.4], [1 x %class.matrix.float.2.4]* @g.1.0.1, i32 0, i32 0), %class.matrix.float.2.4 %4) #0, !dbg !26 ; line:13 col:17
  %6 = load float, float* getelementptr inbounds ([1 x float], [1 x float]* @g.1.0.0, i32 0, i32 0), !dbg !32 ; line:17 col:9
  %7 = getelementptr inbounds %class.matrix.float.2.4, %class.matrix.float.2.4* %h.0.1, i32 0, i32 0, i32 0, !dbg !32 ; line:17 col:9
  %8 = load <4 x float>, <4 x float>* getelementptr inbounds ([1 x %class.matrix.float.2.4], [1 x %class.matrix.float.2.4]* @g.1.0.1, i32 0, i32 0, i32 0, i32 0), !dbg !32 ; line:17 col:9
  store <4 x float> %8, <4 x float>* %7, !dbg !32 ; line:17 col:9
  %9 = getelementptr inbounds %class.matrix.float.2.4, %class.matrix.float.2.4* %h.0.1, i32 0, i32 0, i32 1, !dbg !32 ; line:17 col:9
  %10 = load <4 x float>, <4 x float>* getelementptr inbounds ([1 x %class.matrix.float.2.4], [1 x %class.matrix.float.2.4]* @g.1.0.1, i32 0, i32 0, i32 0, i32 1), !dbg !32 ; line:17 col:9
  store <4 x float> %10, <4 x float>* %9, !dbg !32 ; line:17 col:9
  ret void, !dbg !33 ; line:18 col:3
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

; Function Attrs: nounwind readnone
declare %class.matrix.float.2.4 @"dx.hl.init.rn.%class.matrix.float.2.4 (i32, <8 x float>)"(i32, <8 x float>) #1

; Function Attrs: nounwind readnone
declare %class.matrix.float.2.4 @"dx.hl.cast.rowMatToColMat.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4)"(i32, %class.matrix.float.2.4) #1

; Function Attrs: nounwind
declare %class.matrix.float.2.4 @"dx.hl.matldst.colStore.%class.matrix.float.2.4 (i32, %class.matrix.float.2.4*, %class.matrix.float.2.4)"(i32, %class.matrix.float.2.4*, %class.matrix.float.2.4) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6, !15}
!dx.entryPoints = !{!19}
!dx.fnprops = !{!23}
!dx.options = !{!24, !25}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4640 (issue-785, 45018c752d)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 0, %struct.e undef, !7, %struct.d undef, !10, %struct.a undef, !11}
!7 = !{i32 152, !8, !9}
!8 = !{i32 6, !"c", i32 3, i32 0}
!9 = !{i32 6, !"f", i32 3, i32 80}
!10 = !{i32 72, !8}
!11 = !{i32 72, !12, !13}
!12 = !{i32 6, !"b", i32 3, i32 0, i32 7, i32 9}
!13 = !{i32 6, !"c", i32 2, !14, i32 3, i32 16, i32 7, i32 9}
!14 = !{i32 2, i32 4, i32 2}
!15 = !{i32 1, void ()* @main, !16}
!16 = !{!17}
!17 = !{i32 1, !18, !18}
!18 = !{}
!19 = !{void ()* @main, !"main", null, !20, null}
!20 = !{null, null, !21, null}
!21 = !{!22}
!22 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!23 = !{void ()* @main, i32 5, i32 1, i32 1, i32 1}
!24 = !{i32 64}
!25 = !{i32 -1}
!26 = !DILocation(line: 13, column: 17, scope: !27, inlinedAt: !30)
!27 = !DISubprogram(name: "??__Eg@@YAXXZ", scope: !28, file: !28, line: 13, type: !29, isLocal: true, isDefinition: true, scopeLine: 13, flags: DIFlagPrototyped, isOptimized: false)
!28 = !DIFile(filename: "a.hlsl", directory: "")
!29 = !DISubroutineType(types: !18)
!30 = distinct !DILocation(line: 16, scope: !31)
!31 = !DISubprogram(name: "main", scope: !28, file: !28, line: 16, type: !29, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!32 = !DILocation(line: 17, column: 9, scope: !31)
!33 = !DILocation(line: 18, column: 3, scope: !31)
