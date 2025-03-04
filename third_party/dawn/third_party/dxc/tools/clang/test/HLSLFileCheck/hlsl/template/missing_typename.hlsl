// RUN: %dxc -T vs_6_0 -E VSMain -HV 2021 %s | FileCheck %s

// CHECK: error: nested typedefs are not supported in HLSL

template <typename T>
struct Vec2 {
    T x, y;
    T Length() {
        return sqrt(x*x + y*y);
    }
    typedef T value_type;
};

template <typename TVec>
TVec::value_type Length(TVec v) { // <-- Must be "typename TVec::value_type"
    return v.Length();
}

void VSMain() {
    Vec2<int> v_i;
    Vec2<float> v_f;
   
    int   a = Length(v_i);
    float b = Length(v_f);
}
