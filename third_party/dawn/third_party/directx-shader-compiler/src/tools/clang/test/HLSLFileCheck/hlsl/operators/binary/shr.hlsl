// RUN: %dxc -E main -T ps_6_0 -DDEFAULT  %s | FileCheck %s -check-prefix=DEFAULT
// RUN: %dxc -E main -T ps_6_0 -DUINT_SHIFT  %s | FileCheck %s -check-prefix=UINT
// RUN: %dxc -E main -T ps_6_0 -DINT_SHIFT  %s | FileCheck %s -check-prefix=INT

// Make sure generate ashr and lshr
// DEFAULT:ashr
// UINT:lshr
// INT:ashr

int i;

#ifdef DEFAULT
int half_btf(int w,int bit) { return (w + (1<<bit)) >> bit; }
#endif

#ifdef UINT_SHIFT
int half_btf(int w,int bit) { return (w + (1U<<bit)) >> bit; }
#endif

#ifdef INT_SHIFT
int half_btf(int w,int bit) { return (w + (1L<<bit)) >> bit; }
#endif

int main(int w:W) : SV_Target {
  return half_btf(i,12);
}