# Copyright 2014 Google Inc. All Rights Reserved.
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

"""Client for discovery based APIs.

A client library for Google's discovery based APIs.
"""
from __future__ import absolute_import
import six
from six.moves import zip

__author__ = 'jcgregorio@google.com (Joe Gregorio)'
__all__ = [
    'build',
    'build_from_document',
    'fix_method_name',
    'key2param',
    ]

from six import StringIO
from six.moves.urllib.parse import urlencode, urlparse, urljoin, \
  urlunparse, parse_qsl

# Standard library imports
import copy
from email.generator import Generator
from email.mime.multipart import MIMEMultipart
from email.mime.nonmultipart import MIMENonMultipart
import json
import keyword
import logging
import mimetypes
import os
import re

# Third-party imports
import httplib2
import uritemplate

# Local imports
from googleapiclient import mimeparse
from googleapiclient.errors import HttpError
from googleapiclient.errors import InvalidJsonError
from googleapiclient.errors import MediaUploadSizeError
from googleapiclient.errors import UnacceptableMimeTypeError
from googleapiclient.errors import UnknownApiNameOrVersion
from googleapiclient.errors import UnknownFileType
from googleapiclient.http import HttpRequest
from googleapiclient.http import MediaFileUpload
from googleapiclient.http import MediaUpload
from googleapiclient.model import JsonModel
from googleapiclient.model import MediaModel
from googleapiclient.model import RawModel
from googleapiclient.schema import Schemas
from oauth2client.client import GoogleCredentials
from oauth2client.util import _add_query_parameter
from oauth2client.util import positional


# The client library requires a version of httplib2 that supports RETRIES.
httplib2.RETRIES = 1

logger = logging.getLogger(__name__)

URITEMPLATE = re.compile('{[^}]*}')
VARNAME = re.compile('[a-zA-Z0-9_-]+')
DISCOVERY_URI = ('https://www.googleapis.com/discovery/v1/apis/'
                 '{api}/{apiVersion}/rest')
DEFAULT_METHOD_DOC = 'A description of how to use this function'
HTTP_PAYLOAD_METHODS = frozenset(['PUT', 'POST', 'PATCH'])
_MEDIA_SIZE_BIT_SHIFTS = {'KB': 10, 'MB': 20, 'GB': 30, 'TB': 40}
BODY_PARAMETER_DEFAULT_VALUE = {
    'description': 'The request body.',
    'type': 'object',
    'required': True,
}
MEDIA_BODY_PARAMETER_DEFAULT_VALUE = {
    'description': ('The filename of the media request body, or an instance '
                    'of a MediaUpload object.'),
    'type': 'string',
    'required': False,
}

# Parameters accepted by the stack, but not visible via discovery.
# TODO(dhermes): Remove 'userip' in 'v2'.
STACK_QUERY_PARAMETERS = frozenset(['trace', 'pp', 'userip', 'strict'])
STACK_QUERY_PARAMETER_DEFAULT_VALUE = {'type': 'string', 'location': 'query'}

# Library-specific reserved words beyond Python keywords.
RESERVED_WORDS = frozenset(['body'])


def fix_method_name(name):
  """Fix method names to avoid reserved word conflicts.

  Args:
    name: string, method name.

  Returns:
    The name with a '_' prefixed if the name is a reserved word.
  """
  if keyword.iskeyword(name) or name in RESERVED_WORDS:
    return name + '_'
  else:
    return name


def key2param(key):
  """Converts key names into parameter names.

  For example, converting "max-results" -> "max_results"

  Args:
    key: string, the method key name.

  Returns:
    A safe method name based on the key name.
  """
  result = []
  key = list(key)
  if not key[0].isalpha():
    result.append('x')
  for c in key:
    if c.isalnum():
      result.append(c)
    else:
      result.append('_')

  return ''.join(result)


