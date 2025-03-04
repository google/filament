; RUN: %dxopt %s -hlsl-passes-resume -instcombine,NoSink=0 -S | FileCheck %s

; Generated from the following HLSL:
;   cbuffer cbuffer_g : register(b0) {
;     uint4 g[1];
;   };
;   
;   [numthreads(1, 1, 1)]
;   void main() {
;     uint a = 2147483648u;
;     uint b = (g[0].x | 2651317025u);
;     uint c = (b + 2651317025u);
;     while (true) {
;       bool d = (a > c);
;       if (d) {
;         break;
;       } else {
;         while (true) {
;           if (!d) {
;             return;
;           }
;           a = b;
;           bool e = (d ? d : d);
;           if (e) {
;             break;
;           }
;         }
;       }
;     }
;   }
;
; Compiling this was resulting in invalid IR being produced from instcombine.
; Specifically, when optimizing an overflow check of an add followed by a compare,
; the new instruction was being inserted at the compare, and the add removed. This
; broke in cases like this one, where there were other uses of the former add between
; it and the compare. The fix was to make sure to insert the new instruction
; (another add in this case) at the old add rather than at the compare.

; Make sure the new %add is still in the entry block before its uses.
; CHECK-LABEL: entry
; CHECK:         %add = add i32 %or, -1643650271
; CHECK-NEXT:    %cmp.2 = icmp sgt i32 %add, -1,
;
; Make sure the new %add is NOT in the loopexit where %cmp was optimized.
; CHECK-LABEL: while.body.loopexit
; CHECK-NEXT:    br i1 false, label %if.end.preheader, label %while.end.14

;
; Buffer Definitions:
;
; cbuffer cbuffer_g
; {
;
;   struct cbuffer_g
;   {
;
;       uint4 g[1];                                   ; Offset:    0
;   
;   } cbuffer_g;                                      ; Offset:    0 Size:    16
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; cbuffer_g                         cbuffer      NA          NA     CB0            cb0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%cbuffer_g = type { [1 x <4 x i32>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }

@cbuffer_g = external constant %cbuffer_g
@llvm.used = appending global [1 x i8*] [i8* bitcast (%cbuffer_g* @cbuffer_g to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %0 = load %cbuffer_g, %cbuffer_g* @cbuffer_g
  %cbuffer_g = call %dx.types.Handle @dx.op.createHandleForLib.cbuffer_g(i32 160, %cbuffer_g %0)  ; CreateHandleForLib(Resource)
  %1 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %cbuffer_g, %dx.types.ResourceProperties { i32 13, i32 16 })  ; AnnotateHandle(res,props)  resource: CBuffer
  %2 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %1, i32 0), !dbg !19 ; line:8 col:13  ; CBufferLoadLegacy(handle,regIndex)
  %3 = extractvalue %dx.types.CBufRet.i32 %2, 0, !dbg !19 ; line:8 col:13
  %or = or i32 %3, -1643650271, !dbg !23 ; line:8 col:20
  %add = add i32 %or, -1643650271, !dbg !24 ; line:9 col:15
  %cmp.2 = icmp ugt i32 -2147483648, %add, !dbg !25 ; line:11 col:17
  %frombool.3 = zext i1 %cmp.2 to i32, !dbg !26 ; line:11 col:10
  %tobool1.4 = icmp eq i32 %frombool.3, 0, !dbg !27 ; line:12 col:9
  %or.cond.6 = and i1 %tobool1.4, %cmp.2, !dbg !27 ; line:12 col:9
  br i1 %or.cond.6, label %if.end.preheader, label %while.end.14, !dbg !27 ; line:12 col:9

while.body.loopexit:                              ; preds = %if.end
  %cmp = icmp ugt i32 %or, %add, !dbg !25 ; line:11 col:17
  %frombool = zext i1 %cmp to i32, !dbg !26 ; line:11 col:10
  %tobool1 = icmp eq i32 %frombool, 0, !dbg !27 ; line:12 col:9
  %or.cond = and i1 %tobool1, %cmp, !dbg !27 ; line:12 col:9
  br i1 %or.cond, label %if.end.preheader, label %while.end.14, !dbg !27 ; line:12 col:9

if.end.preheader:                                 ; preds = %entry, %while.body.loopexit
  %d.0 = phi i32 [ %frombool, %while.body.loopexit ], [ %frombool.3, %entry ]
  br label %if.end, !dbg !28 ; line:19 col:13

while.body.3:                                     ; preds = %if.end
  %tobool4.old = icmp ne i32 %d.0, 0, !dbg !29 ; line:16 col:14
  br i1 %tobool4.old, label %if.end, label %while.end.14, !dbg !30 ; line:16 col:13

if.end:                                           ; preds = %if.end.preheader, %while.body.3
  %tobool6 = icmp ne i32 %d.0, 0, !dbg !31 ; line:20 col:19
  %tobool7 = icmp ne i32 %d.0, 0, !dbg !32 ; line:20 col:23
  %tobool8 = icmp ne i32 %d.0, 0, !dbg !33 ; line:20 col:27
  %4 = select i1 %tobool6, i1 %tobool7, i1 %tobool8, !dbg !31 ; line:20 col:19
  br i1 %4, label %while.body.loopexit, label %while.body.3, !dbg !34 ; line:21 col:13

while.end.14:                                     ; preds = %while.body.loopexit, %while.body.3, %entry
  ret void, !dbg !35 ; line:27 col:1
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cbuffer_g*, i32)"(i32, %cbuffer_g*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_g)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_g) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.cbuffer_g(i32, %cbuffer_g) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.typeAnnotations = !{!9, !12}
!dx.entryPoints = !{!16}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.4514 (d9bd2a706-dirty)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 0}
!6 = !{null, null, !7, null}
!7 = !{!8}
!8 = !{i32 0, %cbuffer_g* @cbuffer_g, !"cbuffer_g", i32 0, i32 0, i32 1, i32 16, null}
!9 = !{i32 0, %cbuffer_g undef, !10}
!10 = !{i32 16, !11}
!11 = !{i32 6, !"g", i32 3, i32 0, i32 7, i32 5}
!12 = !{i32 1, void ()* @main, !13}
!13 = !{!14}
!14 = !{i32 1, !15, !15}
!15 = !{}
!16 = !{void ()* @main, !"main", null, !6, !17}
!17 = !{i32 4, !18}
!18 = !{i32 1, i32 1, i32 1}
!19 = !DILocation(line: 8, column: 13, scope: !20)
!20 = !DISubprogram(name: "main", scope: !21, file: !21, line: 6, type: !22, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!21 = !DIFile(filename: "/mnt/c/Users/amaiorano/Downloads/342545100/standalone_reduced.hlsl", directory: "")
!22 = !DISubroutineType(types: !15)
!23 = !DILocation(line: 8, column: 20, scope: !20)
!24 = !DILocation(line: 9, column: 15, scope: !20)
!25 = !DILocation(line: 11, column: 17, scope: !20)
!26 = !DILocation(line: 11, column: 10, scope: !20)
!27 = !DILocation(line: 12, column: 9, scope: !20)
!28 = !DILocation(line: 19, column: 13, scope: !20)
!29 = !DILocation(line: 16, column: 14, scope: !20)
!30 = !DILocation(line: 16, column: 13, scope: !20)
!31 = !DILocation(line: 20, column: 19, scope: !20)
!32 = !DILocation(line: 20, column: 23, scope: !20)
!33 = !DILocation(line: 20, column: 27, scope: !20)
!34 = !DILocation(line: 21, column: 13, scope: !20)
!35 = !DILocation(line: 27, column: 1, scope: !20)
