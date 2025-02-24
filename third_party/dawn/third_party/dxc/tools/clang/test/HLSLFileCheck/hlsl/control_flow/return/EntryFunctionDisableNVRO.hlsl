// RUN: %dxc /D PS /Tps_6_0 %s | FileCheck %s
// RUN: %dxc /D PS /Tlib_6_3 %s | FileCheck %s
// RUN: %dxc /D PS /Tps_6_0 -fcgl %s | FileCheck %s -check-prefix=FCGL
// RUN: %dxc /D VS /Tvs_6_0 %s | FileCheck %s
// RUN: %dxc /D VS /Tlib_6_3 %s | FileCheck %s
// RUN: %dxc /D VS /Tvs_6_0 -fcgl %s | FileCheck %s -check-prefix=FCGL
// RUN: %dxc /D DS /Tds_6_0 %s | FileCheck %s
// RUN: %dxc /D DS /Tlib_6_3 %s | FileCheck %s
// RUN: %dxc /D DS /Tds_6_0 -fcgl %s | FileCheck %s -check-prefix=FCGL
// RUN: %dxc /D HS /Ths_6_0 %s | FileCheck %s -check-prefix=HS
// RUN: %dxc /D HS /Tlib_6_3 %s | FileCheck %s -check-prefix=HS
// RUN: %dxc /D HS /Ths_6_0 -fcgl %s | FileCheck %s -check-prefix=HS_FCGL

// This test is for making sure we don't do NRVO for entry functions and patch constant functions.
// For the normal compile version, we check there's the right number of storeOutput calls.
// For the fcgl version, we check there is a return value alloca emitted.

// CHECK: call void @dx.op.storeOutput
// CHECK: call void @dx.op.storeOutput
// CHECK: call void @dx.op.storeOutput
// CHECK: call void @dx.op.storeOutput
// CHECK-NOT: call void @dx.op.storeOutput

// FCGL: alloca %struct.MyReturn

cbuffer cb : register(b0) {
    float4 foo;
    bool a, b, c, d, e, f, g;
}

#ifdef PS
struct MyReturn {
    float4 member : SV_Target;
};

[shader("pixel")]
MyReturn main() {
    MyReturn ret = (MyReturn)0;
    if (a) {
        ret.member = foo;
    }
    else if (b) {
        ret.member = foo * 2;
        if (c) {
            ret.member = foo * 4;
        }
        else if (d) {
            ret.member = foo * 8;
        }
    }
    return ret;
}

#elif defined(VS)

struct MyReturn {
    float4 member : SV_Position;
};

[shader("vertex")]
MyReturn main() {
    MyReturn ret = (MyReturn)0;
    if (a) {
        ret.member = foo;
    }
    else if (b) {
        ret.member = foo * 2;
        if (c) {
            ret.member = foo * 4;
        }
        else if (d) {
            ret.member = foo * 8;
        }
    }
    return ret;
}

#elif defined(DS)

struct MyReturn {
    float4 member : SV_Position;
};

[domain("tri")]
[shader("domain")]
MyReturn main() {
    MyReturn ret = (MyReturn)0;
    if (a) {
        ret.member = foo;
    }
    else if (b) {
        ret.member = foo * 2;
        if (c) {
            ret.member = foo * 4;
        }
        else if (d) {
            ret.member = foo * 8;
        }
    }
    return ret;
}

#elif defined(HS)

// HS-LABEL: @"\01?patch_const{{[@$?.A-Za-z0-9_]+}}"
// HS: call void @dx.op.storePatchConstant
// HS: call void @dx.op.storePatchConstant
// HS: call void @dx.op.storePatchConstant
// HS: call void @dx.op.storePatchConstant
// HS-NOT: call void @dx.op.storePatchConstant

// HS-LABEL: @main
// HS: call void @dx.op.storeOutput
// HS: call void @dx.op.storeOutput
// HS: call void @dx.op.storeOutput
// HS: call void @dx.op.storeOutput
// HS-NOT: call void @dx.op.storeOutput

// HS_FCGL-DAG: alloca %struct.MyReturn
// HS_FCGL-DAG: alloca %struct.MyPatchConstantReturn


struct MyPatchConstantReturn {
    float member[3] : SV_TessFactor0; // <-- with 0;
    float member2 : SV_InsideTessFactor;
};
MyPatchConstantReturn patch_const() {
    MyPatchConstantReturn ret = (MyPatchConstantReturn)0;
    if (a) {
        ret.member[0] = foo[0];
        ret.member[1] = foo[1];
        ret.member[2] = foo[2];
        ret.member2 = foo[3];
    }
    else if (b) {
        ret.member[0] = foo[0] * 2;
        ret.member[1] = foo[1] * 2;
        ret.member[2] = foo[2] * 2;
        ret.member2    = foo[3] * 2;
        if (c) {
            ret.member[0] = foo[0] * 4;
            ret.member[1] = foo[1] * 4;
            ret.member[2] = foo[2] * 4;
            ret.member2 = foo[3] * 4;
        }
        else if (d) {
            ret.member[0] = foo[0] * 8;
            ret.member[1] = foo[1] * 8;
            ret.member[2] = foo[2] * 8;
            ret.member2 = foo[3] * 8;
        }
    }
    return ret;
}

struct MyReturn {
    float4 member : SV_Position;
};

struct MyInput {
    float4 member : POSITION;
};

[domain("tri")]
[outputtopology("triangle_cw")]
[patchconstantfunc("patch_const")]
[partitioning("fractional_odd")]
[outputcontrolpoints(3)]
[maxtessfactor(9)]
[shader("hull")]
MyReturn main( const uint id : SV_OutputControlPointID,
               const InputPatch< MyInput, 3 > points )
{
    MyReturn ret = (MyReturn)0;
    if (a) {
        ret.member = points[id].member;
    }
    else if (b) {
        ret.member = points[id].member;
        if (c) {
            ret.member = points[id].member;
        }
        else if (d) {
            ret.member = points[id].member;
        }
    }
    return ret;
}

#endif