@positional(2)
def build(serviceName,
          version,
          http=None,
          discoveryServiceUrl=DISCOVERY_URI,
          developerKey=None,
          model=None,
          requestBuilder=HttpRequest,
          credentials=None):
  """Construct a Resource for interacting with an API.

  Construct a Resource object for interacting with an API. The serviceName and
  version are the names from the Discovery service.

  Args:
    serviceName: string, name of the service.
    version: string, the version of the service.
    http: httplib2.Http, An instance of httplib2.Http or something that acts
      like it that HTTP requests will be made through.
    discoveryServiceUrl: string, a URI Template that points to the location of
      the discovery service. It should have two parameters {api} and
      {apiVersion} that when filled in produce an absolute URI to the discovery
      document for that service.
    developerKey: string, key obtained from
      https://code.google.com/apis/console.
    model: googleapiclient.Model, converts to and from the wire format.
    requestBuilder: googleapiclient.http.HttpRequest, encapsulator for an HTTP
      request.
    credentials: oauth2client.Credentials, credentials to be used for
      authentication.

  Returns:
    A Resource object with methods for interacting with the service.
  """
  params = {
      'api': serviceName,
      'apiVersion': version
      }

  if http is None:
    http = httplib2.Http()

  requested_url = uritemplate.expand(discoveryServiceUrl, params)

  # REMOTE_ADDR is defined by the CGI spec [RFC3875] as the environment
  # variable that contains the network address of the client sending the
  # request. If it exists then add that to the request for the discovery
  # document to avoid exceeding the quota on discovery requests.
  if 'REMOTE_ADDR' in os.environ:
    requested_url = _add_query_parameter(requested_url, 'userIp',
                                         os.environ['REMOTE_ADDR'])
  logger.info('URL being requested: GET %s' % requested_url)

  resp, content = http.request(requested_url)

  if resp.status == 404:
    raise UnknownApiNameOrVersion("name: %s  version: %s" % (serviceName,
                                                            version))
  if resp.status >= 400:
    raise HttpError(resp, content, uri=requested_url)

  try:
    content = content.decode('utf-8')
  except AttributeError:
    pass

  try:
    service = json.loads(content)
  except ValueError as e:
    logger.error('Failed to parse as JSON: ' + content)
    raise InvalidJsonError()

  return build_from_document(content, base=discoveryServiceUrl, http=http,
      developerKey=developerKey, model=model, requestBuilder=requestBuilder,
      credentials=credentials)


@positional(1)
def build_from_document(
    service,
    base=None,
    future=None,
    http=None,
    developerKey=None,
    model=None,
    requestBuilder=HttpRequest,
    credentials=None):
  """Create a Resource for interacting with an API.

  Same as `build()`, but constructs the Resource object from a discovery
  document that is it given, as opposed to retrieving one over HTTP.

  Args:
    service: string or object, the JSON discovery document describing the API.
      The value passed in may either be the JSON string or the deserialized
      JSON.
    base: string, base URI for all HTTP requests, usually the discovery URI.
      This parameter is no longer used as rootUrl and servicePath are included
      within the discovery document. (deprecated)
    future: string, discovery document with future capabilities (deprecated).
    http: httplib2.Http, An instance of httplib2.Http or something that acts
      like it that HTTP requests will be made through.
    developerKey: string, Key for controlling API usage, generated
      from the API Console.
    model: Model class instance that serializes and de-serializes requests and
      responses.
    requestBuilder: Takes an http request and packages it up to be executed.
    credentials: object, credentials to be used for authentication.

  Returns:
    A Resource object with methods for interacting with the service.
  """

  # future is no longer used.
  future = {}

  if isinstance(service, six.string_types):
    service = json.loads(service)
  base = urljoin(service['rootUrl'], service['servicePath'])
  schema = Schemas(service)

  if credentials:
    # If credentials were passed in, we could have two cases:
    # 1. the scopes were specified, in which case the given credentials
    #    are used for authorizing the http;
    # 2. the scopes were not provided (meaning the Application Default
    #    Credentials are to be used). In this case, the Application Default
    #    Credentials are built and used instead of the original credentials.
    #    If there are no scopes found (meaning the given service requires no
    #    authentication), there is no authorization of the http.
    if (isinstance(credentials, GoogleCredentials) and
        credentials.create_scoped_required()):
      scopes = service.get('auth', {}).get('oauth2', {}).get('scopes', {})
      if scopes:
        credentials = credentials.create_scoped(list(scopes.keys()))
      else:
        # No need to authorize the http object
        # if the service does not require authentication.
        credentials = None

    if credentials:
      http = credentials.authorize(http)

  if model is None:
    features = service.get('features', [])
    model = JsonModel('dataWrapper' in features)
  return Resource(http=http, baseUrl=base, model=model,
                  developerKey=developerKey, requestBuilder=requestBuilder,
                  resourceDesc=service, rootDesc=service, schema=schema)


