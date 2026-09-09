// RUN: %dxc -E main -T vs_6_0 -Zi -Od %s | FileCheck %s

// Test that SROA for local nested arrays of structs/vectors
// produces and preserves the extra metadata to express strides
// in the original user variable.

// CHECK-DAG: alloca [6 x float]
// CHECK-DAG: alloca [6 x float]
// CHECK-DAG: %[[a:.*]] = alloca [12 x i32]

// CHECK-DAG: call void @llvm.dbg.declare(metadata [12 x i32]* %[[a]], metadata !{{.*}}, metadata ![[aexpr:.*]]), !dbg !{{.*}}, !dx.dbg.varlayout ![[alayout:.*]]
// CHECK-DAG: call void @llvm.dbg.declare(metadata [6 x float]* %{{.*}}, metadata !{{.*}}, metadata !{{.*}}), !dbg !{{.*}}, !dx.dbg.varlayout !{{.*}}
// CHECK-DAG: call void @llvm.dbg.declare(metadata [6 x float]* %{{.*}}, metadata !{{.*}}, metadata !{{.*}}), !dbg !{{.*}}, !dx.dbg.varlayout !{{.*}}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK: !DILocalVariable(tag: DW_TAG_auto_variable, name: "var"

// Debug info for field a should include a contiguous chunk of 32*4=128 bits at offset 0,
// and the rest expressed as array stride metadata:
// CHECK-DAG: ![[aexpr]] = !DIExpression(DW_OP_bit_piece, 0, 128)
// CHECK-DAG: ![[alayout]] = !{i32 0, i32 256, i32 3}

// Debug info for b should be in two parts (b.x and b.y),
// it should have bit pieces for the first float,
// and have associated array stride metadata.

// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 128, 32)
// CHECK-DAG: !{i32 128, i32 256, i32 3, i32 64, i32 2}
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 160, 32)
// CHECK-DAG: !{i32 160, i32 256, i32 3, i32 64, i32 2}

typedef struct { int a[4]; float2 b[2]; } type[3];

int main() : OUT {
  type var = (type)0;
  return var[0].a[0];
}