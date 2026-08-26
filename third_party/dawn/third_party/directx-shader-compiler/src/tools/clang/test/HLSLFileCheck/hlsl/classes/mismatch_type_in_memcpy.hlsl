// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure memcpy on _b.a = b.a not crash.
// CHECK:define void @main()


struct A
{
    float a;
    float b;
};

struct B
{
    A a;
};

struct C
{
    float c;
    B b;

    B GetB()
    {
        B _b;
        _b.a = b.a;
        return _b;
    }
};

C CreateC()
{
    C c;
    return c;
}

static const C c = CreateC();
static const B b = c.GetB();

float4 main() : SV_Target
{
    return (float4)0;
}