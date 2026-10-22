// RUN: not %dxc -T ps_6_7 -E main -fcgl %s -spirv 2>&1 | FileCheck %s

template<uint32_t N, typename T=double>
struct GaussLegendreValues 
{
    const static T wi[N];
};

static int test_value = 1567;

float main() : SV_Target
{
    return float(GaussLegendreValues<2>::wi[test_value]);
}

// CHECK: :6:20: warning: variable 'GaussLegendreValues<2, double>::wi' has internal linkage but is not defined [-Wundefined-internal]
// CHECK-NEXT: const static T wi[N];
// CHECK: :13:42: note: used here
// CHECK-NEXT: return float(GaussLegendreValues<2>::wi[test_value]);
// CHECK: :6:20: fatal error: found unregistered decl
// CHECK-NEXT: const static T wi[N];
