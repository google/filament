#if defined(FP32)
INST_TEST_CASE(TestGEMM,
    test_params{'n', 'n', 3, 2, 1, 1.0, 0.0, 2, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'t', 'n', 3, 2, 2, 1.0, 0.0, 1, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 't', 3, 2, 1, 1.0, 0.0, 3, 1, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 'd', 3, 2, 1, 1.0, 0.0, 3, 3, 3, {}, true, mkldnn_invalid_arguments},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, {}, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, {}, false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, {}, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, {}, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, {}, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, {}, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, {}, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, {}, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, {}, false},

    test_params{'n', 'n', 2000, 2000, 2000, 1.0, 0.0, 2000, 2000, 2000, {}, false},
    test_params{'n', 'n', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, {}, false},
    test_params{'t', 'n', 2000, 2000, 2000, 1.0, 0.0, 2000, 2000, 2000, {}, false},
    test_params{'t', 'n', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, {}, false},
    test_params{'n', 't', 2000, 2000, 2000, 1.0, 0.0, 2000, 2000, 2000, {}, false},
    test_params{'n', 't', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, {}, false},
    test_params{'t', 't', 2000, 2000, 2000, 1.0, 0.0, 2000, 2000, 2000, {}, false},
    test_params{'t', 't', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, {}, false}
);

#else
constexpr test_igemm_params fix_use_oc = {'F', true, true, false};
constexpr test_igemm_params col_use_oc = {'C', true, true, false};
constexpr test_igemm_params row_use_oc = {'R', true, true, false};

constexpr test_igemm_params fix_use_all_offsets = {'F', false, false, false};
constexpr test_igemm_params col_use_all_offsets = {'C', false, false, false};
constexpr test_igemm_params row_use_all_offsets = {'R', false, false, false};

constexpr test_igemm_params fix_no_offsets = {'F', true, true, true};
constexpr test_igemm_params col_no_offsets = {'C', true, true, true};
constexpr test_igemm_params row_no_offsets = {'R', true, true, true};

INST_TEST_CASE(TestGEMM_expected_failures,
    test_params{'n', 'n', 3, 2, 1, 1.0, 0.0, 2, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'t', 'n', 3, 2, 2, 1.0, 0.0, 1, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 't', 3, 2, 1, 1.0, 0.0, 3, 1, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 'd', 3, 2, 1, 1.0, 0.0, 3, 3, 3, {}, true, mkldnn_invalid_arguments},

    test_params{'n', 'n', 3, 2, 1, 1.0, 0.0, 2, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'t', 'n', 3, 2, 2, 1.0, 0.0, 1, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 't', 3, 2, 1, 1.0, 0.0, 3, 1, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 'd', 3, 2, 1, 1.0, 0.0, 3, 3, 3, {}, true, mkldnn_invalid_arguments},

    test_params{'n', 'n', 3, 2, 1, 1.0, 0.0, 2, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'t', 'n', 3, 2, 2, 1.0, 0.0, 1, 5, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 't', 3, 2, 1, 1.0, 0.0, 3, 1, 8, {}, true, mkldnn_invalid_arguments},
    test_params{'n', 'd', 3, 2, 1, 1.0, 0.0, 3, 3, 3, {}, true, mkldnn_invalid_arguments}
);

INST_TEST_CASE(TestGEMM_general_cases_fix_offset,
    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, fix_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, fix_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, fix_use_oc, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_all_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, fix_use_all_offsets, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_no_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, fix_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, fix_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, fix_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, fix_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, fix_no_offsets, false}
);

INST_TEST_CASE(TestGEMM_general_cases_col_offset,
    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, col_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, col_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, col_use_oc, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_all_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, col_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, col_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, col_use_all_offsets, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_no_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, col_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, col_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, col_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, col_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, col_no_offsets, false}
);

INST_TEST_CASE(TestGEMM_general_cases_row_offset,
    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, row_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, row_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, row_use_oc, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_all_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, row_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, row_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, row_use_all_offsets, false},

    test_params{'N', 'n', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_no_offsets,false},
    test_params{'T', 'N', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.0, 1.0, 60, 50, 80, row_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.0, 2.0, 100, 100, 100, row_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.0, 2.0, 100, 100, 100, row_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.0, 2.0, 100, 100, 100, row_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.0, 2.0, 2, 10000, 2, row_no_offsets, false}
);

INST_TEST_CASE(TestGEMM_fractional_scales_fix_offset,
    /* alpha and beta have non-zero fractional part */
    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, fix_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, fix_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, fix_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, fix_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, fix_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, fix_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, fix_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, fix_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, fix_use_oc, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, fix_use_all_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, fix_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, fix_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, fix_use_all_offsets, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, fix_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, fix_no_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, fix_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, fix_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, fix_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, fix_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, fix_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, fix_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, fix_no_offsets, false}
);

INST_TEST_CASE(TestGEMM_fractional_scales_col_offset,
    /* alpha and beta have non-zero fractional part */
    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, col_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, col_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, col_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, col_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, col_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, col_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, col_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, col_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, col_use_oc, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, col_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, col_use_all_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, col_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, col_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, col_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, col_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, col_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, col_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, col_use_all_offsets, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, col_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, col_no_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, col_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, col_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, col_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, col_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, col_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, col_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, col_no_offsets, false}
);

