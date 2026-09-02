; RUN: opt %s -reassociate -S | FileCheck %s

; Issue: #6829

; There are cases where Reassociation used to double-count a constant value
; used as a multiplicative factor in an addition tree.
; When rewriting an add-over-muls it asserts out when it has double-counted
; those factors. The code generated is still correct because its add tree is
; still has the same leaves.

; In the input module, the troublesome tree of adds ends in %add.i0

; Reassociation should delete most of the code
; CHECK: entry:
; CHECK-NOT: %add.i0
; CHECK-NOT: = add
; CHECK-NOT: = mul
; CHECK: ret void


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
define void @m() #0 {
entry:
  %DerivCoarseY = call float @dx.op.unary.f32(i32 84, float 0.000000e+00), !dbg !13
  %cmp = fcmp fast ogt float %DerivCoarseY, 0.000000e+00, !dbg !17
  %sub.i0 = select i1 %cmp, i32 2, i32 1, !dbg !18
  %sub.i0.neg = sub i32 0, %sub.i0
  %sub.i1 = select i1 %cmp, i32 2, i32 1, !dbg !18
  %sub.i1.neg = sub i32 0, %sub.i0
  %neg16.i0 = xor i32 %sub.i0, -1, !dbg !19
  %neg16.i0.neg = sub i32 0, %neg16.i0
  %neg16.i1 = xor i32 %sub.i0, -1, !dbg !19
  %neg16.i1.neg = sub i32 0, %neg16.i1
  %neg25.i0 = add i32 -2, %sub.i0.neg, !dbg !20
  %0 = sub nuw nsw i32 0, 0, !dbg !20
  %neg25.i1 = add i32 -2, %sub.i1.neg, !dbg !20
  %1 = sub nuw nsw i32 0, 0, !dbg !20
  %sub26.i0.neg = add i32 %sub.i0.neg, -1, !dbg !21
  %sub27.i0 = add i32 %neg25.i0, %sub26.i0.neg, !dbg !22
  %2 = sub nuw nsw i32 0, 0, !dbg !22
  %sub26.i1.neg = add i32 %sub.i0.neg, -1, !dbg !21
  %sub27.i1 = add i32 %neg25.i1, %sub26.i1.neg, !dbg !22
  %3 = sub nuw nsw i32 0, 0, !dbg !22
  %sub28.i0 = mul nuw nsw i32 %sub.i0, 2, !dbg !23
  %4 = shl nuw nsw i32 undef, 1, !dbg !23
  %sub28.i1 = mul nuw nsw i32 %sub.i0, 2, !dbg !23
  %5 = shl nuw nsw i32 undef, 1, !dbg !23
  %sub28.i0.neg = sub i32 0, %sub28.i0
  %mul.i0.neg = add i32 %sub28.i0.neg, -2, !dbg !23
  %sub29.i0 = add i32 %sub27.i0, %mul.i0.neg, !dbg !24
  %6 = sub nuw nsw i32 0, 0, !dbg !24
  %sub28.i1.neg = sub i32 0, %sub28.i1
  %mul.i1.neg = add i32 %sub28.i1.neg, -2, !dbg !23
  %sub29.i1 = add i32 %sub27.i1, %mul.i1.neg, !dbg !24
  %7 = sub nuw nsw i32 0, 0, !dbg !24
  %sub30.i0 = add i32 %sub29.i0, %neg16.i0.neg, !dbg !25
  %8 = sub nsw i32 0, 0, !dbg !25
  %sub30.i1 = add i32 %sub29.i1, %neg16.i1.neg, !dbg !25
  %9 = sub nsw i32 0, 0, !dbg !25
  %sub31.i0 = add i32 %sub30.i0, %neg16.i0.neg, !dbg !26
  %10 = sub nsw i32 0, 0, !dbg !26
  %sub31.i1 = add i32 %sub30.i1, %neg16.i1.neg, !dbg !26
  %11 = sub nsw i32 0, 0, !dbg !26
  %neg33.i0280 = add nuw nsw i32 %sub.i0, 2, !dbg !27
  %sub34.i0 = add i32 %neg33.i0280, %sub31.i0, !dbg !27
  %neg33.i1281 = add nuw nsw i32 %sub.i0, 2, !dbg !27
  %sub34.i1 = add i32 %neg33.i1281, %sub31.i1, !dbg !27
  %div.i.i0 = sdiv i32 %neg16.i0, 4, !dbg !28
  %div.i.i1 = sdiv i32 %neg16.i1, 4, !dbg !28
  %mul.i.i0 = shl nsw i32 %div.i.i0, 2, !dbg !31
  %mul.i.i1 = shl nsw i32 %div.i.i1, 2, !dbg !31
  %sub.i.i0282 = add i32 %mul.i.i0, %neg16.i0.neg, !dbg !32
  %12 = sub nsw i32 0, 0, !dbg !32
  %sub36.i0 = add i32 %sub.i.i0282, %sub34.i0, !dbg !32
  %sub.i.i1283 = add i32 %mul.i.i1, %neg16.i1.neg, !dbg !32
  %13 = sub nsw i32 0, 0, !dbg !32
  %sub36.i1 = add i32 %sub.i.i1283, %sub34.i1, !dbg !32
  %sub37.i0 = add i32 %sub36.i0, -1, !dbg !33
  %sub37.i1 = add i32 %sub36.i1, -1, !dbg !33
  %add.i0 = add i32 %neg16.i0, %sub37.i0, !dbg !34
   %add.i1 = add i32 %sub37.i1, %neg16.i1, !dbg !34
   %sub39.i0 = sub i32 0, %add.i0, !dbg !35
   %sub39.i1 = sub i32 1, %add.i1, !dbg !35
   %14 = or i32 %sub39.i0, %sub39.i1, !dbg !36
   %15 = icmp slt i32 %10, 0, !dbg !36
   br i1 %15, label %if.then.i.27, label %if.else.i.29, !dbg !36
 
 if.then.i.27:                                     ; preds = %entry
   %div.i.24.i0 = sdiv i32 %sub39.i0, 2, !dbg !38
   %div.i.24.i1 = sdiv i32 %sub39.i1, 2, !dbg !38
   %mul.i.25.i0 = shl nsw i32 %div.i.24.i0, 1, !dbg !39
   %mul.i.25.i1 = shl nsw i32 %div.i.24.i1, 1, !dbg !39
   %sub.i.26.i0 = sub i32 %sub39.i0, %mul.i.25.i0, !dbg !40
   %sub.i.26.i1 = sub i32 %sub39.i1, %mul.i.25.i1, !dbg !40
   br label %"\01?tint_mod@@YA?AV?$vector@H$01@@V1@H@Z.exit.30", !dbg !41
 
 if.else.i.29:                                     ; preds = %entry
   %rem.i.28.i0 = srem i32 %sub39.i0, 2, !dbg !42
   %rem.i.28.i1 = srem i32 %sub39.i1, 2, !dbg !42
   br label %"\01?tint_mod@@YA?AV?$vector@H$01@@V1@H@Z.exit.30", !dbg !43
 
 "\01?tint_mod@@YA?AV?$vector@H$01@@V1@H@Z.exit.30": ; preds = %if.else.i.29, %if.then.i.27
   %retval.i.1.0.i0 = phi i32 [ %sub.i.26.i0, %if.then.i.27 ], [ %rem.i.28.i0, %if.else.i.29 ]
   %retval.i.1.0.i1 = phi i32 [ %sub.i.26.i1, %if.then.i.27 ], [ %rem.i.28.i1, %if.else.i.29 ]
   %cmp.i.42.i0 = icmp eq i32 %add.i0, 0, !dbg !44
   %cmp.i.42.i1 = icmp eq i32 %add.i0, 0, !dbg !44
   %cmp2.i.44.i0 = icmp eq i32 %retval.i.1.0.i0, -2147483648, !dbg !46
   %cmp2.i.44.i1 = icmp eq i32 %retval.i.1.0.i1, -2147483648, !dbg !46
   %cmp5.i.46.i0 = icmp eq i32 %add.i0, -1, !dbg !47
   %cmp5.i.46.i1 = icmp eq i32 %add.i0, -1, !dbg !47
   %and.i.48.i0284 = and i1 %cmp2.i.44.i0, %cmp5.i.46.i0, !dbg !48
   %and.i.48.i1285 = and i1 %cmp2.i.44.i1, %cmp5.i.46.i1, !dbg !48
 %or.i.49.i0286 = or i1 %cmp.i.42.i0, %and.i.48.i0284, !dbg !49
 %or.i.49.i1287 = or i1 %cmp.i.42.i1, %and.i.48.i1285, !dbg !49
  ret void, !dbg !54
}

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #0

