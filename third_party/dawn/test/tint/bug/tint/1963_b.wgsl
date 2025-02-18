@group(0) @binding(0) var<storage, read_write> a : atomic<i32>;

@compute @workgroup_size(1)
fn compute_main() {
    // Secondary bug encountered while fixing crbug.com/tint/1963
    // With the fix, PromoteSideEffectsToDecl will emit a type for 'v', which is the un-typable
    // '__atomic_compare_exchange_result_i32' result. Later, DecomposeMemoryAccess would try to
    // construct its own struct to assign the intrinsic result of atomicCompareExchangeWeak() to
    // that let. The two types wouldn't match, and so this would fail to compile. The fix is to have
    // DecomposeMemoryAccess use the internal '__atomic_compare_exchange_result_i32' result instead
    // of building its own.
    let v = atomicCompareExchangeWeak(&a, 1, 1).old_value;
}