def _cast(value, schema_type):
  """Convert value to a string based on JSON Schema type.

  See http://tools.ietf.org/html/draft-zyp-json-schema-03 for more details on
  JSON Schema.

  Args:
    value: any, the value to convert
    schema_type: string, the type that value should be interpreted as

  Returns:
    A string representation of 'value' based on the schema_type.
  """
  if schema_type == 'string':
    if type(value) == type('') or type(value) == type(u''):
      return value
    else:
      return str(value)
  elif schema_type == 'integer':
    return str(int(value))
  elif schema_type == 'number':
    return str(float(value))
  elif schema_type == 'boolean':
    return str(bool(value)).lower()
  else:
    if type(value) == type('') or type(value) == type(u''):
      return value
    else:
      return str(value)


def _media_size_to_long(maxSize):
  """Convert a string media size, such as 10GB or 3TB into an integer.

  Args:
    maxSize: string, size as a string, such as 2MB or 7GB.

  Returns:
    The size as an integer value.
  """
  if len(maxSize) < 2:
    return 0
  units = maxSize[-2:].upper()
  bit_shift = _MEDIA_SIZE_BIT_SHIFTS.get(units)
  if bit_shift is not None:
    return int(maxSize[:-2]) << bit_shift
  else:
    return int(maxSize)


def _media_path_url_from_info(root_desc, path_url):
  """Creates an absolute media path URL.

  Constructed using the API root URI and service path from the discovery
  document and the relative path for the API method.

  Args:
    root_desc: Dictionary; the entire original deserialized discovery document.
    path_url: String; the relative URL for the API method. Relative to the API
        root, which is specified in the discovery document.

  Returns:
    String; the absolute URI for media upload for the API method.
  """
  return '%(root)supload/%(service_path)s%(path)s' % {
      'root': root_desc['rootUrl'],
      'service_path': root_desc['servicePath'],
      'path': path_url,
  }


def _fix_up_parameters(method_desc, root_desc, http_method):
  """Updates parameters of an API method with values specific to this library.

  Specifically, adds whatever global parameters are specified by the API to the
  parameters for the individual method. Also adds parameters which don't
  appear in the discovery document, but are available to all discovery based
  APIs (these are listed in STACK_QUERY_PARAMETERS).

  SIDE EFFECTS: This updates the parameters dictionary object in the method
  description.

  Args:
    method_desc: Dictionary with metadata describing an API method. Value comes
        from the dictionary of methods stored in the 'methods' key in the
        deserialized discovery document.
    root_desc: Dictionary; the entire original deserialized discovery document.
    http_method: String; the HTTP method used to call the API method described
        in method_desc.

  Returns:
    The updated Dictionary stored in the 'parameters' key of the method
        description dictionary.
  """
  parameters = method_desc.setdefault('parameters', {})

  # Add in the parameters common to all methods.
  for name, description in six.iteritems(root_desc.get('parameters', {})):
    parameters[name] = description

  # Add in undocumented query parameters.
  for name in STACK_QUERY_PARAMETERS:
    parameters[name] = STACK_QUERY_PARAMETER_DEFAULT_VALUE.copy()

  # Add 'body' (our own reserved word) to parameters if the method supports
  # a request payload.
  if http_method in HTTP_PAYLOAD_METHODS and 'request' in method_desc:
    body = BODY_PARAMETER_DEFAULT_VALUE.copy()
    body.update(method_desc['request'])
    parameters['body'] = body

  return parameters


