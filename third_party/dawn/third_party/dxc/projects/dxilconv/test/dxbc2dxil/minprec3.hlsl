// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




int main(min16int a : A) : SV_Target
{
    min16int q = a + 2;
    [branch]
    if (q == min16int(7))
      return q - 3;
    else
      return 1; 
}
