// RUN: %dxc -E main -T vs_6_0 -HV 2018 %s | FileCheck %s

// Regression test for bools in constant initialization lists crashing
// due to a mismatch between register and memory representations (GitHub #1880, #1881)

// CHECK: ret void

bool main() : OUT
{
    // There are special cases around structs, arrays and matrices,
    // so it's important to test all combinations:
    // scalars/vectors can be in memory representation or not
    // matrices always store elements in register representation (until lowering to vector)
    // arrays/structs always store elements in memory representation.

    // Test target types
    static const bool b = { false };
    static const bool2 v = { false, true };
    static const bool2x2 m = { false, true, false, true };
    static const bool ab[] = { false };
    static const bool2 av[] = { false, true };
    static const bool2x2 am[] = { false, true, false, true };
    static const struct { bool x; } sb = { false };
    static const struct { bool2 x; } sv = { false, true };
    static const struct { bool2x2 x; } sm = { false, true, false, true };

    // Test source types
    static const bool ab_b[] = { false, b };
    static const bool ab_v[] = { bool2(false, true), v };
    static const bool ab_m[] = { bool2x2(false, true, false, true), m };
    static const bool ab_a[] = { ab, av, am };
    static const bool ab_s[] = { sb, sv, sm };

    // Reference everything to ensure they get codegen'd
    return b && v.x && m._11
        && ab[0] && av[0].x && am[0]._11
        && sb.x && sv.x && sm.x._11
        && ab_b[0] && ab_v[0] && ab_m[0] && ab_a[0] && ab_s[0];
}
