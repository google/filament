# -*- coding: utf-8 -*-
__doc__ = 'webtest.sel is now in a separate package name webtest-selenium'


class SeleniumApp(object):

    def __init__(self, *args, **kwargs):
        raise ImportError(__doc__)


def selenium(*args, **kwargs):
    raise ImportError(__doc__)
