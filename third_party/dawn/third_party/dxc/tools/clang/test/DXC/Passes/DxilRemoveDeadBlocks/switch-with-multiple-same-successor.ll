; RUN: %dxopt %s -hlsl-passes-resume -dxil-remove-dead-blocks -S | FileCheck %s

; Validate that a switch with a constant condition and multiple of the same successor
; is correctly removed, ensuring that PHIs in the successor are properly updated.
; For instance, in:
;
;
; if.end.1:                                         ; preds = %for.inc
;   switch i32 1, label %sw.epilog.1 [
;     i32 1, label %dx.struct_exit.new_exiting.1
;     i32 2, label %dx.struct_exit.new_exiting.1
;     i32 3, label %dx.struct_exit.new_exiting.1
;     i32 5, label %dx.struct_exit.new_exiting.1
;   ], !dbg !31 ; line:23 col:5
; 
; sw.epilog.1:                                      ; preds = %if.end.1
;   br label %dx.struct_exit.new_exiting.1
; 
; dx.struct_exit.new_exiting.1:                     ; preds = %sw.epilog.1, %if.end.1, %if.end.1, %if.end.1, %if.end.1, %for.inc
;   %dx.struct_exit.prop.1 = phi i32 [ %do_discard.2, %sw.epilog.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %for.inc ]
;   %do_discard.2.1 = phi i32 [ 1, %sw.epilog.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ %do_discard.2, %for.inc ]
;   %g.2.i0.1 = phi float [ 0x3FF921FB60000000, %sw.epilog.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ %g.2.i0, %for.inc ]
;   br i1 false, label %cleanup, label %for.inc.1
;
;
; After dxil-remove-dead-blocks, the multiple `%if.end.1` in preds and in the two phi instructions should be removed,
; and only one instance should be left.

; CHECK:      if.end.1:                                         ; preds = %for.inc
; CHECK-NEXT:   br label %dx.struct_exit.new_exiting.1

; CHECK:     dx.struct_exit.new_exiting.1:                     ; preds = %if.end.1, %for.inc
; CHECK-NEXT:  %do_discard.2.1 = phi i32 [ 1, %if.end.1 ], [ %do_discard.2, %for.inc ]
; CHECK-NEXT:  %g.2.i0.1 = phi float [ 0x3FF921FB60000000, %if.end.1 ], [ %g.2.i0, %for.inc ]

;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; g_buff                            texture    byte         r/o      T0             t0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%struct.ByteAddressBuffer = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%struct.retval = type { <4 x float> }

@"\01?g_buff@@3UByteAddressBuffer@@A" = external global %struct.ByteAddressBuffer, align 4
@llvm.used = appending global [1 x i8*] [i8* bitcast (%struct.ByteAddressBuffer* @"\01?g_buff@@3UByteAddressBuffer@@A" to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
define void @main(<4 x float>* noalias nocapture readnone) #0 {
for.body.lr.ph:
  %1 = load %struct.ByteAddressBuffer, %struct.ByteAddressBuffer* @"\01?g_buff@@3UByteAddressBuffer@@A", align 4, !dbg !23 ; line:15 col:22
  %2 = call %dx.types.Handle @dx.op.createHandleForLib.struct.ByteAddressBuffer(i32 160, %struct.ByteAddressBuffer %1), !dbg !23 ; line:15 col:22  ; CreateHandleForLib(Resource)
  %3 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 11, i32 0 }), !dbg !23 ; line:15 col:22  ; AnnotateHandle(res,props)  resource: ByteAddressBuffer
  %RawBufferLoad = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %3, i32 0, i32 undef, i8 15, i32 4), !dbg !23 ; line:15 col:22  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
  %4 = extractvalue %dx.types.ResRet.i32 %RawBufferLoad, 0, !dbg !23 ; line:15 col:22
  %.i0 = bitcast i32 %4 to float, !dbg !27 ; line:15 col:14
  br label %for.body, !dbg !28 ; line:18 col:3

