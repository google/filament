# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a function for emailing an alert to a sheriff on duty."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.api import mail

from dashboard import email_template


def EmailSheriff(subscriptions, test, anomaly):
  """Sends an email to subscriptions on duty about the given anomaly.

  Args:
    subscriptions: subscription.Subscription entities.
    test: The graph_data.TestMetadata entity associated with the anomaly.
    anomaly: The anomaly.Anomaly entity.
  """
  receivers = email_template.GetSubscriptionEmails(subscriptions)
  if not receivers:
    logging.warning('No email address for %s', subscriptions)
    return
  anomaly_info = email_template.GetAlertInfo(anomaly, test)
  mail.send_mail(
      sender='gasper-alerts@google.com',
      to=receivers,
      subject=anomaly_info['email_subject'],
      body=anomaly_info['email_text'],
      html=anomaly_info['email_html'])
  logging.info('Sent single mail to %s', receivers)
