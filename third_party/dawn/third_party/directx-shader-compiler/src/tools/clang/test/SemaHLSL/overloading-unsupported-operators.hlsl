// RUN: %dxc -Tlib_6_3 -Wno-unused-value -verify -HV 2021 %s
// RUN: %dxc -Tcs_6_0 -verify -HV 2021 %s

// This test checks that dxcompiler generates errors when overloading operators
// that are not supported for overloading in HLSL 2021

struct S
{
    float foo;
    void operator=(S s) {} // expected-error {{overloading 'operator=' is not allowed}}
    void operator+=(S s) {} // expected-error {{overloading 'operator+=' is not allowed}}
    void operator-=(S s) {} // expected-error {{overloading 'operator-=' is not allowed}}
    void operator*=(S s) {} // expected-error {{overloading 'operator*=' is not allowed}}
    void operator/=(S s) {} // expected-error {{overloading 'operator/=' is not allowed}}
    void operator%=(S s) {} // expected-error {{overloading 'operator%=' is not allowed}}
    void operator^=(S s) {} // expected-error {{overloading 'operator^=' is not allowed}}
    void operator&=(S s) {} // expected-error {{overloading 'operator&=' is not allowed}}
    void operator|=(S s) {} // expected-error {{overloading 'operator|=' is not allowed}}
    void operator<<(S s) {} // expected-error {{overloading 'operator<<' is not allowed}}
    void operator>>(S s) {} // expected-error {{overloading 'operator>>' is not allowed}}
    void operator<<=(S s) {} // expected-error {{overloading 'operator<<=' is not allowed}}
    void operator>>=(S s) {} // expected-error {{overloading 'operator>>=' is not allowed}}
    void operator->*(S s) {} // expected-error {{overloading 'operator->*' is not allowed}}
    void operator->(S s) {} // expected-error {{overloading 'operator->' is not allowed}}
    void operator++(S s) {} // expected-error {{overloading 'operator++' is not allowed}}
    void operator--(S s) {} // expected-error {{overloading 'operator--' is not allowed}}
};
