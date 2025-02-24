# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class SlotsMetaclass(type):
  """This metaclass requires all subclasses to define __slots__.

  Usage:
    class Foo(object):
      __metaclass__ = slots_metaclass.SlotsMetaclass
      __slots__ = '_property0', '_property1',

  __slots__ must be a tuple containing string names of all properties that the
  class contains.
  Defining __slots__ reduces memory usage, accelerates property access, and
  prevents dynamically adding unlisted properties.
  If you need to dynamically add unlisted properties to a class with this
  metaclass, then take a step back and rethink your goals. If you really really
  need to dynamically add unlisted properties to a class with this metaclass,
  add '__dict__' to its __slots__.
  """

  def __new__(cls, name, bases, attrs):
    assert '__slots__' in attrs, 'Class "%s" must define __slots__' % name
    assert isinstance(attrs['__slots__'], tuple), '__slots__ must be a tuple'

    return super(SlotsMetaclass, cls).__new__(cls, name, bases, attrs)
