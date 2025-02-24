// RUN: %dxc -E main -T ps_6_0 -HV 2017 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)

enum Vertex : int {
    FIRST,
    SECOND,
    THIRD
};

int4 getValueInt(int i) {
    switch (i) {
        case 0:
            return int4(1,1,1,1);
        case 1:
            return int4(2,2,2,2);
        case 2:
            return int4(3,3,3,3);
        default:
            return int4(0,0,0,0);
    }
}

int4 main(float4 col : COLOR) : SV_Target {
    return getValueInt(Vertex::FIRST);
}
