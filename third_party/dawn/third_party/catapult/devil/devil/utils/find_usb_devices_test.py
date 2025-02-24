#!/usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=protected-access
"""
Unit tests for the contents of find_usb_devices.py.

Device tree for these tests is as follows:
Bus 001:
1: Device 011 "foo"
2: Device 012 "bar"
3: Device 013 "baz"

Bus 002:
1: Device 011 "quux"
2: Device 020 "My Test HUB" #hub 1
2:1: Device 021 "usb_device_p7_h1_t0" #physical port 7 on hub 1, on ttyUSB0
2:3: Device 022 "usb_device_p5_h1_t1" #physical port 5 on hub 1, on ttyUSB1
2:4: Device 023 "My Test Internal HUB" #internal section of hub 1
2:4:2: Device 024 "usb_device_p3_h1_t2" #physical port 3 on hub 1, on ttyUSB2
2:4:3: Device 026 "Not a Battery Monitor" #physical port 1 on hub 1, on ttyUSB3
2:4:4: Device 025 "usb_device_p1_h1_t3" #physical port 1 on hub 1, on ttyUSB3
3: Device 100 "My Test HUB" #hub 2
3:4: Device 101 "My Test Internal HUB" #internal section of hub 2
3:4:4: Device 102 "usb_device_p1_h2_t4" #physical port 1 on hub 2, on ttyusb4
"""

import logging
import unittest

from unittest import mock

from devil.utils import find_usb_devices
from devil.utils import lsusb
from devil.utils import usb_hubs

# Output of lsusb.lsusb().
# We just test that the dictionary is working by creating an
# "ID number" equal to (bus_num*1000)+device_num and seeing if
# it is picked up correctly. Also we test the description

DEVLIST = [(1, 11, 'foo'), (1, 12, 'bar'), (1, 13, 'baz'), (2, 11, 'quux'),
           (2, 20, 'My Test HUB'), (2, 21, 'ID 0403:6001 usb_device_p7_h1_t0'),
           (2, 22, 'ID 0403:6001 usb_device_p5_h1_t1'),
           (2, 23, 'My Test Internal HUB'),
           (2, 24, 'ID 0403:6001 usb_device_p3_h1_t2'),
           (2, 25, 'ID 0403:6001 usb_device_p1_h1_t3'),
           (2, 26, 'Not a Battery Monitor'), (2, 100, 'My Test HUB'),
           (2, 101, 'My Test Internal HUB'),
           (2, 102, 'ID 0403:6001 usb_device_p1_h1_t4')]

LSUSB_OUTPUT = [{
    'bus': b,
    'device': d,
    'desc': t,
    'id': (1000 * b) + d
} for (b, d, t) in DEVLIST]

# Note: "Lev", "Cnt", "Spd", and "MxCh" are not used by parser,
# so we just leave them as zeros here. Also note that the port
# numbers reported here start at 0, so they're 1 less than the
# port numbers reported elsewhere.
USB_DEVICES_OUTPUT = '''
T:  Bus=01 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#= 11 Spd=000 MxCh=00
S:  SerialNumber=FooSerial
T:  Bus=01 Lev=00 Prnt=00 Port=01 Cnt=00 Dev#= 12 Spd=000 MxCh=00
S:  SerialNumber=BarSerial
T:  Bus=01 Lev=00 Prnt=00 Port=02 Cnt=00 Dev#= 13 Spd=000 MxCh=00
S:  SerialNumber=BazSerial

T:  Bus=02 Lev=00 Prnt=00 Port=00 Cnt=00 Dev#= 11 Spd=000 MxCh=00

T:  Bus=02 Lev=00 Prnt=00 Port=01 Cnt=00 Dev#= 20 Spd=000 MxCh=00
T:  Bus=02 Lev=00 Prnt=20 Port=00 Cnt=00 Dev#= 21 Spd=000 MxCh=00
S:  SerialNumber=UsbDevice0
T:  Bus=02 Lev=00 Prnt=20 Port=02 Cnt=00 Dev#= 22 Spd=000 MxCh=00
S:  SerialNumber=UsbDevice1
T:  Bus=02 Lev=00 Prnt=20 Port=03 Cnt=00 Dev#= 23 Spd=000 MxCh=00
T:  Bus=02 Lev=00 Prnt=23 Port=01 Cnt=00 Dev#= 24 Spd=000 MxCh=00
S:  SerialNumber=UsbDevice2
T:  Bus=02 Lev=00 Prnt=23 Port=03 Cnt=00 Dev#= 25 Spd=000 MxCh=00
S:  SerialNumber=UsbDevice3
T:  Bus=02 Lev=00 Prnt=23 Port=02 Cnt=00 Dev#= 26 Spd=000 MxCh=00

T:  Bus=02 Lev=00 Prnt=00 Port=02 Cnt=00 Dev#=100 Spd=000 MxCh=00
T:  Bus=02 Lev=00 Prnt=100 Port=03 Cnt=00 Dev#=101 Spd=000 MxCh=00
T:  Bus=02 Lev=00 Prnt=101 Port=03 Cnt=00 Dev#=102 Spd=000 MxCh=00
'''

