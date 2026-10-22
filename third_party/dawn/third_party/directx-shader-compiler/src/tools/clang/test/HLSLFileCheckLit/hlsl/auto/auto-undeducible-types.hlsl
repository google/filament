// RUN: %dxc -T cs_6_0 -HV 202x -verify %s


void voidFunc() {}

Texture2D<float4> tex : register(t0);
Texture2DMS<float4> texMS : register(t1);

[numthreads(1,1,1)]
void main() {
    int y = 1;

    // expected-error@+1 {{'auto' cannot deduce type}}
    auto str = "abc";

    // expected-error@+1 {{variable has incomplete type 'void'}}
    auto x = voidFunc();

    // expected-error@+1 {{variable has incomplete type 'void'}}
    auto v = (void)y;

    // expected-error@+1 {{'auto' cannot deduce type}}
    auto m = tex.mips;
    // expected-error@+1 {{'auto' cannot deduce type}}
    auto m2 = tex.mips[0];
    // legal: load float4 from mip 0
    auto m3 = tex.mips[0][int2(1,2)];

    // expected-error@+1 {{'auto' cannot deduce type}}
    auto s = texMS.sample;
    // expected-error@+1 {{'auto' cannot deduce type}}
    auto s2 = texMS.sample[0];
    // legal: load float4 from sample 0
    auto s3 = texMS.sample[0][int2(1,2)];
}
