# Copyright 2016 Google Inc.
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

"""Authorization support for gRPC."""

from __future__ import absolute_import

import six
try:
    import grpc
except ImportError as caught_exc:  # pragma: NO COVER
    six.raise_from(
        ImportError(
            'gRPC is not installed, please install the grpcio package '
            'to use the gRPC transport.'
        ),
        caught_exc,
    )


class AuthMetadataPlugin(grpc.AuthMetadataPlugin):
    """A `gRPC AuthMetadataPlugin`_ that inserts the credentials into each
    request.

    .. _gRPC AuthMetadataPlugin:
        http://www.grpc.io/grpc/python/grpc.html#grpc.AuthMetadataPlugin

    Args:
        credentials (google.auth.credentials.Credentials): The credentials to
            add to requests.
        request (google.auth.transport.Request): A HTTP transport request
            object used to refresh credentials as needed.
    """
    def __init__(self, credentials, request):
        # pylint: disable=no-value-for-parameter
        # pylint doesn't realize that the super method takes no arguments
        # because this class is the same name as the superclass.
        super(AuthMetadataPlugin, self).__init__()
        self._credentials = credentials
        self._request = request

    def _get_authorization_headers(self, context):
        """Gets the authorization headers for a request.

        Returns:
            Sequence[Tuple[str, str]]: A list of request headers (key, value)
                to add to the request.
        """
        headers = {}
        self._credentials.before_request(
            self._request,
            context.method_name,
            context.service_url,
            headers)

        return list(six.iteritems(headers))

    def __call__(self, context, callback):
        """Passes authorization metadata into the given callback.

        Args:
            context (grpc.AuthMetadataContext): The RPC context.
            callback (grpc.AuthMetadataPluginCallback): The callback that will
                be invoked to pass in the authorization metadata.
        """
        callback(self._get_authorization_headers(context), None)


def secure_authorized_channel(
        credentials, request, target, ssl_credentials=None, **kwargs):
    """Creates a secure authorized gRPC channel.

    This creates a channel with SSL and :class:`AuthMetadataPlugin`. This
    channel can be used to create a stub that can make authorized requests.

    Example::

        import google.auth
        import google.auth.transport.grpc
        import google.auth.transport.requests
        from google.cloud.speech.v1 import cloud_speech_pb2

        # Get credentials.
        credentials, _ = google.auth.default()

        # Get an HTTP request function to refresh credentials.
        request = google.auth.transport.requests.Request()

        # Create a channel.
        channel = google.auth.transport.grpc.secure_authorized_channel(
            credentials, 'speech.googleapis.com:443', request)

        # Use the channel to create a stub.
        cloud_speech.create_Speech_stub(channel)

    Args:
        credentials (google.auth.credentials.Credentials): The credentials to
            add to requests.
        request (google.auth.transport.Request): A HTTP transport request
            object used to refresh credentials as needed. Even though gRPC
            is a separate transport, there's no way to refresh the credentials
            without using a standard http transport.
        target (str): The host and port of the service.
        ssl_credentials (grpc.ChannelCredentials): Optional SSL channel
            credentials. This can be used to specify different certificates.
        kwargs: Additional arguments to pass to :func:`grpc.secure_channel`.

    Returns:
        grpc.Channel: The created gRPC channel.
    """
    # Create the metadata plugin for inserting the authorization header.
    metadata_plugin = AuthMetadataPlugin(credentials, request)

    # Create a set of grpc.CallCredentials using the metadata plugin.
    google_auth_credentials = grpc.metadata_call_credentials(metadata_plugin)

    if ssl_credentials is None:
        ssl_credentials = grpc.ssl_channel_credentials()

    # Combine the ssl credentials and the authorization credentials.
    composite_credentials = grpc.composite_channel_credentials(
        ssl_credentials, google_auth_credentials)

    return grpc.secure_channel(target, composite_credentials, **kwargs)
