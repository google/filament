from __future__ import with_statement
from collections import defaultdict, namedtuple
from operator import methodcaller
import os
import re
import sys
import copy

from pytest import raises

from schema import (Schema, Use, And, Or, Regex, Optional,
                    SchemaError, SchemaWrongKeyError,
                    SchemaMissingKeyError, SchemaUnexpectedTypeError)

if sys.version_info[0] == 3:
    basestring = str  # Python 3 does not have basestring
    unicode = str  # Python 3 does not have unicode


SE = raises(SchemaError)


def ve(_):
    raise ValueError()


def se(_):
    raise SchemaError('first auto', 'first error')


def test_schema():

    assert Schema(1).validate(1) == 1
    with SE: Schema(1).validate(9)

    assert Schema(int).validate(1) == 1
    with SE: Schema(int).validate('1')
    assert Schema(Use(int)).validate('1') == 1
    with SE: Schema(int).validate(int)

    assert Schema(str).validate('hai') == 'hai'
    with SE: Schema(str).validate(1)
    assert Schema(Use(str)).validate(1) == '1'

    assert Schema(list).validate(['a', 1]) == ['a', 1]
    assert Schema(dict).validate({'a': 1}) == {'a': 1}
    with SE: Schema(dict).validate(['a', 1])

    assert Schema(lambda n: 0 < n < 5).validate(3) == 3
    with SE: Schema(lambda n: 0 < n < 5).validate(-1)


def test_validate_file():
    assert Schema(
            Use(open)).validate('LICENSE-MIT').read().startswith('Copyright')
    with SE: Schema(Use(open)).validate('NON-EXISTENT')
    assert Schema(os.path.exists).validate('.') == '.'
    with SE: Schema(os.path.exists).validate('./non-existent/')
    assert Schema(os.path.isfile).validate('LICENSE-MIT') == 'LICENSE-MIT'
    with SE: Schema(os.path.isfile).validate('NON-EXISTENT')


def test_and():
    assert And(int, lambda n: 0 < n < 5).validate(3) == 3
    with SE: And(int, lambda n: 0 < n < 5).validate(3.33)
    assert And(Use(int), lambda n: 0 < n < 5).validate(3.33) == 3
    with SE: And(Use(int), lambda n: 0 < n < 5).validate('3.33')


def test_or():
    assert Or(int, dict).validate(5) == 5
    assert Or(int, dict).validate({}) == {}
    with SE: Or(int, dict).validate('hai')
    assert Or(int).validate(4)
    with SE: Or().validate(2)


def test_regex():
    # Simple case: validate string
    assert Regex(r'foo').validate('afoot') == 'afoot'
    with SE: Regex(r'bar').validate('afoot')

    # More complex case: validate string
    assert Regex(r'^[a-z]+$').validate('letters') == 'letters'
    with SE:
        Regex(r'^[a-z]+$').validate('letters + spaces') == 'letters + spaces'

    # Validate dict key
    assert (Schema({Regex(r'^foo'): str})
            .validate({'fookey': 'value'}) == {'fookey': 'value'})
    with SE: Schema({Regex(r'^foo'): str}).validate({'barkey': 'value'})

    # Validate dict value
    assert (Schema({str: Regex(r'^foo')}).validate({'key': 'foovalue'}) ==
            {'key': 'foovalue'})
    with SE: Schema({str: Regex(r'^foo')}).validate({'key': 'barvalue'})

    # Error if the value does not have a buffer interface
    with SE: Regex(r'bar').validate(1)
    with SE: Regex(r'bar').validate({})
    with SE: Regex(r'bar').validate([])
    with SE: Regex(r'bar').validate(None)

    # Validate that the pattern has a buffer interface
    assert Regex(re.compile(r'foo')).validate('foo') == 'foo'
    assert Regex(unicode('foo')).validate('foo') == 'foo'
    with raises(TypeError): Regex(1).validate('bar')
    with raises(TypeError): Regex({}).validate('bar')
    with raises(TypeError): Regex([]).validate('bar')
    with raises(TypeError): Regex(None).validate('bar')


