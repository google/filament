#!/usr/bin/env vpython3
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import itertools
import logging
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import metrics_utils
# We have to disable monitoring before importing gclient.
metrics_utils.COLLECT_METRICS = False

import gclient_eval

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


def file_join(lines):
    return ''.join([l + '\n' for l in lines])


class GClientEvalTest(unittest.TestCase):
    def test_str(self):
        self.assertEqual('foo', gclient_eval._gclient_eval('"foo"'))

    def test_tuple(self):
        self.assertEqual(('a', 'b'), gclient_eval._gclient_eval('("a", "b")'))

    def test_list(self):
        self.assertEqual(['a', 'b'], gclient_eval._gclient_eval('["a", "b"]'))

    def test_dict(self):
        self.assertEqual({'a': 'b'}, gclient_eval._gclient_eval('{"a": "b"}'))

    def test_name_safe(self):
        self.assertEqual(True, gclient_eval._gclient_eval('True'))

    def test_name_unsafe(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval._gclient_eval('UnsafeName')
        self.assertIn('invalid name \'UnsafeName\'', str(cm.exception))

    def test_invalid_call(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval._gclient_eval('Foo("bar")')
        self.assertIn('Str and Var are the only allowed functions',
                      str(cm.exception))

    def test_expands_vars(self):
        self.assertEqual(
            'foo',
            gclient_eval._gclient_eval('Var("bar")', vars_dict={'bar': 'foo'}))
        self.assertEqual(
            'baz',
            gclient_eval._gclient_eval(
                'Var("bar")',
                vars_dict={'bar': gclient_eval.ConstantString('baz')}))

    def test_expands_vars_with_braces(self):
        self.assertEqual(
            'foo',
            gclient_eval._gclient_eval('"{bar}"', vars_dict={'bar': 'foo'}))
        self.assertEqual(
            'baz',
            gclient_eval._gclient_eval(
                '"{bar}"',
                vars_dict={'bar': gclient_eval.ConstantString('baz')}))

    def test_invalid_var(self):
        with self.assertRaises(KeyError) as cm:
            gclient_eval._gclient_eval('"{bar}"', vars_dict={})
        self.assertIn('bar was used as a variable, but was not declared',
                      str(cm.exception))

    def test_plus(self):
        self.assertEqual('foo', gclient_eval._gclient_eval('"f" + "o" + "o"'))

    def test_format(self):
        self.assertEqual('foo', gclient_eval._gclient_eval('"%s" % "foo"'))

    def test_not_expression(self):
        with self.assertRaises(SyntaxError) as cm:
            gclient_eval._gclient_eval('def foo():\n  pass')
        self.assertIn('invalid syntax', str(cm.exception))

    def test_not_whitelisted(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval._gclient_eval('[x for x in [1, 2, 3]]')
        self.assertIn('unexpected AST node', str(cm.exception))
        self.assertIn('ast.ListComp object', str(cm.exception))

    def test_dict_ordered(self):
        for test_case in itertools.permutations(range(4)):
            input_data = ['{'] + ['"%s": "%s",' % (n, n)
                                  for n in test_case] + ['}']
            expected = [(str(n), str(n)) for n in test_case]
            result = gclient_eval._gclient_eval(''.join(input_data))
            self.assertEqual(expected, list(result.items()))


class ExecTest(unittest.TestCase):
    def test_multiple_assignment(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.Exec('a, b, c = "a", "b", "c"')
        self.assertIn('invalid assignment: target should be a name',
                      str(cm.exception))

    def test_override(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.Exec('a = "a"\na = "x"')
        self.assertIn('invalid assignment: overrides var \'a\'',
                      str(cm.exception))

    def test_schema_wrong_type(self):
        with self.assertRaises(gclient_eval.Error):
            gclient_eval.Exec('include_rules = {}')

    def test_recursedeps_list(self):
        local_scope = gclient_eval.Exec(
            'recursedeps = [["src/third_party/angle", "DEPS.chromium"]]')
        self.assertEqual(
            {'recursedeps': [['src/third_party/angle', 'DEPS.chromium']]},
            local_scope)

    def test_var(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '  "foo": "bar",',
                '  "baz": Str("quux")',
                '}',
                'deps = {',
                '  "a_dep": "a" + Var("foo") + "b" + Var("baz"),',
                '}',
            ]))
        Str = gclient_eval.ConstantString
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar',
                    'baz': Str('quux')
                },
                'deps': {
                    'a_dep': 'abarbquux'
                },
            }, local_scope)

    def test_braces_var(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '  "foo": "bar",',
                '  "baz": Str("quux")',
                '}',
                'deps = {',
                '  "a_dep": "a{foo}b{baz}",',
                '}',
            ]))
        Str = gclient_eval.ConstantString
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar',
                    'baz': Str('quux')
                },
                'deps': {
                    'a_dep': 'abarbquux'
                },
            }, local_scope)

    def test_empty_deps(self):
        local_scope = gclient_eval.Exec('deps = {}')
        self.assertEqual({'deps': {}}, local_scope)

    def test_overrides_vars(self):
        local_scope = gclient_eval.Exec(file_join([
            'vars = {',
            '  "foo": "bar",',
            '  "quux": Str("quuz")',
            '}',
            'deps = {',
            '  "a_dep": "a{foo}b",',
            '  "b_dep": "c{quux}d",',
            '}',
        ]),
                                        vars_override={
                                            'foo': 'baz',
                                            'quux': 'corge'
                                        })
        Str = gclient_eval.ConstantString
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar',
                    'quux': Str('quuz')
                },
                'deps': {
                    'a_dep': 'abazb',
                    'b_dep': 'ccorged'
                },
            }, local_scope)

    def test_doesnt_override_undeclared_vars(self):
        with self.assertRaises(KeyError) as cm:
            gclient_eval.Exec(file_join([
                'vars = {',
                '  "foo": "bar",',
                '}',
                'deps = {',
                '  "a_dep": "a{baz}b",',
                '}',
            ]),
                              vars_override={'baz': 'lalala'})
        self.assertIn('baz was used as a variable, but was not declared',
                      str(cm.exception))

    def test_doesnt_allow_duplicate_deps(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.Parse(
                file_join([
                    'deps = {',
                    '  "a_dep": {',
                    '    "url": "a_url@a_rev",',
                    '    "condition": "foo",',
                    '  },',
                    '  "a_dep": {',
                    '    "url": "a_url@another_rev",',
                    '    "condition": "not foo",',
                    '  }',
                    '}',
                ]), '<unknown>')
        self.assertIn('duplicate key in dictionary: a_dep', str(cm.exception))


class UpdateConditionTest(unittest.TestCase):
    def test_both_present(self):
        info = {'condition': 'foo'}
        gclient_eval.UpdateCondition(info, 'and', 'bar')
        self.assertEqual(info, {'condition': '(foo) and (bar)'})

        info = {'condition': 'foo'}
        gclient_eval.UpdateCondition(info, 'or', 'bar')
        self.assertEqual(info, {'condition': '(foo) or (bar)'})

    def test_one_present_and(self):
        # If one of info's condition or new_condition is present, and |op| ==
        # 'and' then the the result must be the present condition.
        info = {'condition': 'foo'}
        gclient_eval.UpdateCondition(info, 'and', None)
        self.assertEqual(info, {'condition': 'foo'})

        info = {}
        gclient_eval.UpdateCondition(info, 'and', 'bar')
        self.assertEqual(info, {'condition': 'bar'})

    def test_both_absent_and(self):
        # Nothing happens
        info = {}
        gclient_eval.UpdateCondition(info, 'and', None)
        self.assertEqual(info, {})

    def test_or(self):
        # If one of info's condition and new_condition is not present, then
        # there shouldn't be a condition. An absent value is treated as
        # implicitly True.
        info = {'condition': 'foo'}
        gclient_eval.UpdateCondition(info, 'or', None)
        self.assertEqual(info, {})

        info = {}
        gclient_eval.UpdateCondition(info, 'or', 'bar')
        self.assertEqual(info, {})

        info = {}
        gclient_eval.UpdateCondition(info, 'or', None)
        self.assertEqual(info, {})


class EvaluateConditionTest(unittest.TestCase):
    def test_true(self):
        self.assertTrue(gclient_eval.EvaluateCondition('True', {}))

    def test_variable(self):
        self.assertFalse(gclient_eval.EvaluateCondition('foo',
                                                        {'foo': 'False'}))

    def test_variable_cyclic_reference(self):
        with self.assertRaises(ValueError) as cm:
            self.assertTrue(
                gclient_eval.EvaluateCondition('bar', {'bar': 'bar'}))
        self.assertIn('invalid cyclic reference to \'bar\' (inside \'bar\')',
                      str(cm.exception))

    def test_operators(self):
        self.assertFalse(
            gclient_eval.EvaluateCondition('a and not (b or c)', {
                'a': 'True',
                'b': 'False',
                'c': 'True'
            }))

    def test_expansion(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('a or b', {
                'a': 'b and c',
                'b': 'not c',
                'c': 'False'
            }))

    def test_string_equality(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('foo == "baz"', {'foo': '"baz"'}))
        self.assertFalse(
            gclient_eval.EvaluateCondition('foo == "bar"', {'foo': '"baz"'}))

    def test_string_inequality(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('foo != "bar"', {'foo': '"baz"'}))
        self.assertFalse(
            gclient_eval.EvaluateCondition('foo != "baz"', {'foo': '"baz"'}))

    def test_triple_or(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('a or b or c', {
                'a': 'False',
                'b': 'False',
                'c': 'True'
            }))
        self.assertFalse(
            gclient_eval.EvaluateCondition('a or b or c', {
                'a': 'False',
                'b': 'False',
                'c': 'False'
            }))

    def test_triple_and(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('a and b and c', {
                'a': 'True',
                'b': 'True',
                'c': 'True'
            }))
        self.assertFalse(
            gclient_eval.EvaluateCondition('a and b and c', {
                'a': 'True',
                'b': 'True',
                'c': 'False'
            }))

    def test_triple_and_and_or(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('a and b and c or d or e', {
                'a': 'False',
                'b': 'False',
                'c': 'False',
                'd': 'False',
                'e': 'True'
            }))
        self.assertFalse(
            gclient_eval.EvaluateCondition('a and b and c or d or e', {
                'a': 'True',
                'b': 'True',
                'c': 'False',
                'd': 'False',
                'e': 'False'
            }))

    def test_string_bool(self):
        self.assertFalse(
            gclient_eval.EvaluateCondition('false_str_var and true_var', {
                'false_str_var': 'False',
                'true_var': True
            }))

    def test_string_bool_typo(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.EvaluateCondition('false_var_str and true_var', {
                'false_str_var': 'False',
                'true_var': True
            })
        self.assertIn(
            'invalid "and" operand \'false_var_str\' '
            '(inside \'false_var_str and true_var\')', str(cm.exception))

    def test_non_bool_in_or(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.EvaluateCondition('string_var or true_var', {
                'string_var': 'Kittens',
                'true_var': True
            })
        self.assertIn(
            'invalid "or" operand \'Kittens\' '
            '(inside \'string_var or true_var\')', str(cm.exception))

    def test_non_bool_in_and(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.EvaluateCondition('string_var and true_var', {
                'string_var': 'Kittens',
                'true_var': True
            })
        self.assertIn(
            'invalid "and" operand \'Kittens\' '
            '(inside \'string_var and true_var\')', str(cm.exception))

    def test_tuple_presence(self):
        self.assertTrue(
            gclient_eval.EvaluateCondition('foo in ("bar", "baz")',
                                           {'foo': 'bar'}))
        self.assertFalse(
            gclient_eval.EvaluateCondition('foo in ("bar", "baz")',
                                           {'foo': 'not_bar'}))

    def test_unsupported_tuple_operation(self):
        with self.assertRaises(ValueError) as cm:
            gclient_eval.EvaluateCondition('foo == ("bar", "baz")',
                                           {'foo': 'bar'})
        self.assertIn('unexpected AST node', str(cm.exception))

        with self.assertRaises(ValueError) as cm:
            gclient_eval.EvaluateCondition('(foo,) == "bar"', {'foo': 'bar'})
        self.assertIn('unexpected AST node', str(cm.exception))

    def test_str_in_condition(self):
        Str = gclient_eval.ConstantString
        self.assertTrue(
            gclient_eval.EvaluateCondition('s_var == "foo"',
                                           {'s_var': Str("foo")}))

        self.assertFalse(
            gclient_eval.EvaluateCondition('s_var in ("baz", "quux")',
                                           {'s_var': Str("foo")}))


class VarTest(unittest.TestCase):
    def assert_adds_var(self, before, after):
        local_scope = gclient_eval.Exec(file_join(before))
        gclient_eval.AddVar(local_scope, 'baz', 'lemur')
        results = gclient_eval.RenderDEPSFile(local_scope)
        self.assertEqual(results, file_join(after))

    def test_adds_var(self):
        before = [
            'vars = {',
            '  "foo": "bar",',
            '}',
        ]
        after = [
            'vars = {',
            '  "baz": "lemur",',
            '  "foo": "bar",',
            '}',
        ]
        self.assert_adds_var(before, after)

    def test_adds_var_twice(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '  "foo": "bar",',
                '}',
            ]))

        gclient_eval.AddVar(local_scope, 'baz', 'lemur')
        gclient_eval.AddVar(local_scope, 'v8_revision', 'deadbeef')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'vars = {',
                '  "v8_revision": "deadbeef",',
                '  "baz": "lemur",',
                '  "foo": "bar",',
                '}',
            ]))

    def test_gets_and_sets_var(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '  "foo": "bar",',
                '  "quux": Str("quuz")',
                '}',
            ]))

        self.assertEqual(gclient_eval.GetVar(local_scope, 'foo'), "bar")
        self.assertEqual(gclient_eval.GetVar(local_scope, 'quux'), "quuz")

        gclient_eval.SetVar(local_scope, 'foo', 'baz')
        gclient_eval.SetVar(local_scope, 'quux', 'corge')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'vars = {',
                '  "foo": "baz",',
                '  "quux": Str("corge")',
                '}',
            ]))

    def test_gets_and_sets_var_non_string(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '  "foo": True,',
                '}',
            ]))

        result = gclient_eval.GetVar(local_scope, 'foo')
        self.assertEqual(result, True)

        gclient_eval.SetVar(local_scope, 'foo', 'False')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(result,
                         file_join([
                             'vars = {',
                             '  "foo": False,',
                             '}',
                         ]))

    def test_add_preserves_formatting(self):
        before = [
            '# Copyright stuff',
            '# some initial comments',
            '',
            'vars = { ',
            '  # Some comments.',
            '  "foo": "bar",',
            '',
            '  # More comments.',
            '  # Even more comments.',
            '  "v8_revision":   ',
            '       "deadbeef",',
            ' # Someone formatted this wrong',
            '}',
        ]
        after = [
            '# Copyright stuff',
            '# some initial comments',
            '',
            'vars = { ',
            '  "baz": "lemur",',
            '  # Some comments.',
            '  "foo": "bar",',
            '',
            '  # More comments.',
            '  # Even more comments.',
            '  "v8_revision":   ',
            '       "deadbeef",',
            ' # Someone formatted this wrong',
            '}',
        ]
        self.assert_adds_var(before, after)

    def test_set_preserves_formatting(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '   # Comment with trailing space ',
                ' "foo": \'bar\',',
                '}',
            ]))

        gclient_eval.SetVar(local_scope, 'foo', 'baz')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'vars = {',
                '   # Comment with trailing space ',
                ' "foo": \'baz\',',
                '}',
            ]))


