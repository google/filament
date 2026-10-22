#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct Test
{
    short a;
    short2 b;
    short3 c;
    short4 d;
};

struct Test2
{
    spvUnsafeArray<short, 2> a;
    spvUnsafeArray<short2, 2> b;
    spvUnsafeArray<short3, 2> c;
    spvUnsafeArray<short4, 2> d;
    Test e;
};

constant spvUnsafeArray<bool, 2> _40 = spvUnsafeArray<bool, 2>({ true, false });
constant spvUnsafeArray<bool2, 2> _44 = spvUnsafeArray<bool2, 2>({ bool2(true, false), bool2(false, true) });
constant spvUnsafeArray<bool3, 2> _48 = spvUnsafeArray<bool3, 2>({ bool3(true, false, true), bool3(false, true, false) });
constant spvUnsafeArray<bool4, 2> _52 = spvUnsafeArray<bool4, 2>({ bool4(true, false, true, false), bool4(false, true, false, true) });
constant spvUnsafeArray<bool, 2> _82 = spvUnsafeArray<bool, 2>({ true, true });
constant spvUnsafeArray<bool2, 2> _85 = spvUnsafeArray<bool2, 2>({ bool2(true), bool2(false) });
constant spvUnsafeArray<bool3, 2> _87 = spvUnsafeArray<bool3, 2>({ bool3(true), bool3(false) });
constant spvUnsafeArray<bool4, 2> _89 = spvUnsafeArray<bool4, 2>({ bool4(true), bool4(false) });

fragment void main0()
{
    spvUnsafeArray<Test, 2> _95 = spvUnsafeArray<Test, 2>({ Test{ short(true), short2(bool2(true, false)), short3(bool3(true)), short4(bool4(false)) }, Test{ short(false), short2(bool2(false, true)), short3(bool3(false)), short4(bool4(true)) } });
    
    Test t;
    t.a = short(true);
    t.b = short2(bool2(true, false));
    t.c = short3(bool3(true, false, true));
    t.d = short4(bool4(true, false, true, false));
    Test2 t2;
    t2.a = { short(_40[0]), short(_40[1]) };
    t2.b = { short2(_44[0]), short2(_44[1]) };
    t2.c = { short3(_48[0]), short3(_48[1]) };
    t2.d = { short4(_52[0]), short4(_52[1]) };
    bool a = bool(t.a);
    bool2 b = bool2(t.b);
    bool3 c = bool3(t.c);
    bool4 d = bool4(t.d);
    spvUnsafeArray<bool, 2> a2 = { bool(t2.a[0]), bool(t2.a[1]) };
    spvUnsafeArray<bool2, 2> b2 = { bool2(t2.b[0]), bool2(t2.b[1]) };
    spvUnsafeArray<bool3, 2> c2 = { bool3(t2.c[0]), bool3(t2.c[1]) };
    spvUnsafeArray<bool4, 2> d2 = { bool4(t2.d[0]), bool4(t2.d[1]) };
    t = Test{ short(true), short2(bool2(true, false)), short3(bool3(true)), short4(bool4(false)) };
    t2 = Test2{ spvUnsafeArray<short, 2>({ short(true), short(true) }), spvUnsafeArray<short2, 2>({ short2(bool2(true)), short2(bool2(false)) }), spvUnsafeArray<short3, 2>({ short3(bool3(true)), short3(bool3(false)) }), spvUnsafeArray<short4, 2>({ short4(bool4(true)), short4(bool4(false)) }), Test{ short(true), short2(bool2(true, false)), short3(bool3(true)), short4(bool4(false)) } };
}

