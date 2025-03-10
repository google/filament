// RUN: %dxc -Od -Zi -E main -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -Od -Zi -E main -T vs_6_0 %s | FileCheck %s -check-prefix=POSY
// RUN: %dxc -Od -Zi -E main -T vs_6_0 %s | FileCheck %s -check-prefix=UV0X
// RUN: %dxc -Od -Zi -E main -T vs_6_0 %s | FileCheck %s -check-prefix=UV1X
struct MyInput {
    float4 pos : POSITION0;
    float3 uv0 : TEXCOORD0;
    float3 uv1 : TEXCOORD1;
};
struct MyOutput {
    float4 pos : SV_POSITION;
    float3 uv0 : TEXCOORD0;
    float3 uv1 : TEXCOORD1;
};
MyOutput bar(MyOutput arg_bar) {
    arg_bar.pos *= arg_bar.pos;
    arg_bar.uv0 *= arg_bar.uv0;
    arg_bar.uv1 *= arg_bar.uv1;
    return arg_bar;
}

void foo(inout MyOutput arg_foo) {
    arg_foo = bar(arg_foo);
    arg_foo.pos *= arg_foo.pos;
    arg_foo.uv0 *= arg_foo.uv0;
    arg_foo.uv1 *= arg_foo.uv1;
}

MyOutput main(MyInput input) {
    MyOutput ret = (MyOutput)input;
    foo(ret);
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the value pos.x
    // CHECK-DAG: %[[pos0:.+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)

// Check it's annotated in "main" function
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression(DW_OP_bit_piece, 0, 32) func:"main"

// Check it's annotated in "foo" function
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 0, 32) func:"foo"

// Check it's annotated in "bar" function
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 0, 32) func:"bar"

// Mul 1
    // CHECK-DAG: %[[pos0_1:.+]] = fmul fast float %[[pos0]], %[[pos0]], !dbg !{{[0-9]+}} ; line:16

// Check that the variables' value has been updated
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 0, 32) func:"bar"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 0, 32) func:"foo"

// Mul 2
    // CHECK-DAG: %[[pos0_2:.+]] = fmul fast float %[[pos0_1]], %[[pos0_1]], !dbg !{{[0-9]+}} ; line:24

    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 0, 32) func:"foo"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret" !DIExpression(DW_OP_bit_piece, 0, 32) func:"main"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the value pos.y
    // POSY-DAG: %[[pos0:.+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)

// Check it's annotated in "main" function
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression(DW_OP_bit_piece, 32, 32) func:"main"

// Check it's annotated in "foo" function
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 32, 32) func:"foo"

// Check it's annotated in "bar" function
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 32, 32) func:"bar"

// Mul 1
    // POSY-DAG: %[[pos0_1:.+]] = fmul fast float %[[pos0]], %[[pos0]], !dbg !{{[0-9]+}} ; line:16

// Check that the variables' value has been updated
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 32, 32) func:"bar"
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 32, 32) func:"foo"

// Mul 2
    // POSY-DAG: %[[pos0_2:.+]] = fmul fast float %[[pos0_1]], %[[pos0_1]], !dbg !{{[0-9]+}} ; line:24

    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 32, 32) func:"foo"
    // POSY-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret" !DIExpression(DW_OP_bit_piece, 32, 32) func:"main"


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the value uv0.x
    // UV0X-DAG: %[[pos0:.+]] = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)

// Check it's annotated in "main" function
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression(DW_OP_bit_piece, 128, 32) func:"main"

// Check it's annotated in "foo" function
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 128, 32) func:"foo"

// Check it's annotated in "bar" function
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 128, 32) func:"bar"

// Mul 1
    // UV0X-DAG: %[[pos0_1:.+]] = fmul fast float %[[pos0]], %[[pos0]], !dbg !{{[0-9]+}} ; line:17

// Check that the variables' value has been updated
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 128, 32) func:"bar"
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 128, 32) func:"foo"

// Mul 2
    // UV0X-DAG: %[[pos0_2:.+]] = fmul fast float %[[pos0_1]], %[[pos0_1]], !dbg !{{[0-9]+}} ; line:25

    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 128, 32) func:"foo"
    // UV0X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret" !DIExpression(DW_OP_bit_piece, 128, 32) func:"main"


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// Load the value uv1.x
    // UV1X-DAG: %[[pos0:.+]] = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 undef)

// Check it's annotated in "main" function
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"input" !DIExpression(DW_OP_bit_piece, 224, 32) func:"main"

// Check it's annotated in "foo" function
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 224, 32) func:"foo"

// Check it's annotated in "bar" function
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 224, 32) func:"bar"

// Mul 1
    // UV1X-DAG: %[[pos0_1:.+]] = fmul fast float %[[pos0]], %[[pos0]], !dbg !{{[0-9]+}} ; line:18

// Check that the variables' value has been updated
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_bar" !DIExpression(DW_OP_bit_piece, 224, 32) func:"bar"
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 224, 32) func:"foo"

// Mul 2
    // UV1X-DAG: %[[pos0_2:.+]] = fmul fast float %[[pos0_1]], %[[pos0_1]], !dbg !{{[0-9]+}} ; line:26

    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg_foo" !DIExpression(DW_OP_bit_piece, 224, 32) func:"foo"
    // UV1X-DAG: call void @llvm.dbg.value(metadata float %[[pos0_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret" !DIExpression(DW_OP_bit_piece, 224, 32) func:"main"





