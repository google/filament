# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""A service account credentials class.

This credentials class is implemented on top of rsa library.
"""

import base64
import json
import six
import time

from pyasn1.codec.ber import decoder
from pyasn1_modules.rfc5208 import PrivateKeyInfo
import rsa

from oauth2client import GOOGLE_REVOKE_URI
from oauth2client import GOOGLE_TOKEN_URI
from oauth2client import util
from oauth2client.client import AssertionCredentials


class _ServiceAccountCredentials(AssertionCredentials):
  """Class representing a service account (signed JWT) credential."""

  MAX_TOKEN_LIFETIME_SECS = 3600 # 1 hour in seconds

  def __init__(self, service_account_id, service_account_email, private_key_id,
               private_key_pkcs8_text, scopes, user_agent=None,
               token_uri=GOOGLE_TOKEN_URI, revoke_uri=GOOGLE_REVOKE_URI,
               **kwargs):

    super(_ServiceAccountCredentials, self).__init__(
        None, user_agent=user_agent, token_uri=token_uri, revoke_uri=revoke_uri)

    self._service_account_id = service_account_id
    self._service_account_email = service_account_email
    self._private_key_id = private_key_id
    self._private_key = _get_private_key(private_key_pkcs8_text)
    self._private_key_pkcs8_text = private_key_pkcs8_text
    self._scopes = util.scopes_to_string(scopes)
    self._user_agent = user_agent
    self._token_uri = token_uri
    self._revoke_uri = revoke_uri
    self._kwargs = kwargs

  def _generate_assertion(self):
    """Generate the assertion that will be used in the request."""

    header = {
        'alg': 'RS256',
        'typ': 'JWT',
        'kid': self._private_key_id
    }

    now = int(time.time())
    payload = {
        'aud': self._token_uri,
        'scope': self._scopes,
        'iat': now,
        'exp': now + _ServiceAccountCredentials.MAX_TOKEN_LIFETIME_SECS,
        'iss': self._service_account_email
    }
    payload.update(self._kwargs)

    assertion_input = (_urlsafe_b64encode(header) + b'.' +
                       _urlsafe_b64encode(payload))

    # Sign the assertion.
    rsa_bytes = rsa.pkcs1.sign(assertion_input, self._private_key, 'SHA-256')
    signature = base64.urlsafe_b64encode(rsa_bytes).rstrip(b'=')

    return assertion_input + b'.' + signature

  def sign_blob(self, blob):
    # Ensure that it is bytes
    try:
      blob = blob.encode('utf-8')
    except AttributeError:
      pass
    return (self._private_key_id,
            rsa.pkcs1.sign(blob, self._private_key, 'SHA-256'))

  @property
  def service_account_email(self):
    return self._service_account_email

  @property
  def serialization_data(self):
    return {
        'type': 'service_account',
        'client_id': self._service_account_id,
        'client_email': self._service_account_email,
        'private_key_id': self._private_key_id,
        'private_key': self._private_key_pkcs8_text
    }

  def create_scoped_required(self):
    return not self._scopes

  def create_scoped(self, scopes):
    return _ServiceAccountCredentials(self._service_account_id,
                                      self._service_account_email,
                                      self._private_key_id,
                                      self._private_key_pkcs8_text,
                                      scopes,
                                      user_agent=self._user_agent,
                                      token_uri=self._token_uri,
                                      revoke_uri=self._revoke_uri,
                                      **self._kwargs)


def _urlsafe_b64encode(data):
  return base64.urlsafe_b64encode(
      json.dumps(data, separators=(',', ':')).encode('UTF-8')).rstrip(b'=')


def _get_private_key(private_key_pkcs8_text):
  """Get an RSA private key object from a pkcs8 representation."""

  if not isinstance(private_key_pkcs8_text, six.binary_type):
    private_key_pkcs8_text = private_key_pkcs8_text.encode('ascii')
  der = rsa.pem.load_pem(private_key_pkcs8_text, 'PRIVATE KEY')
  asn1_private_key, _ = decoder.decode(der, asn1Spec=PrivateKeyInfo())
  return rsa.PrivateKey.load_pkcs1(
      asn1_private_key.getComponentByName('privateKey').asOctets(),
      format='DER')
