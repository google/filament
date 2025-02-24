fn original_clusterfuzz_code() {
    _ = atan2(1,.1);
}

fn more_tests_that_would_fail() {
    // Builtin calls with mixed abstract args would fail because AInt would not materialize to AFloat.
    {
        let a = atan2(1, 0.1);
        let b = atan2(0.1, 1);
    }

    // Same for binary operators
    {
        let a = 1 + 1.5;
        let b = 1.5 + 1;
    }

    // Once above was fixed, builtin calls without assignment would also fail in backends because
    // abstract constant value is not handled by backends. These should be removed by RemovePhonies
    // transform.
    {
        _ = atan2(1, 0.1);
        _ = atan2(0.1, 1);
    }
}