class CipdTest(unittest.TestCase):
    def test_gets_and_sets_cipd(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "some/cipd/package",',
                '                "version": "deadbeef",',
                '            },',
                '            {',
                '                "package": "another/cipd/package",',
                '                "version": "version:5678",',
                '            },',
                '        ],',
                '        "condition": "checkout_android",',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))

        self.assertEqual(
            gclient_eval.GetCIPD(local_scope, 'src/cipd/package',
                                 'some/cipd/package'), 'deadbeef')

        self.assertEqual(
            gclient_eval.GetCIPD(local_scope, 'src/cipd/package',
                                 'another/cipd/package'), 'version:5678')

        gclient_eval.SetCIPD(local_scope, 'src/cipd/package',
                             'another/cipd/package', 'version:6789')
        gclient_eval.SetCIPD(local_scope, 'src/cipd/package',
                             'some/cipd/package', 'foobar')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "some/cipd/package",',
                '                "version": "foobar",',
                '            },',
                '            {',
                '                "package": "another/cipd/package",',
                '                "version": "version:6789",',
                '            },',
                '        ],',
                '        "condition": "checkout_android",',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))

    def test_gets_and_sets_cipd_vars(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'vars = {',
                '    "cipd-rev": "git_revision:deadbeef",',
                '    "another-cipd-rev": "version:1.0.3",',
                '}',
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "some/cipd/package",',
                '                "version": Var("cipd-rev"),',
                '            },',
                '            {',
                '                "package": "another/cipd/package",',
                '                "version": "{another-cipd-rev}",',
                '            },',
                '        ],',
                '        "condition": "checkout_android",',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))

        self.assertEqual(
            gclient_eval.GetCIPD(local_scope, 'src/cipd/package',
                                 'some/cipd/package'), 'git_revision:deadbeef')

        self.assertEqual(
            gclient_eval.GetCIPD(local_scope, 'src/cipd/package',
                                 'another/cipd/package'), 'version:1.0.3')

        gclient_eval.SetCIPD(local_scope, 'src/cipd/package',
                             'another/cipd/package', 'version:1.1.0')
        gclient_eval.SetCIPD(local_scope, 'src/cipd/package',
                             'some/cipd/package', 'git_revision:foobar')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'vars = {',
                '    "cipd-rev": "git_revision:foobar",',
                '    "another-cipd-rev": "version:1.1.0",',
                '}',
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "some/cipd/package",',
                '                "version": Var("cipd-rev"),',
                '            },',
                '            {',
                '                "package": "another/cipd/package",',
                '                "version": "{another-cipd-rev}",',
                '            },',
                '        ],',
                '        "condition": "checkout_android",',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))

    def test_preserves_escaped_vars(self):
        local_scope = gclient_eval.Exec(
            file_join([
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "package/${{platform}}",',
                '                "version": "version:abcd",',
                '            },',
                '        ],',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))

        gclient_eval.SetCIPD(local_scope, 'src/cipd/package',
                             'package/${platform}', 'version:dcba')
        result = gclient_eval.RenderDEPSFile(local_scope)

        self.assertEqual(
            result,
            file_join([
                'deps = {',
                '    "src/cipd/package": {',
                '        "packages": [',
                '            {',
                '                "package": "package/${{platform}}",',
                '                "version": "version:dcba",',
                '            },',
                '        ],',
                '        "dep_type": "cipd",',
                '    },',
                '}',
            ]))


class RevisionTest(unittest.TestCase):
    def assert_gets_and_sets_revision(self,
                                      before,
                                      after,
                                      rev_before='deadbeef'):
        local_scope = gclient_eval.Exec(file_join(before))

        result = gclient_eval.GetRevision(local_scope, 'src/dep')
        self.assertEqual(result, rev_before)

        gclient_eval.SetRevision(local_scope, 'src/dep', 'deadfeed')
        self.assertEqual(file_join(after),
                         gclient_eval.RenderDEPSFile(local_scope))

    def test_revision(self):
        before = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@deadbeef",',
            '}',
        ]
        after = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@deadfeed",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_revision_new_line(self):
        before = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@"',
            '             + "deadbeef",',
            '}',
        ]
        after = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@"',
            '             + "deadfeed",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_revision_windows_local_path(self):
        before = [
            'deps = {',
            '  "src/dep": "file:///C:\\\\path.git@deadbeef",',
            '}',
        ]
        after = [
            'deps = {',
            '  "src/dep": "file:///C:\\\\path.git@deadfeed",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_revision_multiline_strings(self):
        deps = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@"',
            '             "deadbeef",',
            '}',
        ]
        with self.assertRaises(ValueError) as e:
            local_scope = gclient_eval.Exec(file_join(deps))
            gclient_eval.SetRevision(local_scope, 'src/dep', 'deadfeed')
        self.assertEqual(
            'Can\'t update value for src/dep. Multiline strings and implicitly '
            'concatenated strings are not supported.\n'
            'Consider reformatting the DEPS file.', str(e.exception))

    def test_revision_implicitly_concatenated_strings(self):
        deps = [
            'deps = {',
            '  "src/dep": "https://example.com" + "/dep.git@" "deadbeef",',
            '}',
        ]
        with self.assertRaises(ValueError) as e:
            local_scope = gclient_eval.Exec(file_join(deps))
            gclient_eval.SetRevision(local_scope, 'src/dep', 'deadfeed')
        self.assertEqual(
            'Can\'t update value for src/dep. Multiline strings and implicitly '
            'concatenated strings are not supported.\n'
            'Consider reformatting the DEPS file.', str(e.exception))

    def test_revision_inside_dict(self):
        before = [
            'deps = {',
            '  "src/dep": {',
            '    "url": "https://example.com/dep.git@deadbeef",',
            '    "condition": "some_condition",',
            '  },',
            '}',
        ]
        after = [
            'deps = {',
            '  "src/dep": {',
            '    "url": "https://example.com/dep.git@deadfeed",',
            '    "condition": "some_condition",',
            '  },',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_follows_var_braces(self):
        before = [
            'vars = {',
            '  "dep_revision": "deadbeef",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@{dep_revision}",',
            '}',
        ]
        after = [
            'vars = {',
            '  "dep_revision": "deadfeed",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@{dep_revision}",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_follows_var_braces_newline(self):
        before = [
            'vars = {',
            '  "dep_revision": "deadbeef",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git"',
            '             + "@{dep_revision}",',
            '}',
        ]
        after = [
            'vars = {',
            '  "dep_revision": "deadfeed",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git"',
            '             + "@{dep_revision}",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_follows_var_function(self):
        before = [
            'vars = {',
            '  "dep_revision": "deadbeef",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@" + Var("dep_revision"),',
            '}',
        ]
        after = [
            'vars = {',
            '  "dep_revision": "deadfeed",',
            '}',
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@" + Var("dep_revision"),',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_pins_revision(self):
        before = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git",',
            '}',
        ]
        after = [
            'deps = {',
            '  "src/dep": "https://example.com/dep.git@deadfeed",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after, rev_before=None)

    def test_preserves_variables(self):
        before = [
            'vars = {',
            '  "src_root": "src"',
            '}',
            'deps = {',
            '  "{src_root}/dep": "https://example.com/dep.git@deadbeef",',
            '}',
        ]
        after = [
            'vars = {',
            '  "src_root": "src"',
            '}',
            'deps = {',
            '  "{src_root}/dep": "https://example.com/dep.git@deadfeed",',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)

    def test_preserves_formatting(self):
        before = [
            'vars = {',
            ' # Some comment on deadbeef ',
            '  "dep_revision": "deadbeef",',
            '}',
            'deps = {',
            '  "src/dep": {',
            '    "url": "https://example.com/dep.git@" + Var("dep_revision"),',
            '',
            '    "condition": "some_condition",',
            ' },',
            '}',
        ]
        after = [
            'vars = {',
            ' # Some comment on deadbeef ',
            '  "dep_revision": "deadfeed",',
            '}',
            'deps = {',
            '  "src/dep": {',
            '    "url": "https://example.com/dep.git@" + Var("dep_revision"),',
            '',
            '    "condition": "some_condition",',
            ' },',
            '}',
        ]
        self.assert_gets_and_sets_revision(before, after)


class ParseTest(unittest.TestCase):
    def callParse(self, vars_override=None):
        return gclient_eval.Parse(
            file_join([
                'vars = {',
                '  "foo": "bar",',
                '}',
                'deps = {',
                '  "a_dep": "a{foo}b",',
                '}',
            ]), '<unknown>', vars_override)

    def test_supports_vars_inside_vars(self):
        deps_file = file_join([
            'vars = {',
            '  "foo": "bar",',
            '  "baz": "\\"{foo}\\" == \\"bar\\"",',
            '}',
            'deps = {',
            '  "src/baz": {',
            '    "url": "baz_url",',
            '    "condition": "baz",',
            '  },',
            '}',
        ])
        local_scope = gclient_eval.Parse(deps_file, '<unknown>', None)
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar',
                    'baz': '"bar" == "bar"'
                },
                'deps': {
                    'src/baz': {
                        'url': 'baz_url',
                        'dep_type': 'git',
                        'condition': 'baz'
                    }
                },
            }, local_scope)

    def test_has_builtin_vars(self):
        builtin_vars = {'builtin_var': 'foo'}
        deps_file = file_join([
            'deps = {',
            '  "a_dep": "a{builtin_var}b",',
            '}',
        ])
        local_scope = gclient_eval.Parse(deps_file, '<unknown>', None,
                                         builtin_vars)
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'afoob',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_declaring_builtin_var_has_no_effect(self):
        builtin_vars = {'builtin_var': 'foo'}
        deps_file = file_join([
            'vars = {',
            '  "builtin_var": "bar",',
            '}',
            'deps = {',
            '  "a_dep": "a{builtin_var}b",',
            '}',
        ])
        local_scope = gclient_eval.Parse(deps_file, '<unknown>', None,
                                         builtin_vars)
        self.assertEqual(
            {
                'vars': {
                    'builtin_var': 'bar'
                },
                'deps': {
                    'a_dep': {
                        'url': 'afoob',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_override_builtin_var(self):
        builtin_vars = {'builtin_var': 'foo'}
        vars_override = {'builtin_var': 'override'}
        deps_file = file_join([
            'deps = {',
            '  "a_dep": "a{builtin_var}b",',
            '}',
        ])
        local_scope = gclient_eval.Parse(deps_file, '<unknown>', vars_override,
                                         builtin_vars)
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'aoverrideb',
                        'dep_type': 'git'
                    }
                },
            }, local_scope, str(local_scope))

    def test_expands_vars(self):
        local_scope = self.callParse()
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar'
                },
                'deps': {
                    'a_dep': {
                        'url': 'abarb',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_overrides_vars(self):
        local_scope = self.callParse(vars_override={'foo': 'baz'})
        self.assertEqual(
            {
                'vars': {
                    'foo': 'bar'
                },
                'deps': {
                    'a_dep': {
                        'url': 'abazb',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_no_extra_vars(self):
        deps_file = file_join([
            'vars = {',
            '  "foo": "bar",',
            '}',
            'deps = {',
            '  "a_dep": "a{baz}b",',
            '}',
        ])

        with self.assertRaises(KeyError) as cm:
            gclient_eval.Parse(deps_file, '<unknown>', {'baz': 'lalala'})
        self.assertIn('baz was used as a variable, but was not declared',
                      str(cm.exception))

    def test_standardizes_deps_string_dep(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": "a_url@a_rev",',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_standardizes_deps_dict_dep(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": {',
                '     "url": "a_url@a_rev",',
                '     "condition": "checkout_android",',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git',
                        'condition': 'checkout_android'
                    }
                },
            }, local_scope)

    def test_ignores_none_in_deps_os(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": "a_url@a_rev",',
                '}',
                'deps_os = {',
                '  "mac": {',
                '     "a_dep": None,',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_merges_deps_os_extra_dep(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": "a_url@a_rev",',
                '}',
                'deps_os = {',
                '  "mac": {',
                '     "b_dep": "b_url@b_rev"',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git'
                    },
                    'b_dep': {
                        'url': 'b_url@b_rev',
                        'dep_type': 'git',
                        'condition': 'checkout_mac'
                    }
                },
            }, local_scope)

    def test_merges_deps_os_existing_dep_with_no_condition(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": "a_url@a_rev",',
                '}',
                'deps_os = {',
                '  "mac": {',
                '     "a_dep": "a_url@a_rev"',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git'
                    }
                },
            }, local_scope)

    def test_merges_deps_os_existing_dep_with_condition(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps = {',
                '  "a_dep": {',
                '    "url": "a_url@a_rev",',
                '    "condition": "some_condition",',
                '  },',
                '}',
                'deps_os = {',
                '  "mac": {',
                '     "a_dep": "a_url@a_rev"',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git',
                        'condition': '(checkout_mac) or (some_condition)'
                    },
                },
            }, local_scope)

    def test_merges_deps_os_multiple_os(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'deps_os = {',
                '  "win": {'
                '     "a_dep": "a_url@a_rev"',
                '  },',
                '  "mac": {',
                '     "a_dep": "a_url@a_rev"',
                '  },',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                'deps': {
                    'a_dep': {
                        'url': 'a_url@a_rev',
                        'dep_type': 'git',
                        'condition': '(checkout_mac) or (checkout_win)'
                    },
                },
            }, local_scope)

    def test_fails_to_merge_same_dep_with_different_revisions(self):
        with self.assertRaises(gclient_eval.Error) as cm:
            gclient_eval.Parse(
                file_join([
                    'deps = {',
                    '  "a_dep": {',
                    '    "url": "a_url@a_rev",',
                    '    "condition": "some_condition",',
                    '  },',
                    '}',
                    'deps_os = {',
                    '  "mac": {',
                    '     "a_dep": "a_url@b_rev"',
                    '  },',
                    '}',
                ]), '<unknown>')
        self.assertIn('conflicts with existing deps', str(cm.exception))

    def test_merges_hooks_os(self):
        local_scope = gclient_eval.Parse(
            file_join([
                'hooks = [',
                '  {',
                '    "action": ["a", "action"],',
                '  },',
                ']',
                'hooks_os = {',
                '  "mac": [',
                '    {',
                '       "action": ["b", "action"]',
                '    },',
                '  ]',
                '}',
            ]), '<unknown>')
        self.assertEqual(
            {
                "hooks": [{
                    "action": ["a", "action"]
                }, {
                    "action": ["b", "action"],
                    "condition": "checkout_mac"
                }],
            }, local_scope)


if __name__ == '__main__':
    level = logging.DEBUG if '-v' in sys.argv else logging.FATAL
    logging.basicConfig(level=level,
                        format='%(asctime).19s %(levelname)s %(filename)s:'
                        '%(lineno)s %(message)s')
    unittest.main()
