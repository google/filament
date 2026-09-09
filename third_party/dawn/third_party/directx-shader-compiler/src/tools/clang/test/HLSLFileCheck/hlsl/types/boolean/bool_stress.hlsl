// RUN: %dxc -E main -T vs_6_0 -O0 %s

// Regression test for compiler crashes in complex bool cases

struct AllTheBools
{
    bool b : B;
    bool ba2[2] :BA2;
    bool1 b1 : B1;
    bool3 b3 : B3;
    bool3 b3a2[2] : B3A2;
    bool1x1 b1x1 : B1X1;
    bool2x3 b2x3 : B2X3;
    row_major bool2x3 rmb2x3 : RMB2X3;
    bool2x3 b2x3a2[2] : B2X3A2;
};

ConstantBuffer<AllTheBools> cb;
StructuredBuffer<AllTheBools> sb;

void not(in out bool value) { value = !value; }

void not(in out bool2 value)
{
  value = !value;
  not(value.x);
  not(value.y);
}

void not(in out bool3 value)
{
  not(value.xz);
  not(value.y);
}

AllTheBools main(AllTheBools input, float f : F)
{
    AllTheBools output;
    output.b = input.b ? cb.b : sb[0].b;
    output.ba2[1] = input.b;
    output.ba2[0] = input.ba2[1];
    output.b1 = input.b3.y;
    output.b3 = input.b.xxx;
    output.b3a2 = sb[0].b3a2;
    if (sb[0].b) return cb;

    output.b1x1 = cb.b2x3._22;
    output.b2x3 = bool2x3(sb[0].b3, bool3(f > 2, input.b, false));
    output.rmb2x3 = input.b2x3;
    not(output.rmb2x3[0]);
    output.b2x3a2[1] = cb.b2x3;
    output.b2x3a2[0] = input.b2x3;
    return output;
}