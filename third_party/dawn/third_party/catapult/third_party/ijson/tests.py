# -*- coding:utf-8 -*-
from __future__ import unicode_literals
import unittest
from io import BytesIO, StringIO
from decimal import Decimal
import threading
from importlib import import_module

from ijson import common
from ijson.backends.python import basic_parse, Lexer
from ijson.compat import IS_PY2


JSON = b'''
{
  "docs": [
    {
      "null": null,
      "boolean": false,
      "true": true,
      "integer": 0,
      "double": 0.5,
      "exponent": 1.0e+2,
      "long": 10000000000,
      "string": "\\u0441\\u0442\\u0440\\u043e\\u043a\\u0430 - \xd1\x82\xd0\xb5\xd1\x81\xd1\x82"
    },
    {
      "meta": [[1], {}]
    },
    {
      "meta": {"key": "value"}
    },
    {
      "meta": null
    }
  ]
}
'''
JSON_EVENTS = [
    ('start_map', None),
        ('map_key', 'docs'),
        ('start_array', None),
            ('start_map', None),
                ('map_key', 'null'),
                ('null', None),
                ('map_key', 'boolean'),
                ('boolean', False),
                ('map_key', 'true'),
                ('boolean', True),
                ('map_key', 'integer'),
                ('number', 0),
                ('map_key', 'double'),
                ('number', Decimal('0.5')),
                ('map_key', 'exponent'),
                ('number', 100),
                ('map_key', 'long'),
                ('number', 10000000000),
                ('map_key', 'string'),
                ('string', 'строка - тест'),
            ('end_map', None),
            ('start_map', None),
                ('map_key', 'meta'),
                ('start_array', None),
                    ('start_array', None),
                        ('number', 1),
                    ('end_array', None),
                    ('start_map', None),
                    ('end_map', None),
                ('end_array', None),
            ('end_map', None),
            ('start_map', None),
                ('map_key', 'meta'),
                ('start_map', None),
                    ('map_key', 'key'),
                    ('string', 'value'),
                ('end_map', None),
            ('end_map', None),
            ('start_map', None),
                ('map_key', 'meta'),
                ('null', None),
            ('end_map', None),
        ('end_array', None),
    ('end_map', None),
]
SCALAR_JSON = b'0'
INVALID_JSONS = [
    b'["key", "value",]',      # trailing comma
    b'["key"  "value"]',       # no comma
    b'{"key": "value",}',      # trailing comma
    b'{"key": "value" "key"}', # no comma
    b'{"key"  "value"}',       # no colon
    b'invalid',                # unknown lexeme
    b'[1, 2] dangling junk'    # dangling junk
]
YAJL1_PASSING_INVALID = INVALID_JSONS[6]
INCOMPLETE_JSONS = [
    b'',
    b'"test',
    b'[',
    b'[1',
    b'[1,',
    b'{',
    b'{"key"',
    b'{"key":',
    b'{"key": "value"',
    b'{"key": "value",',
]
STRINGS_JSON = br'''
{
    "str1": "",
    "str2": "\"",
    "str3": "\\",
    "str4": "\\\\",
    "special\t": "\b\f\n\r\t"
}
'''
NUMBERS_JSON = b'[1, 1.0, 1E2]'
SURROGATE_PAIRS_JSON = b'"\uD83D\uDCA9"'


