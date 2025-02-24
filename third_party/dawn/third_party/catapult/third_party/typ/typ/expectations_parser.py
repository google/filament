# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(dpranke): Rename this to 'expectations.py' to remove the 'parser'
# part and make it a bit more generic. Consider if we can reword this to
# also not talk about 'expectations' so much (i.e., to find a clearer way
# to talk about them that doesn't have quite so much legacy baggage), but
# that might not be possible.

import itertools
import re
import logging

from collections import Counter, OrderedDict
from collections import defaultdict

from typ import python_2_3_compat
from typ.json_results import ResultType

_EXPECTATION_MAP = {
    'Crash': ResultType.Crash,
    'Failure': ResultType.Failure,
    'Pass': ResultType.Pass,
    'Timeout': ResultType.Timeout,
    'Skip': ResultType.Skip
}

RESULT_TAGS = {
    ResultType.Failure: 'Failure',
    ResultType.Crash: 'Crash',
    ResultType.Timeout: 'Timeout',
    ResultType.Pass: 'Pass',
    ResultType.Skip: 'Skip'
}

_SLOW_TAG = 'Slow'
_RETRY_ON_FAILURE_TAG = 'RetryOnFailure'

VALID_RESULT_TAGS = set(
    list(_EXPECTATION_MAP.keys()) +
    [_SLOW_TAG, _RETRY_ON_FAILURE_TAG])


class ConflictResolutionTypes(object):
    UNION = 1
    OVERRIDE = 2


def _group_to_string(group):
    msg = ', '.join(group)
    k = msg.rfind(', ')
    return msg[:k] + ' and ' + msg[k+2:] if k != -1 else msg


def _default_tags_conflict(t1, t2):
    return t1 != t2


class ParseError(Exception):

    def __init__(self, lineno, msg):
        super(ParseError, self).__init__('%d: %s' % (lineno, msg))


