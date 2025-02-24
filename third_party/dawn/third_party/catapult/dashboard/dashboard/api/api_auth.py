# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import six

from google.appengine.api import oauth

from dashboard.common import datastore_hooks
from dashboard.common import utils

OAUTH_CLIENT_ID_ALLOWLIST = [
    # This oauth client id is from Pinpoint.
    '62121018386-aqdfougp0ddn93knqj6g79vvn42ajmrg.apps.googleusercontent.com',
    # This oauth client id is from the 'chromeperf' API console.
    '62121018386-h08uiaftreu4dr3c4alh3l7mogskvb7i.apps.googleusercontent.com',
    # This oauth client id is from chromiumdash-staging.
    '377415874083-slpqb5ur4h9sdfk8anlq4qct9imivnmt.apps.googleusercontent.com',
    # This oauth client id is from chromiumdash.
    '975044924533-p122oecs8h6eibv5j5a8fmj82b0ct0nk.apps.googleusercontent.com',
    # This oauth client id is used to monitor the rendering benchmark.
    '401211562413-g8hq6k75vr2767p1vspbs3vu9vscgrot.apps.googleusercontent.com',
    # This oauth client id is used for go/motionmark-dashboard.
    '147086511812-51cesludd5bhjqsmmooe7lgd6mndjq0j.apps.googleusercontent.com',
    # This oauth client id is used to upload histograms from the perf waterfall.
    '113172445342431053212',
    # This oauth client id used to upload histograms from cronet bots.
    '113172445342431053212',
    # These oauth client ids are used to upload Android performance metrics.
    '528014426327-fptk0tpfi4orpcol559k77v7bi9onpq5.apps.googleusercontent.com',
    '107857144893180953937',
    # This oauth client id is used by all LUCI binaries. In particular, it will
    # allow accessing the APIs by authorized users that generate tokens via
    # luci-auth command.
    '446450136466-2hr92jrq8e6i4tnsa56b52vacp7t3936.apps.googleusercontent.com',
    # This oauth client id will become default LUCI auth at some point.
    # https://chromium-review.googlesource.com/c/infra/luci/luci-go/+/4004539.
    '446450136466-mj75ourhccki9fffaq8bc1e50di315po.apps.googleusercontent.com',
]
if utils.IsStagingEnvironment():
  OAUTH_CLIENT_ID_ALLOWLIST = [
      # Staging oauth client id for Pinpoint.
      '22573382977-u263jlijs2uiio0uq7qm7vso3vuh7ec5.apps.googleusercontent.com'
  ]


class ApiAuthException(Exception):
  pass


class OAuthError(ApiAuthException):

  def __init__(self):
    super().__init__('User authentication error')


class NotLoggedInError(ApiAuthException):

  def __init__(self):
    super().__init__('User not authenticated')


class InternalOnlyError(ApiAuthException):

  def __init__(self):
    super().__init__('User does not have access')


def Authorize():
  try:
    email = utils.GetEmail()
  except oauth.OAuthRequestError as e:
    six.raise_from(OAuthError, e)

  if not email:
    raise NotLoggedInError

  try:
    # TODO(dberris): Migrate to using Cloud IAM and checking roles instead, to
    # allow for dynamic management of the accounts.
    if not email.endswith('.gserviceaccount.com'):
      # For non-service accounts, need to verify that the OAuth client ID
      # is in our allowlist.
      client_id = oauth.get_client_id(utils.OAUTH_SCOPES)
      if client_id not in OAUTH_CLIENT_ID_ALLOWLIST:
        logging.error('OAuth client id %s for user %s not in allowlist',
                      client_id, email)
        raise OAuthError
  except oauth.OAuthRequestError as e:
    # Transient errors when checking the token result should result in HTTP 500,
    # so catch oauth.OAuthRequestError here, not oauth.Error (which would catch
    # both fatal and transient errors).
    six.raise_from(OAuthError, e)

  logging.info('OAuth user logged in as: %s', email)
  if utils.IsInternalUser():
    datastore_hooks.SetPrivilegedRequest()