for.body:                                         ; preds = %for.body.lr.ph
  %cmp3 = fcmp fast une float %.i0, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3, label %dx.struct_exit.new_exiting, label %if.end, !dbg !30 ; line:19 col:9

if.end:                                           ; preds = %for.body
  switch i32 0, label %sw.epilog [
    i32 1, label %dx.struct_exit.new_exiting
    i32 2, label %dx.struct_exit.new_exiting
    i32 3, label %dx.struct_exit.new_exiting
    i32 5, label %dx.struct_exit.new_exiting
  ], !dbg !31 ; line:23 col:5

sw.epilog:                                        ; preds = %if.end
  br label %dx.struct_exit.new_exiting

dx.struct_exit.new_exiting:                       ; preds = %sw.epilog, %if.end, %if.end, %if.end, %if.end, %for.body
  %do_discard.2 = phi i32 [ 0, %for.body ], [ 1, %if.end ], [ 1, %if.end ], [ 1, %if.end ], [ 1, %if.end ], [ 1, %sw.epilog ]
  %g.2.i0 = phi float [ %.i0, %for.body ], [ 0x3FF921FB60000000, %if.end ], [ 0x3FF921FB60000000, %if.end ], [ 0x3FF921FB60000000, %if.end ], [ 0x3FF921FB60000000, %if.end ], [ 0x3FF921FB60000000, %sw.epilog ]
  br i1 false, label %cleanup, label %for.inc

for.inc:                                          ; preds = %dx.struct_exit.new_exiting
  %cmp3.1 = fcmp fast une float %g.2.i0, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.1, label %dx.struct_exit.new_exiting.1, label %if.end.1, !dbg !30 ; line:19 col:9

cleanup:                                          ; preds = %for.inc.9, %dx.struct_exit.new_exiting.9, %dx.struct_exit.new_exiting.8, %dx.struct_exit.new_exiting.7, %dx.struct_exit.new_exiting.6, %dx.struct_exit.new_exiting.5, %dx.struct_exit.new_exiting.4, %dx.struct_exit.new_exiting.3, %dx.struct_exit.new_exiting.2, %dx.struct_exit.new_exiting.1, %dx.struct_exit.new_exiting
  %do_discard.3 = phi i32 [ 0, %dx.struct_exit.new_exiting ], [ %dx.struct_exit.prop.1, %dx.struct_exit.new_exiting.1 ], [ %dx.struct_exit.prop.2, %dx.struct_exit.new_exiting.2 ], [ %dx.struct_exit.prop.3, %dx.struct_exit.new_exiting.3 ], [ %dx.struct_exit.prop.4, %dx.struct_exit.new_exiting.4 ], [ %dx.struct_exit.prop.5, %dx.struct_exit.new_exiting.5 ], [ %dx.struct_exit.prop.6, %dx.struct_exit.new_exiting.6 ], [ %dx.struct_exit.prop.7, %dx.struct_exit.new_exiting.7 ], [ %dx.struct_exit.prop.8, %dx.struct_exit.new_exiting.8 ], [ %dx.struct_exit.prop.9, %dx.struct_exit.new_exiting.9 ], [ %do_discard.2.9, %for.inc.9 ]
  %tobool15 = icmp eq i32 %do_discard.3, 0, !dbg !32 ; line:49 col:7
  br i1 %tobool15, label %if.end.17, label %if.then.16, !dbg !32 ; line:49 col:7

if.then.16:                                       ; preds = %cleanup
  call void @dx.op.discard(i32 82, i1 true), !dbg !33 ; line:49 col:19  ; Discard(condition)
  br label %if.end.17, !dbg !34 ; line:51 col:3

if.end.17:                                        ; preds = %cleanup, %if.then.16
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00), !dbg !35 ; line:53 col:18  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00), !dbg !35 ; line:53 col:18  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00), !dbg !35 ; line:53 col:18  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00), !dbg !35 ; line:53 col:18  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void, !dbg !36 ; line:54 col:1