def test_validate_list():
    assert Schema([1, 0]).validate([1, 0, 1, 1]) == [1, 0, 1, 1]
    assert Schema([1, 0]).validate([]) == []
    with SE: Schema([1, 0]).validate(0)
    with SE: Schema([1, 0]).validate([2])
    assert And([1, 0], lambda l: len(l) > 2).validate([0, 1, 0]) == [0, 1, 0]
    with SE: And([1, 0], lambda l: len(l) > 2).validate([0, 1])


def test_list_tuple_set_frozenset():
    assert Schema([int]).validate([1, 2])
    with SE: Schema([int]).validate(['1', 2])
    assert Schema(set([int])).validate(set([1, 2])) == set([1, 2])
    with SE: Schema(set([int])).validate([1, 2])  # not a set
    with SE: Schema(set([int])).validate(['1', 2])
    assert Schema(tuple([int])).validate(tuple([1, 2])) == tuple([1, 2])
    with SE: Schema(tuple([int])).validate([1, 2])  # not a set


def test_strictly():
    assert Schema(int).validate(1) == 1
    with SE: Schema(int).validate('1')


def test_dict():
    assert Schema({'key': 5}).validate({'key': 5}) == {'key': 5}
    with SE: Schema({'key': 5}).validate({'key': 'x'})
    with SE: Schema({'key': 5}).validate(['key', 5])
    assert Schema({'key': int}).validate({'key': 5}) == {'key': 5}
    assert Schema({'n': int, 'f': float}).validate(
            {'n': 5, 'f': 3.14}) == {'n': 5, 'f': 3.14}
    with SE: Schema({'n': int, 'f': float}).validate(
            {'n': 3.14, 'f': 5})
    with SE:
        try:
            Schema({}).validate({'abc': None, 1: None})
        except SchemaWrongKeyError as e:
            assert e.args[0].startswith("Wrong keys 'abc', 1 in")
            raise
    with SE:
        try:
            Schema({'key': 5}).validate({})
        except SchemaMissingKeyError as e:
            assert e.args[0] == "Missing keys: 'key'"
            raise
    with SE:
        try:
            Schema({'key': 5}).validate({'n': 5})
        except SchemaMissingKeyError as e:
            assert e.args[0] == "Missing keys: 'key'"
            raise
    with SE:
        try:
            Schema({}).validate({'n': 5})
        except SchemaWrongKeyError as e:
            assert e.args[0] == "Wrong keys 'n' in {'n': 5}"
            raise
    with SE:
        try:
            Schema({'key': 5}).validate({'key': 5, 'bad': 5})
        except SchemaWrongKeyError as e:
            assert e.args[0] in ["Wrong keys 'bad' in {'key': 5, 'bad': 5}",
                                 "Wrong keys 'bad' in {'bad': 5, 'key': 5}"]
            raise
    with SE:
        try:
            Schema({}).validate({'a': 5, 'b': 5})
        except SchemaError as e:
            assert e.args[0] in ["Wrong keys 'a', 'b' in {'a': 5, 'b': 5}",
                                 "Wrong keys 'a', 'b' in {'b': 5, 'a': 5}"]
            raise

    with SE:
        try:
            Schema({int: int}).validate({'': ''})
        except SchemaUnexpectedTypeError as e:
            assert e.args[0] in ["'' should be instance of 'int'"]


def test_dict_keys():
    assert Schema({str: int}).validate(
            {'a': 1, 'b': 2}) == {'a': 1, 'b': 2}
    with SE: Schema({str: int}).validate({1: 1, 'b': 2})
    assert Schema({Use(str): Use(int)}).validate(
            {1: 3.14, 3.14: 1}) == {'1': 3, '3.14': 1}


def test_ignore_extra_keys():
    assert Schema({'key': 5}, ignore_extra_keys=True).validate(
            {'key': 5, 'bad': 4}) == {'key': 5}
    assert Schema({'key': 5, 'dk': {'a': 'a'}}, ignore_extra_keys=True).validate(
        {'key': 5, 'bad': 'b', 'dk': {'a': 'a', 'bad': 'b'}}) == \
        {'key': 5, 'dk': {'a': 'a'}}
    assert Schema([{'key': 'v'}], ignore_extra_keys=True).validate(
        [{'key': 'v', 'bad': 'bad'}]) == [{'key': 'v'}]
    assert Schema([{'key': 'v'}], ignore_extra_keys=True).validate(
        [{'key': 'v', 'bad': 'bad'}]) == [{'key': 'v'}]


