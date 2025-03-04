// RUN: %dxc -E main -T vs_6_0 -HV 2021 %s | FileCheck %s

// Tests declarations and uses of anonymous structs.

// CHECK: call i32 @dx.op.loadInput.i32
// CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK: add nsw i32
// CHECK: call void @dx.op.storeOutput.i32

typedef struct { int x; } typedefed;

struct { int x; } global;
struct Outer
{
    struct { int x; } field;
};

int main(Outer input : IN) : OUT
{
    struct { int x; } local = input.field;
    typedefed retval = (typedefed)local;
    return retval.x + global.x;
}
