; RUN: not llvm-as -disable-output <%s 2>&1 | FileCheck %s
; CHECK: invalid llvm.dbg.declare intrinsic address/value
; CHECK-NEXT: call void @llvm.dbg.declare({{.*}})
; CHECK-NEXT: !""

define void @foo(i32 %a) {
entry:
  %s = alloca i32
  call void @llvm.dbg.declare(metadata !"", metadata !DILocalVariable(tag: DW_TAG_arg_variable, scope: !1), metadata !DIExpression()), !dbg !DILocation(scope: !1)
  ret void
}

declare void @llvm.dbg.declare(metadata, metadata, metadata)

!llvm.module.flags = !{!0}
!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !DISubprogram()