RAW_LSUSB_OUTPUT = '''
Bus 001 Device 011: FAST foo
Bus 001 Device 012: FAST bar
Bus 001 Device 013: baz
Bus 002 Device 011: quux
Bus 002 Device 020: My Test HUB
Bus 002 Device 021: ID 0403:6001 usb_device_p7_h1_t0
Bus 002 Device 022: ID 0403:6001 usb_device_p5_h1_t1
Bus 002 Device 023: My Test Internal HUB
Bus 002 Device 024: ID 0403:6001 usb_device_p3_h1_t2
Bus 002 Device 025: ID 0403:6001 usb_device_p1_h1_t3
Bus 002 Device 026: Not a Battery Monitor
Bus 002 Device 100: My Test HUB
Bus 002 Device 101: My Test Internal HUB
Bus 002 Device 102: ID 0403:6001 usb_device_p1_h1_t4
'''

LIST_TTY_OUTPUT = '''
ttyUSB0
Something-else-0
ttyUSB1
ttyUSB2
Something-else-1
ttyUSB3
ttyUSB4
Something-else-2
ttyUSB5
'''

# Note: The real output will have multiple lines with
# ATTRS{busnum} and ATTRS{devnum}, but only the first
# one counts. Thus the test output duplicates this.
UDEVADM_USBTTY0_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="21"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_USBTTY1_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="22"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_USBTTY2_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="24"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_USBTTY3_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="25"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_USBTTY4_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="102"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_USBTTY5_OUTPUT = '''
ATTRS{busnum}=="2"
ATTRS{devnum}=="26"
ATTRS{busnum}=="0"
ATTRS{devnum}=="0"
'''

UDEVADM_OUTPUT_DICT = {
    'ttyUSB0': UDEVADM_USBTTY0_OUTPUT,
    'ttyUSB1': UDEVADM_USBTTY1_OUTPUT,
    'ttyUSB2': UDEVADM_USBTTY2_OUTPUT,
    'ttyUSB3': UDEVADM_USBTTY3_OUTPUT,
    'ttyUSB4': UDEVADM_USBTTY4_OUTPUT,
    'ttyUSB5': UDEVADM_USBTTY5_OUTPUT
}


# Identification criteria for Plugable 7-Port Hub
def isTestHub(node):
  """Check if a node is a Plugable 7-Port Hub
  (Model USB2-HUB7BC)
  The topology of this device is a 4-port hub,
  with another 4-port hub connected on port 4.
  """
  if not isinstance(node, find_usb_devices.USBDeviceNode):
    return False
  if 'Test HUB' not in node.desc:
    return False
  if not node.HasPort(4):
    return False
  return 'Test Internal HUB' in node.PortToDevice(4).desc


TEST_HUB = usb_hubs.HubType(isTestHub, {
    1: 7,
    2: 6,
    3: 5,
    4: {
        1: 4,
        2: 3,
        3: 2,
        4: 1
    }
})


