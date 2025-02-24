# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import uuid


class AbspathInvalidError(Exception):
  """Raised if an abspath cannot be sanitized based on an app's source paths."""


class UserFriendlyStringInvalidError(Exception):
  """Raised if a user friendly string cannot be parsed."""


class ModuleToLoad(object):

  def __init__(self, href=None, filename=None):
    if bool(href) == bool(filename):
      raise Exception('ModuleToLoad must specify exactly one of href and '
                      'filename.')
    self.href = href
    self.filename = filename

  def __repr__(self):
    if self.href:
      return 'ModuleToLoad(href="%s")' % self.href
    return 'ModuleToLoad(filename="%s")' % self.filename

  def AsDict(self):
    if self.href:
      return {'href': self.href}
    return {'filename': self.filename}

  @staticmethod
  def FromDict(module_dict):
    return ModuleToLoad(module_dict.get('href'), module_dict.get('filename'))


class FunctionHandle(object):

  def __init__(self, modules_to_load=None, function_name=None,
               options=None, guid=uuid.uuid4()):
    self.modules_to_load = modules_to_load
    self.function_name = function_name
    self.options = options
    self._guid = guid

  def __repr__(self):
    return 'FunctionHandle(modules_to_load=[%s], function_name="%s")' % (
        ', '.join([str(module) for module in self.modules_to_load]),
        self.function_name)

  @property
  def guid(self):
    return self._guid

  @property
  def has_hrefs(self):
    return any(module.href for module in self.modules_to_load)

  def AsDict(self):
    handle_dict = {
        'function_name': self.function_name
    }

    if self.modules_to_load is not None:
      handle_dict['modules_to_load'] = [module.AsDict() for module in
                                        self.modules_to_load]
    if self.options is not None:
      handle_dict['options'] = self.options

    return handle_dict

  def ConvertHrefsToAbsFilenames(self, app):
    """Converts hrefs to absolute filenames in the context of |app|.

    In an app-serving context, functions must only reside in files which the app
    is serving, in order to prevent directory traversal attacks. In addition, we
    rely on paths being absolute when actually executing functions.

    Args:
      app: A dev server instance requesting abspath conversion.

    Returns:
      A new FunctionHandle instance with no hrefs.

    Raises:
      AbspathInvalidError: If there is no source path with which a given abspath
          shares a common prefix.
    """
    new_modules_to_load = []
    for module in self.modules_to_load:
      if module.href:
        abspath = app.GetAbsFilenameForHref(module.href)
      else:
        assert os.path.abspath(module.filename) == module.filename
        abspath = module.filename

      if not abspath:
        raise AbspathInvalidError('Filename %s invalid' % abspath)

      new_modules_to_load.append(ModuleToLoad(filename=abspath))

    return FunctionHandle(modules_to_load=new_modules_to_load,
                          function_name=self.function_name)

  @staticmethod
  def FromDict(handle_dict):
    if handle_dict.get('modules_to_load') is not None:
      modules_to_load = [ModuleToLoad.FromDict(module_dict) for module_dict in
                         handle_dict['modules_to_load']]
    else:
      modules_to_load = []
    options = handle_dict.get('options')
    return FunctionHandle(modules_to_load=modules_to_load,
                          function_name=handle_dict['function_name'],
                          options=options)

  def AsUserFriendlyString(self, app):
    parts = [module.filename for module in
             self.ConvertHrefsToAbsFilenames(app).modules_to_load]
    parts.append(self.function_name)

    return ':'.join(parts)

  @staticmethod
  def FromUserFriendlyString(user_str):
    parts = user_str.split(':')
    if len(parts) < 2:
      raise UserFriendlyStringInvalidError(
          'Tried to deserialize string with less than two parts: ' + user_str)

    modules_to_load = [ModuleToLoad(filename=name) for name in parts[:-1]]

    return FunctionHandle(modules_to_load=modules_to_load,
                          function_name=parts[-1])
