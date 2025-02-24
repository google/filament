; RUN: %dxopt %s -hlsl-passes-resume -dxil-loop-deletion,NoSink=0 -S | FileCheck %s

; This test was generated from the following HLSL:
;
;  cbuffer cbuffer_g : register(b0) {
;    uint4 gu4[1];
;  };
;  
;  float4 f() {
;    float4 r = float4(0.0f, 0.0f, 0.0f, 0.0f);
;    int i = 0;
;    int j = 0;
;    while (true) {
;      float a = asfloat(gu4[0].y);
;      int ai = int(a);
;      bool b = (j < ai);
;      if (j >= ai) {
;        break;
;      }
;      bool c = (i > 0);
;      if (c) {
;        break;
;      } else {
;        bool3 b3 = bool3(b.xxx);
;        if (b3[i]) {
;          switch(i) {
;            case 0: return r;
;            case -1: return r;
;          }
;          if (c) {
;            break;
;          }
;        } else {
;          r = float4(0.0f, 0.0f, 0.0f, a);
;        }
;      }
;      i = j;
;      j = (j + 1);
;    }
;    r = (0.0f).xxxx;
;    return r;
;  }
;  
;  struct return_val {
;    float4 value : SV_Target0;
;  };
;  
;  return_val main() {
;    float4 inner_result = f();
;    return_val wrapper_result = (return_val)0;
;    wrapper_result.value = inner_result;
;    return wrapper_result;
;  }
;
; When compiling the above with dxc, ASAN reported a use-after-free in simplifycfg,
; which originated from a delete during the dxil-loop-deletion pass. This was due
; to a bug in LoopDeletion::runOnLoop that did not properly handle updated PHIs
; with duplicate input preds. After this test runs, the loop should be deleted,
; and the program optimized to simply write out 0s to the cbuffer.

; CHECK: define void @main
; CHECK-NEXT: entry:
; CHECK-NEXT:  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)
; CHECK-NEXT:  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00)
; CHECK-NEXT:  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00)
; CHECK-NEXT:  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00)
; CHECK-NEXT:  ret void

;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
; cbuffer cbuffer_g
; {
;
;   struct cbuffer_g
;   {
;
;       uint4 gu4[1];                                 ; Offset:    0
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
%struct.return_val = type { <4 x float> }

@cbuffer_g = external constant %cbuffer_g
@.hca = internal unnamed_addr constant [3 x i32] [i32 1, i32 1, i32 1]
@llvm.used = appending global [1 x i8*] [i8* bitcast (%cbuffer_g* @cbuffer_g to i8*)], section "llvm.metadata"

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %cbuffer_g*, i32)"(i32, %cbuffer_g*, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_g)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %cbuffer_g) #0

; Function Attrs: nounwind
define void @main(<4 x float>* noalias nocapture readnone) #1 {
entry:
  %1 = load %cbuffer_g, %cbuffer_g* @cbuffer_g, align 4, !dbg !25 ; line:45 col:25
  %cbuffer_g = call %dx.types.Handle @dx.op.createHandleForLib.cbuffer_g(i32 160, %cbuffer_g %1), !dbg !25 ; line:45 col:25  ; CreateHandleForLib(Resource)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %cbuffer_g, %dx.types.ResourceProperties { i32 13, i32 16 }), !dbg !25 ; line:45 col:25  ; AnnotateHandle(res,props)  resource: CBuffer
  %3 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %2, i32 0), !dbg !29 ; line:10 col:23  ; CBufferLoadLegacy(handle,regIndex)
  %4 = extractvalue %dx.types.CBufRet.i32 %3, 1, !dbg !29 ; line:10 col:23
  %5 = bitcast i32 %4 to float, !dbg !32 ; line:10 col:15
  %conv.i.6 = fptosi float %5 to i32, !dbg !33 ; line:11 col:18
  %cmp1.i.8 = icmp sgt i32 %conv.i.6, 0, !dbg !34 ; line:13 col:11
  br i1 %cmp1.i.8, label %if.end.i, label %"\01?f@@YA?AV?$vector@M$03@@XZ.exit", !dbg !35 ; line:13 col:9

if.end.i:                                         ; preds = %entry, %if.end.19.i
  %6 = phi float [ %9, %if.end.19.i ], [ %5, %entry ]
  %j.i.011 = phi i32 [ %add.i, %if.end.19.i ], [ 0, %entry ]
  %r.i.0.i310 = phi float [ %r.i.0.i310, %if.end.19.i ], [ 0.000000e+00, %entry ]
  %i.i.09 = phi i32 [ %j.i.011, %if.end.19.i ], [ 0, %entry ]
  %cmp4.i = icmp sgt i32 %i.i.09, 0, !dbg !36 ; line:16 col:17
  br i1 %cmp4.i, label %"\01?f@@YA?AV?$vector@M$03@@XZ.exit", label %if.then.12.i, !dbg !37 ; line:17 col:9

if.then.12.i:                                     ; preds = %if.end.i
  switch i32 %i.i.09, label %if.end.19.i [
    i32 0, label %"\01?f@@YA?AV?$vector@M$03@@XZ.exit"
    i32 -1, label %"\01?f@@YA?AV?$vector@M$03@@XZ.exit"
  ], !dbg !38 ; line:22 col:9

