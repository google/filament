// RUN: %dxc -E main -T lib_6_6 %s -verify

// expected-warning@+1{{cannot mix packoffset elements with nonpackoffset elements in a cbuffer}}
cbuffer Mix
{
    float4 M1 : packoffset(c0);
    float M2;
    float M3 : packoffset(c1.y);
}

// expected-warning@+1{{cannot mix packoffset elements with nonpackoffset elements in a cbuffer}}
cbuffer Mix2
{
    float4 M4;
    float M5 : packoffset(c1.y);
    float M6 ;
}

// expected-error@+1{{packoffset is only allowed in a constant buffer}}
float4 g : packoffset(c0);

cbuffer IllegalOffset
{
    // expected-error@+1{{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}}
    float4 i1 : packoffset(t2);
    // expected-error@+1{{packoffset component should indicate offset with one of x, y, z, w, r, g, b, or a}}
    float i2 : packoffset(c1.m);
}

cbuffer Overlap
{
    float4 o1 : packoffset(c0);
    // expected-error@+1{{packoffset overlap between 'o2', 'o1'}}
    float2 o2 : packoffset(c0.z);
}

cbuffer CrossReg
{
    // expected-error@+1{{register or offset bind not valid}}
    float4 c1 : packoffset(c0.y);
    // expected-error@+1{{register or offset bind not valid}}
    float2 c2 : packoffset(c1.w);
}

struct ST {
  float s;
};

cbuffer Aggregate
{
    // expected-error@+1{{register or offset bind not valid}}
    ST A1 : packoffset(c0.y);
    // expected-error@+1{{register or offset bind not valid}}
    float A2[2] : packoffset(c1.w);
    // expected-error@+1{{register or offset bind not valid}}
    float2x1 m2 : packoffset(c12.z);
}
