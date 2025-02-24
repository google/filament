# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Module containing utilities for apk packages."""

import contextlib
import logging
import os
import re
import shutil
import tempfile
import zipfile

import six

from devil import base_error
from devil.android.ndk import abis
from devil.android.sdk import aapt
from devil.android.sdk import bundletool
from devil.android.sdk import split_select
from devil.utils import cmd_helper

_logger = logging.getLogger(__name__)

_MANIFEST_ATTRIBUTE_RE = re.compile(r'\s*A: ([^\(\)= ]*)(?:\([^\(\)= ]*\))?='
                                    r'(?:"(.*)" \(Raw: .*\)|\(type.*?\)(.*))$')
_MANIFEST_ELEMENT_RE = re.compile(r'\s*(?:E|N): (\S*) .*$')
_BASE_APK_APKS_RE = re.compile(
    r'^splits/base-master.*\.apk$|^standalones/standalone.*\.apex$')


class ApkHelperError(base_error.BaseError):
  """Exception for APK helper failures."""

  def __init__(self, message):
    super(ApkHelperError, self).__init__(message)


@contextlib.contextmanager
def _DeleteHelper(files, to_delete):
  """Context manager that returns |files| and deletes |to_delete| on exit."""
  try:
    yield files
  finally:
    paths = to_delete if isinstance(to_delete, list) else [to_delete]
    for path in paths:
      if os.path.isfile(path):
        os.remove(path)
      elif os.path.isdir(path):
        shutil.rmtree(path)
      else:
        raise ApkHelperError('Cannot delete %s' % path)


@contextlib.contextmanager
def _NoopFileHelper(files):
  """Context manager that returns |files|."""
  yield files


def GetPackageName(apk_path):
  """Returns the package name of the apk."""
  return ToHelper(apk_path).GetPackageName()


# TODO(jbudorick): Deprecate and remove this function once callers have been
# converted to ApkHelper.GetInstrumentationName
def GetInstrumentationName(apk_path):
  """Returns the name of the Instrumentation in the apk."""
  return ToHelper(apk_path).GetInstrumentationName()


def ToHelper(path_or_helper):
  """Creates an ApkHelper unless one is already given."""
  if not isinstance(path_or_helper, six.string_types):
    return path_or_helper
  if path_or_helper.endswith('.apk'):
    return ApkHelper(path_or_helper)
  if path_or_helper.endswith('.apks'):
    return ApksHelper(path_or_helper)
  if path_or_helper.endswith('.apex'):
    return ApexHelper(path_or_helper)
  if path_or_helper.endswith('_bundle'):
    return BundleScriptHelper(path_or_helper)

  raise ApkHelperError('Unrecognized APK format %s' % path_or_helper)


def ToIncrementalHelper(path_or_helper):
  """Creates an IncrementalApkHelper unless one is already given.

  Currently supports IncrementalApkHelper and ApkHelper instances as well as
  string paths ending in .apk.
  """
  if isinstance(path_or_helper, IncrementalApkHelper):
    return path_or_helper
  if isinstance(path_or_helper, ApkHelper):
    return IncrementalApkHelper(path_or_helper.path)
  if (isinstance(path_or_helper, six.string_types)
      and path_or_helper.endswith('.apk')):
    return IncrementalApkHelper(path_or_helper)

  raise ApkHelperError('Unrecognized Incremental APK format %s' %
                       path_or_helper)


def ToSplitHelper(path_or_helper, split_apks):
  if isinstance(path_or_helper, SplitApkHelper):
    if sorted(path_or_helper.split_apk_paths) != sorted(split_apks):
      raise ApkHelperError('Helper has different split APKs')
    return path_or_helper
  if (isinstance(path_or_helper, six.string_types)
      and path_or_helper.endswith('.apk')):
    return SplitApkHelper(path_or_helper, split_apks)

  raise ApkHelperError(
      'Unrecognized APK format %s, %s' % (path_or_helper, split_apks))