class USBScriptTest(unittest.TestCase):
  def setUp(self):
    find_usb_devices._GetTtyUSBInfo = mock.Mock(
        side_effect=lambda x: UDEVADM_OUTPUT_DICT[x])
    find_usb_devices._GetParsedLSUSBOutput = mock.Mock(
        return_value=LSUSB_OUTPUT)
    find_usb_devices._GetUSBDevicesOutput = mock.Mock(
        return_value=USB_DEVICES_OUTPUT)
    find_usb_devices._GetCommList = mock.Mock(return_value=LIST_TTY_OUTPUT)
    lsusb.raw_lsusb = mock.Mock(return_value=RAW_LSUSB_OUTPUT)

  def testGetTTYDevices(self):
    pp = find_usb_devices.GetAllPhysicalPortToTTYMaps([TEST_HUB])
    result = list(pp)
    self.assertEqual(result[0], {
        7: 'ttyUSB0',
        5: 'ttyUSB1',
        3: 'ttyUSB2',
        2: 'ttyUSB5',
        1: 'ttyUSB3'
    })
    self.assertEqual(result[1], {1: 'ttyUSB4'})

  def testGetPortDeviceMapping(self):
    pp = find_usb_devices.GetAllPhysicalPortToBusDeviceMaps([TEST_HUB])
    result = list(pp)
    self.assertEqual(result[0], {
        7: (2, 21),
        5: (2, 22),
        3: (2, 24),
        2: (2, 26),
        1: (2, 25)
    })
    self.assertEqual(result[1], {1: (2, 102)})

  def testGetSerialMapping(self):
    pp = find_usb_devices.GetAllPhysicalPortToSerialMaps([TEST_HUB])
    result = list(pp)
    self.assertEqual(result[0], {
        7: 'UsbDevice0',
        5: 'UsbDevice1',
        3: 'UsbDevice2',
        1: 'UsbDevice3'
    })
    self.assertEqual(result[1], {})

  def testFastDeviceDescriptions(self):
    bd = find_usb_devices.GetBusNumberToDeviceTreeMap()
    dev_foo = bd[1].FindDeviceNumber(11)
    dev_bar = bd[1].FindDeviceNumber(12)
    dev_usb_device_p7_h1_t0 = bd[2].FindDeviceNumber(21)
    self.assertEqual(dev_foo.desc, 'FAST foo')
    self.assertEqual(dev_bar.desc, 'FAST bar')
    self.assertEqual(dev_usb_device_p7_h1_t0.desc,
                     'ID 0403:6001 usb_device_p7_h1_t0')

  def testDeviceDescriptions(self):
    bd = find_usb_devices.GetBusNumberToDeviceTreeMap(fast=False)
    dev_foo = bd[1].FindDeviceNumber(11)
    dev_bar = bd[1].FindDeviceNumber(12)
    dev_usb_device_p7_h1_t0 = bd[2].FindDeviceNumber(21)
    self.assertEqual(dev_foo.desc, 'foo')
    self.assertEqual(dev_bar.desc, 'bar')
    self.assertEqual(dev_usb_device_p7_h1_t0.desc,
                     'ID 0403:6001 usb_device_p7_h1_t0')

  def testDeviceInformation(self):
    bd = find_usb_devices.GetBusNumberToDeviceTreeMap(fast=False)
    dev_foo = bd[1].FindDeviceNumber(11)
    dev_bar = bd[1].FindDeviceNumber(12)
    dev_usb_device_p7_h1_t0 = bd[2].FindDeviceNumber(21)
    self.assertEqual(dev_foo.info['id'], 1011)
    self.assertEqual(dev_bar.info['id'], 1012)
    self.assertEqual(dev_usb_device_p7_h1_t0.info['id'], 2021)

  def testSerialNumber(self):
    bd = find_usb_devices.GetBusNumberToDeviceTreeMap(fast=False)
    dev_foo = bd[1].FindDeviceNumber(11)
    dev_bar = bd[1].FindDeviceNumber(12)
    dev_usb_device_p7_h1_t0 = bd[2].FindDeviceNumber(21)
    self.assertEqual(dev_foo.serial, 'FooSerial')
    self.assertEqual(dev_bar.serial, 'BarSerial')
    self.assertEqual(dev_usb_device_p7_h1_t0.serial, 'UsbDevice0')


if __name__ == "__main__":
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
