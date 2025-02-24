from ..useragents import UserAgent
from ..utils import cached_property


class UserAgentMixin(object):
    """Adds a `user_agent` attribute to the request object which
    contains the parsed user agent of the browser that triggered the
    request as a :class:`~werkzeug.useragents.UserAgent` object.
    """

    @cached_property
    def user_agent(self):
        """The current user agent."""
        return UserAgent(self.environ)
