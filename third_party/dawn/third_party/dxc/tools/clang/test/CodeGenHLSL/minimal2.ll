; RUN: opt -simplify-inst -S < %s | FileCheck %s

%"class.Texture3D<unsigned int>" = type { i32, %"class.Texture3D<unsigned int>::mips_type" }
%"class.Texture3D<unsigned int>::mips_type" = type { i32 }
%dx.types.Handle = type { i8* }
%dx.types.ResourceProperties = type { i32, i32 }

@"\01?t1@@3V?$Texture3D@I@@A" = external global %"class.Texture3D<unsigned int>", align 4

; Function Attrs: nounwind
define void @main(<2 x i32> %dtid, i32 %laneID) #0 {
"\01?f1@@s1@@U1@@Z.exit":
  br label %if.end.13

while.cond.6.preheader:                           ; No predecessors!
  ; CHECK: %0 = extractelement <2 x i32> %1, i32 1
  %0 = extractelement <2 x i32> %2, i32 1
  %cmp7.14 = icmp ne i32 %0, 0
  br i1 %cmp7.14, label %while.body, label %if.end.13

while.body:                                       ; preds = %while.cond.6.preheader
  %1 = extractelement <2 x i32> %2, i32 0
  ; CHECK: %sub = sub i32 %sub, 1
  ; CHECK: %1 = insertelement <2 x i32> %1, i32 %sub, i32 0
  %sub = sub i32 %1, 1
  %2 = insertelement <2 x i32> %2, i32 %sub, i32 0
  br label %if.end.13

if.end.13:                                        ; preds = %while.body, %while.cond.6.preheader, %"\01?f1@@s1@@U1@@Z.exit"
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #0

; Function Attrs: nounwind readnone
declare i1 @"dx.hl.op.rn.i1 (i32, i1)"(i32, i1) #1

; Function Attrs: nounwind readonly
declare i32 @"dx.hl.op.ro.i32 (i32, %dx.types.Handle, <4 x i32>)"(i32, %dx.types.Handle, <4 x i32>) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<unsigned int>\22)"(i32, %"class.Texture3D<unsigned int>") #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<unsigned int>\22)"(i32, %dx.types.Handle, %dx.types.ResourceProperties, %"class.Texture3D<unsigned int>") #1
