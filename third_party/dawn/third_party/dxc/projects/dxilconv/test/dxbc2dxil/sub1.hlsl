// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

int g1, g2;

float4 main(float4 a : AAA) : SV_Target
{
  float4 b[8];

  [call]
  switch(g1)
  {
    case 0:
    b[2] = 4;
    break;

    case 1:
    b[2] = a.x;
    break;

    default:
    b[g1] = 0;
    break;
  }

  return b[g2];
}

