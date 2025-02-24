# -*- coding: utf-8 -*-
import os

import webapp2
from webapp2_extras import mako

import test_base

current_dir = os.path.abspath(os.path.dirname(__file__))
template_path = os.path.join(current_dir, 'resources', 'mako_templates')


class TestMako(test_base.BaseTestCase):
    def test_render_template(self):
        app = webapp2.WSGIApplication(config={
            'webapp2_extras.mako': {
                'template_path': template_path,
            },
        })
        req = webapp2.Request.blank('/')
        app.set_globals(app=app, request=req)
        m = mako.Mako(app)

        message = 'Hello, World!'
        res = m.render_template( 'template1.html', message=message)
        self.assertEqual(res, message + '\n')

    def test_set_mako(self):
        app = webapp2.WSGIApplication()
        self.assertEqual(len(app.registry), 0)
        mako.set_mako(mako.Mako(app), app=app)
        self.assertEqual(len(app.registry), 1)
        j = mako.get_mako(app=app)
        self.assertTrue(isinstance(j, mako.Mako))

    def test_get_mako(self):
        app = webapp2.WSGIApplication()
        self.assertEqual(len(app.registry), 0)
        j = mako.get_mako(app=app)
        self.assertEqual(len(app.registry), 1)
        self.assertTrue(isinstance(j, mako.Mako))


if __name__ == '__main__':
    test_base.main()