# To parse the manifest, the function uses a node stack where at each level of
# the stack it keeps the currently in focus node at that level (of indentation
# in the xmltree output, ie. depth in the tree). The height of the stack is
# determinded by line indentation. When indentation is increased so is the stack
# (by pushing a new empty node on to the stack). When indentation is decreased
# the top of the stack is popped (sometimes multiple times, until indentation
# matches the height of the stack). Each line parsed (either an attribute or an
# element) is added to the node at the top of the stack (after the stack has
# been popped/pushed due to indentation).
def _ParseManifestFromApk(apk_path):
  aapt_output = aapt.Dump('xmltree', apk_path, 'AndroidManifest.xml')
  parsed_manifest = {}
  node_stack = [parsed_manifest]
  indent = '  '

  if aapt_output[0].startswith('N'):
    # if the first line is a namespace then the root manifest is indented, and
    # we need to add a dummy namespace node, then skip the first line (we dont
    # care about namespaces).
    node_stack.insert(0, {})
    output_to_parse = aapt_output[1:]
  else:
    output_to_parse = aapt_output

  for line in output_to_parse:
    if len(line) == 0:
      continue

    # If namespaces are stripped, aapt still outputs the full url to the
    # namespace and appends it to the attribute names.
    line = line.replace('http://schemas.android.com/apk/res/android:',
                        'android:')

    indent_depth = 0
    while line[(len(indent) * indent_depth):].startswith(indent):
      indent_depth += 1

    # Pop the stack until the height of the stack is the same is the depth of
    # the current line within the tree.
    node_stack = node_stack[:indent_depth + 1]
    node = node_stack[-1]

    # Element nodes are a list of python dicts while attributes are just a dict.
    # This is because multiple elements, at the same depth of tree and the same
    # name, are all added to the same list keyed under the element name.
    m = _MANIFEST_ELEMENT_RE.match(line[len(indent) * indent_depth:])
    if m:
      manifest_key = m.group(1)
      if manifest_key in node:
        node[manifest_key] += [{}]
      else:
        node[manifest_key] = [{}]
      node_stack += [node[manifest_key][-1]]
      continue

    m = _MANIFEST_ATTRIBUTE_RE.match(line[len(indent) * indent_depth:])
    if m:
      manifest_key = m.group(1)
      if manifest_key in node:
        raise ApkHelperError(
            "A single attribute should have one key and one value: {}".format(
                line))
      node[manifest_key] = m.group(2) or m.group(3)
      continue

  return parsed_manifest


def _ParseNumericKey(obj, key, default=0):
  val = obj.get(key)
  if val is None:
    return default
  return int(val, 0)


def _SplitLocaleString(locale):
  split_locale = locale.split('-')
  if len(split_locale) != 2:
    raise ApkHelperError('Locale has incorrect format: {}'.format(locale))
  return tuple(split_locale)


class _ExportedActivity(object):
  def __init__(self, name):
    self.name = name
    self.actions = set()
    self.categories = set()
    self.schemes = set()


def _IterateExportedActivities(manifest_info):
  app_node = manifest_info['manifest'][0]['application'][0]
  activities = app_node.get('activity', []) + app_node.get('activity-alias', [])
  for activity_node in activities:
    # Presence of intent filters make an activity exported by default.
    has_intent_filter = 'intent-filter' in activity_node
    if not _ParseNumericKey(
        activity_node, 'android:exported', default=has_intent_filter):
      continue

    activity = _ExportedActivity(activity_node.get('android:name'))
    # Merge all intent-filters into a single set because there is not
    # currently a need to keep them separate.
    for intent_filter in activity_node.get('intent-filter', []):
      for action in intent_filter.get('action', []):
        activity.actions.add(action.get('android:name'))
      for category in intent_filter.get('category', []):
        activity.categories.add(category.get('android:name'))
      for data in intent_filter.get('data', []):
        activity.schemes.add(data.get('android:scheme'))
    yield activity