def _fix_up_media_upload(method_desc, root_desc, path_url, parameters):
  """Updates parameters of API by adding 'media_body' if supported by method.

  SIDE EFFECTS: If the method supports media upload and has a required body,
  sets body to be optional (required=False) instead. Also, if there is a
  'mediaUpload' in the method description, adds 'media_upload' key to
  parameters.

  Args:
    method_desc: Dictionary with metadata describing an API method. Value comes
        from the dictionary of methods stored in the 'methods' key in the
        deserialized discovery document.
    root_desc: Dictionary; the entire original deserialized discovery document.
    path_url: String; the relative URL for the API method. Relative to the API
        root, which is specified in the discovery document.
    parameters: A dictionary describing method parameters for method described
        in method_desc.

  Returns:
    Triple (accept, max_size, media_path_url) where:
      - accept is a list of strings representing what content types are
        accepted for media upload. Defaults to empty list if not in the
        discovery document.
      - max_size is a long representing the max size in bytes allowed for a
        media upload. Defaults to 0L if not in the discovery document.
      - media_path_url is a String; the absolute URI for media upload for the
        API method. Constructed using the API root URI and service path from
        the discovery document and the relative path for the API method. If
        media upload is not supported, this is None.
  """
  media_upload = method_desc.get('mediaUpload', {})
  accept = media_upload.get('accept', [])
  max_size = _media_size_to_long(media_upload.get('maxSize', ''))
  media_path_url = None

  if media_upload:
    media_path_url = _media_path_url_from_info(root_desc, path_url)
    parameters['media_body'] = MEDIA_BODY_PARAMETER_DEFAULT_VALUE.copy()
    if 'body' in parameters:
      parameters['body']['required'] = False

  return accept, max_size, media_path_url


def _fix_up_method_description(method_desc, root_desc):
  """Updates a method description in a discovery document.

  SIDE EFFECTS: Changes the parameters dictionary in the method description with
  extra parameters which are used locally.

  Args:
    method_desc: Dictionary with metadata describing an API method. Value comes
        from the dictionary of methods stored in the 'methods' key in the
        deserialized discovery document.
    root_desc: Dictionary; the entire original deserialized discovery document.

  Returns:
    Tuple (path_url, http_method, method_id, accept, max_size, media_path_url)
    where:
      - path_url is a String; the relative URL for the API method. Relative to
        the API root, which is specified in the discovery document.
      - http_method is a String; the HTTP method used to call the API method
        described in the method description.
      - method_id is a String; the name of the RPC method associated with the
        API method, and is in the method description in the 'id' key.
      - accept is a list of strings representing what content types are
        accepted for media upload. Defaults to empty list if not in the
        discovery document.
      - max_size is a long representing the max size in bytes allowed for a
        media upload. Defaults to 0L if not in the discovery document.
      - media_path_url is a String; the absolute URI for media upload for the
        API method. Constructed using the API root URI and service path from
        the discovery document and the relative path for the API method. If
        media upload is not supported, this is None.
  """
  path_url = method_desc['path']
  http_method = method_desc['httpMethod']
  method_id = method_desc['id']

  parameters = _fix_up_parameters(method_desc, root_desc, http_method)
  # Order is important. `_fix_up_media_upload` needs `method_desc` to have a
  # 'parameters' key and needs to know if there is a 'body' parameter because it
  # also sets a 'media_body' parameter.
  accept, max_size, media_path_url = _fix_up_media_upload(
      method_desc, root_desc, path_url, parameters)

  return path_url, http_method, method_id, accept, max_size, media_path_url


def _urljoin(base, url):
  """Custom urljoin replacement supporting : before / in url."""
  # In general, it's unsafe to simply join base and url. However, for
  # the case of discovery documents, we know:
  #  * base will never contain params, query, or fragment
  #  * url will never contain a scheme or net_loc.
  # In general, this means we can safely join on /; we just need to
  # ensure we end up with precisely one / joining base and url. The
  # exception here is the case of media uploads, where url will be an
  # absolute url.
  if url.startswith('http://') or url.startswith('https://'):
    return urljoin(base, url)
  new_base = base if base.endswith('/') else base + '/'
  new_url = url[1:] if url.startswith('/') else url
  return new_base + new_url


