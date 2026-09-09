// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: while statement conditional expressions must evaluate to a scalar
// CHECK: do-while statement conditional expressions must evaluate to a scalar
// CHECK: for statement conditional expressions must evaluate to a scalar
// CHECK: if statement conditional expressions must evaluate to a scalar

float4 m;
float4 main(float4 a:A) : SV_Target
{
    int x=0;
    while (a>0) {
      a -= 2;
      x++;
    }

    do {
      a -= 2;
      x++;
    } while (a>0);


    for (uint i=0; a > 0; a--) {
       x++;
    }

    if (m)
        return x;
    else
        return 1;
}