def test_ignore_extra_keys_validation_and_return_keys():
    assert Schema({'key': 5, object: object}, ignore_extra_keys=True).validate(
            {'key': 5, 'bad': 4}) == {'key': 5, 'bad': 4}
    assert Schema({'key': 5, 'dk': {'a': 'a', object: object}},
                  ignore_extra_keys=True).validate(
        {'key': 5, 'dk': {'a': 'a', 'bad': 'b'}}) == \
        {'key': 5, 'dk': {'a': 'a', 'bad': 'b'}}


def test_dict_optional_keys():
    with SE: Schema({'a': 1, 'b': 2}).validate({'a': 1})
    assert Schema({'a': 1, Optional('b'): 2}).validate({'a': 1}) == {'a': 1}
    assert Schema({'a': 1, Optional('b'): 2}).validate(
            {'a': 1, 'b': 2}) == {'a': 1, 'b': 2}
    # Make sure Optionals are favored over types:
    assert Schema({basestring: 1,
                   Optional('b'): 2}).validate({'a': 1, 'b': 2}) == {'a': 1, 'b': 2}


def test_dict_optional_defaults():
    # Optionals fill out their defaults:
    assert Schema({Optional('a', default=1): 11,
                   Optional('b', default=2): 22}).validate({'a': 11}) == {'a': 11, 'b': 2}

    # Optionals take precedence over types. Here, the "a" is served by the
    # Optional:
    assert Schema({Optional('a', default=1): 11,
                   basestring: 22}).validate({'b': 22}) == {'a': 1, 'b': 22}

    with raises(TypeError):
        Optional(And(str, Use(int)), default=7)


def test_dict_subtypes():
    d = defaultdict(int, key=1)
    v = Schema({'key': 1}).validate(d)
    assert v == d
    assert isinstance(v, defaultdict)
    # Please add tests for Counter and OrderedDict once support for Python2.6
    # is dropped!


def test_dict_key_error():
    try:
        Schema({'k': int}).validate({'k': 'x'})
    except SchemaError as e:
        assert e.code == "Key 'k' error:\n'x' should be instance of 'int'"
    try:
        Schema({'k': {'k2': int}}).validate({'k': {'k2': 'x'}})
    except SchemaError as e:
        code = "Key 'k' error:\nKey 'k2' error:\n'x' should be instance of 'int'"
        assert e.code == code
    try:
        Schema({'k': {'k2': int}}, error='k2 should be int').validate({'k': {'k2': 'x'}})
    except SchemaError as e:
        assert e.code == 'k2 should be int'


def test_complex():
    s = Schema({'<file>': And([Use(open)], lambda l: len(l)),
                '<path>': os.path.exists,
                Optional('--count'): And(int, lambda n: 0 <= n <= 5)})
    data = s.validate({'<file>': ['./LICENSE-MIT'], '<path>': './'})
    assert len(data) == 2
    assert len(data['<file>']) == 1
    assert data['<file>'][0].read().startswith('Copyright')
    assert data['<path>'] == './'


def test_nice_errors():
    try:
        Schema(int, error='should be integer').validate('x')
    except SchemaError as e:
        assert e.errors == ['should be integer']
    try:
        Schema(Use(float), error='should be a number').validate('x')
    except SchemaError as e:
        assert e.code == 'should be a number'
    try:
        Schema({Optional('i'): Use(int, error='should be a number')}).validate({'i': 'x'})
    except SchemaError as e:
        assert e.code == 'should be a number'


def test_use_error_handling():
    try:
        Use(ve).validate('x')
    except SchemaError as e:
        assert e.autos == ["ve('x') raised ValueError()"]
        assert e.errors == [None]
    try:
        Use(ve, error='should not raise').validate('x')
    except SchemaError as e:
        assert e.autos == ["ve('x') raised ValueError()"]
        assert e.errors == ['should not raise']
    try:
        Use(se).validate('x')
    except SchemaError as e:
        assert e.autos == [None, 'first auto']
        assert e.errors == [None, 'first error']
    try:
        Use(se, error='second error').validate('x')
    except SchemaError as e:
        assert e.autos == [None, 'first auto']
        assert e.errors == ['second error', 'first error']


