; RUN: %dxopt %s -hlsl-passes-resume -loop-unroll,StructurizeLoopExits=1 -S | FileCheck %s

; The Loop exit structurizer will wrap the definition of %DerivFineX3 in a conditional block.
; Its value will later be propagated into a phi, and that phi replaces all further uses
; of %DerivFineX3.
;
; Tests that a bug is fixed where the code used to iterate through the uses of a value
; while also updating those uses.  The old code would fail to update the definition
; of %g.i.2.i3 and the result would be an invalid module: %DerivFineX3 would not dominate
; all its uses.


; CHECK: define void @main
; CHECK-NOT:  %DerivFineX3
; CHECK:  "\01?f@@YAXXZ.exit.i":
; CHECK-NEXT:  br i1 true, label %dx.struct_exit.cond_end, label %dx.struct_exit.cond_body

; CHECK: dx.struct_exit.cond_body:
; CHECK:  %DerivFineX3 = call
; CHECK:  br label %dx.struct_exit.cond_end

; CHECK: dx.struct_exit.cond_end:
; CHECK:  = phi {{.*}} %DerivFineX3
; CHECK:  br
; CHECK-NOT:  %DerivFineX3
; CHECK: ret void


;
;
; void f() {
;   int l_1 = 10;
;   for (int l = 0, l_2 = 0; l < 5 && l_2 < 1; l++, l_2++) {
;     while (1 < l_1) { }
;   }
; }
;
;
; struct tint_symbol {
;   float4 value : SV_Target0;
; };
;
; float4 main_inner() {
;   float4 g = float4(0.0f, 0.0f, 0.0f, 0.0f);
;   bool2 true2 = (true).xx;
;   uint2 _e8 = (0u).xx;
;   do {
;     if (_e8.x != 2u) {
;       f();
;       float4 _e15 = ddx_fine(g);
;       if (_e8[_e8.x] == 2u) {
;         g = _e15;
;       } else {
;         f();
;       }
;       switch(_e8.x) {
;         case 3u: {
;           break;
;         }
;         case 2u: {
;           g = _e15;
;           break;
;         }
;         default: {
;           g = _e15;
;         }
;       }
;       f();
;     }
;  } while(!all(true2));
;   return g;
;}
;
;tint_symbol main() {
;  float4 inner_result = main_inner();
;  tint_symbol wrapper_result = (tint_symbol)0;
;  wrapper_result.value = inner_result;
;  return wrapper_result;
;}

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.tint_symbol = type { <4 x float> }

; Function Attrs: nounwind
define void @main(<4 x float>* noalias) #0 {
entry:
  %1 = alloca [2 x i32], align 4
  %2 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 0, !dbg !20 ; line:17 col:9
  store i32 0, i32* %2, align 4, !dbg !20 ; line:17 col:9
  %3 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 1, !dbg !20 ; line:17 col:9
  store i32 0, i32* %3, align 4, !dbg !20 ; line:17 col:9
  br label %do.body.i, !dbg !26 ; line:18 col:3

do.body.i:                                        ; preds = %do.cond.i, %entry
  %g.i.0.i0 = phi float [ 0.000000e+00, %entry ], [ %g.i.3.i0, %do.cond.i ]
  %g.i.0.i1 = phi float [ 0.000000e+00, %entry ], [ %g.i.3.i1, %do.cond.i ]
  %g.i.0.i2 = phi float [ 0.000000e+00, %entry ], [ %g.i.3.i2, %do.cond.i ]
  %g.i.0.i3 = phi float [ 0.000000e+00, %entry ], [ %g.i.3.i3, %do.cond.i ]
  %4 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 0, !dbg !27 ; line:19 col:9
  %5 = load i32, i32* %4, align 4, !dbg !27 ; line:19 col:9
  %cmp.i = icmp ne i32 %5, 2, !dbg !28 ; line:19 col:15
  br i1 %cmp.i, label %for.cond.i.i, label %do.cond.i, !dbg !27 ; line:19 col:9

for.cond.i.i:                                     ; preds = %do.body.i
  br i1 true, label %while.cond.i.i.preheader, label %"\01?f@@YAXXZ.exit.i", !dbg !29 ; line:4 col:3

while.cond.i.i.preheader:                         ; preds = %for.cond.i.i
  br label %while.cond.i.i, !dbg !32 ; line:5 col:5

while.cond.i.i:                                   ; preds = %while.cond.i.i.preheader, %while.cond.i.i
  br label %while.cond.i.i, !dbg !32 ; line:5 col:5

