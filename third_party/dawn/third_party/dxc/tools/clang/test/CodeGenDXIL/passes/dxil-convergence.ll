; RUN: %dxopt %s -hlsl-passes-resume -hlsl-dxil-convergent-mark -S | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
declare <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32, <2 x float>) #1

; Function Attrs: nounwind
define void @main(<2 x float>* noalias %arg, <2 x float> %arg1, <2 x float> %arg2, i32 %arg3) #0 {
bb:
  ; CHECK: [[val:%.*]] = extractelement <2 x float> %tmp, i64 0
  ; CHECK: [[conv:%.*]] = call float @dxil.convergent.marker.float(float [[val]])
  ; CHECK: [[vec0:%.*]] = insertelement <2 x float> undef, float [[conv]], i64 0
  ; CHECK: [[val:%.*]] = extractelement <2 x float> %tmp, i64 1
  ; CHECK: [[conv:%.*]] = call float @dxil.convergent.marker.float(float [[val]])
  ; CHECK: [[vec:%.*]] = insertelement <2 x float> [[vec0]], float [[conv]], i64 1
  %tmp = fadd <2 x float> %arg1, %arg2
  %tmp4 = icmp sgt i32 %arg3, 2
  %tmp5 = icmp ne i1 %tmp4, false
  %tmp6 = icmp ne i1 %tmp5, false
  br i1 %tmp6, label %bb7, label %bb10

bb7:                                              ; preds = %bb
  ; CHECK: call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32 128, <2 x float> [[vec]])
  %tmp8 = call <2 x float> @"dx.hl.op.rn.<2 x float> (i32, <2 x float>)"(i32 128, <2 x float> %tmp)
  %tmp9 = fsub <2 x float> zeroinitializer, %tmp8
  br label %bb10

bb10:                                             ; preds = %bb7, %bb
  %res.0 = phi <2 x float> [ %tmp9, %bb7 ], [ zeroinitializer, %bb ]
  store <2 x float> %res.0, <2 x float>* %arg
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!19}
!dx.fnprops = !{!20}
!dx.options = !{!21, !22}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4959 (coopvec-tests, 43e1db83c-dirty)"}
!3 = !{i32 1, i32 8}
!4 = !{i32 1, i32 9}
!5 = !{!"ps", i32 6, i32 8}
!6 = !{i32 1, void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, !7}
!7 = !{!8, !10, !13, !15, !17}
!8 = !{i32 0, !9, !9}
!9 = !{}
!10 = !{i32 1, !11, !12}
!11 = !{i32 4, !"SV_Target", i32 7, i32 9}
!12 = !{i32 0}
!13 = !{i32 0, !14, !12}
!14 = !{i32 4, !"A", i32 7, i32 9}
!15 = !{i32 0, !16, !12}
!16 = !{i32 4, !"B", i32 7, i32 9}
!17 = !{i32 0, !18, !12}
!18 = !{i32 4, !"C", i32 7, i32 4}
!19 = !{void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, !"main", null, null, null}
!20 = !{void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main, i32 0, i1 false}
!21 = !{i32 64}
!22 = !{i32 -1}
!23 = !DILocation(line: 26, column: 20, scope: !24)
!24 = !DISubprogram(name: "main", scope: !25, file: !25, line: 24, type: !26, isLocal: false, isDefinition: true, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: false, function: void (<2 x float>*, <2 x float>, <2 x float>, i32)* @main)
!25 = !DIFile(filename: "/Users/pow2clk/dxc/tools/clang/test/CodeGenDXIL/passes/convergent-derivs.hlsl", directory: "")
!26 = !DISubroutineType(types: !9)
!27 = !DILocation(line: 28, column: 9, scope: !24)
!28 = !DILocation(line: 28, column: 7, scope: !24)
!29 = !DILocation(line: 29, column: 12, scope: !24)
!30 = !DILocation(line: 29, column: 9, scope: !24)
!31 = !DILocation(line: 29, column: 5, scope: !24)
!32 = !DILocation(line: 31, column: 3, scope: !24)
