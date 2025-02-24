# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Validates sheriff configuration protobufs.

This module defines functions for validating protocol buffers for defining
sheriff subscription configurations.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from google.protobuf import text_format
import matcher
import re

from dashboard.protobuf import sheriff_pb2


class Error(Exception):
  """Top-level module error for validator."""


class InvalidConfig(Error):
  """Raised when the provided proto config is not valid."""


class MissingFields(Error):
  """Raised when a subscription is missing semantically required fields.

  Attributes:
    message: the actual protobuf message.
    index: the index for the subscription with the missing fields.
    field: the name for the missing field.
  """

  def __init__(self, message, index, field):
    super().__init__()
    self.message = message
    self.index = index
    self.field = field

  def __str__(self):
    return 'Subscription #%d is missing a "%s" field.' % (self.index,
                                                          self.field)


class MissingEmail(MissingFields):
  """Raised when a subscription is missing a contact_email field."""

  def __init__(self, message, index):
    super().__init__(message, index, 'contact_email')


class MissingName(MissingFields):
  """Raised when a subscription is missing a name."""

  def __init__(self, message, index):
    super().__init__(message, index, 'name')


class MissingPatterns(MissingFields):
  """Raised when a subscription has no patterns."""

  def __init__(self, message, index):
    super().__init__(message, index, 'patterns')


class InvalidPattern(MissingFields):
  """Raised when a subscription has an invalid pattern.

  Attributes:
    pattern_index: Index of the invalid pattern definition.
    reason: A text description of the error.
  """

  def __init__(self, message, index, pattern_index, reason, group):
    super().__init__(message, index, 'patterns')
    self.pattern_index = pattern_index
    self.reason = reason
    self.group = group

  def __str__(self):
    return 'Subscription #%d has a bad pattern (%s #%d); reason: %s' % (
        self.index, self.group, self.pattern_index, self.reason)


def Validate(content):
  """Takes raw content to determine whether it is a valid configuration.

  This function ensures that the protocol buffer contents are a syntactically
  and semantically valid sheriff configuration.

  Args:
    content: Bytes representing a text format protocol buffer.

  Returns:
    A valid SherifConfig object.
  """
  try:
    result = text_format.Parse(content, sheriff_pb2.SheriffConfig())
  except text_format.ParseError as error:
    raise InvalidConfig('SheriffConfig Validation Error: %s' %
                        (error)) from error

  # Go through each of the subscriptions, and ensure we find the semantically
  # required fields.
  def ValidatePattern(index, pattern_idx, pattern, group):
    field = pattern.WhichOneof('pattern')
    if field is None:
      raise InvalidPattern(result, index, pattern_idx,
                           'must provide either \'glob\' or \'regex\'', group)
    if field == 'glob' and len(pattern.glob) == 0:
      raise InvalidPattern(result, index, pattern_idx, 'glob must not be empty',
                           group)
    if field == 'regex' and len(pattern.regex) == 0:
      raise InvalidPattern(result, index, pattern_idx,
                           'regex must not be empty', group)
    try:
      matcher.CompilePattern(pattern)
    except re.error as e:
      raise InvalidPattern(result, index, pattern_idx, 'failed: %s' % (e,),
                           group) from e

  for (index, subscription) in enumerate(result.subscriptions):
    if subscription.contact_email is None or len(
        subscription.contact_email) == 0:
      raise MissingEmail(result, index)
    if subscription.name is None or len(subscription.name) == 0:
      raise MissingName(result, index)
    if not subscription.rules.match:
      raise MissingPatterns(result, index)

    for (pattern_idx, pattern) in enumerate(subscription.rules.match):
      ValidatePattern(index, pattern_idx, pattern, 'rules.match')
    for (pattern_idx, pattern) in enumerate(subscription.rules.exclude):
      ValidatePattern(index, pattern_idx, pattern, 'rules.exclude')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_triage.rules.match):
      ValidatePattern(index, pattern_idx, pattern, 'auto_triage.rules.match')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_triage.rules.exclude):
      ValidatePattern(index, pattern_idx, pattern, 'auto_triage.rules.exclude')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_merge.rules.match):
      ValidatePattern(index, pattern_idx, pattern, 'auto_merge.rules.match')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_merge.rules.exclude):
      ValidatePattern(index, pattern_idx, pattern, 'auto_merge.rules.exclude')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_bisection.rules.match):
      ValidatePattern(index, pattern_idx, pattern, 'auto_bisection.rules.match')
    for (pattern_idx,
         pattern) in enumerate(subscription.auto_bisection.rules.exclude):
      ValidatePattern(index, pattern_idx, pattern,
                      'auto_bisection.rules.exclude')

    # Validate patterns for anomaly configurations.
    for (anomaly_config_idx,
         anomaly_config) in enumerate(subscription.anomaly_configs):
      for (pattern_idx, pattern) in enumerate(anomaly_config.rules.match):
        ValidatePattern(
            index, pattern_idx, pattern,
            'anomaly_configs[{}].rules.match'.format(anomaly_config_idx))
      for (pattern_idx, pattern) in enumerate(anomaly_config.rules.exclude):
        ValidatePattern(
            index, pattern_idx, pattern,
            'anomaly_configs[{}].rules.exclude'.format(anomaly_config_idx))

  return result