def test_or_error_handling():
    try:
        Or(ve).validate('x')
    except SchemaError as e:
        assert e.autos[0].startswith('Or(')
        assert e.autos[0].endswith(") did not validate 'x'")
        assert e.autos[1] == "ve('x') raised ValueError()"
        assert len(e.autos) == 2
        assert e.errors == [None, None]
    try:
        Or(ve, error='should not raise').validate('x')
    except SchemaError as e:
        assert e.autos[0].startswith('Or(')
        assert e.autos[0].endswith(") did not validate 'x'")
        assert e.autos[1] == "ve('x') raised ValueError()"
        assert len(e.autos) == 2
        assert e.errors == ['should not raise', 'should not raise']
    try:
        Or('o').validate('x')
    except SchemaError as e:
        assert e.autos == ["Or('o') did not validate 'x'",
                           "'o' does not match 'x'"]
        assert e.errors == [None, None]
    try:
        Or('o', error='second error').validate('x')
    except SchemaError as e:
        assert e.autos == ["Or('o') did not validate 'x'",
                           "'o' does not match 'x'"]
        assert e.errors == ['second error', 'second error']


def test_and_error_handling():
    try:
        And(ve).validate('x')
    except SchemaError as e:
        assert e.autos == ["ve('x') raised ValueError()"]
        assert e.errors == [None]
    try:
        And(ve, error='should not raise').validate('x')
    except SchemaError as e:
        assert e.autos == ["ve('x') raised ValueError()"]
        assert e.errors == ['should not raise']
    try:
        And(str, se).validate('x')
    except SchemaError as e:
        assert e.autos == [None, 'first auto']
        assert e.errors == [None, 'first error']
    try:
        And(str, se, error='second error').validate('x')
    except SchemaError as e:
        assert e.autos == [None, 'first auto']
        assert e.errors == ['second error', 'first error']


def test_schema_error_handling():
    try:
        Schema(Use(ve)).validate('x')
    except SchemaError as e:
        assert e.autos == [None, "ve('x') raised ValueError()"]
        assert e.errors == [None, None]
    try:
        Schema(Use(ve), error='should not raise').validate('x')
    except SchemaError as e:
        assert e.autos == [None, "ve('x') raised ValueError()"]
        assert e.errors == ['should not raise', None]
    try:
        Schema(Use(se)).validate('x')
    except SchemaError as e:
        assert e.autos == [None, None, 'first auto']
        assert e.errors == [None, None, 'first error']
    try:
        Schema(Use(se), error='second error').validate('x')
    except SchemaError as e:
        assert e.autos == [None, None, 'first auto']
        assert e.errors == ['second error', None, 'first error']


def test_use_json():
    import json
    gist_schema = Schema(And(Use(json.loads),  # first convert from JSON
                             {Optional('description'): basestring,
                              'public': bool,
                              'files': {basestring: {'content': basestring}}}))
    gist = '''{"description": "the description for this gist",
               "public": true,
               "files": {
                   "file1.txt": {"content": "String file contents"},
                   "other.txt": {"content": "Another file contents"}}}'''
    assert gist_schema.validate(gist)


def test_error_reporting():
    s = Schema({'<files>': [Use(open, error='<files> should be readable')],
                '<path>': And(os.path.exists, error='<path> should exist'),
                '--count': Or(None, And(Use(int), lambda n: 0 < n < 5),
                              error='--count should be integer 0 < n < 5')},
               error='Error:')
    s.validate({'<files>': [], '<path>': './', '--count': 3})

    try:
        s.validate({'<files>': [], '<path>': './', '--count': '10'})
    except SchemaError as e:
        assert e.code == 'Error:\n--count should be integer 0 < n < 5'
    try:
        s.validate({'<files>': [], '<path>': './hai', '--count': '2'})
    except SchemaError as e:
        assert e.code == 'Error:\n<path> should exist'
    try:
        s.validate({'<files>': ['hai'], '<path>': './', '--count': '2'})
    except SchemaError as e:
        assert e.code == 'Error:\n<files> should be readable'


