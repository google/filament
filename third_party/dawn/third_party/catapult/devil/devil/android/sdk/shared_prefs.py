# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Helper object to read and modify Shared Preferences from Android apps.

See e.g.:
  http://developer.android.com/reference/android/content/SharedPreferences.html
"""

import logging
import posixpath
from xml.etree import ElementTree

import six

from devil.android import device_errors
from devil.android.sdk import version_codes

logger = logging.getLogger(__name__)

_XML_DECLARATION = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\n"


class BasePref(object):
  """Base class for getting/setting the value of a specific preference type.

  Should not be instantiated directly. The SharedPrefs collection will
  instantiate the appropriate subclasses, which directly manipulate the
  underlying xml document, to parse and serialize values according to their
  type.

  Args:
    elem: An xml ElementTree object holding the preference data.

  Properties:
    tag_name: A string with the tag that must be used for this preference type.
  """
  tag_name = None

  def __init__(self, elem):
    if elem.tag != type(self).tag_name:
      raise TypeError('Property %r has type %r, but trying to access as %r' %
                      (elem.get('name'), elem.tag, type(self).tag_name))
    self._elem = elem

  def __str__(self):
    """Get the underlying xml element as a string."""
    if six.PY2:
      return ElementTree.tostring(self._elem)

    return ElementTree.tostring(self._elem, encoding='unicode')

  def get(self):
    """Get the value of this preference."""
    return self._elem.get('value')

  def set(self, value):
    """Set from a value casted as a string."""
    self._elem.set('value', str(value))

  @property
  def has_value(self):
    """Check whether the element has a value."""
    return self._elem.get('value') is not None


class BooleanPref(BasePref):
  """Class for getting/setting a preference with a boolean value.

  The underlying xml element has the form, e.g.:
      <boolean name="featureEnabled" value="false" />
  """
  tag_name = 'boolean'
  VALUES = {'true': True, 'false': False}

  def get(self):
    """Get the value as a Python bool."""
    return type(self).VALUES[super(BooleanPref, self).get()]

  def set(self, value):
    """Set from a value casted as a bool."""
    super(BooleanPref, self).set('true' if value else 'false')


class FloatPref(BasePref):
  """Class for getting/setting a preference with a float value.

  The underlying xml element has the form, e.g.:
      <float name="someMetric" value="4.7" />
  """
  tag_name = 'float'

  def get(self):
    """Get the value as a Python float."""
    return float(super(FloatPref, self).get())


class IntPref(BasePref):
  """Class for getting/setting a preference with an int value.

  The underlying xml element has the form, e.g.:
      <int name="aCounter" value="1234" />
  """
  tag_name = 'int'

  def get(self):
    """Get the value as a Python int."""
    return int(super(IntPref, self).get())


class LongPref(IntPref):
  """Class for getting/setting a preference with a long value.

  The underlying xml element has the form, e.g.:
      <long name="aLongCounter" value="1234" />

  We use the same implementation from IntPref.
  """
  tag_name = 'long'


class StringPref(BasePref):
  """Class for getting/setting a preference with a string value.

  The underlying xml element has the form, e.g.:
      <string name="someHashValue">249b3e5af13d4db2</string>
  """
  tag_name = 'string'

  def get(self):
    """Get the value as a Python string."""
    return self._elem.text

  def set(self, value):
    """Set from a value casted as a string."""
    self._elem.text = str(value)


class StringSetPref(StringPref):
  """Class for getting/setting a preference with a set of string values.

  The underlying xml element has the form, e.g.:
      <set name="managed_apps">
          <string>com.mine.app1</string>
          <string>com.mine.app2</string>
          <string>com.mine.app3</string>
      </set>
  """
  tag_name = 'set'

  def get(self):
    """Get a list with the string values contained."""
    value = []
    for child in self._elem:
      assert child.tag == 'string'
      value.append(child.text)
    return value

  def set(self, value):
    """Set from a sequence of values, each casted as a string."""
    for child in list(self._elem):
      self._elem.remove(child)
    for item in value:
      ElementTree.SubElement(self._elem, 'string').text = str(item)


_PREF_TYPES = {
    c.tag_name: c
    for c in
    [BooleanPref, FloatPref, IntPref, LongPref, StringPref, StringSetPref]
}


class SharedPrefs(object):
  def __init__(self, device, package, filename, use_encrypted_path=False):
    """Helper object to read and update "Shared Prefs" of Android apps.

    Such files typically look like, e.g.:

        <?xml version='1.0' encoding='utf-8' standalone='yes' ?>
        <map>
          <int name="databaseVersion" value="107" />
          <boolean name="featureEnabled" value="false" />
          <string name="someHashValue">249b3e5af13d4db2</string>
        </map>

    Example usage:

        prefs = shared_prefs.SharedPrefs(device, 'com.my.app', 'my_prefs.xml')
        prefs.Load()
        prefs.GetString('someHashValue') # => '249b3e5af13d4db2'
        prefs.SetInt('databaseVersion', 42)
        prefs.Remove('featureEnabled')
        prefs.Commit()

    The object may also be used as a context manager to automatically load and
    commit, respectively, upon entering and leaving the context.

    Args:
      device: A DeviceUtils object.
      package: A string with the package name of the app that owns the shared
        preferences file.
      filename: A string with the name of the preferences file to read/write.
      use_encrypted_path: Whether to read and write to the shared prefs location
        in the device-encrypted path (/data/user_de) instead of the older,
        unencrypted path (/data/data). Only supported on N+, but falls back to
        the unencrypted path if the encrypted path is not supported on the given
        device.
    """
    self._device = device
    self._xml = None
    self._package = package
    self._filename = filename
    self._unencrypted_path = '/data/data/%s/shared_prefs/%s' % (package,
                                                                filename)
    self._encrypted_path = '/data/user_de/0/%s/shared_prefs/%s' % (package,
                                                                   filename)
    self._path = self._unencrypted_path
    self._encrypted = use_encrypted_path
    if use_encrypted_path:
      if self._device.build_version_sdk < version_codes.NOUGAT:
        logging.info('SharedPrefs set to use encrypted path, but given device '
                     'is not running N+. Falling back to unencrypted path')
        self._encrypted = False
      else:
        self._path = self._encrypted_path
    self._changed = False

  def __repr__(self):
    """Get a useful printable representation of the object."""
    return '<{cls} file {filename} for {package} on {device}>'.format(
        cls=type(self).__name__,
        filename=self.filename,
        package=self.package,
        device=str(self._device))

  def __str__(self):
    """Get the underlying xml document as a string."""
    if six.PY2:
      return _XML_DECLARATION + ElementTree.tostring(self.xml)

    return _XML_DECLARATION + \
        ElementTree.tostring(self.xml, encoding='unicode')

  @property
  def package(self):
    """Get the package name of the app that owns the shared preferences."""
    return self._package

  @property
  def filename(self):
    """Get the filename of the shared preferences file."""
    return self._filename

  @property
  def path(self):
    """Get the full path to the shared preferences file on the device."""
    return self._path

  @property
  def changed(self):
    """True if properties have changed and a commit would be needed."""
    return self._changed

  @property
  def xml(self):
    """Get the underlying xml document as an ElementTree object."""
    if self._xml is None:
      self._xml = ElementTree.Element('map')
    return self._xml

  def Load(self):
    """Load the shared preferences file from the device.

    A empty xml document, which may be modified and saved on |commit|, is
    created if the file does not already exist.
    """
    if self._device.FileExists(self.path):
      self._xml = ElementTree.fromstring(
          self._device.ReadFile(self.path, as_root=True))
      assert self._xml.tag == 'map'
    else:
      self._xml = None
    self._changed = False

  def Clear(self):
    """Clear all of the preferences contained in this object."""
    if self._xml is not None and len(self):  # only clear if not already empty
      self._xml = None
      self._changed = True

  def Commit(self, force_commit=False):
    """Save the current set of preferences to the device.

    Only actually saves if some preferences have been modified or force_commit
    is set to True.

    Args:
      force_commit: Commit even if no changes have been made to the SharedPrefs
        instance.
    """
    if not (self.changed or force_commit):
      return
    self._device.RunShellCommand(
        ['mkdir', '-p', posixpath.dirname(self.path)],
        as_root=True,
        check_return=True)
    self._device.WriteFile(self.path, str(self), as_root=True)
    # Creating the directory/file can cause issues with SELinux if they did
    # not already exist. As a workaround, apply the package's security context
    # to the shared_prefs directory, which mimics the behavior of a file
    # created by the app itself
    if self._device.build_version_sdk >= version_codes.MARSHMALLOW:
      security_context = self._device.GetSecurityContextForPackage(
          self.package, encrypted=self._encrypted)
      if security_context is None:
        raise device_errors.CommandFailedError(
            'Failed to get security context for %s' % self.package)
      paths = [posixpath.dirname(self.path), self.path]
      self._device.ChangeSecurityContext(security_context, paths)

    # Ensure that there isn't both an encrypted and unencrypted version of the
    # file on the device at the same time.
    if self._device.build_version_sdk >= version_codes.NOUGAT:
      remove_path = (self._unencrypted_path
                     if self._encrypted else self._encrypted_path)
      if self._device.PathExists(remove_path, as_root=True):
        logging.warning('Found an equivalent shared prefs file at %s, removing',
                        remove_path)
        self._device.RemovePath(remove_path, as_root=True)

    self._device.KillAll(self.package, exact=True, as_root=True, quiet=True)
    self._changed = False

  def __len__(self):
    """Get the number of preferences in this collection."""
    return len(self.xml)

  def PropertyType(self, key):
    """Get the type (i.e. tag name) of a property in the collection."""
    return self._GetChild(key).tag

  def HasProperty(self, key):
    try:
      self._GetChild(key)
      return True
    except KeyError:
      return False

  def GetBoolean(self, key):
    """Get a boolean property."""
    return BooleanPref(self._GetChild(key)).get()

  def SetBoolean(self, key, value):
    """Set a boolean property."""
    self._SetPrefValue(key, value, BooleanPref)

  def GetFloat(self, key):
    """Get a float property."""
    return FloatPref(self._GetChild(key)).get()

  def SetFloat(self, key, value):
    """Set a float property."""
    self._SetPrefValue(key, value, FloatPref)

  def GetInt(self, key):
    """Get an int property."""
    return IntPref(self._GetChild(key)).get()

  def SetInt(self, key, value):
    """Set an int property."""
    self._SetPrefValue(key, value, IntPref)

  def GetLong(self, key):
    """Get a long property."""
    return LongPref(self._GetChild(key)).get()

  def SetLong(self, key, value):
    """Set a long property."""
    self._SetPrefValue(key, value, LongPref)

  def GetString(self, key):
    """Get a string property."""
    return StringPref(self._GetChild(key)).get()

  def SetString(self, key, value):
    """Set a string property."""
    self._SetPrefValue(key, value, StringPref)

  def GetStringSet(self, key):
    """Get a string set property."""
    return StringSetPref(self._GetChild(key)).get()

  def SetStringSet(self, key, value):
    """Set a string set property."""
    self._SetPrefValue(key, value, StringSetPref)

  def Remove(self, key):
    """Remove a preference from the collection."""
    self.xml.remove(self._GetChild(key))

  def AsDict(self):
    """Return the properties and their values as a dictionary."""
    d = {}
    for child in self.xml:
      pref = _PREF_TYPES[child.tag](child)
      d[child.get('name')] = pref.get()
    return d

  def __enter__(self):
    """Load preferences file from the device when entering a context."""
    self.Load()
    return self

  def __exit__(self, exc_type, _exc_value, _traceback):
    """Save preferences file to the device when leaving a context."""
    if not exc_type:
      self.Commit()

  def _GetChild(self, key):
    """Get the underlying xml node that holds the property of a given key.

    Raises:
      KeyError when the key is not found in the collection.
    """
    for child in self.xml:
      if child.get('name') == key:
        return child
    raise KeyError(key)

  def _SetPrefValue(self, key, value, pref_cls):
    """Set the value of a property.

    Args:
      key: The key of the property to set.
      value: The new value of the property.
      pref_cls: A subclass of BasePref used to access the property.

    Raises:
      TypeError when the key already exists but with a different type.
    """
    try:
      pref = pref_cls(self._GetChild(key))
      old_value = pref.get()
    except KeyError:
      pref = pref_cls(
          ElementTree.SubElement(self.xml, pref_cls.tag_name, {'name': key}))
      old_value = None
    if old_value != value:
      pref.set(value)
      self._changed = True
      logger.info('Setting property: %s', pref)
