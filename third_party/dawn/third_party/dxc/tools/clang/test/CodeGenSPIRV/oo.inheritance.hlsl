// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Base {
    float4 a;
    float4 b;
};

// Make sure we have the correct indices for fields
// CHECK: OpMemberName %Derived 1 "b"
// CHECK: OpMemberName %Derived 2 "c"
// CHECK: OpMemberName %Derived 3 "x"

// Placing the implicit base object at the beginning
// CHECK: %Derived = OpTypeStruct %Base %v4float %v4float %Base
struct Derived : Base {
    float4 b;
    float4 c;
    Base   x;
};

// CHECK: %DerivedAgain = OpTypeStruct %Derived %v4float %v4float
struct DerivedAgain : Derived {
    float4 c;
    float4 d;
};

float4 main() : SV_Target {
    Derived d;

    // Accessing a field from the implicit base object
// CHECK:       [[base:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %d %uint_0
// CHECK-NEXT: [[base_a:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[base]] %int_0
// CHECK-NEXT:                   OpStore [[base_a]] {{%[0-9]+}}
    d.a   = 1.;

    // Accessing fields from the derived object (shadowing)
 // CHECK-NEXT:      [[b:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %d %int_1
// CHECK-NEXT:                   OpStore [[b]] {{%[0-9]+}}
    d.b   = 2.;

    // Accessing fields from the derived object
// CHECK-NEXT:      [[c:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %d %int_2
// CHECK-NEXT:                   OpStore [[c]] {{%[0-9]+}}
    d.c   = 3.;

    // Embedding another object of the implict base object's type
// CHECK-NEXT:    [[x_a:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %d %int_3 %int_0
// CHECK-NEXT:                   OpStore [[x_a]] {{%[0-9]+}}
// CHECK-NEXT:    [[x_b:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %d %int_3 %int_1
// CHECK-NEXT:                   OpStore [[x_b]] {{%[0-9]+}}
    d.x.a = 4.;
    d.x.b = 5.;

    DerivedAgain dd;

    // Accessing a field from the deep implicit base object
// CHECK-NEXT:   [[base_0:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %dd %uint_0 %uint_0
// CHECK-NEXT: [[base_a_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[base_0]] %int_0
// CHECK-NEXT:                   OpStore [[base_a_0]] {{%[0-9]+}}
    dd.a  = 6.;
    // Accessing a field from the immediate implicit base object
// CHECK-NEXT:    [[drv:%[0-9]+]] = OpAccessChain %_ptr_Function_Derived %dd %uint_0
// CHECK-NEXT:  [[drv_b:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[drv]] %int_1
// CHECK-NEXT:                   OpStore [[drv_b]] {{%[0-9]+}}
    dd.b  = 7.;
    // Accessing fields from the derived object (shadowing)
// CHECK-NEXT:      [[c_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %dd %int_1
// CHECK-NEXT:                   OpStore [[c_0]] {{%[0-9]+}}
    // Accessing fields from the derived object
    dd.c  = 8.;
// CHECK-NEXT:      [[d:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %dd %int_2
// CHECK-NEXT:                   OpStore [[d]] {{%[0-9]+}}
    dd.d  = 9.;

    DerivedAgain dd2;

// CHECK-NEXT:  [[dd_base_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_Derived %dd %uint_0
// CHECK-NEXT:      [[dd_base:%[0-9]+]] = OpLoad %Derived [[dd_base_ptr]]
// CHECK-NEXT: [[dd2_base_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_Derived %dd2 %uint_0
// CHECK-NEXT:                         OpStore [[dd2_base_ptr]] [[dd_base]]
    (Derived)dd2 = (Derived)dd;

// CHECK-NEXT:   [[d_base_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %d %uint_0
// CHECK-NEXT:       [[d_base:%[0-9]+]] = OpLoad %Base [[d_base_ptr]]
// CHECK-NEXT: [[dd2_base_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %dd2 %uint_0 %uint_0
// CHECK-NEXT:                         OpStore [[dd2_base_ptr_0]] [[d_base]]
    (Base)dd2    = (Base)d;

    // Make sure reads are good
// CHECK:        [[base_1:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %d %uint_0
// CHECK-NEXT:        {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float [[base_1]] %int_0
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %d %int_1
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %d %int_2
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %d %int_3 %int_0
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %d %int_3 %int_1
    return d.a + d.b + d.c + d.x.a + d.x.b +
// CHECK:        [[base_2:%[0-9]+]] = OpAccessChain %_ptr_Function_Base %dd %uint_0 %uint_0
// CHECK-NEXT:        {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float [[base_2]] %int_0
// CHECK:         [[drv_0:%[0-9]+]] = OpAccessChain %_ptr_Function_Derived %dd %uint_0
// CHECK-NEXT:        {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float [[drv_0]] %int_1
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %dd %int_1
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_Function_v4float %dd %int_2
           dd.a + dd.b + dd.c + dd.d;
}