def test_schema_repr():  # what about repr with `error`s?
    schema = Schema([Or(None, And(str, Use(float)))])
    repr_ = "Schema([Or(None, And(<type 'str'>, Use(<type 'float'>)))])"
    # in Python 3 repr contains <class 'str'>, not <type 'str'>
    assert repr(schema).replace('class', 'type') == repr_


def test_validate_object():
    schema = Schema({object: str})
    assert schema.validate({42: 'str'}) == {42: 'str'}
    with SE: schema.validate({42: 777})


def test_issue_9_prioritized_key_comparison():
    validate = Schema({'key': 42, object: 42}).validate
    assert validate({'key': 42, 777: 42}) == {'key': 42, 777: 42}


def test_issue_9_prioritized_key_comparison_in_dicts():
    # http://stackoverflow.com/questions/14588098/docopt-schema-validation
    s = Schema({'ID': Use(int, error='ID should be an int'),
                'FILE': Or(None, Use(open, error='FILE should be readable')),
                Optional(str): object})
    data = {'ID': 10, 'FILE': None, 'other': 'other', 'other2': 'other2'}
    assert s.validate(data) == data
    data = {'ID': 10, 'FILE': None}
    assert s.validate(data) == data


def test_missing_keys_exception_with_non_str_dict_keys():
    s = Schema({And(str, Use(str.lower), 'name'): And(str, len)})
    with SE: s.validate(dict())
    with SE:
        try:
            Schema({1: 'x'}).validate(dict())
        except SchemaMissingKeyError as e:
            assert e.args[0] == "Missing keys: 1"
            raise


def test_issue_56_cant_rely_on_callables_to_have_name():
    s = Schema(methodcaller('endswith', '.csv'))
    assert s.validate('test.csv') == 'test.csv'
    with SE:
        try:
            s.validate('test.py')
        except SchemaError as e:
            assert "operator.methodcaller" in e.args[0]
            raise


def test_exception_handling_with_bad_validators():
    BadValidator = namedtuple("BadValidator", ["validate"])
    s = Schema(BadValidator("haha"))
    with SE:
        try:
            s.validate("test")
        except SchemaError as e:
            assert "TypeError" in e.args[0]
            raise


def test_issue_83_iterable_validation_return_type():
    TestSetType = type("TestSetType", (set,), dict())
    data = TestSetType(["test", "strings"])
    s = Schema(set([str]))
    assert isinstance(s.validate(data), TestSetType)


def test_optional_key_convert_failed_randomly_while_with_another_optional_object():
    """
    In this test, created_at string "2015-10-10 00:00:00" is expected to be converted
    to a datetime instance.
        - it works when the schema is

            s = Schema({
                    'created_at': _datetime_validator,
                    Optional(basestring): object,
                })

        - but when wrapping the key 'created_at' with Optional, it fails randomly
    :return:
    """
    import datetime
    fmt = '%Y-%m-%d %H:%M:%S'
    _datetime_validator = Or(None, Use(lambda i: datetime.datetime.strptime(i, fmt)))
    # FIXME given tests enough
    for i in range(1024):
        s = Schema({
            Optional('created_at'): _datetime_validator,
            Optional('updated_at'): _datetime_validator,
            Optional('birth'): _datetime_validator,
            Optional(basestring): object,
        })
        data = {
            'created_at': '2015-10-10 00:00:00'
        }
        validated_data = s.validate(data)
        # is expected to be converted to a datetime instance, but fails randomly
        # (most of the time)
        assert isinstance(validated_data['created_at'], datetime.datetime)
        # assert isinstance(validated_data['created_at'], basestring)


def test_copy():
    s1 = SchemaError('a', None)
    s2 = copy.deepcopy(s1)
    assert s1 is not s2
    assert type(s1) is type(s2)


def test_inheritance():
    def convert(data):
        if isinstance(data, int):
            return data + 1
        return data

    class MySchema(Schema):
        def validate(self, data):
            return super(MySchema, self).validate(convert(data))

    s = {'k': int, 'd': {'k': int, 'l': [{'l': [int]}]}}
    v = {'k': 1, 'd': {'k': 2, 'l': [{'l': [3, 4, 5]}]}}
    d = MySchema(s).validate(v)
    assert d['k'] == 2 and d['d']['k'] == 3 and d['d']['l'][0]['l'] == [4, 5, 6]
