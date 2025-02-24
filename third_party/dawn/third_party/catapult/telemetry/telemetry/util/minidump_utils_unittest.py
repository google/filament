# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

import six

if six.PY3:
  from unittest import mock

# pylint: disable=wrong-import-position
from telemetry.util import minidump_utils
# pylint: enable=wrong-import-position


@unittest.skipIf(six.PY2, 'Tested code is Python 3-only')
@mock.patch('sys.platform', 'linux2')
class GetCrashpadAnnotationPosixUnittest(unittest.TestCase):
  """POSIX test cases for GetCrashpadAnnotation() and its specific versions."""
  def setUp(self):
    self._dump_patcher = mock.patch.object(minidump_utils, 'DumpMinidump')
    self._dump_mock = self._dump_patcher.start()
    self.addCleanup(self._dump_patcher.stop)
    # pylint: disable=line-too-long
    self._dump_mock.return_value = """\
MDRawCrashpadInfo
  version = 1
  report_id = 95ebd8b0-1f08-4a39-9359-d26725ea9144
  client_id = 3826aae7-e60a-4ef5-89af-e9d00ef7fd06
  simple_annotations["lsb-release"] = Debian GNU/Linux rodete
  simple_annotations["plat"] = Linux
  simple_annotations["prod"] = Chrome_Linux
  simple_annotations["ver"] = 112.0.5599.0
  module_list[0].minidump_module_list_index = 0
  module_list[0].version = 1
  module_list[0].crashpad_annotations["gpu-gl-context-is-virtual"] (type = 1) = 0
  module_list[0].crashpad_annotations["gr-context-type"] (type = 1) = 1
  module_list[0].crashpad_annotations["num-experiments"] (type = 1) = 36
  module_list[0].crashpad_annotations["vulkan-api-version"] (type = 1) = 1.3.240
  module_list[0].crashpad_annotations["egl-display-type"] (type = 1) = angle:Vulkan
  module_list[0].crashpad_annotations["gpu-gl-renderer"] (type = 1) = ANGLE (NVIDIA, Vulkan 1.3.194 (NVIDIA Quadro P1000 (0x00001CB1)), NVIDIA-510.85.2.0)
  module_list[0].crashpad_annotations["gpu-gl-vendor"] (type = 1) = Google Inc. (NVIDIA)
  module_list[0].crashpad_annotations["gpu-generation-intel"] (type = 1) = 0
  module_list[0].crashpad_annotations["gpu-vsver"] (type = 1) = 1.00
  module_list[0].crashpad_annotations["gpu-psver"] (type = 1) = 1.00
  module_list[0].crashpad_annotations["gpu-driver"] (type = 1) = 510.85.02
  module_list[0].crashpad_annotations["gpu_count"] (type = 1) = 1
  module_list[0].crashpad_annotations["gpu-devid"] (type = 1) = 0x1cb1
  module_list[0].crashpad_annotations["gpu-venid"] (type = 1) = 0x10de
  module_list[0].crashpad_annotations["switch-8"] (type = 1) = --shared-files
  module_list[0].crashpad_annotations["switch-7"] (type = 1) = --use-gl=angle
  module_list[0].crashpad_annotations["osarch"] (type = 1) = x86_64
  module_list[0].crashpad_annotations["pid"] (type = 1) = 771521
  module_list[0].crashpad_annotations["ptype"] (type = 1) = gpu-process
  module_list[0].crashpad_annotations["switch-6"] (type = 1) = --change-stack-guard-on-fork=enable
  module_list[0].crashpad_annotations["switch-5"] (type = 1) = --user-data-dir=/tmp/tmpr1y2qx1t
  module_list[0].crashpad_annotations["switch-4"] (type = 1) = --noerrdialogs
  module_list[0].crashpad_annotations["switch-3"] (type = 1) = --enable-crash-reporter=,
  module_list[0].crashpad_annotations["switch-2"] (type = 1) = --crashpad-handler-pid=771487
  module_list[0].crashpad_annotations["switch-1"] (type = 1) = --use-cmd-decoder=passthrough
  module_list[0].crashpad_annotations["num-switches"] (type = 1) = 13
  address_mask = 0
"""
    # pylint: enable=line-too-long

  def testGetCrashpadAnnotationValidNameNoType(self):
    self.assertEqual(
        minidump_utils.GetCrashpadAnnotation('', 'gpu-devid'), '0x1cb1')

  def testGetCrashpadAnnotationValidNameSpecificType(self):
    self.assertEqual(minidump_utils.GetCrashpadAnnotation('', 'switch-1', 1),
                     '--use-cmd-decoder=passthrough')

  def testGetCrashpadAnnotationValidNameWrongType(self):
    self.assertEqual(
        minidump_utils.GetCrashpadAnnotation('', 'switch-1', 2), None)

  def testGetCrashpadAnnotationInvalidName(self):
    self.assertEqual(minidump_utils.GetCrashpadAnnotation('', 'asdf'), None)

  def testGetCrashpadAnnotationWithSpaces(self):
    self.assertEqual(minidump_utils.GetCrashpadAnnotation(
        '', 'gpu-gl-vendor'), 'Google Inc. (NVIDIA)')

  def testGetProcessTypeFromMinidump(self):
    self.assertEqual(
        minidump_utils.GetProcessTypeFromMinidump(''), 'gpu-process')


