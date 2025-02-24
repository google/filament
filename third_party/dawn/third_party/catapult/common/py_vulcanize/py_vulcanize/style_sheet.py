# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import base64
import os
import re


class Image(object):

  def __init__(self, resource):
    self.resource = resource
    self.aliases = []

  @property
  def relative_path(self):
    return self.resource.relative_path

  @property
  def absolute_path(self):
    return self.resource.absolute_path

  @property
  def contents(self):
    return self.resource.contents


class ParsedStyleSheet(object):

  def __init__(self, loader, containing_dirname, contents):
    self.loader = loader
    self.contents = contents
    self._images = None
    self._Load(containing_dirname)

  @property
  def images(self):
    return self._images

  def AppendDirectlyDependentFilenamesTo(self, dependent_filenames):
    for i in self.images:
      dependent_filenames.append(i.resource.absolute_path)

  @property
  def contents_with_inlined_images(self):
    images_by_url = {}
    for i in self.images:
      for a in i.aliases:
        images_by_url[a] = i

    def InlineUrl(m):
      url = m.group('url')
      image = images_by_url[url]

      ext = os.path.splitext(image.absolute_path)[1]
      data = base64.standard_b64encode(image.contents)

      return 'url(data:image/%s;base64,%s)' % (ext[1:], data.decode('utf-8'))

    # I'm assuming we only have url()'s associated with images
    return re.sub(r'url\((?P<quote>"|\'|)(?P<url>[^"\'()]*)(?P=quote)\)',
                  InlineUrl, self.contents)

  def AppendDirectlyDependentFilenamesTo(self, dependent_filenames):
    for i in self.images:
      dependent_filenames.append(i.resource.absolute_path)

  def _Load(self, containing_dirname):
    if self.contents.find('@import') != -1:
      raise Exception('@imports are not supported')

    matches = re.findall(
        r'url\((?:["|\']?)([^"\'()]*)(?:["|\']?)\)',
        self.contents)

    def resolve_url(url):
      if os.path.isabs(url):
        # FIXME: module is used here, but py_vulcanize.module is never imported.
        # However, py_vulcanize.module cannot be imported since py_vulcanize.module may import
        # style_sheet, leading to an import loop.
        raise module.DepsException('URL references must be relative')
      # URLS are relative to this module's directory
      abs_path = os.path.abspath(os.path.join(containing_dirname, url))
      image = self.loader.LoadImage(abs_path)
      image.aliases.append(url)
      return image

    self._images = [resolve_url(x) for x in matches]


class StyleSheet(object):
  """Represents a stylesheet resource referenced by a module via the
  base.requireStylesheet(xxx) directive."""

  def __init__(self, loader, name, resource):
    self.loader = loader
    self.name = name
    self.resource = resource
    self._parsed_style_sheet = None

  @property
  def filename(self):
    return self.resource.absolute_path

  @property
  def contents(self):
    return self.resource.contents

  def __repr__(self):
    return 'StyleSheet(%s)' % self.name

  @property
  def images(self):
    self._InitParsedStyleSheetIfNeeded()
    return self._parsed_style_sheet.images

  def AppendDirectlyDependentFilenamesTo(self, dependent_filenames):
    self._InitParsedStyleSheetIfNeeded()

    dependent_filenames.append(self.resource.absolute_path)
    self._parsed_style_sheet.AppendDirectlyDependentFilenamesTo(
        dependent_filenames)

  @property
  def contents_with_inlined_images(self):
    self._InitParsedStyleSheetIfNeeded()
    return self._parsed_style_sheet.contents_with_inlined_images

  def load(self):
    self._InitParsedStyleSheetIfNeeded()

  def _InitParsedStyleSheetIfNeeded(self):
    if self._parsed_style_sheet:
      return
    module_dirname = os.path.dirname(self.resource.absolute_path)
    self._parsed_style_sheet = ParsedStyleSheet(
        self.loader, module_dirname, self.contents)