if.end.19.i:                                      ; preds = %if.then.12.i
  %add.i = add nuw nsw i32 %j.i.011, 1, !dbg !39 ; line:34 col:12
  %7 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %2, i32 0), !dbg !29 ; line:10 col:23  ; CBufferLoadLegacy(handle,regIndex)
  %8 = extractvalue %dx.types.CBufRet.i32 %7, 1, !dbg !29 ; line:10 col:23
  %9 = bitcast i32 %8 to float, !dbg !32 ; line:10 col:15
  %conv.i = fptosi float %9 to i32, !dbg !33 ; line:11 col:18
  %cmp.i = icmp slt i32 %add.i, %conv.i, !dbg !40 ; line:12 col:17
  br i1 %cmp.i, label %if.end.i, label %"\01?f@@YA?AV?$vector@M$03@@XZ.exit", !dbg !35 ; line:13 col:9

"\01?f@@YA?AV?$vector@M$03@@XZ.exit":             ; preds = %if.then.12.i, %if.then.12.i, %if.end.i, %if.end.19.i, %entry
  %retval.i.0.i3 = phi float [ 0.000000e+00, %entry ], [ %r.i.0.i310, %if.then.12.i ], [ %r.i.0.i310, %if.then.12.i ], [ 0.000000e+00, %if.end.i ], [ 0.000000e+00, %if.end.19.i ]
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00), !dbg !41 ; line:48 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00), !dbg !41 ; line:48 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00), !dbg !41 ; line:48 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %retval.i.0.i3), !dbg !41 ; line:48 col:10  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void, !dbg !42 ; line:48 col:3
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.cbuffer_g(i32, %cbuffer_g) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.resources = !{!6}
!dx.typeAnnotations = !{!9, !14}
!dx.entryPoints = !{!21}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.4514 (d9bd2a706-dirty)"}
!3 = !{i32 1, i32 5}
!4 = !{i32 1, i32 8}
!5 = !{!"ps", i32 6, i32 5}
!6 = !{null, null, !7, null}
!7 = !{!8}
!8 = !{i32 0, %cbuffer_g* @cbuffer_g, !"cbuffer_g", i32 0, i32 0, i32 1, i32 16, null}
!9 = !{i32 0, %struct.return_val undef, !10, %cbuffer_g undef, !12}
!10 = !{i32 16, !11}
!11 = !{i32 6, !"value", i32 3, i32 0, i32 4, !"SV_Target0", i32 7, i32 9}
!12 = !{i32 16, !13}
!13 = !{i32 6, !"gu4", i32 3, i32 0, i32 7, i32 5}
!14 = !{i32 1, void (<4 x float>*)* @main, !15}
!15 = !{!16, !18}
!16 = !{i32 0, !17, !17}
!17 = !{}
!18 = !{i32 1, !19, !20}
!19 = !{i32 4, !"SV_Target0", i32 7, i32 9}
!20 = !{i32 0}
!21 = !{void (<4 x float>*)* @main, !"main", !22, !6, null}
!22 = !{null, !23, null}
!23 = !{!24}
!24 = !{i32 0, !"SV_Target", i8 9, i8 16, !20, i8 0, i32 1, i8 4, i32 0, i8 0, null}
!25 = !DILocation(line: 45, column: 25, scope: !26)
!26 = !DISubprogram(name: "main", scope: !27, file: !27, line: 44, type: !28, isLocal: false, isDefinition: true, scopeLine: 44, flags: DIFlagPrototyped, isOptimized: false, function: void (<4 x float>*)* @main)
!27 = !DIFile(filename: "/mnt/c/Users/amaiorano/Downloads/340196361/standalone_reduced.hlsl", directory: "")
!28 = !DISubroutineType(types: !17)
!29 = !DILocation(line: 10, column: 23, scope: !30, inlinedAt: !31)
!30 = !DISubprogram(name: "f", scope: !27, file: !27, line: 5, type: !28, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false)
!31 = distinct !DILocation(line: 45, column: 25, scope: !26)
!32 = !DILocation(line: 10, column: 15, scope: !30, inlinedAt: !31)
!33 = !DILocation(line: 11, column: 18, scope: !30, inlinedAt: !31)
!34 = !DILocation(line: 13, column: 11, scope: !30, inlinedAt: !31)
!35 = !DILocation(line: 13, column: 9, scope: !30, inlinedAt: !31)
!36 = !DILocation(line: 16, column: 17, scope: !30, inlinedAt: !31)
!37 = !DILocation(line: 17, column: 9, scope: !30, inlinedAt: !31)
!38 = !DILocation(line: 22, column: 9, scope: !30, inlinedAt: !31)
!39 = !DILocation(line: 34, column: 12, scope: !30, inlinedAt: !31)
!40 = !DILocation(line: 12, column: 17, scope: !30, inlinedAt: !31)
!41 = !DILocation(line: 48, column: 10, scope: !26)
!42 = !DILocation(line: 48, column: 3, scope: !26)
