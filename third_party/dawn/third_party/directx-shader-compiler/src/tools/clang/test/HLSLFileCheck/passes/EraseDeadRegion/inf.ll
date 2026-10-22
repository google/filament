; RUN: %opt %s -dxil-erase-dead-region -S | FileCheck %s

; CHECK: @main

; Regression test for an infinite loop in EraseDeadRegion.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%"class.RWBuffer<float>" = type { float }
%CBUF = type { i32, i32 }
%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }

@"\01?output@@3V?$RWBuffer@M@@A" = external global %"class.RWBuffer<float>", align 4
@CBUF = external constant %CBUF
@llvm.used = appending global [2 x i8*] [i8* bitcast (%"class.RWBuffer<float>"* @"\01?output@@3V?$RWBuffer@M@@A" to i8*), i8* bitcast (%CBUF* @CBUF to i8*)], section "llvm.metadata"

; Function Attrs: nounwind
define void @main(<4 x float>* noalias nocapture readnone, i32) #0 {
entry:
  %2 = load %"class.RWBuffer<float>", %"class.RWBuffer<float>"* @"\01?output@@3V?$RWBuffer@M@@A", align 4
  %3 = load %CBUF, %CBUF* @CBUF, align 4
  %CBUF = call %dx.types.Handle @dx.op.createHandleForLib.CBUF(i32 160, %CBUF %3)
  %4 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %5 = extractvalue %dx.types.CBufRet.i32 %4, 0
  %cond.i = icmp eq i32 %5, 0
  br i1 %cond.i, label %while.cond.i.preheader, label %"\01?f@@YAXXZ.exit"

while.cond.i.preheader:                           ; preds = %entry
  %6 = extractvalue %dx.types.CBufRet.i32 %4, 1
  %tobool2.i.1 = icmp eq i32 %6, 0
  br i1 %tobool2.i.1, label %for.inc.i, label %while.body.i.preheader

while.body.i.preheader:                           ; preds = %while.cond.i.preheader
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i.preheader, %while.cond.i.backedge
  %7 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %8 = extractvalue %dx.types.CBufRet.i32 %7, 1
  %cond6.i = icmp eq i32 %8, 2
  br i1 %cond6.i, label %sw.bb.3.i, label %while.cond.i.backedge

while.cond.i.backedge:                            ; preds = %sw.bb.3.i, %while.body.i, %if.then.i
  %9 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %10 = extractvalue %dx.types.CBufRet.i32 %9, 1
  %tobool2.i = icmp eq i32 %10, 0
  br i1 %tobool2.i, label %for.inc.i.loopexit, label %while.body.i

sw.bb.3.i:                                        ; preds = %while.body.i
  %11 = extractvalue %dx.types.CBufRet.i32 %7, 0
  %rem.i = and i32 %11, 1
  %tobool4.i = icmp eq i32 %rem.i, 0
  br i1 %tobool4.i, label %if.then.i, label %while.cond.i.backedge

if.then.i:                                        ; preds = %sw.bb.3.i
  %12 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<float>"(i32 160, %"class.RWBuffer<float>" %2)
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %12, i32 0, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, i8 15)
  br label %while.cond.i.backedge

for.inc.i.loopexit:                               ; preds = %while.cond.i.backedge
  br label %for.inc.i

for.inc.i:                                        ; preds = %for.inc.i.loopexit, %while.cond.i.preheader
  %13 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %14 = extractvalue %dx.types.CBufRet.i32 %13, 1
  %tobool2.i.1.1 = icmp eq i32 %14, 0
  br i1 %tobool2.i.1.1, label %for.inc.i.1, label %while.body.i.1.preheader

while.body.i.1.preheader:                         ; preds = %for.inc.i
  br label %while.body.i.1

"\01?f@@YAXXZ.exit.loopexit":                     ; preds = %while.cond.i.backedge.2
  br label %"\01?f@@YAXXZ.exit"