attributes #0 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.4686 (issue-351, ff5955a4ed-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{i32 1, void ()* @m, !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{void ()* @m, !"m", null, null, !11}
!11 = !{i32 4, !12}
!12 = !{i32 1, i32 1, i32 1}
!13 = !DILocation(line: 22, column: 13, scope: !14)
!14 = !DISubprogram(name: "m", scope: !15, file: !15, line: 21, type: !16, isLocal: false, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @m)
!15 = !DIFile(filename: "case/a.hlsl", directory: "")
!16 = !DISubroutineType(types: !9)
!17 = !DILocation(line: 23, column: 21, scope: !14)
!18 = !DILocation(line: 24, column: 26, scope: !14)
!19 = !DILocation(line: 24, column: 12, scope: !14)
!20 = !DILocation(line: 25, column: 25, scope: !14)
!21 = !DILocation(line: 25, column: 53, scope: !14)
!22 = !DILocation(line: 25, column: 51, scope: !14)
!23 = !DILocation(line: 25, column: 67, scope: !14)
!24 = !DILocation(line: 25, column: 59, scope: !14)
!25 = !DILocation(line: 25, column: 73, scope: !14)
!26 = !DILocation(line: 25, column: 78, scope: !14)
!27 = !DILocation(line: 25, column: 83, scope: !14)
!28 = !DILocation(line: 5, column: 25, scope: !29, inlinedAt: !30)
!29 = !DISubprogram(name: "tint_mod", scope: !15, file: !15, line: 1, type: !16, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false)
!30 = distinct !DILocation(line: 25, column: 96, scope: !14)
!31 = !DILocation(line: 5, column: 39, scope: !29, inlinedAt: !30)
!32 = !DILocation(line: 25, column: 94, scope: !14)
!33 = !DILocation(line: 25, column: 112, scope: !14)
!34 = !DILocation(line: 25, column: 16, scope: !14)
!35 = !DILocation(line: 26, column: 22, scope: !14)
!36 = !DILocation(line: 4, column: 7, scope: !29, inlinedAt: !37)
!37 = distinct !DILocation(line: 26, column: 13, scope: !14)
!38 = !DILocation(line: 5, column: 25, scope: !29, inlinedAt: !37)
!39 = !DILocation(line: 5, column: 39, scope: !29, inlinedAt: !37)
!40 = !DILocation(line: 5, column: 17, scope: !29, inlinedAt: !37)
!41 = !DILocation(line: 5, column: 5, scope: !29, inlinedAt: !37)
!42 = !DILocation(line: 7, column: 17, scope: !29, inlinedAt: !37)
!43 = !DILocation(line: 7, column: 5, scope: !29, inlinedAt: !37)
!44 = !DILocation(line: 3, column: 26, scope: !29, inlinedAt: !45)
!45 = distinct !DILocation(line: 27, column: 13, scope: !14)
!46 = !DILocation(line: 3, column: 45, scope: !29, inlinedAt: !45)
!47 = !DILocation(line: 3, column: 71, scope: !29, inlinedAt: !45)
!48 = !DILocation(line: 3, column: 66, scope: !29, inlinedAt: !45)
!49 = !DILocation(line: 3, column: 37, scope: !29, inlinedAt: !45)
!50 = !DILocation(line: 3, column: 22, scope: !29, inlinedAt: !45)
!51 = !DILocation(line: 5, column: 39, scope: !29, inlinedAt: !45)
!52 = !DILocation(line: 4, column: 7, scope: !29, inlinedAt: !53)
!53 = distinct !DILocation(line: 28, column: 13, scope: !14)
!54 = !DILocation(line: 32, column: 3, scope: !14)
