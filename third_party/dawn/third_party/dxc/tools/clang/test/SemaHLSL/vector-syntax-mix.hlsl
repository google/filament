// RUN: %dxc -Tlib_6_3 -verify %s

void fn() {
    // Can't use this initialization syntax yet.
    // float4 myvar = float4(1,2,3,4);
    float4 myvar;
    myvar.x = 1.0f;
    myvar.y = 1.0f;
    myvar.z = 1.0f;
    myvar.w = 1.0f;

    float4 myothervar;
    myothervar.xgba = myvar.xyzw;   // expected-error {{vector component names cannot mix 'xyzw' and 'rgba'}} fxc-error {{X3018: invalid subscript 'xgba'}}
    myothervar.rgbx = myvar.xyzw;   // expected-error {{vector component names cannot mix 'xyzw' and 'rgba'}} fxc-error {{X3018: invalid subscript 'rgbx'}}
}
