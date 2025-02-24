# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class GPUDevice():
  """Provides information about an individual GPU device.

     On platforms which support them, the vendor_id and device_id are
     PCI IDs. On other platforms, the vendor_string and device_string
     are platform-dependent strings.
  """

  _VENDOR_ID_MAP = {
      0x1002: 'ATI',
      0x8086: 'Intel',
      0x10de: 'Nvidia',
  }

  def __init__(self, vendor_id, device_id, sub_sys_id, revision, vendor_string,
               device_string, driver_vendor, driver_version):
    """Initialize GPUDevice with GPU and driver information.

    Args:
        vendor_id: The GPU's 16-bit vendor ID. Zero on Android.
        device_id: The GPU's 16-bit devcie ID. Zero on Android.
        sus_sys_id: The GPU's 32-bit sub system ID. Available on Windows.
        revision: The GPU's 16-bit revision. Available on Windows.
        vendor_string: The GPU's vendor name string.
        device_string: The GPU's device name string.
        driver_vendor: The GPU's driver vendor name string.
        driver_version: The GPU's driver version string.
    """
    self._vendor_id = vendor_id
    self._device_id = device_id
    self._sub_sys_id = sub_sys_id
    self._revision = revision
    self._vendor_string = vendor_string
    self._device_string = device_string
    self._driver_vendor = driver_vendor
    self._driver_version = driver_version

  def __str__(self):
    vendor = 'VENDOR = 0x%x' % self._vendor_id
    vendor_string = self._vendor_string
    if not vendor_string and self._vendor_id in self._VENDOR_ID_MAP:
      vendor_string = self._VENDOR_ID_MAP[self._vendor_id]
    if vendor_string:
      vendor += ' (%s)' % vendor_string
    device = 'DEVICE = 0x%x' % self._device_id
    if self._device_string:
      device += ' (%s)' % self._device_string
    summary = '%s, %s' % (vendor, device)
    if self._sub_sys_id or self._revision:
      summary += ', SUBSYS = 0x%x, REV = %u' % (self._sub_sys_id,
                                                self._revision)
    summary += ', DRIVER_VENDOR = %s, DRIVER_VERSION = %s' % (
        self._driver_vendor, self._driver_version)
    return summary

  @classmethod
  def FromDict(cls, attrs):
    """Constructs a GPUDevice from a dictionary. Requires the
       following attributes to be present in the dictionary:

         vendor_id
         device_id
         vendor_string
         device_string

       Raises an exception if any attributes are missing.

       The following attributes are optional:

         sub_sys_id
         revision
         driver_vendor
         driver_version
    """
    return cls(
        attrs['vendor_id'],
        attrs['device_id'],
        # --browser=reference may use an old build of Chrome
        # where sub_sys_id, revision, driver_vendor or driver_version
        # aren't part of the dict.
        # sub_sys_id and revision are available on Windows only.
        attrs.get('sub_sys_id', 0),
        attrs.get('revision', 0),
        attrs['vendor_string'],
        attrs['device_string'],
        attrs.get('driver_vendor', ''),
        attrs.get('driver_version', ''))

  @property
  def vendor_id(self):
    """The GPU vendor's PCI ID as a number, or 0 if not available.

       Most desktop machines supply this information rather than the
       vendor and device strings."""
    return self._vendor_id

  @property
  def device_id(self):
    """The GPU device's PCI ID as a number, or 0 if not available.

       Most desktop machines supply this information rather than the
       vendor and device strings."""
    return self._device_id

  @property
  def sub_sys_id(self):
    """The GPU device's sub sys ID as a number, or 0 if not available.

       Only available on Windows platforms."""
    return self._sub_sys_id

  @property
  def revision(self):
    """The GPU device's revision as a number, or 0 if not available.

       Only available on Windows platforms."""
    return self._revision

  @property
  def vendor_string(self):
    """The GPU vendor's name as a string, or the empty string if not
       available.

       Most mobile devices supply this information rather than the PCI
       IDs."""
    return self._vendor_string

  @property
  def device_string(self):
    """The GPU device's name as a string, or the empty string if not
       available.

       Most mobile devices supply this information rather than the PCI
       IDs."""
    return self._device_string

  @property
  def driver_vendor(self):
    """The GPU driver vendor as a string, or the empty string if not
       available."""
    return self._driver_vendor

  @property
  def driver_version(self):
    """The GPU driver version as a string, or the empty string if not
       available."""
    return self._driver_version
