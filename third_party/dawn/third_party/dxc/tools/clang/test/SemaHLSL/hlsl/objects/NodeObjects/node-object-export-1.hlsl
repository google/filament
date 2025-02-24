// RUN: %dxc -T lib_6_x -ast-dump %s | FileCheck %s  --check-prefix=AST

struct RECORD {
  int X;
};

// AST: FunctionDecl 0x[[FOO:[0-9a-f]+]] {{.+}} used foo 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)'
// AST: `-NoInlineAttr
[noinline]
DispatchNodeInputRecord<RECORD> foo(DispatchNodeInputRecord<RECORD> input) {
  return input;
}

// AST:FunctionDecl 0x{{.+}} bar 'void (DispatchNodeInputRecord<RECORD>, __restrict DispatchNodeInputRecord<RECORD>)'
// AST: | |-ParmVarDecl 0x[[BarInput:[0-9a-f]+]] <col:10, col:42> col:42 used input 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST: | |-ParmVarDecl 0x[[BarOutput:[0-9a-f]+]] <col:49, col:85> col:85 used output '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>'
// AST: | | `-HLSLOutAttr
export
void bar(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {
// AST: | |-CompoundStmt
// AST: | | `-BinaryOperator 0x{{.+}} '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>' '='
// AST: | |   |-DeclRefExpr 0x{{.+}} <col:3> '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[BarOutput]] 'output' '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>'
// AST: | |   `-CallExpr {{.+}} <col:12, col:21> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST: | |     |-ImplicitCastExpr 0x{{.+}} <col:12> 'DispatchNodeInputRecord<RECORD> (*)(DispatchNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST: | |     | `-DeclRefExpr 0x{{.+}} <col:12> 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)' lvalue Function 0x[[FOO]] 'foo' 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)'
// AST: | |     `-ImplicitCastExpr 0x{{.+}} <col:16> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' <LValueToRValue>
// AST: | |       `-DeclRefExpr 0x{{.+}} <col:16> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[BarInput]] 'input' 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST: | `-HLSLExportAttr
  output = foo(input);
}

// AST:FunctionDecl 0x[[FOO2:[0-9a-f]+]] {{.+}} used foo2 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)'
DispatchNodeInputRecord<RECORD> foo2(DispatchNodeInputRecord<RECORD> input) {
  return input;
}

// AST:FunctionDecl 0x{{.+}} bar2 'void (DispatchNodeInputRecord<RECORD>, __restrict DispatchNodeInputRecord<RECORD>)'
// AST: ParmVarDecl 0x[[Bar2Input:[0-9a-f]+]] <col:11, col:43> col:43 used input 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST: ParmVarDecl 0x[[Bar2Output:[0-9a-f]+]] <col:50, col:86> col:86 used output '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>'
// AST: HLSLOutAttr
[noinline]
export
void bar2(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {
// AST:   |-CompoundStmt 0x{{.+}}
// AST:   | `-BinaryOperator 0x{{.+}} '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>' '='
// AST:   |   |-DeclRefExpr 0x{{.+}} <col:3> '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Bar2Output]] 'output' '__restrict DispatchNodeInputRecord<RECORD>':'__restrict DispatchNodeInputRecord<RECORD>'
// AST:   |   `-CallExpr 0x{{.+}} <col:12, col:22> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST:   |     |-ImplicitCastExpr 0x{{.+}} <col:12> 'DispatchNodeInputRecord<RECORD> (*)(DispatchNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST:   |     | `-DeclRefExpr 0x{{.+}} <col:12> 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)' lvalue Function 0x[[FOO2]] 'foo2' 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)'
// AST:   |     `-ImplicitCastExpr 0x{{.+}} <col:17> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' <LValueToRValue>
// AST:   |       `-DeclRefExpr 0x{{.+}} <col:17> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Bar2Input]] 'input' 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
  output = foo2(input);
}
// AST: NoInlineAttr
// AST: HLSLExportAttr

