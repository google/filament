#!/usr/bin/env python

from boto.logs.layer1 import CloudWatchLogsConnection
from tests.unit import AWSMockServiceTestCase


class TestDescribeLogs(AWSMockServiceTestCase):
    connection_class = CloudWatchLogsConnection

    def default_body(self):
        return b'{"logGroups": []}'

    def test_describe(self):
        self.set_http_response(status_code=200)
        api_response = self.service_connection.describe_log_groups()

        self.assertEqual(0, len(api_response['logGroups']))

        self.assert_request_parameters({})

        target = self.actual_request.headers['X-Amz-Target']
        self.assertTrue('DescribeLogGroups' in target)
