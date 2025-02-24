# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""An example of using perf dashboard API with your own user account.

#############  DEPRECATION WARNING  #############
#  Oauth2client is now deprecated.
#  As of Oct 11th, 2022, we are migrating oauth2client to google-auth.
#  Update on examples will be available later.
#  Please contact our team for urgent needs:
#    browser-perf-engprod@google.com
#################################################

Getting set up:
1. Install httplib2and oauth2client python modules:
`pip install httplib2`
`pip install oauth2client`

This example is very bare-bones, but should allow you to use the API with
minimal setup and no service account credentials. For more details on how to
write API-using code with OAuth2, see:
https://developers.google.com/identity/protocols/OAuth2InstalledApp
https://developers.google.com/api-client-library/python/auth/installed-app
For a more complete working example, see the depot_tools auth code:
https://cs.chromium.org/chromium/tools/depot_tools/auth.py
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import httplib2
from oauth2client import client
from six.moves import input  # pylint:disable=redefined-builtin

# See security notes about why the 'secret' doesn't need to be kept secret here:
# https://cs.chromium.org/chromium/tools/depot_tools/auth.py
# This client ID and secret are managed in API console project 'chromeperf'.
# Any user may use them to authenticate.
OAUTH_CLIENT_ID = (
    '62121018386-h08uiaftreu4dr3c4alh3l7mogskvb7i.apps.googleusercontent.com')
OAUTH_CLIENT_SECRET = 'vc1fZfV1cZC6mgDSHV-KSPOz'
SCOPES = ['https://www.googleapis.com/auth/userinfo.email']


#############  DEPRECATION WARNING  #############
# Request using OOB flow will be blocked from Oct 3, 2022.
# More info:
# https://developers.googleblog.com/2022/02/making-oauth-flows-safer.html#disallowed-oob
# Please do *not* refer to this example. We will update it with a
# valid solution.
# For urgent request, please contact our team directly:
#   browser-perf-engprod@google.com
#############  DEPRECATION WARNING  #############
def MakeApiRequest():
  flow = client.OAuth2WebServerFlow(
      OAUTH_CLIENT_ID, OAUTH_CLIENT_SECRET, SCOPES, approval_prompt='force')
  flow.redirect_uri = client.OOB_CALLBACK_URN
  authorize_url = flow.step1_get_authorize_url()
  print('Go to the following link in your browser:\n\n'
        '    %s\n' % authorize_url)

  code = input('Enter verification code: ').strip()
  try:
    creds = flow.step2_exchange(code)
  except client.FlowExchangeError as e:
    print('Authentication has failed: %s' % e)
    return None, None

  http_auth = creds.authorize(httplib2.Http())
  resp, content = http_auth.request(
      "https://chromeperf.appspot.com/api/alerts/bug_id/713717",
      method="POST",
      headers={'Content-length': 0})
  return (resp, content)


if __name__ == '__main__':
  RESPONSE, CONTENT = MakeApiRequest()
  # Check response and do stuff with content!
  print(RESPONSE, CONTENT)