if.end.1:                                         ; preds = %for.inc
  switch i32 1, label %sw.epilog.1 [
    i32 1, label %dx.struct_exit.new_exiting.1
    i32 2, label %dx.struct_exit.new_exiting.1
    i32 3, label %dx.struct_exit.new_exiting.1
    i32 5, label %dx.struct_exit.new_exiting.1
  ], !dbg !31 ; line:23 col:5

sw.epilog.1:                                      ; preds = %if.end.1
  br label %dx.struct_exit.new_exiting.1

dx.struct_exit.new_exiting.1:                     ; preds = %sw.epilog.1, %if.end.1, %if.end.1, %if.end.1, %if.end.1, %for.inc
  %dx.struct_exit.prop.1 = phi i32 [ %do_discard.2, %sw.epilog.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %if.end.1 ], [ 0, %for.inc ]
  %do_discard.2.1 = phi i32 [ 1, %sw.epilog.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ 1, %if.end.1 ], [ %do_discard.2, %for.inc ]
  %g.2.i0.1 = phi float [ 0x3FF921FB60000000, %sw.epilog.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ 0x3FF921FB60000000, %if.end.1 ], [ %g.2.i0, %for.inc ]
  br i1 false, label %cleanup, label %for.inc.1

for.inc.1:                                        ; preds = %dx.struct_exit.new_exiting.1
  %cmp3.2 = fcmp fast une float %g.2.i0.1, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.2, label %dx.struct_exit.new_exiting.2, label %if.end.2, !dbg !30 ; line:19 col:9

if.end.2:                                         ; preds = %for.inc.1
  switch i32 2, label %sw.epilog.2 [
    i32 1, label %dx.struct_exit.new_exiting.2
    i32 2, label %dx.struct_exit.new_exiting.2
    i32 3, label %dx.struct_exit.new_exiting.2
    i32 5, label %dx.struct_exit.new_exiting.2
  ], !dbg !31 ; line:23 col:5

sw.epilog.2:                                      ; preds = %if.end.2
  br label %dx.struct_exit.new_exiting.2

dx.struct_exit.new_exiting.2:                     ; preds = %sw.epilog.2, %if.end.2, %if.end.2, %if.end.2, %if.end.2, %for.inc.1
  %dx.struct_exit.prop.2 = phi i32 [ %do_discard.2.1, %sw.epilog.2 ], [ 0, %if.end.2 ], [ 0, %if.end.2 ], [ 0, %if.end.2 ], [ 0, %if.end.2 ], [ 0, %for.inc.1 ]
  %do_discard.2.2 = phi i32 [ 1, %sw.epilog.2 ], [ 1, %if.end.2 ], [ 1, %if.end.2 ], [ 1, %if.end.2 ], [ 1, %if.end.2 ], [ %do_discard.2.1, %for.inc.1 ]
  %g.2.i0.2 = phi float [ 0x3FF921FB60000000, %sw.epilog.2 ], [ 0x3FF921FB60000000, %if.end.2 ], [ 0x3FF921FB60000000, %if.end.2 ], [ 0x3FF921FB60000000, %if.end.2 ], [ 0x3FF921FB60000000, %if.end.2 ], [ %g.2.i0.1, %for.inc.1 ]
  br i1 false, label %cleanup, label %for.inc.2

for.inc.2:                                        ; preds = %dx.struct_exit.new_exiting.2
  %cmp3.3 = fcmp fast une float %g.2.i0.2, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.3, label %dx.struct_exit.new_exiting.3, label %if.end.3, !dbg !30 ; line:19 col:9

if.end.3:                                         ; preds = %for.inc.2
  switch i32 3, label %sw.epilog.3 [
    i32 1, label %dx.struct_exit.new_exiting.3
    i32 2, label %dx.struct_exit.new_exiting.3
    i32 3, label %dx.struct_exit.new_exiting.3
    i32 5, label %dx.struct_exit.new_exiting.3
  ], !dbg !31 ; line:23 col:5

sw.epilog.3:                                      ; preds = %if.end.3
  br label %dx.struct_exit.new_exiting.3

