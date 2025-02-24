# -*- coding: utf-8 -*-
__doc__ = 'webtest.ext is now in a separate package name webtest-casperjs'


def casperjs(*args, **kwargs):
    raise ImportError(__doc__)
