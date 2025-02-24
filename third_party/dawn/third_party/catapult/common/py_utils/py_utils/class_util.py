# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import inspect

def IsMethodOverridden(parent_cls, child_cls, method_name):
  assert inspect.isclass(parent_cls), '%s should be a class' % parent_cls
  assert inspect.isclass(child_cls), '%s should be a class' % child_cls
  assert parent_cls.__dict__.get(method_name), '%s has no method %s' % (
      parent_cls, method_name)

  if child_cls.__dict__.get(method_name):
    # It's overridden
    return True

  if parent_cls in child_cls.__bases__:
    # The parent is the base class of the child, we did not find the
    # overridden method.
    return False

  # For all the base classes of this class that are not object, check if
  # they override the method.
  base_cls = [cls for cls in child_cls.__bases__ if cls and cls != object]
  return any(
      IsMethodOverridden(parent_cls, base, method_name) for base in base_cls)