dx.struct_exit.new_exiting.3:                     ; preds = %sw.epilog.3, %if.end.3, %if.end.3, %if.end.3, %if.end.3, %for.inc.2
  %dx.struct_exit.prop.3 = phi i32 [ %do_discard.2.2, %sw.epilog.3 ], [ 0, %if.end.3 ], [ 0, %if.end.3 ], [ 0, %if.end.3 ], [ 0, %if.end.3 ], [ 0, %for.inc.2 ]
  %do_discard.2.3 = phi i32 [ 1, %sw.epilog.3 ], [ 1, %if.end.3 ], [ 1, %if.end.3 ], [ 1, %if.end.3 ], [ 1, %if.end.3 ], [ %do_discard.2.2, %for.inc.2 ]
  %g.2.i0.3 = phi float [ 0x3FF921FB60000000, %sw.epilog.3 ], [ 0x3FF921FB60000000, %if.end.3 ], [ 0x3FF921FB60000000, %if.end.3 ], [ 0x3FF921FB60000000, %if.end.3 ], [ 0x3FF921FB60000000, %if.end.3 ], [ %g.2.i0.2, %for.inc.2 ]
  br i1 false, label %cleanup, label %for.inc.3

for.inc.3:                                        ; preds = %dx.struct_exit.new_exiting.3
  %cmp3.4 = fcmp fast une float %g.2.i0.3, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.4, label %dx.struct_exit.new_exiting.4, label %if.end.4, !dbg !30 ; line:19 col:9

if.end.4:                                         ; preds = %for.inc.3
  switch i32 4, label %sw.epilog.4 [
    i32 1, label %dx.struct_exit.new_exiting.4
    i32 2, label %dx.struct_exit.new_exiting.4
    i32 3, label %dx.struct_exit.new_exiting.4
    i32 5, label %dx.struct_exit.new_exiting.4
  ], !dbg !31 ; line:23 col:5

sw.epilog.4:                                      ; preds = %if.end.4
  br label %dx.struct_exit.new_exiting.4

dx.struct_exit.new_exiting.4:                     ; preds = %sw.epilog.4, %if.end.4, %if.end.4, %if.end.4, %if.end.4, %for.inc.3
  %dx.struct_exit.prop.4 = phi i32 [ %do_discard.2.3, %sw.epilog.4 ], [ 0, %if.end.4 ], [ 0, %if.end.4 ], [ 0, %if.end.4 ], [ 0, %if.end.4 ], [ 0, %for.inc.3 ]
  %do_discard.2.4 = phi i32 [ 1, %sw.epilog.4 ], [ 1, %if.end.4 ], [ 1, %if.end.4 ], [ 1, %if.end.4 ], [ 1, %if.end.4 ], [ %do_discard.2.3, %for.inc.3 ]
  %g.2.i0.4 = phi float [ 0x3FF921FB60000000, %sw.epilog.4 ], [ 0x3FF921FB60000000, %if.end.4 ], [ 0x3FF921FB60000000, %if.end.4 ], [ 0x3FF921FB60000000, %if.end.4 ], [ 0x3FF921FB60000000, %if.end.4 ], [ %g.2.i0.3, %for.inc.3 ]
  br i1 false, label %cleanup, label %for.inc.4

for.inc.4:                                        ; preds = %dx.struct_exit.new_exiting.4
  %cmp3.5 = fcmp fast une float %g.2.i0.4, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.5, label %dx.struct_exit.new_exiting.5, label %if.end.5, !dbg !30 ; line:19 col:9

if.end.5:                                         ; preds = %for.inc.4
  switch i32 5, label %sw.epilog.5 [
    i32 1, label %dx.struct_exit.new_exiting.5
    i32 2, label %dx.struct_exit.new_exiting.5
    i32 3, label %dx.struct_exit.new_exiting.5
    i32 5, label %dx.struct_exit.new_exiting.5
  ], !dbg !31 ; line:23 col:5

sw.epilog.5:                                      ; preds = %if.end.5
  br label %dx.struct_exit.new_exiting.5