# TODO(dhermes): Convert this class to ResourceMethod and make it callable
class ResourceMethodParameters(object):
  """Represents the parameters associated with a method.

  Attributes:
    argmap: Map from method parameter name (string) to query parameter name
        (string).
    required_params: List of required parameters (represented by parameter
        name as string).
    repeated_params: List of repeated parameters (represented by parameter
        name as string).
    pattern_params: Map from method parameter name (string) to regular
        expression (as a string). If the pattern is set for a parameter, the
        value for that parameter must match the regular expression.
    query_params: List of parameters (represented by parameter name as string)
        that will be used in the query string.
    path_params: Set of parameters (represented by parameter name as string)
        that will be used in the base URL path.
    param_types: Map from method parameter name (string) to parameter type. Type
        can be any valid JSON schema type; valid values are 'any', 'array',
        'boolean', 'integer', 'number', 'object', or 'string'. Reference:
        http://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.1
    enum_params: Map from method parameter name (string) to list of strings,
       where each list of strings is the list of acceptable enum values.
  """

  def __init__(self, method_desc):
    """Constructor for ResourceMethodParameters.

    Sets default values and defers to set_parameters to populate.

    Args:
      method_desc: Dictionary with metadata describing an API method. Value
          comes from the dictionary of methods stored in the 'methods' key in
          the deserialized discovery document.
    """
    self.argmap = {}
    self.required_params = []
    self.repeated_params = []
    self.pattern_params = {}
    self.query_params = []
    # TODO(dhermes): Change path_params to a list if the extra URITEMPLATE
    #                parsing is gotten rid of.
    self.path_params = set()
    self.param_types = {}
    self.enum_params = {}

    self.set_parameters(method_desc)

  def set_parameters(self, method_desc):
    """Populates maps and lists based on method description.

    Iterates through each parameter for the method and parses the values from
    the parameter dictionary.

    Args:
      method_desc: Dictionary with metadata describing an API method. Value
          comes from the dictionary of methods stored in the 'methods' key in
          the deserialized discovery document.
    """
    for arg, desc in six.iteritems(method_desc.get('parameters', {})):
      param = key2param(arg)
      self.argmap[param] = arg

      if desc.get('pattern'):
        self.pattern_params[param] = desc['pattern']
      if desc.get('enum'):
        self.enum_params[param] = desc['enum']
      if desc.get('required'):
        self.required_params.append(param)
      if desc.get('repeated'):
        self.repeated_params.append(param)
      if desc.get('location') == 'query':
        self.query_params.append(param)
      if desc.get('location') == 'path':
        self.path_params.add(param)
      self.param_types[param] = desc.get('type', 'string')

    # TODO(dhermes): Determine if this is still necessary. Discovery based APIs
    #                should have all path parameters already marked with
    #                'location: path'.
    for match in URITEMPLATE.finditer(method_desc['path']):
      for namematch in VARNAME.finditer(match.group(0)):
        name = key2param(namematch.group(0))
        self.path_params.add(name)
        if name in self.query_params:
          self.query_params.remove(name)