class BaseApkHelper(object):
  """Abstract base class representing an installable Android app."""

  def __init__(self):
    self._manifest = None

  @property
  def path(self):
    raise NotImplementedError()

  def __repr__(self):
    return '%s(%s)' % (self.__class__.__name__, self.path)

  def _GetBaseApkPath(self):
    """Returns context manager providing path to this app's base APK.

    Must be implemented by subclasses.
    """
    raise NotImplementedError()

  def GetActivityName(self):
    """Returns the name of the first launcher Activity in the apk."""
    manifest_info = self._GetManifest()
    for activity in _IterateExportedActivities(manifest_info):
      if ('android.intent.action.MAIN' in activity.actions
          and 'android.intent.category.LAUNCHER' in activity.categories):
        return self._ResolveName(activity.name)
    return None

  def GetViewActivityName(self):
    """Returns name of the first action=View Activity that can handle http."""
    manifest_info = self._GetManifest()
    for activity in _IterateExportedActivities(manifest_info):
      if ('android.intent.action.VIEW' in activity.actions
          and 'http' in activity.schemes):
        return self._ResolveName(activity.name)
    return None

  def GetInstrumentationName(self,
                             default='android.test.InstrumentationTestRunner'):
    """Returns the name of the Instrumentation in the apk."""
    all_instrumentations = self.GetAllInstrumentations(default=default)
    if len(all_instrumentations) != 1:
      raise ApkHelperError(
          'There is more than one instrumentation. Expected one.')
    return self._ResolveName(all_instrumentations[0]['android:name'])

  def GetAllInstrumentations(self,
                             default='android.test.InstrumentationTestRunner'):
    """Returns a list of all Instrumentations in the apk."""
    try:
      return self._GetManifest()['manifest'][0]['instrumentation']
    except KeyError:
      return [{'android:name': default}]

  def GetPackageName(self):
    """Returns the package name of the apk."""
    manifest_info = self._GetManifest()
    try:
      return manifest_info['manifest'][0]['package']
    except KeyError:
      raise ApkHelperError('Failed to determine package name of %s' % self.path)

  def GetPermissions(self):
    manifest_info = self._GetManifest()
    try:
      return [
          p['android:name']
          for p in manifest_info['manifest'][0]['uses-permission']
      ]
    except KeyError:
      return []

  def GetSplitName(self):
    """Returns the name of the split of the apk."""
    manifest_info = self._GetManifest()
    try:
      return manifest_info['manifest'][0]['split']
    except KeyError:
      return None

  def HasIsolatedProcesses(self):
    """Returns whether any services exist that use isolatedProcess=true."""
    manifest_info = self._GetManifest()
    try:
      application = manifest_info['manifest'][0]['application'][0]
      services = application['service']
      return any(
          _ParseNumericKey(s, 'android:isolatedProcess') for s in services)
    except KeyError:
      return False

  def GetAllMetadata(self):
    """Returns a list meta-data tags as (name, value) tuples."""
    manifest_info = self._GetManifest()
    try:
      application = manifest_info['manifest'][0]['application'][0]
      metadata = application['meta-data']
      return [(x.get('android:name'), x.get('android:value')) for x in metadata]
    except KeyError:
      return []

  def GetLibraryVersion(self):
    """Returns the version of the <static-library> or None if not applicable."""
    manifest_info = self._GetManifest()
    try:
      application = manifest_info['manifest'][0]['application'][0]
      return int(application['static-library'][0]['android:version'], 16)
    except KeyError:
      return None

  def Get32BitAbiOverride(self):
    """Returns the value of android:use32bitAbi or None if not available."""
    manifest_info = self._GetManifest()
    try:
      application = manifest_info['manifest'][0]['application'][0]
      return application.get('android:use32bitAbi')
    except KeyError:
      return None

  def GetVersionCode(self):
    """Returns the versionCode as an integer, or None if not available."""
    manifest_info = self._GetManifest()
    try:
      version_code = manifest_info['manifest'][0]['android:versionCode']
      return int(version_code, 16)
    except KeyError:
      return None

  def GetVersionName(self):
    """Returns the versionName as a string."""
    manifest_info = self._GetManifest()
    try:
      version_name = manifest_info['manifest'][0]['android:versionName']
      return version_name
    except KeyError:
      return ''

  def GetMinSdkVersion(self):
    """Returns the minSdkVersion as a string, or None if not available.

    Note: this cannot always be cast to an integer."""
    manifest_info = self._GetManifest()
    try:
      uses_sdk = manifest_info['manifest'][0]['uses-sdk'][0]
      min_sdk_version = uses_sdk['android:minSdkVersion']
      try:
        # The common case is for this to be an integer. Convert to decimal
        # notation (rather than hexadecimal) for readability, but convert back
        # to a string for type consistency with the general case.
        return str(int(min_sdk_version, 16))
      except ValueError:
        # In general (ex. apps with minSdkVersion set to pre-release Android
        # versions), minSdkVersion can be a string (usually, the OS codename
        # letter). For simplicity, don't do any validation on the value.
        return min_sdk_version
    except KeyError:
      return None

  def GetTargetSdkVersion(self):
    """Returns the targetSdkVersion as a string, or None if not available.

    Note: this cannot always be cast to an integer. If this application targets
    a pre-release SDK, this returns the SDK codename instead (ex. "R").
    """
    manifest_info = self._GetManifest()
    try:
      uses_sdk = manifest_info['manifest'][0]['uses-sdk'][0]
      target_sdk_version = uses_sdk['android:targetSdkVersion']
      try:
        # The common case is for this to be an integer. Convert to decimal
        # notation (rather than hexadecimal) for readability, but convert back
        # to a string for type consistency with the general case.
        return str(int(target_sdk_version, 16))
      except ValueError:
        # In general (ex. apps targeting pre-release Android versions),
        # targetSdkVersion can be a string (usually, the OS codename letter).
        # For simplicity, don't do any validation on the value.
        return target_sdk_version
    except KeyError:
      return None

  def _GetManifest(self):
    if not self._manifest:
      with self._GetBaseApkPath() as base_apk_path:
        self._manifest = _ParseManifestFromApk(base_apk_path)
    return self._manifest

  def _ResolveName(self, name):
    name = name.lstrip('.')
    if '.' not in name:
      return '%s.%s' % (self.GetPackageName(), name)
    return name

  def _ListApkPaths(self):
    with self._GetBaseApkPath() as base_apk_path:
      with zipfile.ZipFile(base_apk_path) as z:
        return z.namelist()

  def GetAbis(self):
    """Returns a list of ABIs in the apk (empty list if no native code)."""
    # Use lib/* to determine the compatible ABIs.
    libs = set()
    for path in self._ListApkPaths():
      path_tokens = path.split('/')
      if len(path_tokens) >= 2 and path_tokens[0] == 'lib':
        libs.add(path_tokens[1])
    lib_to_abi = {
        abis.ARM: [abis.ARM, abis.ARM_64],
        abis.ARM_64: [abis.ARM_64],
        abis.X86: [abis.X86, abis.X86_64],
        abis.X86_64: [abis.X86_64]
    }
    try:
      output = set()
      for lib in libs:
        for abi in lib_to_abi[lib]:
          output.add(abi)
      return sorted(output)
    except KeyError:
      raise ApkHelperError('Unexpected ABI in lib/* folder.')

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    """Returns context manager providing list of split APK paths for |device|.

    The paths may be deleted when the context manager exits. Must be implemented
    by subclasses.

    args:
      device: The device for which to return split APKs.
      modules: Extra feature modules to install.
      allow_cached_props: Allow using cache when querying propery values from
        |device|.
    """
    # pylint: disable=unused-argument
    raise NotImplementedError()

  @staticmethod
  def SupportsSplits():
    return False