dx.struct_exit.new_exiting.5:                     ; preds = %sw.epilog.5, %if.end.5, %if.end.5, %if.end.5, %if.end.5, %for.inc.4
  %dx.struct_exit.prop.5 = phi i32 [ %do_discard.2.4, %sw.epilog.5 ], [ 0, %if.end.5 ], [ 0, %if.end.5 ], [ 0, %if.end.5 ], [ 0, %if.end.5 ], [ 0, %for.inc.4 ]
  %do_discard.2.5 = phi i32 [ 1, %sw.epilog.5 ], [ 1, %if.end.5 ], [ 1, %if.end.5 ], [ 1, %if.end.5 ], [ 1, %if.end.5 ], [ %do_discard.2.4, %for.inc.4 ]
  %g.2.i0.5 = phi float [ 0x3FF921FB60000000, %sw.epilog.5 ], [ 0x3FF921FB60000000, %if.end.5 ], [ 0x3FF921FB60000000, %if.end.5 ], [ 0x3FF921FB60000000, %if.end.5 ], [ 0x3FF921FB60000000, %if.end.5 ], [ %g.2.i0.4, %for.inc.4 ]
  br i1 false, label %cleanup, label %for.inc.5

for.inc.5:                                        ; preds = %dx.struct_exit.new_exiting.5
  %cmp3.6 = fcmp fast une float %g.2.i0.5, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.6, label %dx.struct_exit.new_exiting.6, label %if.end.6, !dbg !30 ; line:19 col:9

if.end.6:                                         ; preds = %for.inc.5
  switch i32 6, label %sw.epilog.6 [
    i32 1, label %dx.struct_exit.new_exiting.6
    i32 2, label %dx.struct_exit.new_exiting.6
    i32 3, label %dx.struct_exit.new_exiting.6
    i32 5, label %dx.struct_exit.new_exiting.6
  ], !dbg !31 ; line:23 col:5

sw.epilog.6:                                      ; preds = %if.end.6
  br label %dx.struct_exit.new_exiting.6

dx.struct_exit.new_exiting.6:                     ; preds = %sw.epilog.6, %if.end.6, %if.end.6, %if.end.6, %if.end.6, %for.inc.5
  %dx.struct_exit.prop23.6 = phi i1 [ true, %sw.epilog.6 ], [ false, %if.end.6 ], [ false, %if.end.6 ], [ false, %if.end.6 ], [ false, %if.end.6 ], [ false, %for.inc.5 ]
  %dx.struct_exit.prop.6 = phi i32 [ %do_discard.2.5, %sw.epilog.6 ], [ 0, %if.end.6 ], [ 0, %if.end.6 ], [ 0, %if.end.6 ], [ 0, %if.end.6 ], [ 0, %for.inc.5 ]
  %do_discard.2.6 = phi i32 [ 1, %sw.epilog.6 ], [ 1, %if.end.6 ], [ 1, %if.end.6 ], [ 1, %if.end.6 ], [ 1, %if.end.6 ], [ %do_discard.2.5, %for.inc.5 ]
  %g.2.i0.6 = phi float [ 0x3FF921FB60000000, %sw.epilog.6 ], [ 0x3FF921FB60000000, %if.end.6 ], [ 0x3FF921FB60000000, %if.end.6 ], [ 0x3FF921FB60000000, %if.end.6 ], [ 0x3FF921FB60000000, %if.end.6 ], [ %g.2.i0.5, %for.inc.5 ]
  br i1 %dx.struct_exit.prop23.6, label %cleanup, label %for.inc.6

for.inc.6:                                        ; preds = %dx.struct_exit.new_exiting.6
  %cmp3.7 = fcmp fast une float %g.2.i0.6, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.7, label %dx.struct_exit.new_exiting.7, label %if.end.7, !dbg !30 ; line:19 col:9

if.end.7:                                         ; preds = %for.inc.6
  switch i32 7, label %sw.epilog.7 [
    i32 1, label %dx.struct_exit.new_exiting.7
    i32 2, label %dx.struct_exit.new_exiting.7
    i32 3, label %dx.struct_exit.new_exiting.7
    i32 5, label %dx.struct_exit.new_exiting.7
  ], !dbg !31 ; line:23 col:5

