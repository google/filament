// Copyright (C) [2025] World Wide Web Consortium,
// (Massachusetts Institute of Technology, European Research Consortium for
// Informatics and Mathematics, Keio University, Beihang).
// All Rights Reserved.
//
// This work is distributed under the W3C (R) Software License [1] in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// [1] http://www.w3.org/Consortium/Legal/copyright-software

// **** This file is auto-generated. Do not edit. ****

module.exports = grammar({
    name: 'wgsl',

    externals: $ => [
        $._block_comment,
        $._disambiguate_template,
        $._template_args_start,
        $._template_args_end,
        $._less_than,
        $._less_than_equal,
        $._shift_left,
        $._shift_left_assign,
        $._greater_than,
        $._greater_than_equal,
        $._shift_right,
        $._shift_right_assign,
        $._error_sentinel,
    ],

    extras: $ => [
        $._comment,
        $._block_comment,
        $._blankspace,
    ],

    inline: $ => [
        $.global_decl,
        $._reserved,
    ],

    // WGSL has no parsing conflicts.
    conflicts: $ => [],

    word: $ => $.ident_pattern_token,

    rules: {
        translation_unit: $ => seq(repeat($.global_directive), repeat($.global_decl)),

        global_directive: $ => choice($.diagnostic_directive, $.enable_directive, $.requires_directive),

        global_decl: $ => choice(';', seq($.global_variable_decl, ';'), seq($.global_value_decl, ';'), seq($.type_alias_decl, ';'), $.struct_decl, $.function_decl, seq($.const_assert_statement, ';')),

        bool_literal: $ => choice('true', 'false'),

        int_literal: $ => choice($.decimal_int_literal, $.hex_int_literal),

        decimal_int_literal: $ => choice(/0[iu]?/, /[1-9][0-9]*[iu]?/),

        hex_int_literal: $ => /0[xX][0-9a-fA-F]+[iu]?/,

        float_literal: $ => choice($.decimal_float_literal, $.hex_float_literal),

        decimal_float_literal: $ => choice(/0[fh]/, /[1-9][0-9]*[fh]/, /[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?[fh]?/, /[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?[fh]?/, /[0-9]+[eE][+-]?[0-9]+[fh]?/),

        hex_float_literal: $ => choice(/0[xX][0-9a-fA-F]*\.[0-9a-fA-F]+([pP][+-]?[0-9]+[fh]?)?/, /0[xX][0-9a-fA-F]+\.[0-9a-fA-F]*([pP][+-]?[0-9]+[fh]?)?/, /0[xX][0-9a-fA-F]+[pP][+-]?[0-9]+[fh]?/),

        diagnostic_directive: $ => seq('diagnostic', $.diagnostic_control, ';'),

        literal: $ => choice($.int_literal, $.float_literal, $.bool_literal),

        ident: $ => seq($.ident_pattern_token, $._disambiguate_template),

        member_ident: $ => $.ident_pattern_token,

        diagnostic_name_token: $ => $.ident_pattern_token,

        diagnostic_rule_name: $ => choice($.diagnostic_name_token, seq($.diagnostic_name_token, '.', $.diagnostic_name_token)),

        template_list: $ => seq($._template_args_start, $.template_arg_comma_list, $._template_args_end),

        template_arg_comma_list: $ => seq($.template_arg_expression, repeat(seq(',', $.template_arg_expression)), optional(',')),

        template_arg_expression: $ => $.expression,

        align_attr: $ => seq('@', 'align', '(', $.expression, optional(','), ')'),

        binding_attr: $ => seq('@', 'binding', '(', $.expression, optional(','), ')'),

        blend_src_attr: $ => seq('@', 'blend_src', '(', $.expression, optional(','), ')'),

        builtin_attr: $ => seq('@', 'builtin', '(', $.builtin_value_name, optional(','), ')'),

        builtin_value_name: $ => $.ident_pattern_token,

        const_attr: $ => seq('@', 'const'),

        diagnostic_attr: $ => seq('@', 'diagnostic', $.diagnostic_control),

        group_attr: $ => seq('@', 'group', '(', $.expression, optional(','), ')'),

        id_attr: $ => seq('@', 'id', '(', $.expression, optional(','), ')'),

        interpolate_attr: $ => choice(seq('@', 'interpolate', '(', $.interpolate_type_name, optional(','), ')'), seq('@', 'interpolate', '(', $.interpolate_type_name, ',', $.interpolate_sampling_name, optional(','), ')')),

        interpolate_type_name: $ => $.ident_pattern_token,

        interpolate_sampling_name: $ => $.ident_pattern_token,

        invariant_attr: $ => seq('@', 'invariant'),

        location_attr: $ => seq('@', 'location', '(', $.expression, optional(','), ')'),

        must_use_attr: $ => seq('@', 'must_use'),

        size_attr: $ => seq('@', 'size', '(', $.expression, optional(','), ')'),

        workgroup_size_attr: $ => choice(seq('@', 'workgroup_size', '(', $.expression, optional(','), ')'), seq('@', 'workgroup_size', '(', $.expression, ',', $.expression, optional(','), ')'), seq('@', 'workgroup_size', '(', $.expression, ',', $.expression, ',', $.expression, optional(','), ')')),

        vertex_attr: $ => seq('@', 'vertex'),

        fragment_attr: $ => seq('@', 'fragment'),

        compute_attr: $ => seq('@', 'compute'),

        attribute: $ => choice(seq('@', $.ident_pattern_token, optional($.argument_expression_list)), $.align_attr, $.binding_attr, $.blend_src_attr, $.builtin_attr, $.const_attr, $.diagnostic_attr, $.group_attr, $.id_attr, $.interpolate_attr, $.invariant_attr, $.location_attr, $.must_use_attr, $.size_attr, $.workgroup_size_attr, $.vertex_attr, $.fragment_attr, $.compute_attr),

        diagnostic_control: $ => seq('(', $.severity_control_name, ',', $.diagnostic_rule_name, optional(','), ')'),

        struct_decl: $ => seq('struct', $.ident, $.struct_body_decl),

        struct_body_decl: $ => seq('{', $.struct_member, repeat(seq(',', $.struct_member)), optional(','), '}'),

        struct_member: $ => seq(repeat($.attribute), $.member_ident, ':', $.type_specifier),

        type_alias_decl: $ => seq('alias', $.ident, '=', $.type_specifier),

        type_specifier: $ => $.template_elaborated_ident,

        template_elaborated_ident: $ => seq($.ident, $._disambiguate_template, optional($.template_list)),

        variable_or_value_statement: $ => choice($.variable_decl, seq($.variable_decl, '=', $.expression), seq('let', $.optionally_typed_ident, '=', $.expression), seq('const', $.optionally_typed_ident, '=', $.expression)),

        variable_decl: $ => seq('var', $._disambiguate_template, optional($.template_list), $.optionally_typed_ident),

        optionally_typed_ident: $ => seq($.ident, optional(seq(':', $.type_specifier))),

        global_variable_decl: $ => seq(repeat($.attribute), $.variable_decl, optional(seq('=', $.expression))),

        global_value_decl: $ => choice(seq('const', $.optionally_typed_ident, '=', $.expression), seq(repeat($.attribute), 'override', $.optionally_typed_ident, optional(seq('=', $.expression)))),

        primary_expression: $ => choice($.template_elaborated_ident, $.call_expression, $.literal, $.paren_expression),

        call_expression: $ => $.call_phrase,

        call_phrase: $ => seq($.template_elaborated_ident, $.argument_expression_list),

        paren_expression: $ => seq('(', $.expression, ')'),

        argument_expression_list: $ => seq('(', optional($.expression_comma_list), ')'),

        expression_comma_list: $ => seq($.expression, repeat(seq(',', $.expression)), optional(',')),

        component_or_swizzle_specifier: $ => choice(seq('[', $.expression, ']', optional($.component_or_swizzle_specifier)), seq('.', $.member_ident, optional($.component_or_swizzle_specifier)), seq('.', $.swizzle_name, optional($.component_or_swizzle_specifier))),

        unary_expression: $ => choice($.singular_expression, seq('-', $.unary_expression), seq('!', $.unary_expression), seq('~', $.unary_expression), seq('*', $.unary_expression), seq('&', $.unary_expression)),

        singular_expression: $ => seq($.primary_expression, optional($.component_or_swizzle_specifier)),

        lhs_expression: $ => choice(seq($.core_lhs_expression, optional($.component_or_swizzle_specifier)), seq('*', $.lhs_expression), seq('&', $.lhs_expression)),

        core_lhs_expression: $ => choice(seq($.ident, $._disambiguate_template), seq('(', $.lhs_expression, ')')),

        multiplicative_expression: $ => choice($.unary_expression, seq($.multiplicative_expression, $.multiplicative_operator, $.unary_expression)),

        multiplicative_operator: $ => choice('*', '/', '%'),

        additive_expression: $ => choice($.multiplicative_expression, seq($.additive_expression, $.additive_operator, $.multiplicative_expression)),

        additive_operator: $ => choice('+', '-'),

        shift_expression: $ => choice($.additive_expression, seq($.unary_expression, $._shift_left, $.unary_expression), seq($.unary_expression, $._shift_right, $.unary_expression)),

        relational_expression: $ => choice($.shift_expression, seq($.shift_expression, $._less_than, $.shift_expression), seq($.shift_expression, $._greater_than, $.shift_expression), seq($.shift_expression, $._less_than_equal, $.shift_expression), seq($.shift_expression, $._greater_than_equal, $.shift_expression), seq($.shift_expression, '==', $.shift_expression), seq($.shift_expression, '!=', $.shift_expression)),

        short_circuit_and_expression: $ => choice($.relational_expression, seq($.short_circuit_and_expression, '&&', $.relational_expression)),

        short_circuit_or_expression: $ => choice($.relational_expression, seq($.short_circuit_or_expression, '||', $.relational_expression)),

        binary_or_expression: $ => choice($.unary_expression, seq($.binary_or_expression, '|', $.unary_expression)),

        binary_and_expression: $ => choice($.unary_expression, seq($.binary_and_expression, '&', $.unary_expression)),

        binary_xor_expression: $ => choice($.unary_expression, seq($.binary_xor_expression, '^', $.unary_expression)),

        bitwise_expression: $ => choice(seq($.binary_and_expression, '&', $.unary_expression), seq($.binary_or_expression, '|', $.unary_expression), seq($.binary_xor_expression, '^', $.unary_expression)),

        expression: $ => choice($.relational_expression, seq($.short_circuit_or_expression, '||', $.relational_expression), seq($.short_circuit_and_expression, '&&', $.relational_expression), $.bitwise_expression),

        compound_statement: $ => seq(repeat($.attribute), '{', repeat($.statement), '}'),

        assignment_statement: $ => choice(seq($.lhs_expression, choice('=', $.compound_assignment_operator), $.expression), seq('_', '=', $.expression)),

        compound_assignment_operator: $ => choice('+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', $._shift_right_assign, $._shift_left_assign),

        increment_statement: $ => seq($.lhs_expression, '++'),

        decrement_statement: $ => seq($.lhs_expression, '--'),

        if_statement: $ => seq(repeat($.attribute), $.if_clause, repeat($.else_if_clause), optional($.else_clause)),

        if_clause: $ => seq('if', $.expression, $.compound_statement),

        else_if_clause: $ => seq('else', 'if', $.expression, $.compound_statement),

        else_clause: $ => seq('else', $.compound_statement),

        switch_statement: $ => seq(repeat($.attribute), 'switch', $.expression, $.switch_body),

        switch_body: $ => seq(repeat($.attribute), '{', repeat1($.switch_clause), '}'),

        switch_clause: $ => choice($.case_clause, $.default_alone_clause),

        case_clause: $ => seq('case', $.case_selectors, optional(':'), $.compound_statement),

        default_alone_clause: $ => seq('default', optional(':'), $.compound_statement),

        case_selectors: $ => seq($.case_selector, repeat(seq(',', $.case_selector)), optional(',')),

        case_selector: $ => choice('default', $.expression),

        loop_statement: $ => seq(repeat($.attribute), 'loop', repeat($.attribute), '{', repeat($.statement), optional($.continuing_statement), '}'),

        for_statement: $ => seq(repeat($.attribute), 'for', '(', $.for_header, ')', $.compound_statement),

        for_header: $ => seq(optional($.for_init), ';', optional($.expression), ';', optional($.for_update)),

        for_init: $ => choice($.variable_or_value_statement, $.variable_updating_statement, $.func_call_statement),

        for_update: $ => choice($.variable_updating_statement, $.func_call_statement),

        while_statement: $ => seq(repeat($.attribute), 'while', $.expression, $.compound_statement),

        break_statement: $ => 'break',

        break_if_statement: $ => seq('break', 'if', $.expression, ';'),

        continue_statement: $ => 'continue',

        continuing_statement: $ => seq('continuing', $.continuing_compound_statement),

        continuing_compound_statement: $ => seq(repeat($.attribute), '{', repeat($.statement), optional($.break_if_statement), '}'),

        return_statement: $ => seq('return', optional($.expression)),

        func_call_statement: $ => $.call_phrase,

        const_assert_statement: $ => seq('const_assert', $.expression),

        statement: $ => choice(';', seq($.return_statement, ';'), $.if_statement, $.switch_statement, $.loop_statement, $.for_statement, $.while_statement, seq($.func_call_statement, ';'), seq($.variable_or_value_statement, ';'), seq($.break_statement, ';'), seq($.continue_statement, ';'), seq('discard', ';'), seq($.variable_updating_statement, ';'), $.compound_statement, seq($.const_assert_statement, ';')),

        variable_updating_statement: $ => choice($.assignment_statement, $.increment_statement, $.decrement_statement),

        function_decl: $ => seq(repeat($.attribute), $.function_header, $.compound_statement),

        function_header: $ => seq('fn', $.ident, '(', optional($.param_list), ')', optional(seq('->', repeat($.attribute), $.template_elaborated_ident))),

        param_list: $ => seq($.param, repeat(seq(',', $.param)), optional(',')),

        param: $ => seq(repeat($.attribute), $.ident, ':', $.type_specifier),

        enable_directive: $ => seq('enable', $.enable_extension_list, ';'),

        enable_extension_list: $ => seq($.enable_extension_name, repeat(seq(',', $.enable_extension_name)), optional(',')),

        requires_directive: $ => seq('requires', $.software_extension_list, ';'),

        software_extension_list: $ => seq($.software_extension_name, repeat(seq(',', $.software_extension_name)), optional(',')),

        enable_extension_name: $ => $.ident_pattern_token,

        software_extension_name: $ => $.ident_pattern_token,

        ident_pattern_token: $ => /([_\p{XID_Start}][\p{XID_Continue}]+)|([\p{XID_Start}])/u,

        severity_control_name: $ => $.ident_pattern_token,

        swizzle_name: $ => choice(/[rgba]/, /[rgba][rgba]/, /[rgba][rgba][rgba]/, /[rgba][rgba][rgba][rgba]/, /[xyzw]/, /[xyzw][xyzw]/, /[xyzw][xyzw][xyzw]/, /[xyzw][xyzw][xyzw][xyzw]/),

        ident: $ => seq($.ident_pattern_token, $._disambiguate_template),

        _comment: $ => /\/\/.*/,

        _blankspace: $ => /[\u0020\u0009\u000a\u000b\u000c\u000d\u0085\u200e\u200f\u2028\u2029]/u
    }
})