class Expectation(object):
    def __init__(self, reason=None, test='*', tags=None, results=None, lineno=0,
                 retry_on_failure=False, is_slow_test=False,
                 conflict_resolution=ConflictResolutionTypes.UNION, raw_tags=None, raw_results=None,
                 is_glob=False, trailing_comments=None, encode_func=None):
        """Constructor for expectations.

        Args:
          reason: String that indicates the reason for the expectation.
          test: String indicating which test is being affected.
          tags: List of tags that the expectation applies to. Tags are combined
              using a logical and, i.e., all of the tags need to be present for
              the expectation to apply. For example, if tags = ['Mac', 'Debug'],
              then the test must be running with the 'Mac' and 'Debug' tags
              set; just 'Mac', or 'Mac' and 'Release', would not qualify.
          results: List of outcomes for test. Example: ['Skip', 'Pass']
          encode_func: A reference that takes a string and returns an encoded
              string. This encoding will be applied when creating a string
              representation of the expectation. If unset, no encoding will be
              performed.
        """
        tags = tags or []
        self._is_default_pass = not results
        results = results or {ResultType.Pass}
        reason = reason or ''
        trailing_comments = trailing_comments or ''
        assert python_2_3_compat.is_str(reason)
        assert python_2_3_compat.is_str(test)
        self._reason = reason
        self._test = test
        self._tags = frozenset(tags)
        self._results = frozenset(results)
        self._lineno = lineno
        self._raw_tags = raw_tags
        self._raw_results = raw_results
        self.should_retry_on_failure = retry_on_failure
        self.is_slow_test = is_slow_test
        self.conflict_resolution = conflict_resolution
        self._is_glob = is_glob
        self._trailing_comments = trailing_comments
        self.encode_func = encode_func

    def __eq__(self, other):
        return (self.reason == other.reason and self.test == other.test
                and self.should_retry_on_failure == other.should_retry_on_failure
                and self.is_slow_test == other.is_slow_test
                and self.tags == other.tags and self.results == other.results
                and self.lineno == other.lineno
                and self.trailing_comments == other.trailing_comments
                and self.encode_func == other.encode_func)

    def _set_string_value(self):
        """This method will create an expectation line in string form and set the
        _string_value member variable to it. If the _raw_results lists and _raw_tags
        list are not set then the _tags list and _results set will be used to set them.
        Setting the _raw_results and _raw_tags list to the original lists through the constructor
        during parsing stops unintended modifications to test expectations when rewriting files.
        """
        # If this instance is for a glob type expectation then do not escape
        # the last asterisk
        if self.is_glob:
            assert len(self._test) and self._test[-1] == '*', (
                'For Expectation instances for glob type expectations, the test value '
                'must end in an asterisk')
            pattern = self._test[:-1].replace('*', '\\*') + '*'
        else:
            pattern = self._test.replace('*', '\\*')
        if self.encode_func:
            pattern = self.encode_func(pattern)
        self._string_value = ''
        if self._reason:
            self._string_value += self._reason + ' '
        if self.raw_tags:
            self._string_value += '[ %s ] ' % ' '.join(sorted(self.raw_tags))
        self._string_value += pattern + ' '
        self._string_value += '[ %s ]' % ' '.join(sorted(self.raw_results))
        if self._trailing_comments:
            self._string_value += self._trailing_comments

    def add_expectations(self, results, reason=None):
        if reason:
            self._reason = ' '.join(sorted(set(self._reason.split() + reason.split())))
        if not results <= self._results:
            self._results = frozenset(self._results | results)
            self._raw_results = sorted(
                [RESULT_TAGS[t] for t in self._results])

    @property
    def raw_tags(self):
        if not self._raw_tags:
            self._raw_tags = {t[0].upper() + t[1:].lower() for t in self._tags}
        return self._raw_tags

    @property
    def raw_results(self):
        if not self._raw_results:
            self._raw_results = {RESULT_TAGS[t] for t in self._results}
            if self.is_slow_test:
                self._raw_results.add(_SLOW_TAG)
            if self.should_retry_on_failure:
                self._raw_results.add(_RETRY_ON_FAILURE_TAG)
        return self._raw_results

    def to_string(self):
        self._set_string_value()
        return self._string_value

    @property
    def is_default_pass(self):
        return self._is_default_pass

    @property
    def reason(self):
        return self._reason

    @property
    def test(self):
        return self._test

    @test.setter
    def test(self, v):
        if not len(v):
            raise ValueError('Cannot set test to empty string')
        if self.is_glob and v[-1] != '*':
            raise ValueError(
                'test value for glob type expectations must end with an asterisk')
        self._test = v

    @property
    def tags(self):
        return self._tags

    @property
    def results(self):
        return self._results

    @property
    def lineno(self):
        return self._lineno

    @lineno.setter
    def lineno(self, lineno):
        self._lineno = lineno

    @property
    def is_glob(self):
        return self._is_glob

    @property
    def trailing_comments(self):
        return self._trailing_comments