sw.epilog.7:                                      ; preds = %if.end.7
  br label %dx.struct_exit.new_exiting.7

dx.struct_exit.new_exiting.7:                     ; preds = %sw.epilog.7, %if.end.7, %if.end.7, %if.end.7, %if.end.7, %for.inc.6
  %dx.struct_exit.prop.7 = phi i32 [ %do_discard.2.6, %sw.epilog.7 ], [ 0, %if.end.7 ], [ 0, %if.end.7 ], [ 0, %if.end.7 ], [ 0, %if.end.7 ], [ 0, %for.inc.6 ]
  %do_discard.2.7 = phi i32 [ 1, %sw.epilog.7 ], [ 1, %if.end.7 ], [ 1, %if.end.7 ], [ 1, %if.end.7 ], [ 1, %if.end.7 ], [ %do_discard.2.6, %for.inc.6 ]
  %g.2.i0.7 = phi float [ 0x3FF921FB60000000, %sw.epilog.7 ], [ 0x3FF921FB60000000, %if.end.7 ], [ 0x3FF921FB60000000, %if.end.7 ], [ 0x3FF921FB60000000, %if.end.7 ], [ 0x3FF921FB60000000, %if.end.7 ], [ %g.2.i0.6, %for.inc.6 ]
  br i1 false, label %cleanup, label %for.inc.7

for.inc.7:                                        ; preds = %dx.struct_exit.new_exiting.7
  %cmp3.8 = fcmp fast une float %g.2.i0.7, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.8, label %dx.struct_exit.new_exiting.8, label %if.end.8, !dbg !30 ; line:19 col:9

if.end.8:                                         ; preds = %for.inc.7
  switch i32 8, label %sw.epilog.8 [
    i32 1, label %dx.struct_exit.new_exiting.8
    i32 2, label %dx.struct_exit.new_exiting.8
    i32 3, label %dx.struct_exit.new_exiting.8
    i32 5, label %dx.struct_exit.new_exiting.8
  ], !dbg !31 ; line:23 col:5

sw.epilog.8:                                      ; preds = %if.end.8
  br label %dx.struct_exit.new_exiting.8

dx.struct_exit.new_exiting.8:                     ; preds = %sw.epilog.8, %if.end.8, %if.end.8, %if.end.8, %if.end.8, %for.inc.7
  %dx.struct_exit.prop.8 = phi i32 [ %do_discard.2.7, %sw.epilog.8 ], [ 0, %if.end.8 ], [ 0, %if.end.8 ], [ 0, %if.end.8 ], [ 0, %if.end.8 ], [ 0, %for.inc.7 ]
  %do_discard.2.8 = phi i32 [ 1, %sw.epilog.8 ], [ 1, %if.end.8 ], [ 1, %if.end.8 ], [ 1, %if.end.8 ], [ 1, %if.end.8 ], [ %do_discard.2.7, %for.inc.7 ]
  %g.2.i0.8 = phi float [ 0x3FF921FB60000000, %sw.epilog.8 ], [ 0x3FF921FB60000000, %if.end.8 ], [ 0x3FF921FB60000000, %if.end.8 ], [ 0x3FF921FB60000000, %if.end.8 ], [ 0x3FF921FB60000000, %if.end.8 ], [ %g.2.i0.7, %for.inc.7 ]
  br i1 false, label %cleanup, label %for.inc.8

for.inc.8:                                        ; preds = %dx.struct_exit.new_exiting.8
  %cmp3.9 = fcmp fast une float %g.2.i0.8, 0.000000e+00, !dbg !29 ; line:19 col:13
  br i1 %cmp3.9, label %dx.struct_exit.new_exiting.9, label %if.end.9, !dbg !30 ; line:19 col:9

if.end.9:                                         ; preds = %for.inc.8
  switch i32 9, label %sw.epilog.9 [
    i32 1, label %dx.struct_exit.new_exiting.9
    i32 2, label %dx.struct_exit.new_exiting.9
    i32 3, label %dx.struct_exit.new_exiting.9
    i32 5, label %dx.struct_exit.new_exiting.9
  ], !dbg !31 ; line:23 col:5

