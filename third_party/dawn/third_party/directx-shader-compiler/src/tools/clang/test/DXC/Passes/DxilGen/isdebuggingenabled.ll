; REQUIRES: dxil-1-10
; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; CHECK: call i1 @dx.op.isDebuggingEnabled(i32 -2147483614)
; CHECK: declare i1 @dx.op.isDebuggingEnabled(i32) #[[QUERY_ATTR:[0-9]+]]
; CHECK: attributes #[[QUERY_ATTR]] = { nounwind }

; Reduced high-level IR for dx::IsDebuggingEnabled().

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<unsigned int>" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?Output@@3V?$RWStructuredBuffer@I@@A" = external global %"class.RWStructuredBuffer<unsigned int>", align 4

; Function Attrs: nounwind
define void @main(<3 x i32> %threadId) #0 {
entry:
  %0 = call i1 @"dx.hl.op..i1 (i32)"(i32 421)
  br i1 %0, label %if.then, label %if.else

if.then:
  %1 = extractelement <3 x i32> %threadId, i32 0
  %2 = load %"class.RWStructuredBuffer<unsigned int>", %"class.RWStructuredBuffer<unsigned int>"* @"\01?Output@@3V?$RWStructuredBuffer@I@@A"
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 0, %"class.RWStructuredBuffer<unsigned int>" %2)
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4108, i32 4 }, %"class.RWStructuredBuffer<unsigned int>" undef)
  %5 = call i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %4, i32 %1)
  store i32 1, i32* %5
  br label %if.end

if.else:
  %6 = extractelement <3 x i32> %threadId, i32 0
  %7 = load %"class.RWStructuredBuffer<unsigned int>", %"class.RWStructuredBuffer<unsigned int>"* @"\01?Output@@3V?$RWStructuredBuffer@I@@A"
  %8 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 0, %"class.RWStructuredBuffer<unsigned int>" %7)
  %9 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 14, %dx.types.Handle %8, %dx.types.ResourceProperties { i32 4108, i32 4 }, %"class.RWStructuredBuffer<unsigned int>" undef)
  %10 = call i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %9, i32 %6)
  store i32 0, i32* %10
  br label %if.end

if.end:
  ret void
}

; Function Attrs: nounwind
declare i1 @"dx.hl.op..i1 (i32)"(i32) #1

; Function Attrs: nounwind readnone
declare i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32, %"class.RWStructuredBuffer<unsigned int>") #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<unsigned int>") #2

attributes #0 = { nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone }

!pauseresume = !{!0}
!llvm.ident = !{!1}
!dx.version = !{!2}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4, !10}
!dx.entryPoints = !{!16}
!dx.fnprops = !{!22}
!dx.options = !{!23, !24}

!0 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!1 = !{!"dxc(private) 1.9.0.5167 (Debug-Break, c89129305)"}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 10}
!4 = !{i32 0, %"class.RWStructuredBuffer<unsigned int>" undef, !5}
!5 = !{i32 4, !6, !7}
!6 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!7 = !{i32 0, !8}
!8 = !{!9}
!9 = !{i32 0, i32 undef}
!10 = !{i32 1, void (<3 x i32>)* @main, !11}
!11 = !{!12, !14}
!12 = !{i32 1, !13, !13}
!13 = !{}
!14 = !{i32 0, !15, !13}
!15 = !{i32 4, !"SV_DispatchThreadID", i32 7, i32 5, i32 13, i32 3}
!16 = !{void (<3 x i32>)* @main, !"main", null, !17, null}
!17 = !{null, !18, null, null}
!18 = !{!19}
!19 = !{i32 0, %"class.RWStructuredBuffer<unsigned int>"* @"\01?Output@@3V?$RWStructuredBuffer@I@@A", !"Output", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !20}
!20 = !{i32 1, i32 4}
!21 = !{i32 0}
!22 = !{void (<3 x i32>)* @main, i32 5, i32 8, i32 8, i32 1}
!23 = !{i32 -2147483584}
!24 = !{i32 -1}
