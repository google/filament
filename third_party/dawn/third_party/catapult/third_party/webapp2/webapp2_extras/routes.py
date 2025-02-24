# -*- coding: utf-8 -*-
"""
    webapp2_extras.routes
    =====================

    Extra route classes for webapp2.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import re
import urllib

from webob import exc

import webapp2


class MultiRoute(object):
    """Base class for routes with nested routes."""

    routes = None
    children = None
    match_children = None
    build_children = None

    def __init__(self, routes):
        self.routes = routes

    def get_children(self):
        if self.children is None:
            self.children = []
            for route in self.routes:
                for r in route.get_routes():
                    self.children.append(r)

        for rv in self.children:
            yield rv

    def get_match_children(self):
        if self.match_children is None:
            self.match_children = []
            for route in self.get_children():
                for r in route.get_match_routes():
                    self.match_children.append(r)

        for rv in self.match_children:
            yield rv

    def get_build_children(self):
        if self.build_children is None:
            self.build_children = {}
            for route in self.get_children():
                for n, r in route.get_build_routes():
                    self.build_children[n] = r

        for rv in self.build_children.iteritems():
            yield rv

    get_routes = get_children
    get_match_routes = get_match_children
    get_build_routes = get_build_children


class DomainRoute(MultiRoute):
    """A route used to restrict route matches to a given domain or subdomain.

    For example, to restrict routes to a subdomain of the appspot domain::

        app = WSGIApplication([
            DomainRoute('<subdomain>.app-id.appspot.com', [
                Route('/foo', 'FooHandler', 'subdomain-thing'),
            ]),
            Route('/bar', 'BarHandler', 'normal-thing'),
        ])

    The template follows the same syntax used by :class:`webapp2.Route` and
    must define named groups if any value must be added to the match results.
    In the example above, an extra `subdomain` keyword is passed to the
    handler, but if the regex didn't define any named groups, nothing would
    be added.
    """

    def __init__(self, template, routes):
        """Initializes a URL route.

        :param template:
            A route template to match against ``environ['SERVER_NAME']``.
            See a syntax description in :meth:`webapp2.Route.__init__`.
        :param routes:
            A list of :class:`webapp2.Route` instances.
        """
        super(DomainRoute, self).__init__(routes)
        self.template = template

    def get_match_routes(self):
        # This route will do pre-matching before matching the nested routes!
        yield self

    def match(self, request):
        # Use SERVER_NAME to ignore port number that comes with request.host?
        # host_match = self.regex.match(request.host.split(':', 1)[0])
        host_match = self.regex.match(request.environ['SERVER_NAME'])

        if host_match:
            args, kwargs = webapp2._get_route_variables(host_match)
            return _match_routes(self.get_match_children, request, None,
                                 kwargs)

    @webapp2.cached_property
    def regex(self):
        regex, reverse_template, args_count, kwargs_count, variables = \
            webapp2._parse_route_template(self.template,
                                          default_sufix='[^\.]+')
        return regex


class NamePrefixRoute(MultiRoute):
    """The idea of this route is to set a base name for other routes::

        app = WSGIApplication([
            NamePrefixRoute('user-', [
                Route('/users/<user:\w+>/', UserOverviewHandler, 'overview'),
                Route('/users/<user:\w+>/profile', UserProfileHandler,
                      'profile'),
                Route('/users/<user:\w+>/projects', UserProjectsHandler,
                      'projects'),
            ]),
        ])

    The example above is the same as setting the following routes, just more
    convenient as you can reuse the name prefix::

        app = WSGIApplication([
            Route('/users/<user:\w+>/', UserOverviewHandler, 'user-overview'),
            Route('/users/<user:\w+>/profile', UserProfileHandler,
                  'user-profile'),
            Route('/users/<user:\w+>/projects', UserProjectsHandler,
                  'user-projects'),
        ])
    """

    _attr = 'name'

    def __init__(self, prefix, routes):
        """Initializes a URL route.

        :param prefix:
            The prefix to be prepended.
        :param routes:
            A list of :class:`webapp2.Route` instances.
        """
        super(NamePrefixRoute, self).__init__(routes)
        self.prefix = prefix
        # Prepend a prefix to a route attribute.
        for route in self.get_routes():
            setattr(route, self._attr, prefix + getattr(route, self._attr))


class HandlerPrefixRoute(NamePrefixRoute):
    """Same as :class:`NamePrefixRoute`, but prefixes the route handler."""

    _attr = 'handler'


class PathPrefixRoute(NamePrefixRoute):
    """Same as :class:`NamePrefixRoute`, but prefixes the route path.

    For example, imagine we have these routes::

        app = WSGIApplication([
            Route('/users/<user:\w+>/', UserOverviewHandler,
                  'user-overview'),
            Route('/users/<user:\w+>/profile', UserProfileHandler,
                  'user-profile'),
            Route('/users/<user:\w+>/projects', UserProjectsHandler,
                  'user-projects'),
        ])

    We could refactor them to reuse the common path prefix::

        app = WSGIApplication([
            PathPrefixRoute('/users/<user:\w+>', [
                Route('/', UserOverviewHandler, 'user-overview'),
                Route('/profile', UserProfileHandler, 'user-profile'),
                Route('/projects', UserProjectsHandler, 'user-projects'),
            ]),
        ])

    This is not only convenient, but also performs better: the nested routes
    will only be tested if the path prefix matches.
    """

    _attr = 'template'

    def __init__(self, prefix, routes):
        """Initializes a URL route.

        :param prefix:
            The prefix to be prepended. It must start with a slash but not
            end with a slash.
        :param routes:
            A list of :class:`webapp2.Route` instances.
        """
        assert prefix.startswith('/') and not prefix.endswith('/'), \
            'Path prefixes must start with a slash but not end with a slash.'
        super(PathPrefixRoute, self).__init__(prefix, routes)

    def get_match_routes(self):
        # This route will do pre-matching before matching the nested routes!
        yield self

    def match(self, request):
        if not self.regex.match(urllib.unquote(request.path)):
            return None

        return _match_routes(self.get_match_children, request)

    @webapp2.cached_property
    def regex(self):
        regex, reverse_template, args_count, kwargs_count, variables = \
            webapp2._parse_route_template(self.prefix + '<:/.*>')
        return regex


class RedirectRoute(webapp2.Route):
    """A convenience route class for easy redirects.

    It adds redirect_to, redirect_to_name and strict_slash options to
    :class:`webapp2.Route`.
    """

    def __init__(self, template, handler=None, name=None, defaults=None,
                 build_only=False, handler_method=None, methods=None,
                 schemes=None, redirect_to=None, redirect_to_name=None,
                 strict_slash=False):
        """Initializes a URL route. Extra arguments compared to
        :meth:`webapp2.Route.__init__`:

        :param redirect_to:
            A URL string or a callable that returns a URL. If set, this route
            is used to redirect to it. The callable is called passing
            ``(handler, *args, **kwargs)`` as arguments. This is a
            convenience to use :class:`RedirectHandler`. These two are
            equivalent::

                route = Route('/foo', handler=webapp2.RedirectHandler,
                              defaults={'_uri': '/bar'})
                route = Route('/foo', redirect_to='/bar')

        :param redirect_to_name:
            Same as `redirect_to`, but the value is the name of a route to
            redirect to. In the example below, accessing '/hello-again' will
            redirect to the route named 'hello'::

                route = Route('/hello', handler=HelloHandler, name='hello')
                route = Route('/hello-again', redirect_to_name='hello')

        :param strict_slash:
            If True, redirects access to the same URL with different trailing
            slash to the strict path defined in the route. For example, take
            these routes::

                route = Route('/foo', FooHandler, strict_slash=True)
                route = Route('/bar/', BarHandler, strict_slash=True)

            Because **strict_slash** is True, this is what will happen:

            - Access to ``/foo`` will execute ``FooHandler`` normally.
            - Access to ``/bar/`` will execute ``BarHandler`` normally.
            - Access to ``/foo/`` will redirect to ``/foo``.
            - Access to ``/bar`` will redirect to ``/bar/``.
        """
        super(RedirectRoute, self).__init__(
            template, handler=handler, name=name, defaults=defaults,
            build_only=build_only, handler_method=handler_method,
            methods=methods, schemes=schemes)

        if strict_slash and not name:
            raise ValueError('Routes with strict_slash must have a name.')

        self.strict_slash = strict_slash
        self.redirect_to_name = redirect_to_name

        if redirect_to is not None:
            assert redirect_to_name is None
            self.handler = webapp2.RedirectHandler
            self.defaults['_uri'] = redirect_to

    def get_match_routes(self):
        """Generator to get all routes that can be matched from a route.

        :yields:
            This route or all nested routes that can be matched.
        """
        if self.redirect_to_name:
            main_route = self._get_redirect_route(name=self.redirect_to_name)
        else:
            main_route = self

        if not self.build_only:
            if self.strict_slash is True:
                if self.template.endswith('/'):
                    template = self.template[:-1]
                else:
                    template = self.template + '/'

                yield main_route
                yield self._get_redirect_route(template=template)
            else:
                yield main_route

    def _get_redirect_route(self, template=None, name=None):
        template = template or self.template
        name = name or self.name
        defaults = self.defaults.copy()
        defaults.update({
            '_uri': self._redirect,
            '_name': name,
        })
        new_route = webapp2.Route(template, webapp2.RedirectHandler,
                                  defaults=defaults)
        return new_route

    def _redirect(self, handler, *args, **kwargs):
        # Get from request because args is empty if named routes are set?
        # args, kwargs = (handler.request.route_args,
        #                 handler.request.route_kwargs)
        kwargs.pop('_uri', None)
        kwargs.pop('_code', None)
        return handler.uri_for(kwargs.pop('_name'), *args, **kwargs)


def _match_routes(iter_func, request, extra_args=None, extra_kwargs=None):
    """Tries to match a route given an iterator."""
    method_not_allowed = False
    for route in iter_func():
        try:
            match = route.match(request)
            if match:
                route, args, kwargs = match
                if extra_args:
                    args += extra_args

                if extra_kwargs:
                    kwargs.update(extra_kwargs)

                return route, args, kwargs
        except exc.HTTPMethodNotAllowed:
            method_not_allowed = True

    if method_not_allowed:
        raise exc.HTTPMethodNotAllowed()
