; RUN: %dxopt %s -hlsl-passes-resume -simplifycfg -S | FileCheck %s

; The IR below comes from the following HLSL.
; Compiling this HLSL with dxc was resulting in an ASAN
; use-after-free in SimplifyCFG during
; SimplifyTerminatorOnSelect because it was deleting
; a PHI node with an input value that the pass later
; emits (the select condition value).

; ByteAddressBuffer buff : register(t0);
; 
; [numthreads(1, 1, 1)]
; void main() {
;   if (buff.Load(0u)) {
;     return;
;   }
; 
;   int i = 0;
;   int j = 0;
;   while (true) {
;     bool a = (i < 2);
;     switch(i) {
;       case 0: {
;         while (true) {
;           bool b = (j < 2);
;           if (b) {
;           } else {
;             break;
;           }
;           while (true) {
;             int unused = 0;
;             while (true) {
;               if (a) break;
;             }
;             while (true) {
;               while (true) {
;                 if (b) {
;                   if (b) return;
;                 } else {
;                   break;
;                 }
;                 while (true) {
;                   i = 0;
;                   if (b) break;
;                 }
;                 if (a) break;
;               }
;               if (a) break;
;             }
;             if (a) break;
;           }
;           j = (j + 2);
;         }
;       }
;     }
;   }
; }

; Make sure the phi node did not get deleted by simplifycfg
; CHECK:       cleanup:
; CHECK-NEXT:    %cleanup.dest.slot.0 = phi i32 [ 1, %while.body.20 ], [ %.mux, %while.end.37 ]
; CHECK-NEXT:    switch i32 %cleanup.dest.slot.0, label %cleanup.46 [

;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; buff                              texture    byte         r/o      T0             t0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.ByteAddressBuffer = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?buff@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4

; Function Attrs: nounwind
define void @main() #0 {
entry:
  %0 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?buff@@3UByteAddressBuffer@@A", !dbg !17 ; line:5 col:7
  %1 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.ByteAddressBuffer)"(i32 0, %struct.ByteAddressBuffer %0), !dbg !17 ; line:5 col:7
  %2 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.ByteAddressBuffer)"(i32 14, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 11, i32 0 }, %struct.ByteAddressBuffer undef), !dbg !17 ; line:5 col:7
  %3 = call i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle %2, i32 0), !dbg !17 ; line:5 col:7
  %tobool = icmp ne i32 %3, 0, !dbg !17 ; line:5 col:7
  br i1 %tobool, label %return, label %while.body, !dbg !17 ; line:5 col:7

while.body:                                       ; preds = %while.body.3, %while.body, %cleanup.46, %entry
  %j.0 = phi i32 [ 0, %entry ], [ %j.1, %cleanup.46 ], [ %j.0, %while.body ], [ %j.1, %while.body.3 ]
  %i.0 = phi i32 [ 0, %entry ], [ %i.1, %cleanup.46 ], [ %i.0, %while.body ], [ %i.1, %while.body.3 ]
  %cmp = icmp slt i32 %i.0, 2, !dbg !21 ; line:12 col:17
  %cond = icmp eq i32 %i.0, 0, !dbg !22 ; line:13 col:5
  br i1 %cond, label %while.body.3, label %while.body, !dbg !22 ; line:13 col:5

while.body.3:                                     ; preds = %cleanup.46.thread, %while.body, %cleanup.46
  %j.1 = phi i32 [ %j.1, %cleanup.46 ], [ %j.0, %while.body ], [ %add, %cleanup.46.thread ]
  %i.1 = phi i32 [ %i.1, %cleanup.46 ], [ %i.0, %while.body ], [ %i.1, %cleanup.46.thread ]
  %cmp4 = icmp slt i32 %j.1, 2, !dbg !23 ; line:16 col:23
  br i1 %cmp4, label %while.body.11, label %while.body, !dbg !24 ; line:17 col:15

while.body.11:                                    ; preds = %while.body.3, %cleanup
  br label %while.body.13, !dbg !25 ; line:23 col:13

while.body.13:                                    ; preds = %while.body.13, %while.body.11
  br i1 %cmp, label %while.body.20, label %while.body.13, !dbg !26 ; line:24 col:19

while.body.20:                                    ; preds = %while.body.13, %while.end.37
  br i1 %cmp4, label %cleanup, label %while.end.37, !dbg !27 ; line:28 col:21

while.end.37:                                     ; preds = %while.body.20
  br i1 %cmp, label %cleanup, label %while.body.20, !dbg !28 ; line:39 col:19

cleanup:                                          ; preds = %while.end.37, %while.body.20
  %cleanup.dest.slot.0 = phi i32 [ 1, %while.body.20 ], [ 8, %while.end.37 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup.46 [
    i32 0, label %while.body.11
    i32 8, label %cleanup.46.thread
  ]

cleanup.46.thread:                                ; preds = %cleanup
  %add = add nsw i32 %j.1, 2, !dbg !29 ; line:43 col:18
  br label %while.body.3

cleanup.46:                                       ; preds = %cleanup
  switch i32 %cleanup.dest.slot.0, label %return [
    i32 0, label %while.body.3
    i32 6, label %while.body
  ]

return:                                           ; preds = %cleanup.46, %entry
  ret void, !dbg !30 ; line:48 col:1
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

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
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!10}
!dx.fnprops = !{!14}
!dx.options = !{!15, !16}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.4514 (d9bd2a706-dirty)"}
!3 = !{i32 1, i32 6}
!4 = !{i32 1, i32 8}
!5 = !{!"cs", i32 6, i32 6}
!6 = !{i32 1, void ()* @main, !7}
!7 = !{!8}
!8 = !{i32 1, !9, !9}
!9 = !{}
!10 = !{void ()* @main, !"main", null, !11, null}
!11 = !{!12, null, null, null}
!12 = !{!13}
!13 = !{i32 0, %struct.ByteAddressBuffer* @"\01?buff@@3UByteAddressBuffer@@A", !"buff", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!14 = !{void ()* @main, i32 5, i32 1, i32 1, i32 1}
!15 = !{i32 64}
!16 = !{i32 -1}
!17 = !DILocation(line: 5, column: 7, scope: !18)
!18 = !DISubprogram(name: "main", scope: !19, file: !19, line: 4, type: !20, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, function: void ()* @main)
!19 = !DIFile(filename: "/mnt/c/Users/amaiorano/Downloads/338103465/standalone_reduced.hlsl", directory: "")
!20 = !DISubroutineType(types: !9)
!21 = !DILocation(line: 12, column: 17, scope: !18)
!22 = !DILocation(line: 13, column: 5, scope: !18)
!23 = !DILocation(line: 16, column: 23, scope: !18)
!24 = !DILocation(line: 17, column: 15, scope: !18)
!25 = !DILocation(line: 23, column: 13, scope: !18)
!26 = !DILocation(line: 24, column: 19, scope: !18)
!27 = !DILocation(line: 28, column: 21, scope: !18)
!28 = !DILocation(line: 39, column: 19, scope: !18)
!29 = !DILocation(line: 43, column: 18, scope: !18)
!30 = !DILocation(line: 48, column: 1, scope: !18)
