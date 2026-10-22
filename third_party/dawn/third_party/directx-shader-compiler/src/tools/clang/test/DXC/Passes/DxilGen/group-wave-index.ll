; REQUIRES: dxil-1-10
; RUN: %dxopt %s -hlsl-passes-resume -dxilgen -S | FileCheck %s

; CHECK: call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)
; CHECK: call i32 @dx.op.getGroupWaveCount(i32 -2147483646)

; Generated from:
; utils/hct/ExtractIRForPassTest.py -p dxilgen -o tools/clang/test/DXC/Passes/DxilGen/group-wave-index.ll tools/clang/test/HLSLFileCheckLit/hlsl/intrinsics/wave/group-wave-index.hlsl -- -T cs_6_10 -E main
; Debug info manually stripped.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWStructuredBuffer<unsigned int>" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?output0@@3V?$RWStructuredBuffer@I@@A" = external global %"class.RWStructuredBuffer<unsigned int>", align 4

; Function Attrs: nounwind
define void @main(<3 x i32> %id) #0 {
entry:
  %0 = call i32 @"dx.hl.op.rn.i32 (i32)"(i32 392)
  %1 = call i32 @"dx.hl.op.rn.i32 (i32)"(i32 391)
  %2 = load %"class.RWStructuredBuffer<unsigned int>", %"class.RWStructuredBuffer<unsigned int>"* @"\01?output0@@3V?$RWStructuredBuffer@I@@A"
  %3 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 0, %"class.RWStructuredBuffer<unsigned int>" %2)
  %4 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 14, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 4108, i32 4 }, %"class.RWStructuredBuffer<unsigned int>" zeroinitializer)
  %5 = call i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %4, i32 0)
  store i32 %0, i32* %5
  %6 = load %"class.RWStructuredBuffer<unsigned int>", %"class.RWStructuredBuffer<unsigned int>"* @"\01?output0@@3V?$RWStructuredBuffer@I@@A"
  %7 = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 0, %"class.RWStructuredBuffer<unsigned int>" %6)
  %8 = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32 14, %dx.types.Handle %7, %dx.types.ResourceProperties { i32 4108, i32 4 }, %"class.RWStructuredBuffer<unsigned int>" zeroinitializer)
  %9 = call i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %8, i32 16)
  store i32 %1, i32* %9
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind readnone
declare i32 @"dx.hl.op.rn.i32 (i32)"(i32) #1

; Function Attrs: nounwind readnone
declare i32* @"dx.hl.subscript.[].rn.i32* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32, %"class.RWStructuredBuffer<unsigned int>") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<unsigned int>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.RWStructuredBuffer<unsigned int>") #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!3}
!dx.shaderModel = !{!4}
!dx.typeAnnotations = !{!5, !11}
!dx.entryPoints = !{!18}
!dx.fnprops = !{!23}
!dx.options = !{!24, !25}

!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.8.0.5134 (Group-Wave-Intrinsics, 84e7262d3)"}
!3 = !{i32 1, i32 10}
!4 = !{!"cs", i32 6, i32 10}
!5 = !{i32 0, %"class.RWStructuredBuffer<unsigned int>" undef, !6}
!6 = !{i32 4, !7, !8}
!7 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 5}
!8 = !{i32 0, !9}
!9 = !{!10}
!10 = !{i32 0, i32 undef}
!11 = !{i32 1, void (<3 x i32>)* @main, !12}
!12 = !{!13, !15}
!13 = !{i32 1, !14, !14}
!14 = !{}
!15 = !{i32 0, !16, !17}
!16 = !{i32 4, !"SV_DispatchThreadID", i32 7, i32 5, i32 13, i32 3}
!17 = !{i32 0}
!18 = !{void (<3 x i32>)* @main, !"main", null, !19, null}
!19 = !{null, !20, null, null}
!20 = !{!21}
!21 = !{i32 0, %"class.RWStructuredBuffer<unsigned int>"* @"\01?output0@@3V?$RWStructuredBuffer@I@@A", !"output0", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !22}
!22 = !{i32 1, i32 4}
!23 = !{void (<3 x i32>)* @main, i32 5, i32 1, i32 1, i32 1}
!24 = !{i32 64}
!25 = !{i32 -1}
