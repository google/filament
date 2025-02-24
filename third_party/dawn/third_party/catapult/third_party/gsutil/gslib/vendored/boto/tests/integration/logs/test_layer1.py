# Copyright (c) 2014 Amazon.com, Inc. or its affiliates.
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish, dis-
# tribute, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to the fol-
# lowing conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
# ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
# SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import boto
from tests.compat import unittest


class TestCloudWatchLogs(unittest.TestCase):
    def setUp(self):
        self.logs = boto.connect_logs()

    def test_logs(self):
        logs = self.logs

        response = logs.describe_log_groups(log_group_name_prefix='test')
        self.assertIsInstance(response['logGroups'], list)

        mfilter = '[ip, id, user, ..., status_code=500, size]'
        sample = [
            '127.0.0.1 - frank "GET /apache_pb.gif HTTP/1.0" 200 1534',
            '127.0.0.1 - frank "GET /apache_pb.gif HTTP/1.0" 500 5324',
        ]
        response = logs.test_metric_filter(mfilter, sample)
        self.assertEqual(len(response['matches']), 1)
