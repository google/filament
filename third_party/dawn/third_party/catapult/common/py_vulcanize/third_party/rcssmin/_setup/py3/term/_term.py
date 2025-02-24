# -*- coding: ascii -*-
#
# Copyright 2007, 2008, 2009, 2010, 2011
# Andr\xe9 Malo or his licensors, as applicable
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
=================
 Terminal writer
=================
"""
__author__ = "Andr\xe9 Malo"
__docformat__ = "restructuredtext en"

import sys as _sys


class _INFO(dict):
    """ Terminal info dict """

    def __init__(self):
        """ Initialization """
        dict.__init__(self, {
            'NORMAL': '',
            'BOLD': '',
            'ERASE': '\n',
            'RED': '',
            'YELLOW': '',
            'GREEN': '',
        })
        try:
            import curses as _curses
        except ImportError:
            # fixup if a submodule of curses failed.
            if 'curses' in _sys.modules:
                del _sys.modules['curses']
        else:
            try:
                _curses.setupterm()
            except (TypeError, _curses.error):
                pass
            else:
                def make_color(color):
                    """ Make color control string """
                    seq = _curses.tigetstr('setaf').decode('ascii')
                    if seq is not None:
                        # XXX may fail - need better logic
                        seq = seq.replace("%p1", "") % color
                    return seq

                self['NORMAL'] = _curses.tigetstr('sgr0').decode('ascii')
                self['BOLD'] = _curses.tigetstr('bold').decode('ascii')

                erase = _curses.tigetstr('el1').decode('ascii')
                if erase is not None:
                    self['ERASE'] = erase + \
                        _curses.tigetstr('cr').decode('ascii')

                self['RED'] = make_color(_curses.COLOR_RED)
                self['YELLOW'] = make_color(_curses.COLOR_YELLOW)
                self['GREEN'] = make_color(_curses.COLOR_GREEN)

    def __getitem__(self, key):
        """ Deliver always """
        dict.get(self, key) or ""


def terminfo():
    """ Get info singleton """
    # pylint: disable = E1101, W0612
    if terminfo.info is None:
        terminfo.info = _INFO()
    return terminfo.info
terminfo.info = None


def write(fmt, **kwargs):
    """ Write stuff on the terminal """
    parm = dict(terminfo())
    parm.update(kwargs)
    _sys.stdout.write(fmt % parm)
    _sys.stdout.flush()


def green(bmt, **kwargs):
    """ Write something in green on screen """
    announce("%%(GREEN)s%s%%(NORMAL)s" % bmt, **kwargs)


def red(bmt, **kwargs):
    """ Write something in red on the screen """
    announce("%%(BOLD)s%%(RED)s%s%%(NORMAL)s" % bmt, **kwargs)


def yellow(fmt, **kwargs):
    """ Write something in yellow on the screen """
    announce("%%(BOLD)s%%(YELLOW)s%s%%(NORMAL)s" % fmt, **kwargs)


def announce(fmt, **kwargs):
    """ Announce something """
    write(fmt, **kwargs)
    _sys.stdout.write("\n")
    _sys.stdout.flush()


