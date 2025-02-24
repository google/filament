import os

import six
import webob


__all__ = ['DebugApp', 'make_debug_app']


class DebugApp(object):
    """The WSGI application used for testing"""

    def __init__(self, form=None, show_form=False):
        if form and os.path.isfile(form):
            fd = open(form, 'rb')
            self.form = fd.read()
            fd.close()
        else:
            self.form = form
        self.show_form = show_form

    def __call__(self, environ, start_response):
        req = webob.Request(environ)
        if req.path_info == '/form.html' and req.method == 'GET':
            resp = webob.Response(content_type='text/html')
            resp.body = self.form
            return resp(environ, start_response)

        if 'error' in req.GET:
            raise Exception('Exception requested')

        if 'errorlog' in req.GET:
            log = req.GET['errorlog']
            if not six.PY3 and not isinstance(log, six.binary_type):
                log = log.encode('utf8')
            req.environ['wsgi.errors'].write(log)

        status = str(req.GET.get('status', '200 OK'))

        parts = []
        if not self.show_form:
            for name, value in sorted(environ.items()):
                if name.upper() != name:
                    value = repr(value)
                parts.append(str('%s: %s\n') % (name, value))

            body = ''.join(parts)
            if not isinstance(body, six.binary_type):
                body = body.encode('latin1')

            if req.content_length:
                body += six.b('-- Body ----------\n')
                body += req.body
        else:
            body = ''
            for name, value in req.POST.items():
                body += '%s=%s\n' % (name, value)

        if status[:3] in ('204', '304') and not req.content_length:
            body = ''

        headers = [
            ('Content-Type', str('text/plain')),
            ('Content-Length', str(len(body)))]

        if not self.show_form:
            for name, value in req.GET.items():
                if name.startswith('header-'):
                    header_name = name[len('header-'):]
                    if isinstance(header_name, six.text_type):
                        header_name = str(header_name)
                    header_name = header_name.title()
                    headers.append((header_name, str(value)))

        resp = webob.Response()
        resp.status = status
        resp.headers.update(headers)
        if req.method != 'HEAD':
            if isinstance(body, six.text_type):
                resp.body = body.encode('utf8')
            else:
                resp.body = body
        return resp(environ, start_response)


debug_app = DebugApp(form=six.b('''<html><body>
<form action="/form-submit" method="POST">
    <input type="text" name="name">
    <input type="submit" name="submit" value="Submit!">
</form></body></html>'''))


def make_debug_app(global_conf, **local_conf):
    """An application that displays the request environment, and does
    nothing else (useful for debugging and test purposes).
    """
    return DebugApp(**local_conf)
