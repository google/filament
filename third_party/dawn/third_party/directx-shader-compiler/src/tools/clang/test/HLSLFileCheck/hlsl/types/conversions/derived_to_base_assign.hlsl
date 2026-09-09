// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Test assignments to and from the base class part of an object only.

struct Base { int i; };
struct Derived : Base { int j; };
struct Output
{
    Derived d0;
    Derived d1;
    Base b0;
    Base b1;
};

Output main(Derived d : IN) : OUT
{
    // Use CHECK-DAG because there are actually storeOutputs for both the zero initialization and the final value.
    Output output = (Output)0;

    // With LHS cast and implicit RHS conversion (derived field untouched)
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 %{{.*}})
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 1, i32 0, i8 0, i32 0)
    (Base)output.d0 = d;
    
    // With LHS and RHS cast (derived field untouched)
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 2, i32 0, i8 0, i32 %{{.*}})
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 3, i32 0, i8 0, i32 0)
    (Base)output.d1 = (Base)d;
    
    // With implicit RHS conversion
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 4, i32 0, i8 0, i32 %{{.*}})
    output.b0 = d;
    
    // With RHS cast
    // CHECK-DAG: call void @dx.op.storeOutput.i32(i32 5, i32 5, i32 0, i8 0, i32 %{{.*}})
    output.b1 = (Base)d;

    return output;
} 