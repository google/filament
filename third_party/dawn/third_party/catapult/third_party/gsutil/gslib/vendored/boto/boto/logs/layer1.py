# Copyright (c) 2014 Amazon.com, Inc. or its affiliates.  All Rights Reserved
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
#

import boto
from boto.connection import AWSQueryConnection
from boto.regioninfo import RegionInfo
from boto.exception import JSONResponseError
from boto.logs import exceptions
from boto.compat import json


class CloudWatchLogsConnection(AWSQueryConnection):
    """
    Amazon CloudWatch Logs Service API Reference
    This is the Amazon CloudWatch Logs API Reference . Amazon
    CloudWatch Logs is a managed service for real time monitoring and
    archival of application logs. This guide provides detailed
    information about Amazon CloudWatch Logs actions, data types,
    parameters, and errors. For detailed information about Amazon
    CloudWatch Logs features and their associated API calls, go to the
    `Amazon CloudWatch Logs Developer Guide`_.

    Use the following links to get started using the Amazon CloudWatch
    API Reference :


    + `Actions`_: An alphabetical list of all Amazon CloudWatch Logs
      actions.
    + `Data Types`_: An alphabetical list of all Amazon CloudWatch
      Logs data types.
    + `Common Parameters`_: Parameters that all Query actions can use.
    + `Common Errors`_: Client and server errors that all actions can
      return.
    + `Regions and Endpoints`_: Itemized regions and endpoints for all
      AWS products.


    In addition to using the Amazon CloudWatch Logs API, you can also
    use the following SDKs and third-party libraries to access Amazon
    CloudWatch Logs programmatically.


    + `AWS SDK for Java Documentation`_
    + `AWS SDK for .NET Documentation`_
    + `AWS SDK for PHP Documentation`_
    + `AWS SDK for Ruby Documentation`_


    Developers in the AWS developer community also provide their own
    libraries, which you can find at the following AWS developer
    centers:


    + `AWS Java Developer Center`_
    + `AWS PHP Developer Center`_
    + `AWS Python Developer Center`_
    + `AWS Ruby Developer Center`_
    + `AWS Windows and .NET Developer Center`_
    """
    APIVersion = "2014-03-28"
    DefaultRegionName = "us-east-1"
    DefaultRegionEndpoint = "logs.us-east-1.amazonaws.com"
    ServiceName = "CloudWatchLogs"
    TargetPrefix = "Logs_20140328"
    ResponseError = JSONResponseError

    _faults = {
        "LimitExceededException": exceptions.LimitExceededException,
        "DataAlreadyAcceptedException": exceptions.DataAlreadyAcceptedException,
        "ResourceInUseException": exceptions.ResourceInUseException,
        "ServiceUnavailableException": exceptions.ServiceUnavailableException,
        "InvalidParameterException": exceptions.InvalidParameterException,
        "ResourceNotFoundException": exceptions.ResourceNotFoundException,
        "ResourceAlreadyExistsException": exceptions.ResourceAlreadyExistsException,
        "OperationAbortedException": exceptions.OperationAbortedException,
        "InvalidSequenceTokenException": exceptions.InvalidSequenceTokenException,
    }

    def __init__(self, **kwargs):
        region = kwargs.pop('region', None)
        if not region:
            region = RegionInfo(self, self.DefaultRegionName,
                                self.DefaultRegionEndpoint)

        if 'host' not in kwargs or kwargs['host'] is None:
            kwargs['host'] = region.endpoint

        super(CloudWatchLogsConnection, self).__init__(**kwargs)
        self.region = region

    def _required_auth_capability(self):
        return ['hmac-v4']

    def create_log_group(self, log_group_name):
        """
        Creates a new log group with the specified name. The name of
        the log group must be unique within a region for an AWS
        account. You can create up to 100 log groups per account.

        You must use the following guidelines when naming a log group:

        + Log group names can be between 1 and 512 characters long.
        + Allowed characters are az, AZ, 09, '_' (underscore), '-'
          (hyphen), '/' (forward slash), and '.' (period).



        Log groups are created with a default retention of 14 days.
        The retention attribute allow you to configure the number of
        days you want to retain log events in the specified log group.
        See the `SetRetention` operation on how to modify the
        retention of your log groups.

        :type log_group_name: string
        :param log_group_name:

        """
        params = {'logGroupName': log_group_name, }
        return self.make_request(action='CreateLogGroup',
                                 body=json.dumps(params))

    def create_log_stream(self, log_group_name, log_stream_name):
        """
        Creates a new log stream in the specified log group. The name
        of the log stream must be unique within the log group. There
        is no limit on the number of log streams that can exist in a
        log group.

        You must use the following guidelines when naming a log
        stream:

        + Log stream names can be between 1 and 512 characters long.
        + The ':' colon character is not allowed.

        :type log_group_name: string
        :param log_group_name:

        :type log_stream_name: string
        :param log_stream_name:

        """
        params = {
            'logGroupName': log_group_name,
            'logStreamName': log_stream_name,
        }
        return self.make_request(action='CreateLogStream',
                                 body=json.dumps(params))

    def delete_log_group(self, log_group_name):
        """
        Deletes the log group with the specified name. Amazon
        CloudWatch Logs will delete a log group only if there are no
        log streams and no metric filters associated with the log
        group. If this condition is not satisfied, the request will
        fail and the log group will not be deleted.

        :type log_group_name: string
        :param log_group_name:

        """
        params = {'logGroupName': log_group_name, }
        return self.make_request(action='DeleteLogGroup',
                                 body=json.dumps(params))

    def delete_log_stream(self, log_group_name, log_stream_name):
        """
        Deletes a log stream and permanently deletes all the archived
        log events associated with it.

        :type log_group_name: string
        :param log_group_name:

        :type log_stream_name: string
        :param log_stream_name:

        """
        params = {
            'logGroupName': log_group_name,
            'logStreamName': log_stream_name,
        }
        return self.make_request(action='DeleteLogStream',
                                 body=json.dumps(params))

    def delete_metric_filter(self, log_group_name, filter_name):
        """
        Deletes a metric filter associated with the specified log
        group.

        :type log_group_name: string
        :param log_group_name:

        :type filter_name: string
        :param filter_name: The name of the metric filter.

        """
        params = {
            'logGroupName': log_group_name,
            'filterName': filter_name,
        }
        return self.make_request(action='DeleteMetricFilter',
                                 body=json.dumps(params))

    def delete_retention_policy(self, log_group_name):
        """
        

        :type log_group_name: string
        :param log_group_name:

        """
        params = {'logGroupName': log_group_name, }
        return self.make_request(action='DeleteRetentionPolicy',
                                 body=json.dumps(params))

    def describe_log_groups(self, log_group_name_prefix=None,
                            next_token=None, limit=None):
        """
        Returns all the log groups that are associated with the AWS
        account making the request. The list returned in the response
        is ASCII-sorted by log group name.

        By default, this operation returns up to 50 log groups. If
        there are more log groups to list, the response would contain
        a `nextToken` value in the response body. You can also limit
        the number of log groups returned in the response by
        specifying the `limit` parameter in the request.

        :type log_group_name_prefix: string
        :param log_group_name_prefix:

        :type next_token: string
        :param next_token: A string token used for pagination that points to
            the next page of results. It must be a value obtained from the
            response of the previous `DescribeLogGroups` request.

        :type limit: integer
        :param limit: The maximum number of items returned in the response. If
            you don't specify a value, the request would return up to 50 items.

        """
        params = {}
        if log_group_name_prefix is not None:
            params['logGroupNamePrefix'] = log_group_name_prefix
        if next_token is not None:
            params['nextToken'] = next_token
        if limit is not None:
            params['limit'] = limit
        return self.make_request(action='DescribeLogGroups',
                                 body=json.dumps(params))

    def describe_log_streams(self, log_group_name,
                             log_stream_name_prefix=None, next_token=None,
                             limit=None):
        """
        Returns all the log streams that are associated with the
        specified log group. The list returned in the response is
        ASCII-sorted by log stream name.

        By default, this operation returns up to 50 log streams. If
        there are more log streams to list, the response would contain
        a `nextToken` value in the response body. You can also limit
        the number of log streams returned in the response by
        specifying the `limit` parameter in the request.

        :type log_group_name: string
        :param log_group_name:

        :type log_stream_name_prefix: string
        :param log_stream_name_prefix:

        :type next_token: string
        :param next_token: A string token used for pagination that points to
            the next page of results. It must be a value obtained from the
            response of the previous `DescribeLogStreams` request.

        :type limit: integer
        :param limit: The maximum number of items returned in the response. If
            you don't specify a value, the request would return up to 50 items.

        """
        params = {'logGroupName': log_group_name, }
        if log_stream_name_prefix is not None:
            params['logStreamNamePrefix'] = log_stream_name_prefix
        if next_token is not None:
            params['nextToken'] = next_token
        if limit is not None:
            params['limit'] = limit
        return self.make_request(action='DescribeLogStreams',
                                 body=json.dumps(params))

    def describe_metric_filters(self, log_group_name,
                                filter_name_prefix=None, next_token=None,
                                limit=None):
        """
        Returns all the metrics filters associated with the specified
        log group. The list returned in the response is ASCII-sorted
        by filter name.

        By default, this operation returns up to 50 metric filters. If
        there are more metric filters to list, the response would
        contain a `nextToken` value in the response body. You can also
        limit the number of metric filters returned in the response by
        specifying the `limit` parameter in the request.

        :type log_group_name: string
        :param log_group_name:

        :type filter_name_prefix: string
        :param filter_name_prefix: The name of the metric filter.

        :type next_token: string
        :param next_token: A string token used for pagination that points to
            the next page of results. It must be a value obtained from the
            response of the previous `DescribeMetricFilters` request.

        :type limit: integer
        :param limit: The maximum number of items returned in the response. If
            you don't specify a value, the request would return up to 50 items.

        """
        params = {'logGroupName': log_group_name, }
        if filter_name_prefix is not None:
            params['filterNamePrefix'] = filter_name_prefix
        if next_token is not None:
            params['nextToken'] = next_token
        if limit is not None:
            params['limit'] = limit
        return self.make_request(action='DescribeMetricFilters',
                                 body=json.dumps(params))

    def get_log_events(self, log_group_name, log_stream_name,
                       start_time=None, end_time=None, next_token=None,
                       limit=None, start_from_head=None):
        """
        Retrieves log events from the specified log stream. You can
        provide an optional time range to filter the results on the
        event `timestamp`.

        By default, this operation returns as much log events as can
        fit in a response size of 1MB, up to 10,000 log events. The
        response will always include a `nextForwardToken` and a
        `nextBackwardToken` in the response body. You can use any of
        these tokens in subsequent `GetLogEvents` requests to paginate
        through events in either forward or backward direction. You
        can also limit the number of log events returned in the
        response by specifying the `limit` parameter in the request.

        :type log_group_name: string
        :param log_group_name:

        :type log_stream_name: string
        :param log_stream_name:

        :type start_time: long
        :param start_time: A point in time expressed as the number milliseconds
            since Jan 1, 1970 00:00:00 UTC.

        :type end_time: long
        :param end_time: A point in time expressed as the number milliseconds
            since Jan 1, 1970 00:00:00 UTC.

        :type next_token: string
        :param next_token: A string token used for pagination that points to
            the next page of results. It must be a value obtained from the
            `nextForwardToken` or `nextBackwardToken` fields in the response of
            the previous `GetLogEvents` request.

        :type limit: integer
        :param limit: The maximum number of log events returned in the
            response. If you don't specify a value, the request would return as
            much log events as can fit in a response size of 1MB, up to 10,000
            log events.

        :type start_from_head: boolean
        :param start_from_head:

        """
        params = {
            'logGroupName': log_group_name,
            'logStreamName': log_stream_name,
        }
        if start_time is not None:
            params['startTime'] = start_time
        if end_time is not None:
            params['endTime'] = end_time
        if next_token is not None:
            params['nextToken'] = next_token
        if limit is not None:
            params['limit'] = limit
        if start_from_head is not None:
            params['startFromHead'] = start_from_head
        return self.make_request(action='GetLogEvents',
                                 body=json.dumps(params))

    def put_log_events(self, log_group_name, log_stream_name, log_events,
                       sequence_token=None):
        """
        Uploads a batch of log events to the specified log stream.

        Every PutLogEvents request must include the `sequenceToken`
        obtained from the response of the previous request. An upload
        in a newly created log stream does not require a
        `sequenceToken`.

        The batch of events must satisfy the following constraints:

        + The maximum batch size is 32,768 bytes, and this size is
          calculated as the sum of all event messages in UTF-8, plus 26
          bytes for each log event.
        + None of the log events in the batch can be more than 2 hours
          in the future.
        + None of the log events in the batch can be older than 14
          days or the retention period of the log group.
        + The log events in the batch must be in chronological ordered
          by their `timestamp`.
        + The maximum number of log events in a batch is 1,000.

        :type log_group_name: string
        :param log_group_name:

        :type log_stream_name: string
        :param log_stream_name:

        :type log_events: list
        :param log_events: A list of events belonging to a log stream.

        :type sequence_token: string
        :param sequence_token: A string token that must be obtained from the
            response of the previous `PutLogEvents` request.

        """
        params = {
            'logGroupName': log_group_name,
            'logStreamName': log_stream_name,
            'logEvents': log_events,
        }
        if sequence_token is not None:
            params['sequenceToken'] = sequence_token
        return self.make_request(action='PutLogEvents',
                                 body=json.dumps(params))

    def put_metric_filter(self, log_group_name, filter_name, filter_pattern,
                          metric_transformations):
        """
        Creates or updates a metric filter and associates it with the
        specified log group. Metric filters allow you to configure
        rules to extract metric data from log events ingested through
        `PutLogEvents` requests.

        :type log_group_name: string
        :param log_group_name:

        :type filter_name: string
        :param filter_name: The name of the metric filter.

        :type filter_pattern: string
        :param filter_pattern:

        :type metric_transformations: list
        :param metric_transformations:

        """
        params = {
            'logGroupName': log_group_name,
            'filterName': filter_name,
            'filterPattern': filter_pattern,
            'metricTransformations': metric_transformations,
        }
        return self.make_request(action='PutMetricFilter',
                                 body=json.dumps(params))

    def put_retention_policy(self, log_group_name, retention_in_days):
        """
        

        :type log_group_name: string
        :param log_group_name:

        :type retention_in_days: integer
        :param retention_in_days: Specifies the number of days you want to
            retain log events in the specified log group. Possible values are:
            1, 3, 5, 7, 14, 30, 60, 90, 120, 150, 180, 365, 400, 547, 730.

        """
        params = {
            'logGroupName': log_group_name,
            'retentionInDays': retention_in_days,
        }
        return self.make_request(action='PutRetentionPolicy',
                                 body=json.dumps(params))

    def set_retention(self, log_group_name, retention_in_days):
        """
        Sets the retention of the specified log group. Log groups are
        created with a default retention of 14 days. The retention
        attribute allow you to configure the number of days you want
        to retain log events in the specified log group.

        :type log_group_name: string
        :param log_group_name:

        :type retention_in_days: integer
        :param retention_in_days: Specifies the number of days you want to
            retain log events in the specified log group. Possible values are:
            1, 3, 5, 7, 14, 30, 60, 90, 120, 150, 180, 365, 400, 547, 730.

        """
        params = {
            'logGroupName': log_group_name,
            'retentionInDays': retention_in_days,
        }
        return self.make_request(action='SetRetention',
                                 body=json.dumps(params))

    def test_metric_filter(self, filter_pattern, log_event_messages):
        """
        Tests the filter pattern of a metric filter against a sample
        of log event messages. You can use this operation to validate
        the correctness of a metric filter pattern.

        :type filter_pattern: string
        :param filter_pattern:

        :type log_event_messages: list
        :param log_event_messages:

        """
        params = {
            'filterPattern': filter_pattern,
            'logEventMessages': log_event_messages,
        }
        return self.make_request(action='TestMetricFilter',
                                 body=json.dumps(params))

    def make_request(self, action, body):
        headers = {
            'X-Amz-Target': '%s.%s' % (self.TargetPrefix, action),
            'Host': self.region.endpoint,
            'Content-Type': 'application/x-amz-json-1.1',
            'Content-Length': str(len(body)),
        }
        http_request = self.build_base_http_request(
            method='POST', path='/', auth_path='/', params={},
            headers=headers, data=body)
        response = self._mexe(http_request, sender=None,
                              override_num_retries=10)
        response_body = response.read().decode('utf-8')
        boto.log.debug(response_body)
        if response.status == 200:
            if response_body:
                return json.loads(response_body)
        else:
            json_body = json.loads(response_body)
            fault_name = json_body.get('__type', None)
            exception_class = self._faults.get(fault_name, self.ResponseError)
            raise exception_class(response.status, response.reason,
                                  body=json_body)
