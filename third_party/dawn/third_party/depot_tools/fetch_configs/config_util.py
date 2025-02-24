# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This module holds utilities which make writing configs easier."""

import json


class Config(object):
    """Base class for all configs.

    Provides methods that are expected to be overridden by child classes. Also
    provides an command-line parsing method that converts the unified
    command-line interface used in depot_tools to the unified python interface
    defined here.
    """
    @staticmethod
    def fetch_spec(_props):
        """Returns instructions to check out the project based on |props|."""
        raise NotImplementedError

    @staticmethod
    def expected_root(_props):
        """Returns the directory into which the checkout will be performed."""
        raise NotImplementedError

    def handle_args(self, argv):
        """Passes command-line arguments through to the appropriate method."""
        methods = {'fetch': self.fetch_spec, 'root': self.expected_root}
        if len(argv) <= 1 or argv[1] not in methods:
            print('Must specify a a fetch/root action')
            return 1

        def looks_like_arg(arg):
            return arg.startswith('--') and arg.count('=') == 1

        bad_parms = [x for x in argv[2:] if not looks_like_arg(x)]
        if bad_parms:
            print('Got bad arguments %s' % bad_parms)
            return 1

        method = methods[argv[1]]
        props = dict(x.split('=', 1) for x in (y.lstrip('-') for y in argv[2:]))

        self.output(method(props))

    @staticmethod
    def output(data):
        print(json.dumps(data))
