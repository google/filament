#!/usr/bin/env vpython3
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import http
import sys
import urllib.error
import urllib.request

import auth
import metrics_utils


def main():
    metrics = input()
    try:
        headers = {}
        if 'bot_metrics' in metrics:
            token = auth.Authenticator().get_access_token().token
            headers = {'Authorization': 'Bearer ' + token}
        urllib.request.urlopen(
            urllib.request.Request(url=metrics_utils.APP_URL + '/upload',
                                   data=metrics.encode('utf-8'),
                                   headers=headers))
    except (urllib.error.HTTPError, urllib.error.URLError,
            http.client.RemoteDisconnected):
        pass

    return 0


if __name__ == '__main__':
    sys.exit(main())
