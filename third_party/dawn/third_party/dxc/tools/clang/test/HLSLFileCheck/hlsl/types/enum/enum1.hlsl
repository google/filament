// RUN: %dxc -E main -T ps_6_0 -HV 2017 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 2)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 3)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 4)

enum class MyEnum {
    FIRST,
    SECOND,
    THIRD,
    FOURTH,
};

int f(MyEnum v) {
    switch (v) {
        case MyEnum::FIRST:
            return 1;
        case MyEnum::SECOND:
            return 2;
        case MyEnum::THIRD:
            return 3;
        case MyEnum::FOURTH:
            return 4;
        default:
            return 0;
    }
}

int4 main() : SV_Target {
    return int4(f(MyEnum::FIRST), f(MyEnum::SECOND), f(MyEnum::THIRD), f(MyEnum::FOURTH));
}