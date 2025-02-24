# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from typ import expectations_parser
from typ import json_results

ConflictResolutionTypes = expectations_parser.ConflictResolutionTypes
ResultType = json_results.ResultType
Expectation = expectations_parser.Expectation
TestExpectations = expectations_parser.TestExpectations


class ExpectationTest(unittest.TestCase):
    def create_expectation_with_values(self,
                                       reason='crbug.com/1234',
                                       test='foo',
                                       retry_on_failure=True,
                                       is_slow_test=True,
                                       tags=['win'],
                                       results=['FAIL'],
                                       lineno=10,
                                       trailing_comments=' # comment',
                                       encode_func=None):
        return expectations_parser.Expectation(
            reason=reason,
            test=test,
            retry_on_failure=retry_on_failure,
            is_slow_test=is_slow_test,
            tags=tags,
            results=results,
            lineno=lineno,
            trailing_comments=trailing_comments,
            encode_func=encode_func)

    def testEquality(self):
        e = self.create_expectation_with_values()
        other = self.create_expectation_with_values()
        self.assertEqual(e, other)

        other = self.create_expectation_with_values(reason='crbug.com/2345')
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(test='bar')
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(retry_on_failure=False)
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(is_slow_test=False)
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(tags=['linux'])
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(results=['Pass'])
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(lineno=20)
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(trailing_comments='c')
        self.assertNotEqual(e, other)

        other = self.create_expectation_with_values(encode_func=lambda x: x)
        self.assertNotEqual(e, other)

    def testEncodeApplied(self):
        e = expectations_parser.Expectation(reason='crbug.com/1234',
                                            test='foo',
                                            tags=['win'],
                                            results=['FAIL'],
                                            trailing_comments=' # comment',
                                            encode_func=lambda _: 'bar')
        self.assertEqual(e.to_string(),
                        'crbug.com/1234 [ Win ] bar [ Failure ] # comment')


