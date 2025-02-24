// RUN: %dxc -T lib_6_6 -HV 2021 %s -verify

// This test verifies that the set of operators that HLSL 2021 allows overriding
// for correctly generates an error if the override is placed outside a class or
// struct definition.

struct Test {};

Test operator*(float L, Test R) { // expected-error {{overloading non-member 'operator*' is not allowed}}
    return R;
}

Test operator+(float L, Test R) { // expected-error {{overloading non-member 'operator+' is not allowed}}
    return R;
}

Test operator-(float L, Test R) { // expected-error {{overloading non-member 'operator-' is not allowed}}
    return R;
}

Test operator/(float L, Test R) { // expected-error {{overloading non-member 'operator/' is not allowed}}
    return R;
}

Test operator%(float L, Test R) { // expected-error {{overloading non-member 'operator%' is not allowed}}
    return R;
}

Test operator&(float L, Test R) { // expected-error {{overloading non-member 'operator&' is not allowed}}
    return R;
}

Test operator|(float L, Test R) { // expected-error {{overloading non-member 'operator|' is not allowed}}
    return R;
}

Test operator^(float L, Test R) { // expected-error {{overloading non-member 'operator^' is not allowed}}
    return R;
}

Test operator<(float L, Test R) { // expected-error {{overloading non-member 'operator<' is not allowed}}
    return R;
}

Test operator>(float L, Test R) { // expected-error {{overloading non-member 'operator>' is not allowed}}
    return R;
}

Test operator<=(float L, Test R) { // expected-error {{overloading non-member 'operator<=' is not allowed}}
    return R;
}

Test operator>=(float L, Test R) { // expected-error {{overloading non-member 'operator>=' is not allowed}}
    return R;
}

Test operator==(float L, Test R) { // expected-error {{overloading non-member 'operator==' is not allowed}}
    return R;
}

Test operator!=(float L, Test R) { // expected-error {{overloading non-member 'operator!=' is not allowed}}
    return R;
}

namespace SomeNamespace {

Test operator*(float L, Test R) { // expected-error {{overloading non-member 'operator*' is not allowed}}
    return R;
}

Test operator+(float L, Test R) { // expected-error {{overloading non-member 'operator+' is not allowed}}
    return R;
}

Test operator-(float L, Test R) { // expected-error {{overloading non-member 'operator-' is not allowed}}
    return R;
}

Test operator/(float L, Test R) { // expected-error {{overloading non-member 'operator/' is not allowed}}
    return R;
}

Test operator%(float L, Test R) { // expected-error {{overloading non-member 'operator%' is not allowed}}
    return R;
}

Test operator<(float L, Test R) { // expected-error {{overloading non-member 'operator<' is not allowed}}
    return R;
}

Test operator>(float L, Test R) { // expected-error {{overloading non-member 'operator>' is not allowed}}
    return R;
}

Test operator<=(float L, Test R) { // expected-error {{overloading non-member 'operator<=' is not allowed}}
    return R;
}

Test operator>=(float L, Test R) { // expected-error {{overloading non-member 'operator>=' is not allowed}}
    return R;
}

Test operator==(float L, Test R) { // expected-error {{overloading non-member 'operator==' is not allowed}}
    return R;
}

Test operator!=(float L, Test R) { // expected-error {{overloading non-member 'operator!=' is not allowed}}
    return R;
}

}