class TaggedTestListParser(object):
    """Parses lists of tests and expectations for them.

    This parser covers the 'tagged' test lists format in:
        bit.ly/chromium-test-list-format

    Takes raw expectations data as a string read from the expectation file
    in the format:

      # This is an example expectation file.
      #
      # tags: [
      #   Mac Mac10.1 Mac10.2
      #   Win Win8
      # ]
      # tags: [ Release Debug ]

      crbug.com/123 [ Win ] benchmark/story [ Skip ]
      ...
    """
    CONFLICTS_ALLOWED = '# conflicts_allowed: '
    CONFLICT_RESOLUTION = '# conflict_resolution: '
    RESULT_TOKEN = '# results: ['
    TAG_TOKEN = '# tags: ['
    # The bug field (optional), including optional subproject.
    BUG_PREFIX_REGEX = '(?:crbug.com/|skbug.com/|webkit.org/|b/)'
    _MATCH_STRING = r'^(?:(' + BUG_PREFIX_REGEX + r'(?:[^/]*/)?\d+\s)*)'
    _MATCH_STRING += r'(?:\[ (.+) \] )?'  # The label field (optional).
    _MATCH_STRING += r'(\S+) '  # The test path field.
    _MATCH_STRING += r'\[ ([^\[.]+) \]'  # The expectation field.
    _MATCH_STRING += r'(\s+#.*)?$'  # End comment (optional).
    MATCHER = re.compile(_MATCH_STRING)

    def __init__(self, raw_data,
                 conflict_resolution=ConflictResolutionTypes.UNION,
                 encode_func=None, decode_func=None):
        self.tag_sets = set()
        self.conflicts_allowed = False
        self.expectations = []
        self._allowed_results = set()
        self._tag_to_tag_set = {}
        self.conflict_resolution = conflict_resolution
        self._encode_func = encode_func
        self._decode_func = decode_func
        self._parse_raw_expectation_data(raw_data)

    def _parse_raw_expectation_data(self, raw_data):
        all_lines = raw_data.splitlines()
        lineno = 1
        tag_sets_intersection = set()
        first_tag_line = None
        while lineno <= len(all_lines):
            line = all_lines[lineno - 1].strip()
            if (line.startswith((self.TAG_TOKEN, self.RESULT_TOKEN))):
                if not first_tag_line:
                    first_tag_line = lineno
                tag_sets_intersection.update(
                    self._parse_header_token_line(lineno, line, all_lines))
            elif line.startswith(self.CONFLICT_RESOLUTION):
                self._parse_conflict_resolution_line(lineno, line)
            elif line.startswith(self.CONFLICTS_ALLOWED):
                self._parse_conflicts_allowed_line(lineno, line)
            elif line.startswith('#') or not line:
                # Ignore, it is just a comment or empty.
                lineno += 1
                continue
            elif not tag_sets_intersection:
                self.expectations.append(
                    self._parse_expectation_line(lineno, line))
            else:
                break
            lineno += 1
        if tag_sets_intersection:
            is_multiple_tags = len(tag_sets_intersection) > 1
            tag_tags = 'tags' if is_multiple_tags else 'tag'
            was_were = 'were' if is_multiple_tags else 'was'
            error_msg = 'The {0} {1} {2} found in multiple tag sets'.format(
                tag_tags, _group_to_string(
                    sorted(list(tag_sets_intersection))), was_were)
            raise ParseError(first_tag_line, error_msg)

    def _parse_header_token_line(self, lineno, line, all_lines):
        """Helper function for parsing lines that start with header tokens.

        Returns:
            A set of strings containing any tag set intersections found while
            parsing the given line.
        """
        if self.expectations:
            raise ParseError(lineno,
                             'Tag found after first expectation.')
        if line.startswith(self.TAG_TOKEN):
            token = self.TAG_TOKEN
        else:
            token = self.RESULT_TOKEN

        tag_counts = self._get_tag_counts_from_header_line(
            lineno, line, all_lines, token)
        tag_set = set(tag_counts.keys())
        duplicate_tags = {tag for tag, count in tag_counts.items() if count > 1}

        if duplicate_tags:
            message = 'duplicate tag(s): {}'.format(
                ', '.join(sorted(duplicate_tags)))
            raise ParseError(lineno, message)

        tag_sets_intersection = set()
        if token == self.TAG_TOKEN:
            tag_set = frozenset([t.lower() for t in tag_set])
            tag_sets_intersection.update(
                (t for t in tag_set if t in self._tag_to_tag_set))
            self.tag_sets.add(tag_set)
            self._tag_to_tag_set.update(
                {tg: id(tag_set) for tg in tag_set})
        else:
            for t in tag_set:
                if t not in VALID_RESULT_TAGS:
                    raise ParseError(
                        lineno,
                        'Result tag set [%s] contains values not in '
                        'the list of known values [%s]' %
                        (', '.join(tag_set),
                         ', '.join(VALID_RESULT_TAGS)))
            self._allowed_results.update(tag_set)
        return tag_sets_intersection

    def _get_tag_counts_from_header_line(self, lineno, line, all_lines, token):
        """Helper function for parsing tags from a header line.

        Returns:
            A Counter object containing counts for each found tag.
        """
        right_bracket = line.find(']')
        prefix_size = len(token)
        tag_counts = Counter()
        # Loop through every line until we find the closing ], adding any tags
        # we fine. The line with the closing ] is handled after the loop.
        while lineno <= len(all_lines) and right_bracket == -1:
            tag_counts.update(line[prefix_size:].split())
            lineno += 1
            line = all_lines[lineno - 1].strip()
            prefix_size = 1
            if line[0] != '#':
                raise ParseError(
                    lineno,
                    'Multi-line tag set missing leading "#"')
            right_bracket = line.find(']')

        if line[right_bracket+1:]:
            raise ParseError(
                lineno,
                'Nothing is allowed after a closing tag '
                'bracket')

        tag_counts.update(line[prefix_size:right_bracket].split())
        return tag_counts

    def _parse_conflict_resolution_line(self, lineno, line):
        """Helper function for parsing conflict resolution annotations."""
        value = line[len(self.CONFLICT_RESOLUTION):].lower()
        if value not in ('union', 'override'):
            raise ParseError(
                lineno,
                ("Unrecognized value '%s' given for conflict_resolution"
                 "descriptor" %
                 value))
        if value == 'union':
            self.conflict_resolution = ConflictResolutionTypes.UNION
        else:
            self.conflict_resolution = ConflictResolutionTypes.OVERRIDE

    def _parse_conflicts_allowed_line(self, lineno, line):
        """Helper function for parsing conflicts allowed annotations."""
        bool_value = line[len(self.CONFLICTS_ALLOWED):].lower()
        if bool_value not in ('true', 'false'):
            raise ParseError(
                lineno,
                ("Unrecognized value '%s' given for conflicts_allowed "
                 "descriptor" %
                 bool_value))
        self.conflicts_allowed = bool_value == 'true'

    def _parse_expectation_line(self, lineno, line):
        reason, raw_tags, test, raw_results, trailing_comments =\
            self._parse_expectation_line_into_components(lineno, line)
        tags = [raw_tag.lower()
                for raw_tag in raw_tags.split()] if raw_tags else []
        self._validate_expectation_structure(lineno, test, tags)
        results, retry_on_failure, is_slow_test =\
            self._parse_and_validate_raw_results(lineno, raw_results)

        if self._decode_func:
            test = self._decode_func(test)

        # remove escapes for asterisks
        is_glob = not test.endswith('\\*') and test.endswith('*')
        test = test.replace('\\*', '*')
        if raw_tags:
            raw_tags = raw_tags.split()
        if raw_results:
            raw_results = raw_results.split()
        # Tags from tag groups will be stored in lower case in the Expectation
        # instance. These tags will be compared to the tags passed in to
        # the Runner instance which are also stored in lower case.
        return Expectation(
            reason, test, tags, results, lineno, retry_on_failure, is_slow_test,
            self.conflict_resolution, raw_tags=raw_tags,
            raw_results=raw_results, is_glob=is_glob,
            trailing_comments=trailing_comments, encode_func=self._encode_func)

    def _parse_expectation_line_into_components(self, lineno, line):
        """Helper function to break a single expectation line into components.

        Returns:
            A tuple of strings (reason, raw_tags, test, raw_results, trailing
            comments) which correspond to the components that make up an
            expectation line in an expectation file. Values may be empty strings
            if that particular component was not present.
        """
        match = self.MATCHER.match(line)
        if not match:
            raise ParseError(lineno, 'Syntax error: %s' % line)

        reason, raw_tags, test, raw_results, trailing_comments = match.groups()

        # TODO(rmhasan): Find a better regex to capture the reasons. The '*' in
        # the reasons regex only allows us to get the last bug. We need to write
        # the below code to get the full list of reasons.
        if reason:
            reason = reason.strip()
            index = line.find(reason)
            reason = line[:index] + reason
        return reason, raw_tags, test, raw_results, trailing_comments

    def _validate_expectation_structure(self, lineno, test, tags):
        """Helper function to validate aspects of an expectation being parsed.

        Specifically, checks for:
            * Correct use of wildcard characters
            * All tags used are known
            * Only one tag from each tag set is used
        """
        tag_set_ids = set()
        for i in range(len(test)-1):
            if test[i] == '*' and ((i > 0 and test[i-1] != '\\') or i == 0):
                raise ParseError(lineno,
                    'Invalid glob, \'*\' can only be at the end of the pattern')

        for t in tags:
            if not t in  self._tag_to_tag_set:
                raise ParseError(lineno, 'Unknown tag "%s"' % t)
            else:
                tag_set_ids.add(self._tag_to_tag_set[t])

        if len(tag_set_ids) != len(tags):
            error_msg = ('The tag group contains tags that are '
                         'part of the same tag set')
            tags_by_tag_set_id = defaultdict(list)
            for t in tags:
              tags_by_tag_set_id[self._tag_to_tag_set[t]].append(t)
            for tag_intersection in tags_by_tag_set_id.values():
                error_msg += ('\n  - Tags %s are part of the same tag set' %
                              _group_to_string(sorted(tag_intersection)))
            raise ParseError(lineno, error_msg)

    def _parse_and_validate_raw_results(self, lineno, raw_results):
        """Helper function to validate and parse raw results into known values.

        Returns:
            A tuple (results, retry_on_failure, is_slow_test). |results| is a
            list of parsed results. |retry_on_failure| is a boolean denoting
            whether the test should be retried on failure or not. |is_slow_test|
            is a boolean denoting whether the test should be considered slow or
            not.
        """
        results = []
        retry_on_failure = False
        is_slow_test = False
        for r in raw_results.split():
            if r not in self._allowed_results:
                raise ParseError(lineno, 'Unknown result type "%s"' % r)
            try:
                # The test expectations may contain expected results and
                # the RetryOnFailure tag
                if r in  _EXPECTATION_MAP:
                    results.append(_EXPECTATION_MAP[r])
                elif r == _RETRY_ON_FAILURE_TAG:
                    retry_on_failure = True
                elif r == _SLOW_TAG:
                    is_slow_test = True
                else:
                    raise KeyError
            except KeyError:
                raise ParseError(lineno, 'Unknown result type "%s"' % r)
        return results, retry_on_failure, is_slow_test