INST_TEST_CASE(TestGEMM_fractional_scales_row_offset,
    /* alpha and beta have non-zero fractional part */
    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, row_use_oc, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, row_use_oc, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, row_use_oc, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, row_use_oc, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, row_use_oc, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, row_use_oc, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, row_use_oc, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, row_use_oc, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, row_use_oc, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, row_use_all_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, row_use_all_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, row_use_all_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, row_use_all_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, row_use_all_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, row_use_all_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, row_use_all_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, row_use_all_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, row_use_all_offsets, false},

    test_params{'n', 'T', 30, 20, 10, 2.33f, 1.66f, 60, 50, 80, row_no_offsets, false},
    test_params{'n', 'T', 30, 20, 10, 2.19f, 1.99f, 120, 120, 120, row_no_offsets, false},
    test_params{'T', 'N', 30, 20, 10, 2.01f, 1.01f, 60, 50, 80, row_no_offsets, false},
    test_params{'t', 't', 30, 20, 10, 2.99f, 1.19f, 60, 50, 80, row_no_offsets, false},
    test_params{'n', 'n', 100, 100, 2, 1.33f, 2.33f, 100, 100, 100, row_no_offsets, false},
    test_params{'n', 't', 100, 2, 100, 1.19f, 2.99f, 100, 100, 100, row_no_offsets, false},
    test_params{'t', 'n', 2, 100, 100, 1.01f, 2.01f, 100, 100, 100, row_no_offsets, false},
    test_params{'t', 't', 2, 100, 100, 1.99f, 2.19f, 100, 100, 100, row_no_offsets, false},
    test_params{'n', 'n', 2, 2, 10000, 1.66f, 2.33f, 2, 10000, 2, row_no_offsets, false}
);


INST_TEST_CASE(TestGEMV,
    test_params{'n', 'n', 2000, 1, 1000, 1.0f, 0.0f, 2000, 1000, 2000, fix_no_offsets, false},
    test_params{'n', 'n', 1, 3000, 2000, 1.0f, 0.0f, 1, 2000, 1, fix_no_offsets, false},
    test_params{'t', 'n', 2000, 1, 1000, 1.0f, 0.0f, 2000, 1000, 2000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 3000, 2000, 1.0f, 0.0f, 2000, 2000, 1, fix_no_offsets, false},
    test_params{'n', 't', 2000, 1, 1000, 1.0f, 0.0f, 2000, 1, 2000, fix_no_offsets, false},
    test_params{'n', 't', 1, 3000, 2000, 1.0f, 0.0f, 1, 3000, 1, fix_no_offsets, false},
    test_params{'t', 't', 2000, 1, 1000, 1.0f, 0.0f, 1000, 1, 2000, fix_no_offsets, false},
    test_params{'t', 't', 1, 3000, 2000, 1.0f, 0.0f, 2000, 3000, 1, fix_no_offsets, false},

    test_params{'n', 'n', 2000, 1, 1000, 1.0f, 1.0f, 2000, 1000, 2000, fix_no_offsets, false},
    test_params{'n', 'n', 1, 3000, 2000, 1.0f, 1.0f, 1, 2000, 1, fix_no_offsets, false},
    test_params{'t', 'n', 2000, 1, 1000, 1.0f, 1.0f, 2000, 1000, 2000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 3000, 2000, 1.0f, 1.0f, 2000, 2000, 1, fix_no_offsets, false},
    test_params{'n', 't', 2000, 1, 1000, 1.0f, 1.0f, 2000, 1, 2000, fix_no_offsets, false},
    test_params{'n', 't', 1, 3000, 2000, 1.0f, 1.0f, 1, 3000, 1, fix_no_offsets, false},
    test_params{'t', 't', 2000, 1, 1000, 1.0f, 1.0f, 1000, 1, 2000, fix_no_offsets, false},
    test_params{'t', 't', 1, 3000, 2000, 1.0f, 1.0f, 2000, 3000, 1, fix_no_offsets, false}
);

INST_TEST_CASE(TestGEMV_kblocking,
    test_params{'t', 'n', 20, 1, 7000, 1.0f, 0.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 't', 50, 1, 7000, 1.0f, 0.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 400, 1, 7000, 1.0f, 0.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 't', 500, 1, 7000, 1.0f, 0.0f, 7000, 1, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 20, 1, 7000, 1.0f, 1.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 't', 50, 1, 7000, 1.0f, 1.0f, 7000, 1, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 500, 1, 7000, 1.0f, 1.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 't', 500, 1, 7000, 1.0f, 1.0f, 7000, 7000, 7000, fix_no_offsets, false},

    test_params{'n', 'n', 1, 40, 7000, 1.0f, 0.0f, 1, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 10, 7000, 1.0f, 0.0f, 7000, 7000, 1, fix_no_offsets, false},
    test_params{'n', 'n', 1, 400, 7000, 1.0f, 0.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 100, 7000, 1.0f, 0.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'n', 'n', 1, 40, 7000, 1.0f, 1.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 10, 7000, 1.0f, 1.0f, 7000, 7000, 7000, fix_no_offsets, false},
    test_params{'n', 'n', 1, 400, 7000, 1.0f, 1.0f, 1, 7000, 7000, fix_no_offsets, false},
    test_params{'t', 'n', 1, 550, 7000, 1.0f, 1.0f, 7000, 7000, 1, fix_no_offsets, false}
);


INST_TEST_CASE(TestGEMM_heavy,
    test_params{'n', 'n', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'t', 'n', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'n', 't', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'t', 't', 3000, 3000, 3000, 1.0, 0.0, 3000, 3000, 3000, fix_use_oc, false},

    test_params{'n', 'n', 3000, 3000, 3000, 2.19f, 1.99f, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'t', 'n', 3000, 3000, 3000, 2.99f, 1.19f, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'n', 't', 3000, 3000, 3000, 1.19f, 2.99f, 3000, 3000, 3000, fix_use_oc, false},
    test_params{'t', 't', 3000, 3000, 3000, 1.99f, 2.19f, 3000, 3000, 3000, fix_use_oc, false}
);

#endif
