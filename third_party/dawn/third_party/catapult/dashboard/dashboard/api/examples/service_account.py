# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""An example of using perf dashboard API with a service account.

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

2. Create a service account and ask the dashboard team to add it. If the team
   does not add your account, you will still be able to access the API, but not
   Google-internal data.
  a. Follow the steps here to create an account:
     https://developers.google.com/api-client-library/python/auth/service-accounts#creatinganaccount
  b. Remember to download and save the .json keyfile (referred to below as
     /path/to/keyfile.json)
  c. Contact the perf dashboard team to add the account to
     https://chrome-infra-auth.appspot.com/auth/groups/project-chromeperf-api-access

3. Modify the example below to use httplib2 and oauth2client libraries to
   authenticate.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import httplib2
from oauth2client import service_account


def MakeApiRequest():
  scopes = ['https://www.googleapis.com/auth/userinfo.email']

  # Need to update the path here.
  creds = service_account.ServiceAccountCredentials.from_json_keyfile_name(
      '/path/to/keyfile.json', scopes)

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
