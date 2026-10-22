; RUN: opt -S -dxilgen %s | FileCheck %s

; Make sure we can correctly generate the buffer load when
; its gep goes through a single-value phi node.

; CHECK: exit:
; CHECK-NEXT: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32


target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque
%"class.StructuredBuffer<S>" = type { %struct.S }
%struct.S = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }
@"\01?buf@@3V?$StructuredBuffer@US@@@@A" = external global %"class.StructuredBuffer<S>", align 4

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define void @main(<4 x float>* noalias) #0 {
entry:
  %a = load %"class.StructuredBuffer<S>", %"class.StructuredBuffer<S>"* @"\01?buf@@3V?$StructuredBuffer@US@@@@A"
  %b = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<S>\22)"(i32 0, %"class.StructuredBuffer<S>" %a)
  %c = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<S>\22)"(i32 11, %dx.types.Handle %b, %dx.types.ResourceProperties { i32 524, i32 4 }, %"class.StructuredBuffer<S>" zeroinitializer)
  %d = call %struct.S* @"dx.hl.subscript.[].rn.%struct.S* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle %c, i32 0)
  %gep = getelementptr inbounds %struct.S, %struct.S* %d, i32 0, i32 0
  br label %exit

exit:
  %lcssa = phi i32* [ %gep, %entry ]
  %struct_buffer_load = load i32, i32* %lcssa, align 4

  ret void
}

; Function Attrs: nounwind readnone
declare %struct.S* @"dx.hl.subscript.[].rn.%struct.S* (i32, %dx.types.Handle, i32)"(i32, %dx.types.Handle, i32) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<S>\22)"(i32, %"class.StructuredBuffer<S>") #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<S>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.StructuredBuffer<S>") #0


attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!13}
!dx.fnprops = !{!17}
!dx.options = !{!18, !19}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"dxc(private) 1.7.0.14024 (main, 1d6b5627a)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 7}
!5 = !{!"ps", i32 6, i32 0}
!6 = !{i32 1, void (<4 x float>*)* @main, !7}
!7 = !{!8, !10}
!8 = !{i32 0, !9, !9}
!9 = !{}
!10 = !{i32 1, !11, !12}
!11 = !{i32 4, !"SV_Target", i32 7, i32 9}
!12 = !{i32 0}
!13 = !{void (<4 x float>*)* @main, !"main", null, !14, null}
!14 = !{null, null, !15, null}
!15 = !{!16}
!16 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!17 = !{void (<4 x float>*)* @main, i32 0, i1 false}
!18 = !{i32 152}
!19 = !{i32 -1}
!20 = !DILocation(line: 2, column: 3, scope: !21)
!21 = !DISubprogram(name: "main", scope: !22, file: !22, line: 1, type: !23, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, function: void (<4 x float>*)* @main)
!22 = !DIFile(filename: ".\5Ct2.hlsl", directory: "")
!23 = !DISubroutineType(types: !9)