class TestExpectations(object):

    def __init__(self, tags=None, ignored_tags=None, encode_func=None,
                 decode_func=None):
        self.tag_sets = set()
        self.ignored_tags = set(ignored_tags or [])
        self.set_tags(tags or [])
        # Expectations may either refer to individual tests, or globs of
        # tests. Each test (or glob) may have multiple sets of tags and
        # expected results, so we store these in dicts ordered by the string
        # for ease of retrieve. glob_exps use an OrderedDict rather than
        # a regular dict for reasons given below.
        self.individual_exps = OrderedDict()
        self.glob_exps = OrderedDict()
        self._conflict_resolution = ConflictResolutionTypes.UNION
        self._encode_func = encode_func
        self._decode_func = decode_func

    def set_tags(self, tags, raise_ex_for_bad_tags=False):
        self.validate_condition_tags(tags, raise_ex_for_bad_tags)
        self._tags = [tag.lower() for tag in tags]

    def add_tags(self, new_tags, raise_ex_for_bad_tags=False):
        self.validate_condition_tags(new_tags, raise_ex_for_bad_tags)
        self._tags = list(
            set(self._tags) | set([tag.lower() for tag in new_tags]))

    @property
    def tags(self):
        return self._tags[:]

    def validate_condition_tags(self, tags, raise_ex_for_bad_tags):
        # This function will be used to validate if each tag in the tags list
        # is declared in a test expectations file. This validation will make
        # sure that the tags written in the test expectations files match tags
        # that are generated by the test runner.
        def _pluralize_unknown(missing):
            if len(missing) > 1:
                return ('s %s and %s are' % (', '.join(missing[:-1]),
                                             missing[-1]),
                        'have', 's are')
            else:
                return (' %s is' % missing[0], 'has', ' is')
        tags = set(t.lower() for t in tags)
        unknown_tags = set()
        if self.tag_sets:
            known_and_ignored_tags = set().union(
                *self.tag_sets).union(self.ignored_tags)
            unknown_tags = tags - known_and_ignored_tags
        if unknown_tags:
            msg = (
                'Tag%s not declared in the expectations file and %s not been '
                'explicitly ignored by the test. There may have been a typo in '
                'the expectations file. Please make sure the aforementioned '
                'tag%s declared at the top of the expectations file.' %
                _pluralize_unknown(sorted(unknown_tags)))
            if raise_ex_for_bad_tags:
                raise ValueError(msg)
            else:
                logging.warning(msg)

    def parse_tagged_list(self, raw_data, file_name='',
                          tags_conflict=None,
                          conflict_resolution=ConflictResolutionTypes.UNION):
        ret = 0
        self.file_name = file_name
        self._conflict_resolution = conflict_resolution
        tags_conflict = tags_conflict or _default_tags_conflict
        try:
            parser = TaggedTestListParser(raw_data,
                                          conflict_resolution,
                                          encode_func=self._encode_func,
                                          decode_func=self._decode_func)
        except ParseError as e:
            return 1, str(e)
        # If we have parsed another tagged list before, ensure that the tag sets
        # are the same in order to prevent any ambiguity about which set a tag
        # belongs to.
        if self.tag_sets:
            if not self.tag_sets == parser.tag_sets:
                raise RuntimeError(
                    'Existing tag sets %s do not match incoming sets %s' % (
                        sorted(self.tag_sets), sorted(parser.tag_sets)))
        else:
            self.tag_sets = parser.tag_sets
        # Conflict resolution tag in raw data will take precedence
        self._conflict_resolution = parser.conflict_resolution
        # TODO(crbug.com/83560) - Add support for multiple policies
        # for supporting multiple matching lines, e.g., allow/union,
        # reject, etc. Right now, you effectively just get a union.
        glob_exps = []
        for exp in parser.expectations:
            if exp.is_glob:
                glob_exps.append(exp)
            else:
                self.individual_exps.setdefault(exp.test, []).append(exp)

        # Each glob may also have multiple matching lines. By ordering the
        # globs by decreasing length, this allows us to find the most
        # specific glob by a simple linear search in expected_results_for().
        glob_exps.sort(key=lambda exp: len(exp.test), reverse=True)
        for exp in glob_exps:
            self.glob_exps.setdefault(exp.test, []).append(exp)

        errors = ''
        if not parser.conflicts_allowed:
            errors = self.check_test_expectations_patterns_for_conflicts(
                tags_conflict)
            ret = 1 if errors else 0
        return ret, errors

    def merge_test_expectations(self, other):
        # Merges another TestExpectation instance into this instance.
        # It will merge the other instance's and this instance's
        # individual_exps and glob_exps dictionaries.
        self.add_tags(other.tags)
        for pattern, exps in other.individual_exps.items():
            self.individual_exps.setdefault(pattern, []).extend(exps)
        for pattern, exps in other.glob_exps.items():
            self.glob_exps.setdefault(pattern, []).extend(exps)
        # resort the glob patterns by length in self.glob_exps ordered
        # dictionary
        glob_exps = self.glob_exps
        self.glob_exps = OrderedDict()
        for pattern, exps in sorted(
              glob_exps.items(), key=lambda item: len(item[0]), reverse=True):
            self.glob_exps[pattern] = exps

    def expectations_for(self, test):
        # Returns an Expectation.
        #
        # A given test may have multiple expectations, each with different
        # sets of tags that apply and different expected results, e.g.:
        #
        #  [ Mac ] TestFoo.test_bar [ Skip ]
        #  [ Debug Win ] TestFoo.test_bar [ Pass Failure ]
        #
        # To determine the expected results for a test, we have to loop over
        # all of the failures matching a test, find the ones whose tags are
        # a subset of the ones in effect, and  return the union of all of the
        # results. For example, if the runner is running with {Debug, Mac, Mac10.12}
        # then lines with no tags, {Mac}, or {Debug, Mac} would all match, but
        # {Debug, Win} would not. We also have to set the should_retry_on_failure
        # boolean variable to True if any of the expectations have the
        # should_retry_on_failure flag set to true
        #
        # The longest matching test string (name or glob) has priority.

        # Ensure that the given test name is in the same decoded format that
        # is used internally if the user uses a special encoding.
        if self._decode_func:
            test = self._decode_func(test)
        self._results = set()
        self._reasons = set()
        self._exp_tags = set()
        self._should_retry_on_failure = False
        self._is_slow_test = False
        self._trailing_comments = str()

        def _update_expected_results(exp):
            if exp.tags.issubset(self._tags):
                if exp.conflict_resolution == ConflictResolutionTypes.UNION:
                    if not exp.is_default_pass:
                        self._results.update(exp.results)
                    self._should_retry_on_failure |= exp.should_retry_on_failure
                    self._is_slow_test |= exp.is_slow_test
                    self._exp_tags.update(exp.tags)
                    if exp.trailing_comments:
                        self._trailing_comments += exp.trailing_comments + '\n'
                    if exp.reason:
                        self._reasons.update([exp.reason])
                else:
                    self._results = set(exp.results)
                    self._should_retry_on_failure = exp.should_retry_on_failure
                    self._is_slow_test = exp.is_slow_test
                    self._exp_tags = set(exp.tags)
                    self._trailing_comments = exp.trailing_comments
                    if exp.reason:
                        self._reasons = {exp.reason}

        # First, check for an exact match on the test name.
        for exp in self.individual_exps.get(test, []):
            _update_expected_results(exp)

        if self._results or self._is_slow_test or self._should_retry_on_failure:
            return Expectation(
                    test=test, results=self._results, tags=self._exp_tags,
                    retry_on_failure=self._should_retry_on_failure,
                    conflict_resolution=self._conflict_resolution,
                    is_slow_test=self._is_slow_test,
                    reason=' '.join(sorted(self._reasons)),
                    trailing_comments=self._trailing_comments,
                    encode_func=self._encode_func)

        # If we didn't find an exact match, check for matching globs. Match by
        # the most specific (i.e., longest) glob first. Because self.globs_exps
        # is ordered by length, this is a simple linear search
        for glob, exps in self.glob_exps.items():
            glob = glob[:-1]
            if test.startswith(glob):
                for exp in exps:
                    _update_expected_results(exp)
                # if *any* of the exps matched, results will be non-empty,
                # and we're done. If not, keep looking through ever-shorter
                # globs.
                if self._results or self._is_slow_test or self._should_retry_on_failure:
                    return Expectation(
                            test=test, results=self._results,
                            tags=self._exp_tags,
                            retry_on_failure=self._should_retry_on_failure,
                            conflict_resolution=self._conflict_resolution,
                            is_slow_test=self._is_slow_test,
                            reason=' '.join(sorted(self._reasons)),
                            trailing_comments=self._trailing_comments,
                            encode_func=self._encode_func)

        # Nothing matched, so by default, the test is expected to pass.
        return Expectation(test=test, encode_func=self._encode_func)

    def tag_sets_conflict(self, s1, s2, tags_conflict_fn):
        # Tag sets s1 and s2 have no conflict when there exists a tag in s1
        # and tag in s2 that are from the same tag declaration set and do not
        # conflict with each other.
        for tag_set in self.tag_sets:
            for t1, t2 in itertools.product(s1, s2):
                if (t1 in tag_set and t2 in tag_set and
                    tags_conflict_fn(t1, t2)):
                    return False
        return True

    def check_test_expectations_patterns_for_conflicts(self, tags_conflict_fn):
        # This function makes sure that any test expectations that have the same
        # pattern do not conflict with each other. Test expectations conflict
        # if their tag sets do not have conflicting tags. Tags conflict when
        # they belong to the same tag declaration set. For example an
        # expectations file may have a tag declaration set for operating systems
        # which might look like [ win linux]. A test expectation that has the
        # linux tag will not conflict with an expectation that has the win tag.
        error_msg = ''
        patterns_to_exps = dict(self.individual_exps)
        patterns_to_exps.update(self.glob_exps)
        for pattern, exps in patterns_to_exps.items():
            conflicts_exist = False
            for e1, e2 in itertools.combinations(exps, 2):
                if self.tag_sets_conflict(e1.tags, e2.tags, tags_conflict_fn):
                    if not conflicts_exist:
                        error_msg += (
                            '\nFound conflicts for pattern %s%s:\n' %
                            (pattern,
                             (' in %s' %
                              self.file_name if self.file_name else '')))
                    conflicts_exist = True
                    error_msg += ('  line %d conflicts with line %d\n' %
                                  (e1.lineno, e2.lineno))
        return error_msg

    def check_for_broken_expectations(self, test_names):
        # It returns a list expectations that do not apply to any test names in
        # the test_names list.
        #
        # args:
        # test_names: list of test names that are used to find test expectations
        # that do not apply to any of test names in the list.
        broken_exps = []
        # Apply the same temporary encoding we do when ingesting/comparing
        # expectations.
        if self._decode_func:
            test_names = [self._decode_func(tn) for tn in test_names]
        test_names = set(test_names)
        for pattern, exps in self.individual_exps.items():
            if pattern not in test_names:
                broken_exps.extend(exps)

        # look for broken glob expectations
        # first create a trie of test names
        trie = {}
        broken_glob_exps = []
        for test in test_names:
            _trie = trie.setdefault(test[0], {})
            for l in test[1:]:
                _trie = _trie.setdefault(l, {})
            _trie.setdefault('\0', {})

        # look for globs that do not match any test names and append their
        # expectations to glob_broken_exps
        for pattern, exps in self.glob_exps.items():
            _trie = trie
            for i, l in enumerate(pattern):
                if l == '*' and i == len(pattern) - 1:
                    break
                if l not in _trie:
                    broken_glob_exps.extend(exps)
                    break
                _trie = _trie[l]
        return broken_exps + broken_glob_exps
