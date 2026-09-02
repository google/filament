// RUN: %dxc -HV 2021 -E main -T ps_6_0 -fcgl %s | FileCheck %s
// RUN: %dxc -HV 2021 -E main -T ps_6_0 -fcgl -Zi -Qembed_debug %s | FileCheck -check-prefixes=CHECK,DI %s

// The goal of this test is to verify the instantiations of `foo` are correctly
// generated _and_ to verify that the correct debug information is generated for
// the instantiations.


template<typename T>
struct Test {

T t;
T foo(T t1) {
  return t * t1;
}

};

float2 main(float4 a:A) : SV_Target {
  Test<float> t0;
  t0.t = a.x;
  Test<float2> t1;
  t1.t = a.xy;
  return t0.foo(a.y) + t1.foo(a.zw);
}


// CHECK: define internal float @"\01?foo@?$Test@{{[A-Z0-9@]+}}"
// CHECK-SAME: (%"struct.Test<float>"* [[this:%.*]], float [[t1:%.*]])

// CHECK: [[t1LocalAddr:%.*]] = alloca float,
// CHECK-NEXT: store float [[t1]], float* [[t1LocalAddr]]

// DI-NEXT: call void @llvm.dbg.declare(metadata float* [[t1LocalAddr]], metadata [[t1FVar:![0-9]+]]
// DI-NEXT: call void @llvm.dbg.declare(metadata %"struct.Test<float>"* [[this]], metadata [[thisFVar:![0-9]+]]

// CHECK-NEXT: [[this_tAddr:%.*]] = getelementptr inbounds %"struct.Test<float>", %"struct.Test<float>"* [[this]], i32 0, i32 0,
// CHECK-NEXT: [[this_t:%.*]] = load float, float* [[this_tAddr]],
// CHECK-NEXT: [[t1Local:%.*]] = load float, float* [[t1LocalAddr]],
// CHECK-NEXT: [[Res:%.*]] = fmul float [[this_t]], [[t1Local]]
// CHECK-NEXT: ret float [[Res]]


// CHECK: define internal <2 x float> @"\01?foo@?$Test@V?$vector{{[A-Za-z0-9@?$]+}}"
// CHECK-SAME: (%"struct.Test<vector<float, 2> >"* [[this:%.*]], <2 x float> [[t1:%.*]])

// CHECK: [[t1LocalAddr:%.*]] = alloca <2 x float>
// CHECK-NEXT: store <2 x float> [[t1]], <2 x float>* [[t1LocalAddr]]

// DI-NEXT: call void @llvm.dbg.declare(metadata <2 x float>* [[t1LocalAddr]], metadata [[t1VVar:![0-9]+]]
// DI-NEXT: call void @llvm.dbg.declare(metadata %"struct.Test<vector<float, 2> >"* [[this]], metadata [[thisVVar:![0-9]+]]

// CHECK-NEXT: [[this_tAddr:%.*]] = getelementptr inbounds %"struct.Test<vector<float, 2> >", %"struct.Test<vector<float, 2> >"* [[this]], i32 0, i32 0
// CHECK-NEXT: [[this_t:%.*]] = load <2 x float>, <2 x float>* [[this_tAddr]]
// CHECK-NEXT: [[t1Local:%.*]] = load <2 x float>, <2 x float>* [[t1LocalAddr]]
// CHECK-NEXT: [[Res:%.*]] = fmul <2 x float> [[this_t]], [[t1Local]]
// CHECK-NEXT  ret <2 x float> [[Res]]


// DI: [[DIFile:![0-9]+]] = !DIFile
// DI-DAG: [[DIFloat:![0-9]+]] = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)

// DI-DAG: [[FSub:![0-9]+]] = !DISubprogram(name: "foo", linkageName: "\01?foo@?$Test@{{[A-Z@0-9]+}}", scope: [[FloatScope:![0-9]+]], file: [[DIFile]], line: {{[0-9]+}}, type: {{![0-9]+}}, isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: float (%"struct.Test<float>"*, float)* @"\01?foo@?$Test@{{[A-Z@0-9]+}}",
// DI-DAG: [[DITestF:![0-9]+]] = !DICompositeType(tag: DW_TAG_structure_type, name: "Test<float>"

// DI-DAG: [[VSub:![0-9]+]] = !DISubprogram(name: "foo", linkageName: "\01?foo@?$Test@V?$vector{{[A-Za-z0-9@?$]+}}", scope: [[VecScope:![0-9]+]], file: [[DIFile]], line: {{[0-9]+}}, type: {{![0-9]+}}, isLocal: false, isDefinition: true, scopeLine: {{[0-9]+}}, flags: DIFlagPrototyped, isOptimized: false, function: <2 x float> (%"struct.Test<vector<float, 2> >"*, <2 x float>)* @"\01?foo@?$Test@V?$vector{{[A-Za-z0-9@?$]+}}",
// DI-DAG: [[DITestV:![0-9]+]] = !DICompositeType(tag: DW_TAG_structure_type, name: "Test<vector<float, 2> >", file: [[DIFile]], line: {{[0-9]+}}, size: 64, align: 32

// DI-DAG: [[t1FVar]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "t1", arg: 2, scope: [[FSub]], file: [[DIFile]], line: {{[0-9]+}}, type: [[DIFloat]])
// DI-DAG: [[thisFVar]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "this", arg: 1, scope: [[FSub]], type: [[DITestF]])

// DI-DAG: [[t1VVar]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "t1", arg: 2, scope: [[VSub]], file: [[DIFile]]
// DI-DAG: [[thisVVar]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "this", arg: 1, scope: [[VSub]], type: [[DITestV]])