class TaggedTestListParserTest(unittest.TestCase):
    def testInitWithGoodData(self):
        good_data = """
# This is a test expectation file.
#
# tags: [ Release Debug ]
# tags: [ Linux
#   Mac Mac10.1 Mac10.2
#   Win ]
# results: [ Skip ]

crbug.com/12345 [ Mac ] b1/s1 [ Skip ] # foo
crbug.com/23456 [ Mac Debug ] b1/s2 [ Skip ]
"""
        parser = expectations_parser.TaggedTestListParser(good_data)
        tag_sets = set([frozenset(['debug', 'release']),
                    frozenset(['linux', 'mac', 'mac10.1', 'mac10.2', 'win'])])
        self.assertEqual(tag_sets, parser.tag_sets)
        expected_outcome = [
            expectations_parser.Expectation('crbug.com/12345', 'b1/s1',
                                            ['mac'], ['SKIP'], 10, trailing_comments=' # foo'),
            expectations_parser.Expectation('crbug.com/23456', 'b1/s2',
                                            ['mac', 'debug'], ['SKIP'], 11)
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testInitWithBadData(self):
        bad_data = """
# This is a test expectation file.
#
# tags: [ tag1 tag2 tag3 ]
# tags: [ tag4 ]
# results: [ Skip ]

crbug.com/12345 [ Mac b1/s1 [ Skip ]
"""
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(bad_data)

    def testInitWithNonCanonicalResultTagDefinition(self):
        bad_data = """
# This is a test expectation file.
#
# tags: [ tag1 tag2 tag3 ]
# tags: [ tag4 ]
# results: [ skip ]
"""
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(bad_data)

    def testInitWithNonCanonicalResultTagUsage(self):
        bad_data = """
# This is a test expectation file.
#
# tags: [ tag1 tag2 tag3 ]
# tags: [ tag4 ]
# results: [ Skip ]

crbug.com/12345 [ tag1 ] some/test/name [ SKIP ]
"""
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(bad_data)

    def testTagAfterExpectationsStart(self):
        bad_data = """
# This is a test expectation file.
#
# tags: [ tag1 tag2 tag3 ]

crbug.com/12345 [ tag1 ] b1/s1 [ Skip ]

# tags: [ tag4 ]
"""
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(bad_data)

    def testParseExpectationLineEverythingThere(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/23456 [ Mac ] b1/s2 [ Skip ]'
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation('crbug.com/23456', 'b1/s2',
                                            ['mac'], ['SKIP'], 3)
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineAngleProjectNoTags(self):
        raw_data = '# tags: [ ]\n# results: [ Skip ]\ncrbug.com/angleproject/23456 b1/s2 [ Skip ]'
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation('crbug.com/angleproject/23456',
                                            'b1/s2', [], ['SKIP'], 3)
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineBadProject(self):
        raw_data = '# tags: [ ]\n# results: [ Skip ]\ncrbug.com/bad/project/23456 b1/s2 [ Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineBadTag(self):
        raw_data = '# tags: None\n# results: [ Skip ]\ncrbug.com/23456 [ Mac ] b1/s2 [ Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineNoTags(self):
        raw_data = '# tags: [ All ]\n# results: [ Skip ]\ncrbug.com/12345 b1/s1 [ Skip ]'
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation('crbug.com/12345', 'b1/s1', [],
                                            ['SKIP'], 3),
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineNoBug(self):
        raw_data = '# tags: [ All ]\n# results: [ Skip ]\n[ All ] b1/s1 [ Skip ]'
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation(None, 'b1/s1', ['all'], ['SKIP'], 3),
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineNoBugNoTags(self):
        raw_data = '# tags: [ All ]\n# results: [ Skip ]\nb1/s1 [ Skip ]'
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation(None, 'b1/s1', [], ['SKIP'], 3),
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineMultipleTags(self):
        raw_data = ('# tags: [ All None batman ]\n'
                    '# results: [ Skip Pass Failure ]\n'
                    'crbug.com/123 [ all ] b1/s1 [ Skip ]\n'
                    'crbug.com/124 [ None ] b1/s2 [ Pass ]\n'
                    'crbug.com/125 [ Batman ] b1/s3 [ Failure ]')
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation(
                'crbug.com/123', 'b1/s1', ['all'], ['SKIP'], 3),
            expectations_parser.Expectation(
                'crbug.com/124', 'b1/s2', ['none'], ['PASS'], 4),
            expectations_parser.Expectation(
                'crbug.com/125', 'b1/s3', ['batman'], ['FAIL'], 5)
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testParseExpectationLineBadTagBracket(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/23456 ] Mac ] b1/s2 [ Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineBadResultBracket(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/23456 ] Mac ] b1/s2 ] Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineBadTagBracketSpacing(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/2345 [Mac] b1/s1 [ Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineBadResultBracketSpacing(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/2345 [ Mac ] b1/s1 [Skip]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineNoClosingTagBracket(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/2345 [ Mac b1/s1 [ Skip ]'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineNoClosingResultBracket(self):
        raw_data = '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/2345 [ Mac ] b1/s1 [ Skip'
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseExpectationLineUrlInTestName(self):
        raw_data = (
            '# tags: [ Mac ]\n# results: [ Skip ]\ncrbug.com/123 [ Mac ] b.1/http://google.com [ Skip ]'
        )
        expected_outcomes = [
            expectations_parser.Expectation(
                'crbug.com/123', 'b.1/http://google.com', ['mac'], ['SKIP'], 3)
        ]
        parser = expectations_parser.TaggedTestListParser(raw_data)
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcomes[i])

    def testParseExpectationLineEndingComment(self):
        raw_data = ('# tags: [ Mac ]\n# results: [ Skip ]\n'
                    'crbug.com/23456 [ Mac ] b1/s2 [ Skip ] # abc 123')
        parser = expectations_parser.TaggedTestListParser(raw_data)
        expected_outcome = [
            expectations_parser.Expectation('crbug.com/23456', 'b1/s2',
                                            ['mac'], ['SKIP'], 3,
                                            trailing_comments=' # abc 123')
        ]
        for i in range(len(parser.expectations)):
            self.assertEqual(parser.expectations[i], expected_outcome[i])

    def testSingleLineTagAfterMultiLineTagWorks(self):
        expectations_file = """
# This is a test expectation file.
#
# tags: [ tag1 tag2
#         tag3 tag5
#         tag6
# ]
# tags: [ tag4 ]
# results: [ Skip ]

crbug.com/12345 [ tag3 tag4 ] b1/s1 [ Skip ]
"""
        expectations_parser.TaggedTestListParser(expectations_file)

    def testParseBadMultiline_1(self):
        raw_data = ('# tags: [ Mac\n'
                    '          Win\n'
                    '# ]\n# results: [ skip ]\n'
                    'crbug.com/23456 [ Mac ] b1/s2 [ SKip ]')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseTwoSetsOfTagsOnOneLineAreNotAllowed(self):
        raw_data = ('# tags: [ Debug ] [ Release ]\n')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseTrailingTextAfterTagSetIsNotAllowed(self):
        raw_data = ('# tags: [ Debug\n'
                    '#  ] # Release\n')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseBadMultiline_2(self):
        raw_data = ('# tags: [ Mac\n'
                    '          Win ]\n'
                    'crbug.com/23456 [ Mac ] b1/s2 [ Skip ]')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testParseUnknownResult(self):
        raw_data = ('# tags: [ Mac ]\n'
                    'crbug.com/23456 [ Mac ] b1/s2 [ UnknownResult ]')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testOneTagInMultipleTagsets(self):
        raw_data = ('# tags: [ Mac Win Linux ]\n'
                    '# tags: [ Mac BMW ]')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertEqual(
            '1: The tag mac was found in multiple tag sets',
            str(context.exception))

    def testTwoTagsinMultipleTagsets(self):
        raw_data = ('\n# tags: [ Mac Linux ]\n# tags: [ Mac BMW Win ]\n'
                    '# tags: [ Win Android ]\n# tags: [ iOS ]')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertEqual(
            '2: The tags mac and win were found in multiple tag sets',
            str(context.exception))

    def testTwoPlusTagsinMultipleTagsets(self):
        raw_data = ('\n\n# tags: [ Mac Linux ]\n# tags: [ Mac BMW Win ]\n'
                    '# tags: [ Win Android ]\n# tags: [ IOS bmw ]')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertEqual(
            '3: The tags bmw, mac and win'
            ' were found in multiple tag sets',
            str(context.exception))

    def testTwoTagsetPairsSharingTags(self):
        raw_data = ('\n\n\n# tags: [ Mac Linux Win ]\n# tags: [ mac BMW Win ]\n'
                    '# tags: [ android ]\n# tags: [ IOS Android ]')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertEqual(
            '4: The tags android, mac and win'
            ' were found in multiple tag sets',
            str(context.exception))

    def testDisjointTagsets(self):
        raw_data = ('# tags: [ Mac Win Linux ]\n'
                    '# tags: [ Honda BMW ]')
        expectations_parser.TaggedTestListParser(raw_data)

    def testEachTagInGroupIsNotFromDisjointTagSets(self):
        raw_data = (
            '# tags: [ Mac Win Amd Intel]\n'
            '# tags: [Linux Batman Robin Superman]\n'
            '# results: [ Pass ]\n'
            'crbug.com/23456 [ mac Win Amd robin Linux ] b1/s1 [ Pass ]\n')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertIn(
            '4: The tag group contains tags '
            'that are part of the same tag set\n',
            str(context.exception))
        self.assertIn('  - Tags linux and robin are part of the same tag set',
                      str(context.exception))
        self.assertIn('  - Tags amd, mac and win are part of the same tag set',
                      str(context.exception))
        self.assertNotIn('  - Tags webgl-version-1', str(context.exception))

    def testEachTagInGroupIsFromDisjointTagSets(self):
        raw_data = (
            '# tags: [ Mac Win Linux ]\n'
            '# tags: [ Batman Robin Superman ]\n'
            '# tags: [ Android Iphone ]\n'
            '# results: [ Failure Pass Skip ]\n'
            'crbug.com/23456 [ android Mac Superman ] b1/s1 [ Failure ]\n'
            'crbug.com/23457 [ Iphone win Robin ] b1/s2 [ Pass ]\n'
            'crbug.com/23458 [ Android linux  ] b1/s3 [ Pass ]\n'
            'crbug.com/23459 [ Batman ] b1/s4 [ Skip ]\n')
        expectations_parser.TaggedTestListParser(raw_data)

    def testDuplicateTagsInGroupRaisesError(self):
        raw_data = (
            '# tags: [ Mac Win Linux ]\n'
            '# tags: [ Batman Robin Superman ]\n'
            '# results: [ Failure ]\n'
            'crbug.com/23456 [ Batman Batman Batman ] b1/s1 [ Failure ]\n')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertIn('4: The tag group contains '
                      'tags that are part of the same tag set\n',
                      str(context.exception))
        self.assertIn('  - Tags batman, batman and batman are'
                      ' part of the same tag set', str(context.exception))

    def testRetryOnFailureExpectation(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ linux ] b1/s1 [ RetryOnFailure ]\n')
        parser = expectations_parser.TaggedTestListParser(raw_data)
        exp = parser.expectations[0]
        self.assertEqual(exp.should_retry_on_failure, True)

    def testDefaultPass(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ Failure ]\n'
            'crbug.com/23456 [ linux ] b1/s1 [ Failure ]\n')
        expectations = expectations_parser.TestExpectations(tags=['linux'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Failure]))
        self.assertFalse(exp.is_default_pass)
        self.assertFalse(exp.is_slow_test)

        exp = expectations.expectations_for('b1/s2')
        self.assertEqual(exp.results, set([ResultType.Pass]))
        self.assertTrue(exp.is_default_pass)
        self.assertFalse(exp.is_slow_test)

    def testSlowDefaultPassAndFailure(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ Failure Slow ]\n'
            'crbug.com/23456 [ Linux ] b1/s1 [ Failure ]\n'
            'crbug.com/23456 b1/s1 [ Slow ]\n')

        expectations = expectations_parser.TestExpectations(tags=['linux'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Failure]))
        self.assertFalse(exp.is_default_pass)
        self.assertTrue(exp.is_slow_test)

        expectations = expectations_parser.TestExpectations(tags=['win'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Pass]))
        self.assertTrue(exp.is_default_pass)
        self.assertTrue(exp.is_slow_test)

    def testRetryOnFailureDefaultPassAndFailure(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ Failure RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] b1/s1 [ Failure ]\n'
            'crbug.com/23456 b1/s1 [ RetryOnFailure ]\n')

        expectations = expectations_parser.TestExpectations(tags=['linux'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Failure]))
        self.assertFalse(exp.is_default_pass)
        self.assertTrue(exp.should_retry_on_failure)

        expectations = expectations_parser.TestExpectations(tags=['win'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Pass]))
        self.assertTrue(exp.is_default_pass)
        self.assertTrue(exp.should_retry_on_failure)

    def testGetExpectationsFromGlob(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ Failure ]\n'
            'crbug.com/23456 [ linux ] b1/s1* [ Failure ]\n')
        expectations = expectations_parser.TestExpectations(tags=['linux'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Failure]))

    def testGetExpectationsFromGlobShorterThanLongestMatchingGlob(self):
        raw_data = (
            '# tags: [ Linux Mac ]\n'
            '# results: [ Failure Pass ]\n'
            'crbug.com/23456 [ linux ] b1/s1* [ Failure ]\n'
            'crbug.com/23456 [ mac ] b1/* [ Pass ]\n')
        expectations = expectations_parser.TestExpectations(tags=['mac'])
        expectations.parse_tagged_list(raw_data)
        exp = expectations.expectations_for('b1/s1')
        self.assertEqual(exp.results, set([ResultType.Pass]))

    def testIsTestRetryOnFailure(self):
        raw_data = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure ]\n'
            '# conflicts_allowed: true\n'
            'crbug.com/23456 [ Linux ] b1/s1 [ Failure ]\n'
            'crbug.com/23456 [ Linux ] b1/s1 [ RetryOnFailure ]\n'
            '[ linux ] b1/s2 [ RetryOnFailure ]\n'
            'crbug.com/24341 [ Linux ] b1/s3 [ Failure ]\n')
        test_expectations = expectations_parser.TestExpectations(['Linux'])
        self.assertEqual(
            test_expectations.parse_tagged_list(raw_data, 'test.txt'), (0,''))
        self.assertEqual(test_expectations.expectations_for('b1/s1'),
                         Expectation(
                             test='b1/s1', results={ResultType.Failure}, retry_on_failure=True,
                             is_slow_test=False, reason='crbug.com/23456',
                             tags={'linux'}))
        self.assertEqual(test_expectations.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2', results={ResultType.Pass}, retry_on_failure=True,
                             is_slow_test=False, tags={'linux'}))
        self.assertEqual(test_expectations.expectations_for('b1/s3'),
                         Expectation(
                             test='b1/s3', results={ResultType.Failure}, retry_on_failure=False,
                             is_slow_test=False, reason='crbug.com/24341', tags={'linux'}))
        self.assertEqual(test_expectations.expectations_for('b1/s4'),
                         Expectation(
                             test='b1/s4', results={ResultType.Pass}, retry_on_failure=False,
                             is_slow_test=False))

    def testParseMultipleListsNonMatchingTagSets(self):
        initial_data = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] foo.html [ Failure ]\n')
        secondary_data = (
            '# tags: [ win ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ win ] foo.html [ Failure ]\n')
        parser = expectations_parser.TestExpectations()
        parser.parse_tagged_list(initial_data)
        with self.assertRaisesRegexp(RuntimeError,
            'Existing tag sets .* do not match incoming sets .*'):
            parser.parse_tagged_list(secondary_data)

    def testParseMultipleListsMatchingTagSets(self):
        """Verify parsing multiple lists is equivalent to a single combined list."""
        initial_data = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] foo.html [ Failure ]\n'
            '[ linux ] asdf* [ Failure ]\n')
        secondary_data = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] bar.html [ Failure ]\n'
            '[ linux ] qwer* [ Failure ]\n')
        combined_data = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] foo.html [ Failure ]\n'
            '[ linux ] bar.html [ Failure ]\n'
            '[ linux ] asdf* [ Failure ]\n'
            '[ linux ] qwer* [ Failure ]\n')

        parser = expectations_parser.TestExpectations(['linux'])
        parser.parse_tagged_list(initial_data)
        parser.parse_tagged_list(secondary_data)
        combined_parser = expectations_parser.TestExpectations(['linux'])
        combined_parser.parse_tagged_list(combined_data)

        foo_expectation = expectations_parser.Expectation(
            test='foo.html', results=[json_results.ResultType.Failure],
            tags=['linux'])
        self.assertEqual(parser.expectations_for('foo.html'),
                         foo_expectation)
        self.assertEqual(combined_parser.expectations_for('foo.html'),
                         foo_expectation)

        bar_expectation = expectations_parser.Expectation(
            test='bar.html', results=[json_results.ResultType.Failure],
            tags=['linux'])
        self.assertEqual(parser.expectations_for('bar.html'),
                         bar_expectation)
        self.assertEqual(combined_parser.expectations_for('bar.html'),
                         bar_expectation)

        asdf_expectation = expectations_parser.Expectation(
            test='asdf/test.html', results=[json_results.ResultType.Failure],
            tags=['linux'])
        self.assertEqual(parser.expectations_for('asdf/test.html'),
                         asdf_expectation)
        self.assertEqual(combined_parser.expectations_for('asdf/test.html'),
                         asdf_expectation)

        qwer_expectation = expectations_parser.Expectation(
            test='qwer/test.html', results=[json_results.ResultType.Failure],
            tags=['linux'])
        self.assertEqual(parser.expectations_for('qwer/test.html'),
                         qwer_expectation)
        self.assertEqual(combined_parser.expectations_for('qwer/test.html'),
                         qwer_expectation)

    def testMergeExpectationsUsingUnionResolution(self):
        raw_data1 = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] b1/s3 [ Failure ]\n'
            'crbug.com/2431 [ linux ] b1/s2 [ Failure RetryOnFailure ] # c1\n'
            'crbug.com/2432 [ linux ] b1/s* [ Failure Slow ]\n')
        raw_data2 = (
            '# tags: [ Intel ]\n'
            '# results: [ Pass RetryOnFailure ]\n'
            '[ intel ] b1/s1 [ RetryOnFailure ]\n'
            'crbug.com/2432 [ intel ] b1/s2 [ Pass ] # c2\n'
            'crbug.com/2431 [ intel ] b1/s* [ RetryOnFailure ]\n')
        test_exp1 = expectations_parser.TestExpectations(['Linux'])
        ret, _ = test_exp1.parse_tagged_list(raw_data1)
        self.assertEqual(ret, 0)
        test_exp2 = expectations_parser.TestExpectations(['Intel'])
        ret, _ = test_exp2.parse_tagged_list(raw_data2)
        self.assertEqual(ret, 0)
        test_exp1.merge_test_expectations(test_exp2)
        self.assertEqual(sorted(test_exp1.tags), ['intel', 'linux'])
        self.assertEqual(test_exp1.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2',
                             results={ResultType.Pass, ResultType.Failure},
                             retry_on_failure=True, is_slow_test=False,
                             reason='crbug.com/2431 crbug.com/2432',
                             trailing_comments=' # c1\n # c2\n',
                             tags={'linux', 'intel'}))
        self.assertEqual(test_exp1.expectations_for('b1/s1'),
                         Expectation(
                             test='b1/s1', results={ResultType.Pass},
                             retry_on_failure=True, is_slow_test=False,
                             tags={'intel'}))
        self.assertEqual(test_exp1.expectations_for('b1/s3'),
                         Expectation(
                             test='b1/s3', results={ResultType.Failure},
                             retry_on_failure=False, is_slow_test=False,
                             tags={'linux'}))
        self.assertEqual(test_exp1.expectations_for('b1/s5'),
                         Expectation(
                             test='b1/s5', results={ResultType.Failure},
                             retry_on_failure=True, is_slow_test=True,
                             reason='crbug.com/2431 crbug.com/2432',
                             tags={'linux', 'intel'}))

    def testResolutionReturnedFromExpectationsFor(self):
        raw_data1 = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] b1/s3 [ Failure ]\n'
            'crbug.com/2431 [ linux ] b1/s2 [ Failure RetryOnFailure ]\n'
            'crbug.com/2432 [ linux ] b1/s* [ Failure ]\n')
        raw_data2 = (
            '# tags: [ Intel ]\n'
            '# results: [ Pass RetryOnFailure Slow ]\n'
            '[ intel ] b1/s1 [ RetryOnFailure ]\n'
            'crbug.com/2432 [ intel ] b1/s2 [ Pass Slow ]\n'
            'crbug.com/2431 [ intel ] b1/s* [ RetryOnFailure ]\n')
        raw_data3 = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '# conflict_resolution: OVERRIDE\n'
            '[ linux ] b1/s3 [ Failure ]\n'
            'crbug.com/2431 [ linux ] b1/s2 [ Failure RetryOnFailure ]\n'
            'crbug.com/2432 [ linux ] b1/s* [ Failure ]\n')
        test_exp1 = expectations_parser.TestExpectations(['Linux'])
        ret, _ = test_exp1.parse_tagged_list(raw_data1)
        self.assertEqual(ret, 0)
        self.assertEqual(test_exp1.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2', results={ResultType.Failure},
                             retry_on_failure=True, is_slow_test=False,
                             reason='crbug.com/2431', tags={'linux'},
                             conflict_resolution=ConflictResolutionTypes.UNION
                         ))

        test_exp2 = expectations_parser.TestExpectations(['Intel'])
        ret, _ = test_exp2.parse_tagged_list(
            raw_data2,
            conflict_resolution = ConflictResolutionTypes.OVERRIDE)
        self.assertEqual(ret, 0)
        self.assertEqual(test_exp2.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2', results={ResultType.Pass},
                             retry_on_failure=False, is_slow_test=True,
                             reason='crbug.com/2432', tags={'intel'},
                             conflict_resolution=ConflictResolutionTypes.OVERRIDE
                         ))

        test_exp3 = expectations_parser.TestExpectations(['Linux'])
        ret, _ = test_exp3.parse_tagged_list(raw_data3)
        self.assertEqual(ret, 0)
        self.assertEqual(test_exp3.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2', results={ResultType.Failure},
                             retry_on_failure=True, is_slow_test=False,
                             reason='crbug.com/2431', tags={'linux'},
                             conflict_resolution=ConflictResolutionTypes.OVERRIDE
                         ))

    def testMergeExpectationsUsingOverrideResolution(self):
        raw_data1 = (
            '# tags: [ linux ]\n'
            '# results: [ Failure RetryOnFailure Slow ]\n'
            '[ linux ] b1/s3 [ Failure ]\n'
            'crbug.com/2431 [ linux ] b1/s2 [ Failure RetryOnFailure ]\n'
            'crbug.com/2432 [ linux ] b1/s* [ Failure ]\n')
        raw_data2 = (
            '# tags: [ Intel ]\n'
            '# results: [ Pass RetryOnFailure Slow ]\n'
            '[ intel ] b1/s1 [ RetryOnFailure ]\n'
            'crbug.com/2432 [ intel ] b1/s2 [ Pass Slow ]\n'
            'crbug.com/2431 [ intel ] b1/s* [ RetryOnFailure ]\n')
        test_exp1 = expectations_parser.TestExpectations(['Linux'])
        ret, _ = test_exp1.parse_tagged_list(raw_data1)
        self.assertEqual(ret, 0)
        test_exp2 = expectations_parser.TestExpectations(['Intel'])
        ret, _ = test_exp2.parse_tagged_list(
            raw_data2, conflict_resolution=ConflictResolutionTypes.OVERRIDE)
        self.assertEqual(ret, 0)
        test_exp1.merge_test_expectations(test_exp2)
        self.assertEqual(sorted(test_exp1.tags), ['intel', 'linux'])
        self.assertEqual(test_exp1.expectations_for('b1/s2'),
                         Expectation(
                             test='b1/s2', results={ResultType.Pass},
                             retry_on_failure=False, is_slow_test=True,
                             reason='crbug.com/2432', tags={'intel'}))
        self.assertEqual(test_exp1.expectations_for('b1/s1'),
                         Expectation(test='b1/s1', results={ResultType.Pass},
                                     retry_on_failure=True, is_slow_test=False,
                                     tags={'intel'}))
        self.assertEqual(test_exp1.expectations_for('b1/s3'),
                         Expectation(test='b1/s3', results={ResultType.Failure},
                                     retry_on_failure=False, is_slow_test=False,
                                     tags={'linux'}))
        self.assertEqual(test_exp1.expectations_for('b1/s5'),
                         Expectation(test='b1/s5', results={ResultType.Pass},
                                     retry_on_failure=True, is_slow_test=False,
                                     reason='crbug.com/2431',
                                     tags={'intel'}))

    def testIsNotTestRetryOnFailureUsingEscapedGlob(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] b1/\* [ RetryOnFailure ]\n')
        test_expectations = expectations_parser.TestExpectations(['Linux'])
        self.assertEqual(
            test_expectations.parse_tagged_list(raw_data, 'test.txt'),
            (0, ''))
        self.assertIn('b1/*', test_expectations.individual_exps)
        self.assertEqual(test_expectations.expectations_for('b1/s1'),
                         Expectation(test='b1/s1', results={ResultType.Pass},
                                     retry_on_failure=False, is_slow_test=False))

    def testIsTestRetryOnFailureUsingGlob(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] b1/* [ RetryOnFailure ]\n')
        test_expectations = expectations_parser.TestExpectations(['Linux'])
        self.assertEqual(
            test_expectations.parse_tagged_list(raw_data, 'test.txt'),
            (0, ''))
        self.assertEqual(test_expectations.expectations_for('b1/s1'),
                         Expectation(test='b1/s1', results={ResultType.Pass},
                                     retry_on_failure=True, is_slow_test=False,
                                     reason='crbug.com/23456', tags={'linux'}))

    def testGlobsCanExistInMiddleofPatternUsingEscapeCharacter(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] b1/\*/c [ RetryOnFailure ]\n')
        expectations_parser.TaggedTestListParser(raw_data)

    def testGlobsCanOnlyHaveStarInEnd(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] b1/*/c [ RetryOnFailure ]\n')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testGlobsCanOnlyHaveStarInEnd1(self):
        raw_data = (
            '# tags: [ Linux ]\n'
            '# results: [ RetryOnFailure ]\n'
            'crbug.com/23456 [ Linux ] */c [ RetryOnFailure ]\n')
        with self.assertRaises(expectations_parser.ParseError):
            expectations_parser.TaggedTestListParser(raw_data)

    def testUseIncorrectvalueForConflictsAllowedDescriptor(self):
        test_expectations = '''# tags: [ mac win linux ]
        # tags: [ intel amd nvidia ]
        # tags: [ debug release ]
        # results: [ Failure Skip ]
        # conflicts_allowed: Unknown
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(test_expectations, 'test.txt')
        self.assertEqual("5: Unrecognized value 'unknown' "
                         "given for conflicts_allowed descriptor", msg)

    def testConflictsInTestExpectation(self):
        expectations = expectations_parser.TestExpectations()
        _, errors = expectations.parse_tagged_list(
            '# tags: [ mac win linux ]\n'
            '# tags: [ intel amd nvidia ]\n'
            '# tags: [ debug release ]\n'
            '# conflicts_allowed: False\n'
            '# results: [ Failure Skip RetryOnFailure ]\n'
            '[ intel win ] a/b/c/* [ Failure ]\n'
            '[ intel win debug ] a/b/c/* [ Skip ]\n'
            '[ intel  ] a/b/c/* [ Failure ]\n'
            '[ amd mac ] a/b [ RetryOnFailure ]\n'
            '[ mac ] a/b [ Skip ]\n'
            '[ amd mac ] a/b/c [ Failure ]\n'
            '[ intel mac ] a/b/c [ Failure ]\n', 'test.txt')
        self.assertIn("Found conflicts for pattern a/b/c/* in test.txt:",
                      errors)
        self.assertIn('line 6 conflicts with line 7', errors)
        self.assertIn('line 6 conflicts with line 8', errors)
        self.assertIn('line 7 conflicts with line 8', errors)
        self.assertIn("Found conflicts for pattern a/b in test.txt:", errors)
        self.assertIn('line 9 conflicts with line 10', errors)
        self.assertNotIn("Found conflicts for pattern a/b/c in test.txt:",
                         errors)

    def testFileNameExcludedFromErrorMessageForExpectationConflicts(self):
        test_expectations = '''# tags: [ mac ]
        # tags: [ intel ]
        # results: [ Failure ]
        [ intel ] a/b/c/d [ Failure ]
        [ mac ] a/b/c/d [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, errors = expectations.parse_tagged_list(test_expectations)
        self.assertIn("Found conflicts for pattern a/b/c/d:", errors)

    def testConflictsUsingUserDefinedTagsConflictFunction(self):
        test_expectations = '''# tags: [ win win7  ]
        # results: [ Failure ]
        [ win ] a/b/c/d [ Failure ]
        [ win7 ] a/b/c/d [ Failure ]
        '''
        map_child_tag_to_parent_tag = {'win7': 'win'}
        tags_conflict = lambda t1, t2: (
            t1 != t2 and t1 != map_child_tag_to_parent_tag.get(t2,t2) and
            t2 != map_child_tag_to_parent_tag.get(t1,t1))
        expectations = expectations_parser.TestExpectations()
        _, errors = expectations.parse_tagged_list(
            test_expectations, tags_conflict=tags_conflict)
        self.assertIn("Found conflicts for pattern a/b/c/d:", errors)

    def testNoCollisionInTestExpectations(self):
        test_expectations = '''# tags: [ mac win linux ]
        # tags: [ intel amd nvidia ]
        # tags: [ debug release ]
        # results: [ Failure ]
        # conflicts_allowed: False
        [ intel debug ] a/b/c/d [ Failure ]
        [ nvidia debug ] a/b/c/d [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, errors = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(errors)

    def testConflictsAllowedIsSetToTrue(self):
        test_expectations = '''# tags: [ mac win linux ]
        # tags: [ intel amd nvidia ]
        # tags: [ debug release ]
        # results: [ Failure ]
        # conflicts_allowed: True
        [ intel debug ] a/b/c/d [ Failure ]
        [ intel ] a/b/c/d [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)

    def testConflictFoundRegardlessOfTagCase(self):
        test_expectations = '''# tags: [ InTel AMD nvidia ]
        # results: [ Failure ]
        [ intel ] a/b/c/d [ Failure ]
        [ Intel ] a/b/c/d [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        ret, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertTrue(ret)
        self.assertIn('Found conflicts for pattern a/b/c/d', msg)

    def testConflictNotFoundRegardlessOfTagCase(self):
        test_expectations = '''# tags: [ InTel AMD nvidia ]
        # results: [ Failure ]
        [ intel ] a/b/c/d [ Failure ]
        [ amd ] a/b/c/d [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)

    def testExpectationPatternIsBroken(self):
        test_expectations = '# results: [ Failure ]\na/\* [ Failure ]'
        expectations = expectations_parser.TestExpectations()
        expectations.parse_tagged_list(test_expectations, 'test.txt')
        broken_expectations = expectations.check_for_broken_expectations(
            ['a/b/c'])
        self.assertEqual(broken_expectations[0].test, 'a/*')

    def testExpectationPatternIsNotBroken(self):
        test_expectations = '# results: [ Failure ]\na/b/d [ Failure ]'
        expectations = expectations_parser.TestExpectations()
        expectations.parse_tagged_list(test_expectations, 'test.txt')
        broken_expectations = expectations.check_for_broken_expectations(
            ['a/b/c'])
        self.assertEqual(broken_expectations[0].test, 'a/b/d')

    def testExpectationWithGlobIsBroken(self):
        test_expectations = '# results: [ Failure ]\na/b/d* [ Failure ]'
        expectations = expectations_parser.TestExpectations()
        expectations.parse_tagged_list(test_expectations, 'test.txt')
        broken_expectations = expectations.check_for_broken_expectations(
            ['a/b/c/d', 'a/b', 'a/b/c'])
        self.assertEqual(broken_expectations[0].test, 'a/b/d*')

    def testExpectationWithGlobIsNotBroken(self):
        test_expectations = '# results: [ Failure ]\na/b* [ Failure ]'
        expectations = expectations_parser.TestExpectations()
        expectations.parse_tagged_list(test_expectations, 'test.txt')
        broken_expectations = expectations.check_for_broken_expectations(
            ['a/b'])
        self.assertFalse(broken_expectations)

    def testNonDeclaredSystemConditionTagRaisesException(self):
        test_expectations = '''# tags: [ InTel AMD nvidia ]
        # tags: [ win ]
        # results: [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)
        with self.assertRaises(ValueError) as context:
            expectations.set_tags(['Unknown'], raise_ex_for_bad_tags=True)
        self.assertEqual(str(context.exception),
            'Tag unknown is not declared in the expectations file and has not '
            'been explicitly ignored by the test. There may have been a typo '
            'in the expectations file. Please make sure the aforementioned tag '
            'is declared at the top of the expectations file.')

    def testNonDeclaredSystemConditionTagsRaisesException_PluralCase(self):
        test_expectations = '''# tags: [ InTel AMD nvidia ]
        # tags: [ win ]
        # results: [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)
        with self.assertRaises(ValueError) as context:
            expectations.set_tags(['Unknown', 'linux', 'nVidia', 'nvidia-0x1010'],
                                  raise_ex_for_bad_tags=True)
        self.assertEqual(str(context.exception),
            'Tags linux, nvidia-0x1010 and unknown are not declared in the '
            'expectations file and have not been explicitly ignored by the '
            'test. There may have been a typo in the expectations file. Please '
            'make sure the aforementioned tags are declared at the top of the '
            'expectations file.')

    def testIgnoredTags(self):
        test_expectations = """# tags: [ foo ]
        # results: [ Failure ]
        """
        expectations = expectations_parser.TestExpectations(
                ignored_tags=['ignored'])
        _, msg = expectations.parse_tagged_list(test_expectations, 'test.txt')
        self.assertFalse(msg)
        expectations.set_tags(['ignored'], raise_ex_for_bad_tags=True)

    def testDeclaredSystemConditionTagsDontRaiseAnException(self):
        test_expectations = '''# tags: [ InTel AMD nvidia nvidia-0x1010 ]
        # tags: [ win ]
        # results: [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)
        expectations.set_tags(['win', 'nVidia', 'nvidia-0x1010'],
                              raise_ex_for_bad_tags=True)

    def testMultipleReasonsForExpectation(self):
        test_expectations = '''# results: [ Failure ]
        skbug.com/111 crbug.com/wpt/222 skbug.com/hello/333 crbug.com/444 test [ Failure ]
        '''
        expectations = expectations_parser.TestExpectations()
        _, msg = expectations.parse_tagged_list(
            test_expectations, 'test.txt')
        self.assertFalse(msg)
        exp = expectations.expectations_for('test')
        self.assertEqual(exp.reason, 'skbug.com/111 crbug.com/wpt/222 skbug.com/hello/333 crbug.com/444')

    def testExpectationToString(self):
        exp = Expectation(reason='crbug.com/123', test='test.html?*', tags=['intel'],
                          results={ResultType.Pass, ResultType.Failure}, is_slow_test=True,
                          retry_on_failure=True)
        self.assertEqual(
            exp.to_string(), 'crbug.com/123 [ Intel ] test.html?\* [ Failure Pass RetryOnFailure Slow ]')

    def testGlobExpectationToString(self):
        exp = Expectation(reason='crbug.com/123', test='a/*/test.html?*', tags=['intel'],
                          results={ResultType.Pass, ResultType.Failure}, is_slow_test=True,
                          retry_on_failure=True, is_glob=True)
        self.assertEqual(
            exp.to_string(), 'crbug.com/123 [ Intel ] a/\*/test.html?* [ Failure Pass RetryOnFailure Slow ]')

    def testExpectationToStringUsingRawSpecifiers(self):
        raw_expectations = (
            '# tags: [ NVIDIA intel ]\n'
            '# results: [ Failure Pass Slow ]\n'
            'crbug.com/123 [ iNteL ] test.html?\* [ Failure Pass ]\n'
            '[ NVIDIA ] test.\*.* [ Slow ]  # hello world\n')
        test_exps = TestExpectations()
        ret, errors = test_exps.parse_tagged_list(raw_expectations)
        assert not ret, errors
        self.assertEqual(test_exps.individual_exps['test.html?*'][0].to_string(),
                         'crbug.com/123 [ iNteL ] test.html?\* [ Failure Pass ]')
        self.assertEqual(test_exps.glob_exps['test.*.*'][0].to_string(),
                         '[ NVIDIA ] test.\*.* [ Slow ]  # hello world')

    def testExpectationToStringAfterRenamingTest(self):
        exp = Expectation(reason='crbug.com/123', test='test.html?*', tags=['intel'],
                          results={ResultType.Pass, ResultType.Failure}, raw_tags=['iNteL'],
                          raw_results=['Failure', 'Pass'])
        exp.test = 'a/*/test.html?*'
        self.assertEqual(exp.to_string(), 'crbug.com/123 [ iNteL ] a/\*/test.html?\* [ Failure Pass ]')

    def testAddExpectationsToExpectation(self):
        raw_expectations = (
            '# tags: [ NVIDIA intel ]\n'
            '# results: [ Failure Pass Slow ]\n'
            'crbug.com/123 [ iNteL ] test.html?\* [ Pass Failure ]\n'
            '[ NVIDIA ] test.\*.* [ Slow ]  # hello world\n')
        test_exps = TestExpectations()
        ret, errors = test_exps.parse_tagged_list(raw_expectations)
        test_exps.individual_exps['test.html?*'][0].add_expectations(
            {ResultType.Timeout}, reason='crbug.com/123 crbug.com/124')
        assert not ret, errors
        self.assertEqual(test_exps.individual_exps['test.html?*'][0].results,
                         frozenset([ResultType.Pass, ResultType.Failure,
                                    ResultType.Timeout]))
        self.assertEqual(test_exps.individual_exps['test.html?*'][0].reason,
                         'crbug.com/123 crbug.com/124')

    def testAddingExistingExpectationsDoesntChangeRawResults(self):
        raw_expectations = (
            '# tags: [ NVIDIA intel ]\n'
            '# results: [ Failure Pass Slow ]\n'
            'crbug.com/123 [ iNteL ] test.html?\* [ Failure Pass ]\n'
            '[ NVIDIA ] test.\*.* [ Slow ]  # hello world\n')
        test_exps = TestExpectations()
        ret, errors = test_exps.parse_tagged_list(raw_expectations)
        test_exps.individual_exps['test.html?*'][0].add_expectations(
            {ResultType.Failure}, reason='crbug.com/124')
        assert not ret, errors
        self.assertIn('[ Failure Pass ]',
            test_exps.individual_exps['test.html?*'][0].to_string())

    def testTopDownOrderMaintainedForNonGlobExps(self):
        raw_expectations = (
            '# tags: [ NVIDIA intel ]\n'
            '# results: [ Failure Pass Slow ]\n'
            'crbug.com/123 [ iNteL ] test1 [ Pass Failure ]\n'
            'crbug.com/123 [ iNteL ] test2 [ Pass Failure ]\n'
            'crbug.com/123 [ iNteL ] test8 [ Pass Failure ]\n'
            'crbug.com/123 [ iNteL ] test9 [ Pass Failure ]\n'
            'crbug.com/123 [ iNteL ] test5 [ Pass Failure ]\n'
            '[ NVIDIA ] test.\* [ Slow ]  # hello world\n')
        test_exps = TestExpectations()
        ret, errors = test_exps.parse_tagged_list(raw_expectations)
        assert not ret, errors
        self.assertEqual(list(test_exps.individual_exps),
                         ['test1','test2','test8', 'test9', 'test5', 'test.*'])

    def testDuplicateTagsInTagsRaisesError(self):
        raw_data = (
            '# tags: [ Mac Win Linux Win ]\n'
            '# tags: [ Batman Robin Superman ]\n'
            '# results: [ Failure ]\n'
            'crbug.com/23456 [ Batman ] b1/s1 [ Failure ]\n')
        with self.assertRaises(expectations_parser.ParseError) as context:
            expectations_parser.TaggedTestListParser(raw_data)
        self.assertIn('1: duplicate tag(s): Win',
                      str(context.exception))
