// RUN: %dxc -T ps_6_0 -opt-enable structurize-returns %s | FileCheck %s

// Make sure we do not crash when structurizing returns from a do {...} while(0) loop.
// CHECK: define void @main

Texture1D<float> tex;

float foo(uint index)
{
    float accum = 0.0;
    do {
        float t = tex[index++];
        if (t == 0.0)
        {
            break;
        }
        
        accum += t;
        
        if (accum > 2.0)
        {
            return t;
        }
    } while (0);

    return accum;
}

[RootSignature("DescriptorTable(SRV(t0))")]
float main(float a : A) : SV_Target
{
    return foo(a);
}