def createMethod(methodName, methodDesc, rootDesc, schema):
  """Creates a method for attaching to a Resource.

  Args:
    methodName: string, name of the method to use.
    methodDesc: object, fragment of deserialized discovery document that
      describes the method.
    rootDesc: object, the entire deserialized discovery document.
    schema: object, mapping of schema names to schema descriptions.
  """
  methodName = fix_method_name(methodName)
  (pathUrl, httpMethod, methodId, accept,
   maxSize, mediaPathUrl) = _fix_up_method_description(methodDesc, rootDesc)

  parameters = ResourceMethodParameters(methodDesc)

  def method(self, **kwargs):
    # Don't bother with doc string, it will be over-written by createMethod.

    for name in six.iterkeys(kwargs):
      if name not in parameters.argmap:
        raise TypeError('Got an unexpected keyword argument "%s"' % name)

    # Remove args that have a value of None.
    keys = list(kwargs.keys())
    for name in keys:
      if kwargs[name] is None:
        del kwargs[name]

    for name in parameters.required_params:
      if name not in kwargs:
        raise TypeError('Missing required parameter "%s"' % name)

    for name, regex in six.iteritems(parameters.pattern_params):
      if name in kwargs:
        if isinstance(kwargs[name], six.string_types):
          pvalues = [kwargs[name]]
        else:
          pvalues = kwargs[name]
        for pvalue in pvalues:
          if re.match(regex, pvalue) is None:
            raise TypeError(
                'Parameter "%s" value "%s" does not match the pattern "%s"' %
                (name, pvalue, regex))

    for name, enums in six.iteritems(parameters.enum_params):
      if name in kwargs:
        # We need to handle the case of a repeated enum
        # name differently, since we want to handle both
        # arg='value' and arg=['value1', 'value2']
        if (name in parameters.repeated_params and
            not isinstance(kwargs[name], six.string_types)):
          values = kwargs[name]
        else:
          values = [kwargs[name]]
        for value in values:
          if value not in enums:
            raise TypeError(
                'Parameter "%s" value "%s" is not an allowed value in "%s"' %
                (name, value, str(enums)))

    actual_query_params = {}
    actual_path_params = {}
    for key, value in six.iteritems(kwargs):
      to_type = parameters.param_types.get(key, 'string')
      # For repeated parameters we cast each member of the list.
      if key in parameters.repeated_params and type(value) == type([]):
        cast_value = [_cast(x, to_type) for x in value]
      else:
        cast_value = _cast(value, to_type)
      if key in parameters.query_params:
        actual_query_params[parameters.argmap[key]] = cast_value
      if key in parameters.path_params:
        actual_path_params[parameters.argmap[key]] = cast_value
    body_value = kwargs.get('body', None)
    media_filename = kwargs.get('media_body', None)

    if self._developerKey:
      actual_query_params['key'] = self._developerKey

    model = self._model
    if methodName.endswith('_media'):
      model = MediaModel()
    elif 'response' not in methodDesc:
      model = RawModel()

    headers = {}
    headers, params, query, body = model.request(headers,
        actual_path_params, actual_query_params, body_value)

    expanded_url = uritemplate.expand(pathUrl, params)
    url = _urljoin(self._baseUrl, expanded_url + query)

    resumable = None
    multipart_boundary = ''

    if media_filename:
      # Ensure we end up with a valid MediaUpload object.
      if isinstance(media_filename, six.string_types):
        (media_mime_type, encoding) = mimetypes.guess_type(media_filename)
        if media_mime_type is None:
          raise UnknownFileType(media_filename)
        if not mimeparse.best_match([media_mime_type], ','.join(accept)):
          raise UnacceptableMimeTypeError(media_mime_type)
        media_upload = MediaFileUpload(media_filename,
                                       mimetype=media_mime_type)
      elif isinstance(media_filename, MediaUpload):
        media_upload = media_filename
      else:
        raise TypeError('media_filename must be str or MediaUpload.')

      # Check the maxSize
      if media_upload.size() is not None and media_upload.size() > maxSize > 0:
        raise MediaUploadSizeError("Media larger than: %s" % maxSize)

      # Use the media path uri for media uploads
      expanded_url = uritemplate.expand(mediaPathUrl, params)
      url = _urljoin(self._baseUrl, expanded_url + query)
      if media_upload.resumable():
        url = _add_query_parameter(url, 'uploadType', 'resumable')

      if media_upload.resumable():
        # This is all we need to do for resumable, if the body exists it gets
        # sent in the first request, otherwise an empty body is sent.
        resumable = media_upload
      else:
        # A non-resumable upload
        if body is None:
          # This is a simple media upload
          headers['content-type'] = media_upload.mimetype()
          body = media_upload.getbytes(0, media_upload.size())
          url = _add_query_parameter(url, 'uploadType', 'media')
        else:
          # This is a multipart/related upload.
          msgRoot = MIMEMultipart('related')
          # msgRoot should not write out it's own headers
          setattr(msgRoot, '_write_headers', lambda self: None)

          # attach the body as one part
          msg = MIMENonMultipart(*headers['content-type'].split('/'))
          msg.set_payload(body)
          msgRoot.attach(msg)

          # attach the media as the second part
          msg = MIMENonMultipart(*media_upload.mimetype().split('/'))
          msg['Content-Transfer-Encoding'] = 'binary'

          payload = media_upload.getbytes(0, media_upload.size())
          msg.set_payload(payload)
          msgRoot.attach(msg)
          # encode the body: note that we can't use `as_string`, because
          # it plays games with `From ` lines.
          fp = StringIO()
          g = Generator(fp, mangle_from_=False)
          g.flatten(msgRoot, unixfrom=False)
          body = fp.getvalue()

          multipart_boundary = msgRoot.get_boundary()
          headers['content-type'] = ('multipart/related; '
                                     'boundary="%s"') % multipart_boundary
          url = _add_query_parameter(url, 'uploadType', 'multipart')

    logger.info('URL being requested: %s %s' % (httpMethod,url))
    return self._requestBuilder(self._http,
                                model.response,
                                url,
                                method=httpMethod,
                                body=body,
                                headers=headers,
                                methodId=methodId,
                                resumable=resumable)

  docs = [methodDesc.get('description', DEFAULT_METHOD_DOC), '\n\n']
  if len(parameters.argmap) > 0:
    docs.append('Args:\n')

  # Skip undocumented params and params common to all methods.
  skip_parameters = list(rootDesc.get('parameters', {}).keys())
  skip_parameters.extend(STACK_QUERY_PARAMETERS)

  all_args = list(parameters.argmap.keys())
  args_ordered = [key2param(s) for s in methodDesc.get('parameterOrder', [])]

  # Move body to the front of the line.
  if 'body' in all_args:
    args_ordered.append('body')

  for name in all_args:
    if name not in args_ordered:
      args_ordered.append(name)

  for arg in args_ordered:
    if arg in skip_parameters:
      continue

    repeated = ''
    if arg in parameters.repeated_params:
      repeated = ' (repeated)'
    required = ''
    if arg in parameters.required_params:
      required = ' (required)'
    paramdesc = methodDesc['parameters'][parameters.argmap[arg]]
    paramdoc = paramdesc.get('description', 'A parameter')
    if '$ref' in paramdesc:
      docs.append(
          ('  %s: object, %s%s%s\n    The object takes the'
          ' form of:\n\n%s\n\n') % (arg, paramdoc, required, repeated,
            schema.prettyPrintByName(paramdesc['$ref'])))
    else:
      paramtype = paramdesc.get('type', 'string')
      docs.append('  %s: %s, %s%s%s\n' % (arg, paramtype, paramdoc, required,
                                          repeated))
    enum = paramdesc.get('enum', [])
    enumDesc = paramdesc.get('enumDescriptions', [])
    if enum and enumDesc:
      docs.append('    Allowed values\n')
      for (name, desc) in zip(enum, enumDesc):
        docs.append('      %s - %s\n' % (name, desc))
  if 'response' in methodDesc:
    if methodName.endswith('_media'):
      docs.append('\nReturns:\n  The media object as a string.\n\n    ')
    else:
      docs.append('\nReturns:\n  An object of the form:\n\n    ')
      docs.append(schema.prettyPrintSchema(methodDesc['response']))

  setattr(method, '__doc__', ''.join(docs))
  return (methodName, method)


