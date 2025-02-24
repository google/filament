# -*- coding: utf-8 -*-
from webapp2_extras import json

import test_base


class TestJson(test_base.BaseTestCase):
    def test_encode(self):
        self.assertEqual(json.encode(
            '<script>alert("hello")</script>'),
            '"<script>alert(\\"hello\\")<\\/script>"')

    def test_decode(self):
        self.assertEqual(json.decode(
            '"<script>alert(\\"hello\\")<\\/script>"'),
            '<script>alert("hello")</script>')

    def test_b64encode(self):
        self.assertEqual(json.b64encode(
            '<script>alert("hello")</script>'),
            'IjxzY3JpcHQ+YWxlcnQoXCJoZWxsb1wiKTxcL3NjcmlwdD4i')

    def test_b64decode(self):
        self.assertEqual(json.b64decode(
            'IjxzY3JpcHQ+YWxlcnQoXCJoZWxsb1wiKTxcL3NjcmlwdD4i'),
            '<script>alert("hello")</script>')

    def test_quote(self):
        self.assertEqual(json.quote('<script>alert("hello")</script>'),
            '%22%3Cscript%3Ealert%28%5C%22hello%5C%22%29%3C%5C/script%3E%22')

    def test_unquote(self):
        self.assertEqual(json.unquote('%22%3Cscript%3Ealert%28%5C%22hello%5C%22%29%3C%5C/script%3E%22'),
            '<script>alert("hello")</script>')

if __name__ == '__main__':
    test_base.main()
