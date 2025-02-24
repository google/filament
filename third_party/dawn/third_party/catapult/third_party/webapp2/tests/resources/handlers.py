import webapp2


class LazyHandler(webapp2.RequestHandler):
    def get(self, **kwargs):
        self.response.out.write('I am a laaazy view.')


class CustomMethodHandler(webapp2.RequestHandler):
    def custom_method(self):
        self.response.out.write('I am a custom method.')


def handle_exception(request, response, exception):
    return webapp2.Response(body='Hello, custom response world!')