class Parse(object):
    '''
    Base class for parsing tests that is used to create test cases for each
    available backends.
    '''
    def test_basic_parse(self):
        events = list(self.backend.basic_parse(BytesIO(JSON)))
        self.assertEqual(events, JSON_EVENTS)

    def test_basic_parse_threaded(self):
        thread = threading.Thread(target=self.test_basic_parse)
        thread.start()
        thread.join()

    def test_scalar(self):
        events = list(self.backend.basic_parse(BytesIO(SCALAR_JSON)))
        self.assertEqual(events, [('number', 0)])

    def test_strings(self):
        events = list(self.backend.basic_parse(BytesIO(STRINGS_JSON)))
        strings = [value for event, value in events if event == 'string']
        self.assertEqual(strings, ['', '"', '\\', '\\\\', '\b\f\n\r\t'])
        self.assertTrue(('map_key', 'special\t') in events)

    def test_surrogate_pairs(self):
        event = next(self.backend.basic_parse(BytesIO(SURROGATE_PAIRS_JSON)))
        parsed_string = event[1]
        self.assertEqual(parsed_string, '💩')

    def test_numbers(self):
        events = list(self.backend.basic_parse(BytesIO(NUMBERS_JSON)))
        types = [type(value) for event, value in events if event == 'number']
        self.assertEqual(types, [int, Decimal, Decimal])

    def test_invalid(self):
        for json in INVALID_JSONS:
            # Yajl1 doesn't complain about additional data after the end
            # of a parsed object. Skipping this test.
            if self.__class__.__name__ == 'YajlParse' and json == YAJL1_PASSING_INVALID:
                continue
            with self.assertRaises(common.JSONError) as cm:
                list(self.backend.basic_parse(BytesIO(json)))

    def test_incomplete(self):
        for json in INCOMPLETE_JSONS:
            with self.assertRaises(common.IncompleteJSONError):
                list(self.backend.basic_parse(BytesIO(json)))

    def test_utf8_split(self):
        buf_size = JSON.index(b'\xd1') + 1
        try:
            events = list(self.backend.basic_parse(BytesIO(JSON), buf_size=buf_size))
        except UnicodeDecodeError:
            self.fail('UnicodeDecodeError raised')

    def test_lazy(self):
        # shouldn't fail since iterator is not exhausted
        self.backend.basic_parse(BytesIO(INVALID_JSONS[0]))
        self.assertTrue(True)

    def test_boundary_lexeme(self):
        buf_size = JSON.index(b'false') + 1
        events = list(self.backend.basic_parse(BytesIO(JSON), buf_size=buf_size))
        self.assertEqual(events, JSON_EVENTS)

    def test_boundary_whitespace(self):
        buf_size = JSON.index(b'   ') + 1
        events = list(self.backend.basic_parse(BytesIO(JSON), buf_size=buf_size))
        self.assertEqual(events, JSON_EVENTS)

    def test_api(self):
        self.assertTrue(list(self.backend.items(BytesIO(JSON), '')))
        self.assertTrue(list(self.backend.parse(BytesIO(JSON))))

# Generating real TestCase classes for each importable backend
for name in ['python', 'yajl', 'yajl2', 'yajl2_cffi']:
    try:
        classname = '%sParse' % ''.join(p.capitalize() for p in name.split('_'))
        if IS_PY2:
            classname = classname.encode('ascii')

        locals()[classname] = type(
            classname,
            (unittest.TestCase, Parse),
            {'backend': import_module('ijson.backends.%s' % name)},
        )
    except ImportError:
        pass


class Common(unittest.TestCase):
    '''
    Backend independent tests. They all use basic_parse imported explicitly from
    the python backend to generate parsing events.
    '''
    def test_object_builder(self):
        builder = common.ObjectBuilder()
        for event, value in basic_parse(BytesIO(JSON)):
            builder.event(event, value)
        self.assertEqual(builder.value, {
            'docs': [
                {
                   'string': 'строка - тест',
                   'null': None,
                   'boolean': False,
                   'true': True,
                   'integer': 0,
                   'double': Decimal('0.5'),
                   'exponent': 100,
                   'long': 10000000000,
                },
                {
                    'meta': [[1], {}],
                },
                {
                    'meta': {'key': 'value'},
                },
                {
                    'meta': None,
                },
            ],
        })

    def test_scalar_builder(self):
        builder = common.ObjectBuilder()
        for event, value in basic_parse(BytesIO(SCALAR_JSON)):
            builder.event(event, value)
        self.assertEqual(builder.value, 0)

    def test_parse(self):
        events = common.parse(basic_parse(BytesIO(JSON)))
        events = [value
            for prefix, event, value in events
            if prefix == 'docs.item.meta.item.item'
        ]
        self.assertEqual(events, [1])

    def test_items(self):
        events = basic_parse(BytesIO(JSON))
        meta = list(common.items(common.parse(events), 'docs.item.meta'))
        self.assertEqual(meta, [
            [[1], {}],
            {'key': 'value'},
            None,
        ])


class Stream(unittest.TestCase):
    def test_bytes(self):
        l = Lexer(BytesIO(JSON))
        self.assertEqual(next(l)[1], '{')

    def test_string(self):
        l = Lexer(StringIO(JSON.decode('utf-8')))
        self.assertEqual(next(l)[1], '{')


if __name__ == '__main__':
    unittest.main()
