# -*- coding: utf-8 -*-
import webapp2
from webapp2_extras import local_app

import test_base


class TestLocalApp(test_base.BaseTestCase):
    def test_dispatch(self):
        def hello_handler(request, *args, **kwargs):
            return webapp2.Response('Hello, World!')

        app = local_app.WSGIApplication([('/', hello_handler)])
        rsp = app.get_response('/')
        self.assertEqual(rsp.status_int, 200)
        self.assertEqual(rsp.body, 'Hello, World!')


if __name__ == '__main__':
    test_base.main()