"\01?f@@YAXXZ.exit":                              ; preds = %"\01?f@@YAXXZ.exit.loopexit", %for.inc.i.1, %entry
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 2.000000e+00)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.000000e+00)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 4.000000e+00)
  ret void

while.body.i.1:                                   ; preds = %while.body.i.1.preheader, %while.cond.i.backedge.1
  %15 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %16 = extractvalue %dx.types.CBufRet.i32 %15, 1
  %cond6.i.1 = icmp eq i32 %16, 2
  br i1 %cond6.i.1, label %sw.bb.3.i.1, label %while.cond.i.backedge.1

sw.bb.3.i.1:                                      ; preds = %while.body.i.1
  %17 = extractvalue %dx.types.CBufRet.i32 %15, 0
  %rem.i.1 = and i32 %17, 1
  %tobool4.i.1 = icmp eq i32 %rem.i.1, 0
  br i1 %tobool4.i.1, label %if.then.i.1, label %while.cond.i.backedge.1

if.then.i.1:                                      ; preds = %sw.bb.3.i.1
  %18 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<float>"(i32 160, %"class.RWBuffer<float>" %2)
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %18, i32 1, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, i8 15)
  br label %while.cond.i.backedge.1

while.cond.i.backedge.1:                          ; preds = %sw.bb.3.i.1, %if.then.i.1, %while.body.i.1
  %19 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %20 = extractvalue %dx.types.CBufRet.i32 %19, 1
  %tobool2.i.1.3 = icmp eq i32 %20, 0
  br i1 %tobool2.i.1.3, label %for.inc.i.1.loopexit, label %while.body.i.1

for.inc.i.1.loopexit:                             ; preds = %while.cond.i.backedge.1
  br label %for.inc.i.1

for.inc.i.1:                                      ; preds = %for.inc.i.1.loopexit, %for.inc.i
  %21 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %22 = extractvalue %dx.types.CBufRet.i32 %21, 1
  %tobool2.i.1.2 = icmp eq i32 %22, 0
  br i1 %tobool2.i.1.2, label %"\01?f@@YAXXZ.exit", label %while.body.i.2.preheader

while.body.i.2.preheader:                         ; preds = %for.inc.i.1
  br label %while.body.i.2

while.body.i.2:                                   ; preds = %while.body.i.2.preheader, %while.cond.i.backedge.2
  %23 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %24 = extractvalue %dx.types.CBufRet.i32 %23, 1
  %cond6.i.2 = icmp eq i32 %24, 2
  br i1 %cond6.i.2, label %sw.bb.3.i.2, label %while.cond.i.backedge.2

sw.bb.3.i.2:                                      ; preds = %while.body.i.2
  %25 = extractvalue %dx.types.CBufRet.i32 %23, 0
  %rem.i.2 = and i32 %25, 1
  %tobool4.i.2 = icmp eq i32 %rem.i.2, 0
  br i1 %tobool4.i.2, label %if.then.i.2, label %while.cond.i.backedge.2

if.then.i.2:                                      ; preds = %sw.bb.3.i.2
  %26 = call %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<float>"(i32 160, %"class.RWBuffer<float>" %2)
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %26, i32 2, i32 undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, i8 15)
  br label %while.cond.i.backedge.2

while.cond.i.backedge.2:                          ; preds = %sw.bb.3.i.2, %if.then.i.2, %while.body.i.2
  %27 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %CBUF, i32 0)
  %28 = extractvalue %dx.types.CBufRet.i32 %27, 1
  %tobool2.i.2 = icmp eq i32 %28, 0
  br i1 %tobool2.i.2, label %"\01?f@@YAXXZ.exit.loopexit", label %while.body.i.2
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind
declare void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandleForLib.CBUF(i32, %CBUF) #2

; Function Attrs: nounwind readonly
declare %dx.types.Handle @"dx.op.createHandleForLib.class.RWBuffer<float>"(i32, %"class.RWBuffer<float>") #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!pauseresume = !{!0}
!llvm.ident = !{!1}

!0 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!1 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
