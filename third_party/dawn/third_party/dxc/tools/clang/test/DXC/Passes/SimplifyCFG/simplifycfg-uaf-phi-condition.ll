; RUN: %dxopt %s -hlsl-passes-resume -simplifycfg -S | FileCheck %s

; The IR below comes from the following HLSL.
; Compiling this HLSL with dxc was resulting in an ASAN use-after-free in
; SimplifyCFG during FoldTwoEntryPHINode because it was deleting a PHI node
; which was itself used as the condition for the select that replaced it.

; struct a {
;   int b[2];
; };
;
; cbuffer cbuffer_c : register(b0) {
;   uint4 c[1];
; };
;
; void d(inout a e, inout int f) {
;   int n = f;
;   int g = asint(c[0].x);
;   int s = f;
;   bool i = (s >= 0);
;   int j = (s * n);
;   bool k = (6 > g);
;   int l = 0;
;   bool q = (s > j);
;   while (true) {
;     while (true) {
;       while (true) {
;         if (k) {
;           {
;             int t[2] = e.b;
;             t[g] = n;
;             e.b = t;
;           }
;         }
;         e.b[1] = g;
;         e.b[0] = s;
;         if (q) {
;           break;
;         }
;       }
;       switch(j) {
;         case 0: {
;           break;
;         }
;         case 9: {
;           break;
;         }
;         default: {
;           {
;             int u[2] = e.b;
;             u[g] = l;
;             e.b = u;
;           }
;           break;
;         }
;       }
;       {
;         if (q) { break; }
;       }
;     }
;     {
;       int v[2] = e.b;
;       v[g] = j;
;       e.b = v;
;     }
;     if (!(i)) {
;       break;
;     }
;   }
; }
;
; [numthreads(1, 1, 1)]
; void main() {
;   int o = 0;
;   a p = (a)0;
;   while (true) {
;     bool i = (o < asint(c[0].x));
;     if (i) {
;       bool r = !(i);
;       if (!(r)) {
;         return;
;       }
;       d(p, o);
;     }
;     o = (o + 1);
;   }
;   return;
; }

; Make sure the phi node did not get deleted by simplifycfg
; CHECK:       while.body:
; CHECK-NEXT:    %o.0 = phi i32 [ 0, %entry ], [ %add, %if.end.6 ]

;
; Buffer Definitions:
;
; cbuffer cbuffer_c
; {
;
;   struct cbuffer_c
;   {
;
;       uint4 c[1];                                   ; Offset:    0
;   
;   } cbuffer_c;                                      ; Offset:    0 Size:    16
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; cbuffer_c                         cbuffer      NA          NA     CB0            cb0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%cbuffer_c = type { [1 x <4 x i32>] }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%struct.a = type { [2 x i32] }

@cbuffer_c = external constant %cbuffer_c
@llvm.used = appending global [1 x i8*] [i8* bitcast (%cbuffer_c* @cbuffer_c to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %0 = load %cbuffer_c, %cbuffer_c* @cbuffer_c, align 4
  %cbuffer_c8 = call %dx.types.Handle @dx.op.createHandleForLib.cbuffer_c(i32 160, %cbuffer_c %0)  ; CreateHandleForLib(Resource)
  %1 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %cbuffer_c8, %dx.types.ResourceProperties { i32 13, i32 16 })  ; AnnotateHandle(res,props)  resource: CBuffer
  %cbuffer_c = call %dx.types.Handle @dx.op.createHandleForLib.cbuffer_c(i32 160, %cbuffer_c %0)  ; CreateHandleForLib(Resource)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %cbuffer_c, %dx.types.ResourceProperties { i32 13, i32 16 })  ; AnnotateHandle(res,props)  resource: CBuffer
  br label %while.body, !dbg !21 ; line:69 col:3

while.body:                                       ; preds = %if.end.6, %entry
  %o.0 = phi i32 [ 0, %entry ], [ %add, %if.end.6 ]
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %1, i32 0), !dbg !25 ; line:70 col:25  ; CBufferLoadLegacy(handle,regIndex)
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 0, !dbg !25 ; line:70 col:25
  %cmp = icmp slt i32 %o.0, %4, !dbg !26 ; line:70 col:17
  br i1 %cmp, label %if.then, label %if.end.6, !dbg !27 ; line:71 col:9

if.then:                                          ; preds = %while.body
  br i1 %cmp, label %if.then.5, label %if.end, !dbg !28 ; line:73 col:11

if.then.5:                                        ; preds = %if.then
  ret void, !dbg !29 ; line:74 col:9

if.end:                                           ; preds = %if.then
  %5 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %2, i32 0), !dbg !30 ; line:11 col:17  ; CBufferLoadLegacy(handle,regIndex)
  %6 = extractvalue %dx.types.CBufRet.i32 %5, 0, !dbg !30 ; line:11 col:17
  %cmp.i = icmp sgt i32 %o.0, -1, !dbg !33 ; line:13 col:15
  %mul.i = mul nsw i32 %o.0, %o.0, !dbg !34 ; line:14 col:14
  %cmp1.i = icmp slt i32 %6, 6, !dbg !35 ; line:15 col:15
  %cmp4.i = icmp sgt i32 %o.0, %mul.i, !dbg !36 ; line:17 col:15
  br label %while.body.10.i, !dbg !37 ; line:18 col:3