def createNextMethod(methodName):
  """Creates any _next methods for attaching to a Resource.

  The _next methods allow for easy iteration through list() responses.

  Args:
    methodName: string, name of the method to use.
  """
  methodName = fix_method_name(methodName)

  def methodNext(self, previous_request, previous_response):
    """Retrieves the next page of results.

Args:
  previous_request: The request for the previous page. (required)
  previous_response: The response from the request for the previous page. (required)

Returns:
  A request object that you can call 'execute()' on to request the next
  page. Returns None if there are no more items in the collection.
    """
    # Retrieve nextPageToken from previous_response
    # Use as pageToken in previous_request to create new request.

    if 'nextPageToken' not in previous_response:
      return None

    request = copy.copy(previous_request)

    pageToken = previous_response['nextPageToken']
    parsed = list(urlparse(request.uri))
    q = parse_qsl(parsed[4])

    # Find and remove old 'pageToken' value from URI
    newq = [(key, value) for (key, value) in q if key != 'pageToken']
    newq.append(('pageToken', pageToken))
    parsed[4] = urlencode(newq)
    uri = urlunparse(parsed)

    request.uri = uri

    logger.info('URL being requested: %s %s' % (methodName,uri))

    return request

  return (methodName, methodNext)


class Resource(object):
  """A class for interacting with a resource."""

  def __init__(self, http, baseUrl, model, requestBuilder, developerKey,
               resourceDesc, rootDesc, schema):
    """Build a Resource from the API description.

    Args:
      http: httplib2.Http, Object to make http requests with.
      baseUrl: string, base URL for the API. All requests are relative to this
          URI.
      model: googleapiclient.Model, converts to and from the wire format.
      requestBuilder: class or callable that instantiates an
          googleapiclient.HttpRequest object.
      developerKey: string, key obtained from
          https://code.google.com/apis/console
      resourceDesc: object, section of deserialized discovery document that
          describes a resource. Note that the top level discovery document
          is considered a resource.
      rootDesc: object, the entire deserialized discovery document.
      schema: object, mapping of schema names to schema descriptions.
    """
    self._dynamic_attrs = []

    self._http = http
    self._baseUrl = baseUrl
    self._model = model
    self._developerKey = developerKey
    self._requestBuilder = requestBuilder
    self._resourceDesc = resourceDesc
    self._rootDesc = rootDesc
    self._schema = schema

    self._set_service_methods()

  def _set_dynamic_attr(self, attr_name, value):
    """Sets an instance attribute and tracks it in a list of dynamic attributes.

    Args:
      attr_name: string; The name of the attribute to be set
      value: The value being set on the object and tracked in the dynamic cache.
    """
    self._dynamic_attrs.append(attr_name)
    self.__dict__[attr_name] = value

  def __getstate__(self):
    """Trim the state down to something that can be pickled.

    Uses the fact that the instance variable _dynamic_attrs holds attrs that
    will be wiped and restored on pickle serialization.
    """
    state_dict = copy.copy(self.__dict__)
    for dynamic_attr in self._dynamic_attrs:
      del state_dict[dynamic_attr]
    del state_dict['_dynamic_attrs']
    return state_dict

  def __setstate__(self, state):
    """Reconstitute the state of the object from being pickled.

    Uses the fact that the instance variable _dynamic_attrs holds attrs that
    will be wiped and restored on pickle serialization.
    """
    self.__dict__.update(state)
    self._dynamic_attrs = []
    self._set_service_methods()

  def _set_service_methods(self):
    self._add_basic_methods(self._resourceDesc, self._rootDesc, self._schema)
    self._add_nested_resources(self._resourceDesc, self._rootDesc, self._schema)
    self._add_next_methods(self._resourceDesc, self._schema)

  def _add_basic_methods(self, resourceDesc, rootDesc, schema):
    # Add basic methods to Resource
    if 'methods' in resourceDesc:
      for methodName, methodDesc in six.iteritems(resourceDesc['methods']):
        fixedMethodName, method = createMethod(
            methodName, methodDesc, rootDesc, schema)
        self._set_dynamic_attr(fixedMethodName,
                               method.__get__(self, self.__class__))
        # Add in _media methods. The functionality of the attached method will
        # change when it sees that the method name ends in _media.
        if methodDesc.get('supportsMediaDownload', False):
          fixedMethodName, method = createMethod(
              methodName + '_media', methodDesc, rootDesc, schema)
          self._set_dynamic_attr(fixedMethodName,
                                 method.__get__(self, self.__class__))

  def _add_nested_resources(self, resourceDesc, rootDesc, schema):
    # Add in nested resources
    if 'resources' in resourceDesc:

      def createResourceMethod(methodName, methodDesc):
        """Create a method on the Resource to access a nested Resource.

        Args:
          methodName: string, name of the method to use.
          methodDesc: object, fragment of deserialized discovery document that
            describes the method.
        """
        methodName = fix_method_name(methodName)

        def methodResource(self):
          return Resource(http=self._http, baseUrl=self._baseUrl,
                          model=self._model, developerKey=self._developerKey,
                          requestBuilder=self._requestBuilder,
                          resourceDesc=methodDesc, rootDesc=rootDesc,
                          schema=schema)

        setattr(methodResource, '__doc__', 'A collection resource.')
        setattr(methodResource, '__is_resource__', True)

        return (methodName, methodResource)

      for methodName, methodDesc in six.iteritems(resourceDesc['resources']):
        fixedMethodName, method = createResourceMethod(methodName, methodDesc)
        self._set_dynamic_attr(fixedMethodName,
                               method.__get__(self, self.__class__))

  def _add_next_methods(self, resourceDesc, schema):
    # Add _next() methods
    # Look for response bodies in schema that contain nextPageToken, and methods
    # that take a pageToken parameter.
    if 'methods' in resourceDesc:
      for methodName, methodDesc in six.iteritems(resourceDesc['methods']):
        if 'response' in methodDesc:
          responseSchema = methodDesc['response']
          if '$ref' in responseSchema:
            responseSchema = schema.get(responseSchema['$ref'])
          hasNextPageToken = 'nextPageToken' in responseSchema.get('properties',
                                                                   {})
          hasPageToken = 'pageToken' in methodDesc.get('parameters', {})
          if hasNextPageToken and hasPageToken:
            fixedMethodName, method = createNextMethod(methodName + '_next')
            self._set_dynamic_attr(fixedMethodName,
                                   method.__get__(self, self.__class__))
