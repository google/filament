// RUN: %dxc -T lib_6_x -fcgl %s | FileCheck %s  --check-prefixes=FCGL,CHECK
// RUN: %dxc -T lib_6_x -Zi %s | FileCheck %s  --check-prefix=DBG
// RUN: %dxc -T lib_6_x %s | FileCheck %s  --check-prefixes=CHECK,DXIL
// RUN: %dxc -T lib_6_x -Od %s | FileCheck %s  --check-prefixes=CHECK,DXIL

// Confirm that linking a shader calling external functions that take node objects as parameters
// correctly includes the external and internal functions

struct RECORD {
  int X;
};

// CHECK: define void @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* noalias {{(nocapture )?}}sret %agg.result, %"struct.DispatchNodeInputRecord<RECORD>"* {{(nocapture readonly )?}}%input)
[noinline]
DispatchNodeInputRecord<RECORD> foo(DispatchNodeInputRecord<RECORD> input) {
// CHECK:  %[[FooLd:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %input
// CHECK:  store %"struct.DispatchNodeInputRecord<RECORD>" %[[FooLd]], %"struct.DispatchNodeInputRecord<RECORD>"* %agg.result

// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %input, metadata ![[FOOINPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression() func:"foo"
  return input;
}

// CHECK:   define void @"\01?bar@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* {{(nocapture readonly )?}}%input, %"struct.DispatchNodeInputRecord<RECORD>"* noalias {{(nocapture )?}}%output)
export
void bar(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {

// CHECK: %[[TMP:.+]] = alloca %"struct.DispatchNodeInputRecord<RECORD>"{{(, align 8)?}}
// CHECK: call void @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* {{(nonnull |noalias )?}}sret %[[TMP]], %"struct.DispatchNodeInputRecord<RECORD>"* %input)
// CHECK: %[[BarLd:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %[[TMP]]{{(, align 8)?}}
// CHECK: store %"struct.DispatchNodeInputRecord<RECORD>" %[[BarLd]], %"struct.DispatchNodeInputRecord<RECORD>"* %output{{(, align 4)?}}

// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %output, metadata ![[BAROUTPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression() func:"bar"
// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %input, metadata ![[BARINPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression() func:"bar"

  output = foo(input);
}

// CHECK:   define void @"\01?foo2@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* noalias {{(nocapture )?}}sret %agg.result, %"struct.DispatchNodeInputRecord<RECORD>"* {{(nocapture readonly )?}}%input)
DispatchNodeInputRecord<RECORD> foo2(DispatchNodeInputRecord<RECORD> input) {
// CHECK: %[[Foo2Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %input{{(, align 4)?}}
// CHECK: store %"struct.DispatchNodeInputRecord<RECORD>" %[[Foo2Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %agg.result{{(, align 4)?}}

// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %input, metadata ![[FOO2INPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression() func:"foo2"

  return input;
}

// CHECK:   define void @"\01?bar2@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* {{(nocapture readonly )?}}%input, %"struct.DispatchNodeInputRecord<RECORD>"* noalias {{(nocapture )?}}%output)
[noinline]
export
void bar2(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {
// FCGL: %[[TMP:.+]] = alloca %"struct.DispatchNodeInputRecord<RECORD>", align 4
// FCGL: call void @"\01?foo2@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* sret %[[TMP]], %"struct.DispatchNodeInputRecord<RECORD>"* %input)
// FCGL: %[[Bar2Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %[[TMP]]
// FCGL: store %"struct.DispatchNodeInputRecord<RECORD>" %[[Bar2Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %output

// DXIL:   %[[Bar2Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %input, {{(align 4, )?}}!noalias
// DXIL:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Bar2Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %output{{(, align 4, )?}}

// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %output, metadata ![[BAR2OUTPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression() func:"bar2"
// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %input, metadata ![[BAR2INPUT:[0-9]+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression() func:"bar2"
// DBG: call void @llvm.dbg.declare(metadata %"struct.DispatchNodeInputRecord<RECORD>"* %input, metadata ![[FOO2INPUT]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression() func:"foo2"

  output = foo2(input);
}


// DBG checks debug info to make sure node object types are correctly saved for function parameters and local variable.
// DBG: ![[Foo:[0-9]+]] = !DISubprogram(name: "foo", linkageName: "\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z", scope: !1, file: !1, line: {{[0-9]+}}, type: ![[FooTy:[0-9]+]], isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: void (%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"*)* @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z")
// DBG: ![[FooTy]] = !DISubroutineType(types: ![[FooTys:[0-9]+]])
// DBG: ![[FooTys]] = !{![[ObjTy:[0-9]+]], ![[ObjTy]]}
// DBG: ![[ObjTy]] = !DICompositeType(tag: DW_TAG_structure_type, name: "DispatchNodeInputRecord<RECORD>", file: !1, size: 32, align: 32, elements: !2, templateParams: ![[TemplateParams:[0-9]+]])
// DBG: ![[TemplateParams]] = !{![[TemplateParam:[0-9]+]]}
// DBG: ![[TemplateParam]] = !DITemplateTypeParameter(name: "recordtype", type: ![[RECORD:[0-9]+]])
// DBG: ![[RECORD]] = !DICompositeType(tag: DW_TAG_structure_type, name: "RECORD", file: !1, line: {{[0-9]+}}, size: 32, align: 32, elements: ![[RecordElts:[0-9]+]])
// DBG: ![[RecordElts]] = !{![[RecordElt:[0-9]+]]}
// DBG: ![[RecordElt]] = !DIDerivedType(tag: DW_TAG_member, name: "X", scope: ![[RECORD]], file: !1, line: {{[0-9]+}}, baseType: ![[INT:[0-9]+]], size: 32, align: 32)
// DBG: ![[INT]] = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
// DBG: ![[Bar:[0-9]+]] = !DISubprogram(name: "bar", linkageName: "\01?bar@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z", scope: !1, file: !1, line: {{[0-9]+}}, type: ![[BarTy:[0-9]+]], isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: void (%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"*)* @"\01?bar@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z")
// DBG: ![[BarTy]] = !DISubroutineType(types: ![[BarTys:[0-9]+]])
// DBG: ![[BarTys]] = !{null, ![[ObjTy]], ![[OutObjTy:[0-9]+]]}
// DBG: ![[OutObjTy]] = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: ![[ObjTy]])
// DBG: ![[Foo2:[0-9]+]] = !DISubprogram(name: "foo2", linkageName: "\01?foo2@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z", scope: !1, file: !1, line: {{[0-9]+}}, type: ![[FooTy]], isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: void (%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"*)* @"\01?foo2@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z")
// DBG: ![[Bar2:[0-9]+]] = !DISubprogram(name: "bar2", linkageName: "\01?bar2@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z", scope: !1, file: !1, line: {{[0-9]+}}, type: ![[BarTy]], isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: void (%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"*)* @"\01?bar2@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z")
// DBG: ![[FOOINPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "input", arg: 1, scope: ![[Foo]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
// DBG: ![[BAROUTPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "output", arg: 2, scope: ![[Bar]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
// DBG: ![[BARINPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "input", arg: 1, scope: ![[Bar]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
// DBG: ![[FOO2INPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "input", arg: 1, scope: ![[Foo2]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
// DBG: ![[BAR2OUTPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "output", arg: 2, scope: ![[Bar2]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
// DBG: ![[BAR2INPUT]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "input", arg: 1, scope: ![[Bar2]], file: !1, line: {{[0-9]+}}, type: ![[ObjTy]])