while.body.10.i:                                  ; preds = %while.end.27.i, %sw.epilog.i, %if.end.i, %if.end
  br i1 %cmp1.i, label %if.then.i, label %if.end.i, !dbg !38 ; line:21 col:13

if.then.i:                                        ; preds = %while.body.10.i
  br label %if.end.i, !dbg !39 ; line:27 col:9

if.end.i:                                         ; preds = %if.then.i, %while.body.10.i
  br i1 %cmp4.i, label %while.end.i, label %while.body.10.i, !dbg !40 ; line:30 col:13

while.end.i:                                      ; preds = %if.end.i
  switch i32 %mul.i, label %sw.default.i [
    i32 0, label %sw.epilog.i
    i32 9, label %sw.epilog.i
  ], !dbg !41 ; line:34 col:7

sw.default.i:                                     ; preds = %while.end.i
  br label %sw.epilog.i, !dbg !42 ; line:47 col:11

sw.epilog.i:                                      ; preds = %sw.default.i, %while.end.i, %while.end.i
  br i1 %cmp4.i, label %while.end.27.i, label %while.body.10.i, !dbg !43 ; line:51 col:13

while.end.27.i:                                   ; preds = %sw.epilog.i
  br i1 %cmp.i, label %while.body.10.i, label %if.end.6, !dbg !44 ; line:59 col:9

if.end.6:                                         ; preds = %while.end.27.i, %while.body
  %add = add nsw i32 %o.0, 1, !dbg !45 ; line:78 col:12
  br label %while.body, !dbg !21 ; line:69 col:3
}

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cbuffer_c*, i32)"(i32, %cbuffer_c*, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_c)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_c) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.cbuffer_c(i32, %cbuffer_c) #2

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
!dx.typeAnnotations = !{!9, !14}
!dx.entryPoints = !{!18}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.14620 (main, 8408ae882)"}
!3 = !{i32 1, i32 2}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 2}
!6 = !{null, null, !7, null}
!7 = !{!8}
!8 = !{i32 0, %cbuffer_c* @cbuffer_c, !"cbuffer_c", i32 0, i32 0, i32 1, i32 16, null}
!9 = !{i32 0, %struct.a undef, !10, %cbuffer_c undef, !12}
!10 = !{i32 20, !11}
!11 = !{i32 6, !"b", i32 3, i32 0, i32 7, i32 4}
!12 = !{i32 16, !13}
!13 = !{i32 6, !"c", i32 3, i32 0, i32 7, i32 5}
!14 = !{i32 1, void ()* @main, !15}
!15 = !{!16}
!16 = !{i32 1, !17, !17}
!17 = !{}
!18 = !{void ()* @main, !"main", null, !6, !19}
!19 = !{i32 4, !20}
!20 = !{i32 1, i32 1, i32 1}
!21 = !DILocation(line: 69, column: 3, scope: !22)
!22 = !DISubprogram(name: "main", scope: !23, file: !23, line: 66, type: !24, isLocal: false, isDefinition: true, scopeLine: 66, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!23 = !DIFile(filename: "/usr/local/google/home/chouinard/Downloads/standalone.hlsl", directory: "")
!24 = !DISubroutineType(types: !17)
!25 = !DILocation(line: 70, column: 25, scope: !22)
!26 = !DILocation(line: 70, column: 17, scope: !22)
!27 = !DILocation(line: 71, column: 9, scope: !22)
!28 = !DILocation(line: 73, column: 11, scope: !22)
!29 = !DILocation(line: 74, column: 9, scope: !22)
!30 = !DILocation(line: 11, column: 17, scope: !31, inlinedAt: !32)
!31 = !DISubprogram(name: "d", scope: !23, file: !23, line: 9, type: !24, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: false)
!32 = distinct !DILocation(line: 76, column: 7, scope: !22)
!33 = !DILocation(line: 13, column: 15, scope: !31, inlinedAt: !32)
!34 = !DILocation(line: 14, column: 14, scope: !31, inlinedAt: !32)
!35 = !DILocation(line: 15, column: 15, scope: !31, inlinedAt: !32)
!36 = !DILocation(line: 17, column: 15, scope: !31, inlinedAt: !32)
!37 = !DILocation(line: 18, column: 3, scope: !31, inlinedAt: !32)
!38 = !DILocation(line: 21, column: 13, scope: !31, inlinedAt: !32)
!39 = !DILocation(line: 27, column: 9, scope: !31, inlinedAt: !32)
!40 = !DILocation(line: 30, column: 13, scope: !31, inlinedAt: !32)
!41 = !DILocation(line: 34, column: 7, scope: !31, inlinedAt: !32)
!42 = !DILocation(line: 47, column: 11, scope: !31, inlinedAt: !32)
!43 = !DILocation(line: 51, column: 13, scope: !31, inlinedAt: !32)
!44 = !DILocation(line: 59, column: 9, scope: !31, inlinedAt: !32)
!45 = !DILocation(line: 78, column: 12, scope: !22)
