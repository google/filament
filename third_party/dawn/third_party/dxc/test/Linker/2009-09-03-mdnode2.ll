; This file is used by 2009-09-03-mdnode.ll, so it doesn't actually do anything itself
;
; RUN: true

define i32 @f(...) nounwind {
entry:
  %retval = alloca i32                            ; <i32*> [#uses=1]
  call void @llvm.dbg.func.start(metadata !0)
  br label %return

return:                                           ; preds = %entry
  %0 = load i32, i32* %retval                          ; <i32> [#uses=1]
  call void @llvm.dbg.stoppoint(i32 3, i32 1, metadata !1)
  call void @llvm.dbg.region.end(metadata !0)
  ret i32 %0
}

declare void @llvm.dbg.func.start(metadata) nounwind readnone

declare void @llvm.dbg.stoppoint(i32, i32, metadata) nounwind readnone

declare void @llvm.dbg.region.end(metadata) nounwind readnone

!0 = !DISubprogram(name: "f", linkageName: "f", line: 1, isLocal: false, isDefinition: true, virtualIndex: 6, isOptimized: false, scope: !1)
!1 = !DICompileUnit(language: DW_LANG_C99, producer: "ellcc 0.1.0", isOptimized: true, emissionKind: 0, file: !2)
!2 = !DIFile(filename: "b.c", directory: "/home/rich/ellcc/test/source")
