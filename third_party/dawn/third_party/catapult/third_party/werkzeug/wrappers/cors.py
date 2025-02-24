from ..http import dump_header
from ..http import parse_set_header
from ..utils import environ_property
from ..utils import header_property


class CORSRequestMixin(object):
    """A mixin for :class:`~werkzeug.wrappers.BaseRequest` subclasses
    that adds descriptors for Cross Origin Resource Sharing (CORS)
    headers.

    .. versionadded:: 1.0
    """

    origin = environ_property(
        "HTTP_ORIGIN",
        doc=(
            "The host that the request originated from. Set"
            " :attr:`~CORSResponseMixin.access_control_allow_origin` on"
            " the response to indicate which origins are allowed."
        ),
    )

    access_control_request_headers = environ_property(
        "HTTP_ACCESS_CONTROL_REQUEST_HEADERS",
        load_func=parse_set_header,
        doc=(
            "Sent with a preflight request to indicate which headers"
            " will be sent with the cross origin request. Set"
            " :attr:`~CORSResponseMixin.access_control_allow_headers`"
            " on the response to indicate which headers are allowed."
        ),
    )

    access_control_request_method = environ_property(
        "HTTP_ACCESS_CONTROL_REQUEST_METHOD",
        doc=(
            "Sent with a preflight request to indicate which method"
            " will be used for the cross origin request. Set"
            " :attr:`~CORSResponseMixin.access_control_allow_methods`"
            " on the response to indicate which methods are allowed."
        ),
    )


class CORSResponseMixin(object):
    """A mixin for :class:`~werkzeug.wrappers.BaseResponse` subclasses
    that adds descriptors for Cross Origin Resource Sharing (CORS)
    headers.

    .. versionadded:: 1.0
    """

    @property
    def access_control_allow_credentials(self):
        """Whether credentials can be shared by the browser to
        JavaScript code. As part of the preflight request it indicates
        whether credentials can be used on the cross origin request.
        """
        return "Access-Control-Allow-Credentials" in self.headers

    @access_control_allow_credentials.setter
    def access_control_allow_credentials(self, value):
        if value is True:
            self.headers["Access-Control-Allow-Credentials"] = "true"
        else:
            self.headers.pop("Access-Control-Allow-Credentials", None)

    access_control_allow_headers = header_property(
        "Access-Control-Allow-Headers",
        load_func=parse_set_header,
        dump_func=dump_header,
        doc="Which headers can be sent with the cross origin request.",
    )

    access_control_allow_methods = header_property(
        "Access-Control-Allow-Methods",
        load_func=parse_set_header,
        dump_func=dump_header,
        doc="Which methods can be used for the cross origin request.",
    )

    access_control_allow_origin = header_property(
        "Access-Control-Allow-Origin",
        doc="The origin or '*' for any origin that may make cross origin requests.",
    )

    access_control_expose_headers = header_property(
        "Access-Control-Expose-Headers",
        load_func=parse_set_header,
        dump_func=dump_header,
        doc="Which headers can be shared by the browser to JavaScript code.",
    )

    access_control_max_age = header_property(
        "Access-Control-Max-Age",
        load_func=int,
        dump_func=str,
        doc="The maximum age in seconds the access control settings can be cached for.",
    )
