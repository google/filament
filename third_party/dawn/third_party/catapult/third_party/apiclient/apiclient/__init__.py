"""Retain apiclient as an alias for googleapiclient."""

from six import iteritems

import googleapiclient

try:
  import oauth2client
except ImportError:
  raise RuntimeError(
      'Previous version of google-api-python-client detected; due to a '
      'packaging issue, we cannot perform an in-place upgrade. To repair, '
      'remove and reinstall this package, along with oauth2client and '
      'uritemplate. One can do this with pip via\n'
      '  pip install -I google-api-python-client'
  )

from googleapiclient import channel
from googleapiclient import discovery
from googleapiclient import errors
from googleapiclient import http
from googleapiclient import mimeparse
from googleapiclient import model
from googleapiclient import sample_tools
from googleapiclient import schema

__version__ = googleapiclient.__version__

_SUBMODULES = {
    'channel': channel,
    'discovery': discovery,
    'errors': errors,
    'http': http,
    'mimeparse': mimeparse,
    'model': model,
    'sample_tools': sample_tools,
    'schema': schema,
}

import sys
for module_name, module in iteritems(_SUBMODULES):
  sys.modules['apiclient.%s' % module_name] = module