"\01?f@@YAXXZ.exit.i":                            ; preds = %for.cond.i.i
  %DerivFineX = call float @dx.op.unary.f32(i32 85, float %g.i.0.i0), !dbg !33 ; line:21 col:21  ; DerivFineX(value)
  %DerivFineX1 = call float @dx.op.unary.f32(i32 85, float %g.i.0.i1), !dbg !33 ; line:21 col:21  ; DerivFineX(value)
  %DerivFineX2 = call float @dx.op.unary.f32(i32 85, float %g.i.0.i2), !dbg !33 ; line:21 col:21  ; DerivFineX(value)
  %DerivFineX3 = call float @dx.op.unary.f32(i32 85, float %g.i.0.i3), !dbg !33 ; line:21 col:21  ; DerivFineX(value)
  %6 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 0, !dbg !34 ; line:22 col:15
  %7 = load i32, i32* %6, align 4, !dbg !34 ; line:22 col:15
  %8 = getelementptr [2 x i32], [2 x i32]* %1, i32 0, i32 %7, !dbg !35 ; line:22 col:11
  %9 = load i32, i32* %8, !dbg !35, !tbaa !36 ; line:22 col:11
  %cmp6.i = icmp eq i32 %9, 2, !dbg !40 ; line:22 col:22
  br i1 %cmp6.i, label %if.end.i, label %for.cond.i.19.i, !dbg !35 ; line:22 col:11

for.cond.i.19.i:                                  ; preds = %"\01?f@@YAXXZ.exit.i"
  br i1 true, label %while.cond.i.24.i.preheader, label %if.end.i, !dbg !41 ; line:4 col:3

while.cond.i.24.i.preheader:                      ; preds = %for.cond.i.19.i
  br label %while.cond.i.24.i, !dbg !43 ; line:5 col:5

while.cond.i.24.i:                                ; preds = %while.cond.i.24.i.preheader, %while.cond.i.24.i
  br label %while.cond.i.24.i, !dbg !43 ; line:5 col:5

if.end.i:                                         ; preds = %for.cond.i.19.i, %"\01?f@@YAXXZ.exit.i"
  %g.i.1.i0 = phi float [ %DerivFineX, %"\01?f@@YAXXZ.exit.i" ], [ %g.i.0.i0, %for.cond.i.19.i ]
  %g.i.1.i1 = phi float [ %DerivFineX1, %"\01?f@@YAXXZ.exit.i" ], [ %g.i.0.i1, %for.cond.i.19.i ]
  %g.i.1.i2 = phi float [ %DerivFineX2, %"\01?f@@YAXXZ.exit.i" ], [ %g.i.0.i2, %for.cond.i.19.i ]
  %g.i.1.i3 = phi float [ %DerivFineX3, %"\01?f@@YAXXZ.exit.i" ], [ %g.i.0.i3, %for.cond.i.19.i ]
  %10 = getelementptr inbounds [2 x i32], [2 x i32]* %1, i32 0, i32 0, !dbg !44 ; line:27 col:14
  %11 = load i32, i32* %10, align 4, !dbg !44 ; line:27 col:14
  switch i32 %11, label %sw.default.i [
    i32 3, label %for.cond.i.5.i
    i32 2, label %sw.bb.10.i
  ], !dbg !45 ; line:27 col:7

sw.bb.10.i:                                       ; preds = %if.end.i
  br label %for.cond.i.5.i, !dbg !46 ; line:33 col:11

sw.default.i:                                     ; preds = %if.end.i
  br label %for.cond.i.5.i, !dbg !47 ; line:38 col:7

for.cond.i.5.i:                                   ; preds = %if.end.i, %sw.bb.10.i, %sw.default.i
  %g.i.2.i0 = phi float [ %DerivFineX, %sw.default.i ], [ %DerivFineX, %sw.bb.10.i ], [ %g.i.1.i0, %if.end.i ]
  %g.i.2.i1 = phi float [ %DerivFineX1, %sw.default.i ], [ %DerivFineX1, %sw.bb.10.i ], [ %g.i.1.i1, %if.end.i ]
  %g.i.2.i2 = phi float [ %DerivFineX2, %sw.default.i ], [ %DerivFineX2, %sw.bb.10.i ], [ %g.i.1.i2, %if.end.i ]
  %g.i.2.i3 = phi float [ %DerivFineX3, %sw.default.i ], [ %DerivFineX3, %sw.bb.10.i ], [ %g.i.1.i3, %if.end.i ]
  br i1 true, label %while.cond.i.10.i.preheader, label %do.cond.i, !dbg !48 ; line:4 col:3

while.cond.i.10.i.preheader:                      ; preds = %for.cond.i.5.i
  br label %while.cond.i.10.i, !dbg !50 ; line:5 col:5

while.cond.i.10.i:                                ; preds = %while.cond.i.10.i.preheader, %while.cond.i.10.i
  br label %while.cond.i.10.i, !dbg !50 ; line:5 col:5

do.cond.i:                                        ; preds = %for.cond.i.5.i, %do.body.i
  %g.i.3.i0 = phi float [ %g.i.0.i0, %do.body.i ], [ %g.i.2.i0, %for.cond.i.5.i ]
  %g.i.3.i1 = phi float [ %g.i.0.i1, %do.body.i ], [ %g.i.2.i1, %for.cond.i.5.i ]
  %g.i.3.i2 = phi float [ %g.i.0.i2, %do.body.i ], [ %g.i.2.i2, %for.cond.i.5.i ]
  %g.i.3.i3 = phi float [ %g.i.0.i3, %do.body.i ], [ %g.i.2.i3, %for.cond.i.5.i ]
  br i1 false, label %do.body.i, label %"\01?main_inner@@YA?AV?$vector@M$03@@XZ.exit", !dbg !51 ; line:41 col:3