@unittest.skipIf(six.PY2, 'Tested code is Python 3-only')
@mock.patch('sys.platform', 'win32')
class GetCrashpadAnnotationWindowsUnittest(unittest.TestCase):
  """Windows test cases for GetCrashpadAnnotation() and its specific versions"""
  def setUp(self):
    self._dump_patcher = mock.patch.object(minidump_utils, 'DumpMinidump')
    self._dump_mock = self._dump_patcher.start()
    self.addCleanup(self._dump_patcher.stop)
    # pylint: disable=line-too-long
    self._dump_mock.return_value = """\
Module: /usr/local/google/code/clankium/src/out/gpu/chrome
  Simple Annotations
  Vectored Annotations
  Annotation Objects
    annotation_objects["gpu-url-chunk"] = http://127.0.0.1:32879/content/test/data/gpu/pixel_video_context_loss.html?src=/media/test/data/four-colors.mp4
    annotation_objects["gpu-gl-context-is-virtual"] = 0
    annotation_objects["gr-context-type"] = 1
    annotation_objects["variations"] = 2510663e-73703436,b13ca3d9-84f6cff8,13e2821-c0236c9e,13427e22-3f4a17df,87f33ad6-3f4a17df,250dda8b-3f4a17df,9f476f76-3f4a17df,65570806-377be55a,92d2eb18-fa10226e,1584cf60-3f4a17df,4852ec7f-3f4a17df,5f2c0f7c-3f4a17df,3fd33f16-27b09c4c,36d5ee52-3f4a17df,391562d6-3f4a17df,8bccc03b-3f4a17df,7e398fb8-3f4a17df,5349039a-3f4a17df,3095b8fe-3f4a17df,5de2eeca-3f4a17df,da493d3c-3f4a17df,e28cd73c-3f4a17df,f3ed486d-3f4a17df,5c783e42-8f8fa88f,4ff8f5b5-caf7a452,5f36436a-f799c15e,baee3c29-3f4a17df,9fe21c85-3f4a17df,ade3efeb-e1cc0f14,db59f83a-3f4a17df,1166396-1166396,bbaef9b4-3f4a17df,55ba4cfa-3f4a17df,fc7e4d22-3f4a17df,8963b549-3f4a17df,c297985a-3f4a17df,
    annotation_objects["num-experiments"] = 36
    annotation_objects["vulkan-api-version"] = 1.3.240
    annotation_objects["egl-display-type"] = angle:Vulkan
    annotation_objects["gpu-gl-renderer"] = ANGLE (NVIDIA, Vulkan 1.3.194 (NVIDIA Quadro P1000 (0x00001CB1)), NVIDIA-510.85.2.0)
    annotation_objects["gpu-gl-vendor"] = Google Inc. (NVIDIA)
    annotation_objects["gpu-generation-intel"] = 0
    annotation_objects["gpu-vsver"] = 1.00
    annotation_objects["gpu-psver"] = 1.00
    annotation_objects["gpu-driver"] = 510.85.02
    annotation_objects["gpu_count"] = 1
    annotation_objects["gpu-devid"] = 0x1cb1
    annotation_objects["gpu-venid"] = 0x10de
    annotation_objects["switch-9"] = --field-trial-handle=0,i,5890857200490588528,8178675155134540264
    annotation_objects["switch-8"] = --shared-files
    annotation_objects["switch-7"] = --use-gl=angle
    annotation_objects["osarch"] = x86_64
    annotation_objects["pid"] = 771521
    annotation_objects["ptype"] = gpu-process
    annotation_objects["switch-6"] = --change-stack-guard-on-fork=enable
    annotation_objects["switch-5"] = --user-data-dir=/tmp/tmpr1y2qx1t
    annotation_objects["switch-4"] = --noerrdialogs
    annotation_objects["switch-3"] = --enable-crash-reporter=,
    annotation_objects["switch-2"] = --crashpad-handler-pid=771487
    annotation_objects["switch-1"] = --use-cmd-decoder=passthrough
    annotation_objects["num-switches"] = 13
Module: linux-vdso.so.1
  Simple Annotations
  Vectored Annotations
  Annotation Objects
Module: libdl.so.2
  Simple Annotations
  Vectored Annotations
  Annotation Objects
"""
    # pylint: enable=line-too-long

  def testGetCrashpadAnnotationValidNameNoType(self):
    self.assertEqual(
        minidump_utils.GetCrashpadAnnotation('', 'gpu-devid'), '0x1cb1')

  def testGetCrashpadAnnotationValidNameSpecificType(self):
    # Type should be ignored on Windows.
    self.assertEqual(minidump_utils.GetCrashpadAnnotation('', 'switch-1', 99),
                     '--use-cmd-decoder=passthrough')

  def testGetCrashpadAnnotationInvalidName(self):
    self.assertEqual(minidump_utils.GetCrashpadAnnotation('', 'asdf'), None)

  def testGetCrashpadAnnotationWithSpaces(self):
    self.assertEqual(minidump_utils.GetCrashpadAnnotation(
        '', 'gpu-gl-vendor'), 'Google Inc. (NVIDIA)')

  def testGetProcessTypeFromMinidump(self):
    self.assertEqual(
        minidump_utils.GetProcessTypeFromMinidump(''), 'gpu-process')
