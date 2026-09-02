// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s

struct [raypayload] Payload {
    int a : 1            : write(caller) : read(anyhit); // expected-error {{bitfields are not allowed with HLSL annotations}}
    int b : 17           : write(miss)   : read(caller); // expected-error {{bitfields are not allowed with HLSL annotations}}
    int c  : write(miss) : 13            : read(caller); // expected-error {{bitfields are not allowed with HLSL annotations}}
    int d  : write(miss) : read(caller);
};

struct Inputs {
    int a : 16 : SV_GroupIndex; // expected-error {{bitfields are not allowed with HLSL annotations}}
    int b : SV_Position : 16;  // expected-error {{bitfields are not allowed with HLSL annotations}}
};