"\01?main_inner@@YA?AV?$vector@M$03@@XZ.exit":    ; preds = %do.cond.i
  %g.i.3.i3.lcssa = phi float [ %g.i.3.i3, %do.cond.i ]
  %g.i.3.i2.lcssa = phi float [ %g.i.3.i2, %do.cond.i ]
  %g.i.3.i1.lcssa = phi float [ %g.i.3.i1, %do.cond.i ]
  %g.i.3.i0.lcssa = phi float [ %g.i.3.i0, %do.cond.i ]
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %g.i.3.i0.lcssa), !dbg !52 ; line:49 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %g.i.3.i1.lcssa), !dbg !52 ; line:49 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %g.i.3.i2.lcssa), !dbg !52 ; line:49 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %g.i.3.i3.lcssa), !dbg !52 ; line:49 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void, !dbg !53 ; line:49 col:3
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #1

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

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.14549 (main, 0781ded87-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"ps", i32 6, i32 0}
!6 = !{i32 0, %struct.tint_symbol undef, !7}
!7 = !{i32 16, !8}
!8 = !{i32 6, !"value", i32 3, i32 0, i32 4, !"SV_Target0", i32 7, i32 9}
!9 = !{i32 1, void (<4 x float>*)* @main, !10}
!10 = !{!11, !13}
!11 = !{i32 0, !12, !12}
!12 = !{}
!13 = !{i32 1, !14, !15}
!14 = !{i32 4, !"SV_Target0", i32 7, i32 9}
!15 = !{i32 0}
!16 = !{void (<4 x float>*)* @main, !"main", !17, null, null}
!17 = !{null, !18, null}
!18 = !{!19}
!19 = !{i32 0, !"SV_Target", i8 9, i8 16, !15, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!20 = !DILocation(line: 17, column: 9, scope: !21, inlinedAt: !24)
!21 = !DISubprogram(name: "main_inner", scope: !22, file: !22, line: 14, type: !23, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: false)
!22 = !DIFile(filename: "s2.hlsl", directory: "")
!23 = !DISubroutineType(types: !12)
!24 = distinct !DILocation(line: 46, column: 25, scope: !25)
!25 = !DISubprogram(name: "main", scope: !22, file: !22, line: 45, type: !23, isLocal: false, isDefinition: true, scopeLine: 45, flags: DIFlagPrototyped, isOptimized: false, function: void (<4 x float>*)* @main)
!26 = !DILocation(line: 18, column: 3, scope: !21, inlinedAt: !24)
!27 = !DILocation(line: 19, column: 9, scope: !21, inlinedAt: !24)
!28 = !DILocation(line: 19, column: 15, scope: !21, inlinedAt: !24)
!29 = !DILocation(line: 4, column: 3, scope: !30, inlinedAt: !31)
!30 = !DISubprogram(name: "f", scope: !22, file: !22, line: 2, type: !23, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false)
!31 = distinct !DILocation(line: 20, column: 7, scope: !21, inlinedAt: !24)
!32 = !DILocation(line: 5, column: 5, scope: !30, inlinedAt: !31)
!33 = !DILocation(line: 21, column: 21, scope: !21, inlinedAt: !24)
!34 = !DILocation(line: 22, column: 15, scope: !21, inlinedAt: !24)
!35 = !DILocation(line: 22, column: 11, scope: !21, inlinedAt: !24)
!36 = !{!37, !37, i64 0}
!37 = !{!"int", !38, i64 0}
!38 = !{!"omnipotent char", !39, i64 0}
!39 = !{!"Simple C/C++ TBAA"}
!40 = !DILocation(line: 22, column: 22, scope: !21, inlinedAt: !24)
!41 = !DILocation(line: 4, column: 3, scope: !30, inlinedAt: !42)
!42 = distinct !DILocation(line: 25, column: 9, scope: !21, inlinedAt: !24)
!43 = !DILocation(line: 5, column: 5, scope: !30, inlinedAt: !42)
!44 = !DILocation(line: 27, column: 14, scope: !21, inlinedAt: !24)
!45 = !DILocation(line: 27, column: 7, scope: !21, inlinedAt: !24)
!46 = !DILocation(line: 33, column: 11, scope: !21, inlinedAt: !24)
!47 = !DILocation(line: 38, column: 7, scope: !21, inlinedAt: !24)
!48 = !DILocation(line: 4, column: 3, scope: !30, inlinedAt: !49)
!49 = distinct !DILocation(line: 39, column: 7, scope: !21, inlinedAt: !24)
!50 = !DILocation(line: 5, column: 5, scope: !30, inlinedAt: !49)
!51 = !DILocation(line: 41, column: 3, scope: !21, inlinedAt: !24)
!52 = !DILocation(line: 49, column: 10, scope: !25)
!53 = !DILocation(line: 49, column: 3, scope: !25)