class ApkHelper(BaseApkHelper):
  """Represents a single APK Android app."""

  def __init__(self, apk_path):
    super(ApkHelper, self).__init__()
    self._apk_path = apk_path

  @property
  def path(self):
    return self._apk_path

  def _GetBaseApkPath(self):
    return _NoopFileHelper(self._apk_path)

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    if modules:
      raise ApkHelperError('Cannot install modules when installing single APK')
    return _NoopFileHelper([self._apk_path])


class ApexHelper(BaseApkHelper):
  """Represents a single APEX mainline module."""

  def __init__(self, apex_path):
    super(ApexHelper, self).__init__()
    self._apex_path = apex_path

  @property
  def path(self):
    return self._apex_path

  def _GetBaseApkPath(self):
    return _NoopFileHelper(self._apex_path)

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    if modules:
      raise ApkHelperError('Cannot install modules when installing an APEX')
    return _NoopFileHelper([self._apex_path])


class IncrementalApkHelper(ApkHelper):
  """Extends ApkHelper for incremental install."""

  def __init__(self, apk_path):
    super(IncrementalApkHelper, self).__init__(apk_path)
    self._extra_apk_paths = []

  def SetExtraApkPaths(self, extra_apk_paths):
    self._extra_apk_paths = extra_apk_paths

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    if modules:
      raise ApkHelperError('Cannot install modules when installing single APK')
    return _NoopFileHelper([self._apk_path] + self._extra_apk_paths)


