// RUN: %dxc -T lib_6_6 -HV 2021 -disable-lifetime-markers -fcgl %s | FileCheck %s

// CHECK-LABEL: define linkonce_odr void @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(%class.Child* %this)
// CHECK: %[[OutArg:.+]] = alloca %class.Parent
// CHECK: %[[OutThisPtr:.+]] = getelementptr inbounds %class.Child, %class.Child* %this, i32 0, i32 0
// CHECK: call void @"\01?lib_func2{{[@$?.A-Za-z0-9_]+}}"(%class.Parent* %[[OutArg]])
// Make sure copy-out.
// CHECK: %[[OutThisCpyPtr:.+]] = bitcast %class.Parent* %[[OutThisPtr]] to i8*
// CHECK: %[[OutArgCpyPtr:.+]] = bitcast %class.Parent* %[[OutArg]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %[[OutThisCpyPtr]], i8* %[[OutArgCpyPtr]], i64 8, i32 1, i1 false)

// CHECK-LABEL: define linkonce_odr void @"\01?bar{{[@$?.A-Za-z0-9_]+}}"(%class.Child* %this)
// CHECK: %[[InOutArg:.+]] = alloca %class.Parent
// CHECK: %[[InOutThisPtr:.+]] = getelementptr inbounds %class.Child, %class.Child* %this, i32 0, i32 0

// Make sure copy-in.
// CHECK: %[[InOutArgCpyPtr:.+]] = bitcast %class.Parent* %[[InOutArg]] to i8*
// CHECK: %[[InOutThisCpyPtr:.+]] = bitcast %class.Parent* %[[InOutThisPtr]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %[[InOutArgCpyPtr]], i8* %[[InOutThisCpyPtr]], i64 8, i32 1, i1 false)

// The call.
// CHECK: call void @"\01?lib_func3{{[@$?.A-Za-z0-9_]+}}"(%class.Parent* %[[InOutArg]])

// Make sure copy-out.
// CHECK: %[[InOutThisCpyOutPtr:.+]] = bitcast %class.Parent* %[[InOutThisPtr]] to i8*
// CHECK: %[[InOutArgCpyOutPtr:.+]] = bitcast %class.Parent* %[[InOutArg]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %[[InOutThisCpyOutPtr]], i8* %[[InOutArgCpyOutPtr]], i64 8, i32 1, i1 false)

// CHECK-LABEL: define linkonce_odr i32 @"\01?foo{{[@$?.A-Za-z0-9_]+}}"(%class.Child* %this, i32 %a, i32 %b)
// CHECK: %[[Arg:.+]] = alloca %class.Parent
// CHECK: %[[Tmp:.+]] = alloca %class.Parent, align 4

// Make sure this is copy to agg tmp.
// CHECK: %[[PtrI:.+]] = getelementptr inbounds %class.Child, %class.Child* %this, i32 0, i32 0, i32 0
// CHECK: %[[PtrJ:.+]] = getelementptr inbounds %class.Child, %class.Child* %this, i32 0, i32 0, i32 1
// CHECK: %[[I:.+]] = load i32, i32* %[[PtrI]]
// CHECK: %[[J:.+]] = load float, float* %[[PtrJ]]
// CHECK: %[[TmpPtrI:.+]] = getelementptr inbounds %class.Parent, %class.Parent* %[[Tmp]], i32 0, i32 0
// CHECK: %[[TmpPtrJ:.+]] = getelementptr inbounds %class.Parent, %class.Parent* %[[Tmp]], i32 0, i32 1
// CHECK: store i32 %[[I]], i32* %[[TmpPtrI]]
// CHECK: store float %[[J]], float* %[[TmpPtrJ]]

// Make sure Tmp copy to Arg.
// CHECK: %[[ArgPtr:.+]] = bitcast %class.Parent* %[[Arg]] to i8*
// CHECK: %[[TmpPtr:.+]] = bitcast %class.Parent* %[[Tmp]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %[[ArgPtr]], i8* %[[TmpPtr]], i64 8, i32 1, i1 false)

// Use Arg to call lib_func.
// CHECK: call i32 @"\01?lib_func{{[@$?.A-Za-z0-9_]+}}"(%class.Parent* %[[Arg]], i32 %{{.*}}, i32 %{{.*}})

class Parent
{
    int i;
    float f;
};

int lib_func(Parent obj, int a, int b);
void lib_func2(out Parent obj);
void lib_func3(inout Parent obj);

class Child : Parent
{
    int foo(int a, int b)
    {
        return lib_func(this, a, b);
    }
    void foo() {
        lib_func2((Parent)this);
    }
    void bar() {
        lib_func3((Parent)this);
    }
    double d;
};

#define RS \
    "RootFlags(0), " \
    "DescriptorTable(UAV(u0))"

RWBuffer<uint> gOut : register(u0);

[shader("compute")]
[RootSignature(RS)]
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    Child c;
    c.i = 110;
    c.foo();
    c.bar();
    gOut[0] = c.foo(111, 112);
}