sw.epilog.9:                                      ; preds = %if.end.9
  br label %dx.struct_exit.new_exiting.9

dx.struct_exit.new_exiting.9:                     ; preds = %sw.epilog.9, %if.end.9, %if.end.9, %if.end.9, %if.end.9, %for.inc.8
  %dx.struct_exit.prop.9 = phi i32 [ %do_discard.2.8, %sw.epilog.9 ], [ 0, %if.end.9 ], [ 0, %if.end.9 ], [ 0, %if.end.9 ], [ 0, %if.end.9 ], [ 0, %for.inc.8 ]
  %do_discard.2.9 = phi i32 [ 1, %sw.epilog.9 ], [ 1, %if.end.9 ], [ 1, %if.end.9 ], [ 1, %if.end.9 ], [ 1, %if.end.9 ], [ %do_discard.2.8, %for.inc.8 ]
  br i1 false, label %cleanup, label %for.inc.9

for.inc.9:                                        ; preds = %dx.struct_exit.new_exiting.9
  br label %cleanup
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

; Function Attrs: nounwind
declare void @dx.op.discard(i32, i1) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32, %dx.types.Handle, i32, i32, i8, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.struct.ByteAddressBuffer(i32, %struct.ByteAddressBuffer) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }
attributes #2 = { nounwind readnone }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.typeAnnotations = !{!9, !12}
!dx.entryPoints = !{!19}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.4514 (d9bd2a706-dirty)"}
!3 = !{i32 1, i32 6}
!4 = !{i32 1, i32 8}
!5 = !{!"ps", i32 6, i32 6}
!6 = !{!7, null, null, null}
!7 = !{!8}
!8 = !{i32 0, %struct.ByteAddressBuffer* @"\01?g_buff@@3UByteAddressBuffer@@A", !"g_buff", i32 0, i32 0, i32 1, i32 11, i32 0, null}
!9 = !{i32 0, %struct.retval undef, !10}
!10 = !{i32 16, !11}
!11 = !{i32 6, !"value", i32 3, i32 0, i32 4, !"SV_Target0", i32 7, i32 9}
!12 = !{i32 1, void (<4 x float>*)* @main, !13}
!13 = !{!14, !16}
!14 = !{i32 0, !15, !15}
!15 = !{}
!16 = !{i32 1, !17, !18}
!17 = !{i32 4, !"SV_Target0", i32 7, i32 9}
!18 = !{i32 0}
!19 = !{void (<4 x float>*)* @main, !"main", !20, !6, null}
!20 = !{null, !21, null}
!21 = !{!22}
!22 = !{i32 0, !"SV_Target", i8 9, i8 16, !18, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!23 = !DILocation(line: 15, column: 22, scope: !24)
!24 = !DISubprogram(name: "main", scope: !25, file: !25, line: 14, type: !26, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: false, function: void (<4 x float>*)* @main)
!25 = !DIFile(filename: "/home/amaiorano/src/external/DirectXShaderCompiler/tools/clang/test/DXC/Passes/DxilRemoveDeadBlocks/switch-with-multiple-same-successor.hlsl", directory: "")
!26 = !DISubroutineType(types: !15)
!27 = !DILocation(line: 15, column: 14, scope: !24)
!28 = !DILocation(line: 18, column: 3, scope: !24)
!29 = !DILocation(line: 19, column: 13, scope: !24)
!30 = !DILocation(line: 19, column: 9, scope: !24)
!31 = !DILocation(line: 23, column: 5, scope: !24)
!32 = !DILocation(line: 49, column: 7, scope: !24)
!33 = !DILocation(line: 49, column: 19, scope: !24)
!34 = !DILocation(line: 51, column: 3, scope: !24)
!35 = !DILocation(line: 53, column: 18, scope: !24)
!36 = !DILocation(line: 54, column: 1, scope: !24)