class SplitApkHelper(BaseApkHelper):
  """Represents a multi APK Android app."""

  def __init__(self, base_apk_path, split_apk_paths):
    super(SplitApkHelper, self).__init__()
    self._base_apk_path = base_apk_path
    self._split_apk_paths = split_apk_paths

  @property
  def path(self):
    return self._base_apk_path

  @property
  def split_apk_paths(self):
    return self._split_apk_paths

  def __repr__(self):
    return '%s(%s, %s)' % (self.__class__.__name__, self.path,
                           self.split_apk_paths)

  def _GetBaseApkPath(self):
    return _NoopFileHelper(self._base_apk_path)

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    if modules:
      raise ApkHelperError('Cannot install modules when installing single APK')
    splits = split_select.SelectSplits(
        device,
        self.path,
        self.split_apk_paths,
        allow_cached_props=allow_cached_props)
    if len(splits) == 1:
      _logger.warning('split-select did not select any from %s', splits)
    return _NoopFileHelper([self._base_apk_path] + splits)

  #override
  @staticmethod
  def SupportsSplits():
    return True


class BaseBundleHelper(BaseApkHelper):
  """Abstract base class representing an Android app bundle."""

  def _GetApksPath(self):
    """Returns context manager providing path to the bundle's APKS archive.

    Must be implemented by subclasses.
    """
    raise NotImplementedError()

  def _GetBaseApkPath(self):
    try:
      base_apk_path = tempfile.mkdtemp()
      with self._GetApksPath() as apks_path:
        with zipfile.ZipFile(apks_path) as z:
          base_apks = [s for s in z.namelist() if _BASE_APK_APKS_RE.match(s)]
          if len(base_apks) < 1:
            raise ApkHelperError('Cannot find base APK in %s' % self.path)
          z.extract(base_apks[0], base_apk_path)
          return _DeleteHelper(
              os.path.join(base_apk_path, base_apks[0]), base_apk_path)
    except:
      shutil.rmtree(base_apk_path)
      raise

  def GetApkPaths(self,
                  device,
                  modules=None,
                  allow_cached_props=False,
                  additional_locales=None):
    locales = [device.GetLocale()]
    if additional_locales:
      locales.extend(_SplitLocaleString(l) for l in additional_locales)
    with self._GetApksPath() as apks_path:
      try:
        split_dir = tempfile.mkdtemp()
        # TODO(tiborg): Support all locales.
        bundletool.ExtractApks(split_dir, apks_path,
                               device.product_cpu_abis, locales,
                               device.GetFeatures(), device.pixel_density,
                               device.build_version_sdk, modules)
        splits = [os.path.join(split_dir, p) for p in os.listdir(split_dir)]
        return _DeleteHelper(splits, split_dir)
      except:
        shutil.rmtree(split_dir)
        raise

  #override
  @staticmethod
  def SupportsSplits():
    return True


class ApksHelper(BaseBundleHelper):
  """Represents a bundle's APKS archive."""

  def __init__(self, apks_path):
    super(ApksHelper, self).__init__()
    self._apks_path = apks_path

  @property
  def path(self):
    return self._apks_path

  def _GetApksPath(self):
    return _NoopFileHelper(self._apks_path)


class BundleScriptHelper(BaseBundleHelper):
  """Represents a bundle install script."""

  def __init__(self, bundle_script_path):
    super(BundleScriptHelper, self).__init__()
    self._bundle_script_path = bundle_script_path

  @property
  def path(self):
    return self._bundle_script_path

  def _GetApksPath(self):
    apks_path = None
    try:
      fd, apks_path = tempfile.mkstemp(suffix='.apks')
      os.close(fd)
      cmd = [
          self._bundle_script_path,
          'build-bundle-apks',
          '--output-apks',
          apks_path,
      ]
      status, stdout, stderr = cmd_helper.GetCmdStatusOutputAndError(cmd)
      if status != 0:
        raise ApkHelperError('Failed running {} with output\n{}\n{}'.format(
            ' '.join(cmd), stdout, stderr))
      return _DeleteHelper(apks_path, apks_path)
    except:
      if apks_path:
        os.remove(apks_path)
      raise
