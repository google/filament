// RUN: %dxc -T lib_6_x %S/node-object-export-1.hlsl -Fo %t.1
// RUN: %dxc -T lib_6_x %s -Fo %t.2
// RUN: %dxc -T lib_6_x -link "%t.1;%t.2" -Fc %t.ll
// RUN: FileCheck %s --input-file %t.ll --check-prefix=FOO2
// RUN: FileCheck %s --input-file %t.ll --check-prefix=BAR
// RUN: FileCheck %s --input-file %t.ll --check-prefix=BAR2
// RUN: FileCheck %s --input-file %t.ll --check-prefix=BAR3
// RUN: FileCheck %s --input-file %t.ll --check-prefix=BAR4
// RUN: FileCheck %s --input-file %t.ll --check-prefix=FOO

// Confirm that linking a shader calling external functions that take node objects as parameters
// correctly includes the external and internal functions

// Confirm that external function "foo2" is correctly included here even though it is called only by external functions
// FOO2: define void @"\01?foo2@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* noalias nocapture sret, %"struct.DispatchNodeInputRecord<RECORD>"* nocapture readonly) #{{[0-9]+}} {
// FOO2-NEXT:   %[[Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4
// FOO2-NEXT:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4

// Confirm that external function "bar" is correctly included here since it is called by bar3
// BAR: define void @"\01?bar@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* nocapture readonly, %"struct.DispatchNodeInputRecord<RECORD>"* noalias nocapture) #{{[0-9]+}} {
// BAR-NEXT:   %[[Alloca:.+]] = alloca %"struct.DispatchNodeInputRecord<RECORD>", align 8
// BAR-NEXT:   call void @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* nonnull sret %[[Alloca]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}})
// BAR-NEXT:   %[[Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %[[Alloca]], align 8
// BAR-NEXT:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4

// Confirm that external function "bar2" is correctly included here since it is called by bar4
// BAR2: define void @"\01?bar2@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* nocapture readonly, %"struct.DispatchNodeInputRecord<RECORD>"* noalias nocapture) #{{[0-9]+}} {
// BAR2-NEXT:   %[[Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4
// BAR2-NEXT:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4

// Confirm that internal function "bar3" is correctly included here and calls external function "foo"
// BAR3: define void @"\01?bar3@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"* noalias) #{{[0-9]+}} {
// BAR3-NEXT:   %[[Alloca:.+]] = alloca %"struct.DispatchNodeInputRecord<RECORD>", align 8
// BAR3-NEXT:   call void @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* nonnull sret %[[Alloca]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}) #{{[0-9]+}}
// BAR3-NEXT:   %[[Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %[[Alloca]], align 8
// BAR3-NEXT:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4

// Confirm that internal function "bar4" is correctly included here and calls outside function "bar2"
// BAR4: define void @"\01?bar4@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"*, %"struct.DispatchNodeInputRecord<RECORD>"* noalias) #{{[0-9]+}} {
// BAR4-NEXT:   call void @"\01?bar2@@YAXU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, %"struct.DispatchNodeInputRecord<RECORD>"* %1) #{{[0-9]+}}

// Confirm that external function "foo" is correctly included here even though it is called only by external functions
// FOO: define void @"\01?foo@@YA?AU?$DispatchNodeInputRecord@URECORD@@@@U1@@Z"(%"struct.DispatchNodeInputRecord<RECORD>"* noalias nocapture sret, %"struct.DispatchNodeInputRecord<RECORD>"* nocapture readonly) #{{[0-9]+}} {
// FOO-NEXT:   %[[Ld:.+]] = load %"struct.DispatchNodeInputRecord<RECORD>", %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4
// FOO-NEXT:   store %"struct.DispatchNodeInputRecord<RECORD>" %[[Ld]], %"struct.DispatchNodeInputRecord<RECORD>"* %{{.+}}, align 4


struct RECORD {
  int X;
};


void bar(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output);

[noinline]
void bar2(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output);

export
void bar3(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {
  bar(input, output);
}

export
void bar4(DispatchNodeInputRecord<RECORD> input, out DispatchNodeInputRecord<RECORD> output) {
  bar2(input, output);
}

