# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import tempfile
import unittest
from unittest import mock

from telemetry.core import debug_data
from telemetry.internal.backends import browser_backend
from telemetry.testing import browser_test_case
from telemetry.testing import options_for_unittests


class BrowserBackendLogsUploadingUnittest(unittest.TestCase):
  def testUploadingToCLoudStorage(self):
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
    temp_file_name = temp_file.name
    try:
      temp_file.write('This is a\ntest log file.\n')
      temp_file.close()

      # pylint: disable=abstract-method
      class FakeBrowserBackend(browser_backend.BrowserBackend):
        @property
        def supports_uploading_logs(self):
          return True

        @property
        def log_file_path(self):
          return temp_file_name

      options = options_for_unittests.GetCopy()
      options.browser_options.logging_verbosity = (
          options.browser_options.VERBOSE_LOGGING)
      options.browser_options.logs_cloud_bucket = 'ABC'
      options.browser_options.logs_cloud_remote_path = 'def'

      b = FakeBrowserBackend(
          platform_backend=None, browser_options=options.browser_options,
          supports_extensions=False, tab_list_backend=None)
      self.assertEqual(b.GetLogFileContents(), 'This is a\ntest log file.\n')
      with mock.patch('py_utils.cloud_storage.Insert') as mock_insert:
        b.UploadLogsToCloudStorage()
        mock_insert.assert_called_with(
            bucket='ABC', remote_path='def', local_path=temp_file_name)
    finally:
      os.remove(temp_file_name)


class BrowserBackendIntegrationTest(browser_test_case.BrowserTestCase):
  def setUp(self):
    super().setUp()
    self._browser_backend = self._browser._browser_backend

  def testSmokeIsBrowserRunningReturnTrue(self):
    self.assertTrue(self._browser_backend.IsBrowserRunning())

  def testSmokeIsBrowserRunningReturnFalse(self):
    self._browser_backend.Close()
    self.assertFalse(self._browser_backend.IsBrowserRunning())

  def testBrowserPid(self):
    pid = self._browser_backend.GetPid()
    self.assertTrue(self._browser_backend.GetPid())
    self.assertEqual(pid, self._browser_backend.GetPid())


class CleanupUnsymbolizedMinidumpsUnittest(unittest.TestCase):
  def testBasicFatal(self):
    """Basic happy path test when unsymbolized minidumps are fatal."""

    # pylint: disable=abstract-method
    class FakeBrowserBackend(browser_backend.BrowserBackend):
      def GetAllUnsymbolizedMinidumpPaths(self, log=True):
        del log
        return ['a', 'b']

      def CollectDebugData(self, _):
        return debug_data.DebugData()

    with mock.patch('telemetry.internal.backends.browser_backend.'
                    '_GetStackSummaries') as stack_mock:
      stack_mock.return_value = [
          ['crash_file', 'middle_file', 'earliest_file'], None]
      fb = FakeBrowserBackend(
          platform_backend=None,
          browser_options=options_for_unittests.GetCopy(),
          supports_extensions=False, tab_list_backend=None)
      with self.assertRaisesRegex(
          RuntimeError,
          'Test left 2 unsymbolized minidumps around after finishing. Stack '
          'summaries: earliest_file > middle_file > crash_file, <unparsable '
          'stack>'):
        fb.CleanupUnsymbolizedMinidumps(fatal=True)


class GetStackSummariesUnittest(unittest.TestCase):
  def testBasic(self):
    """Basic happy path test for stack summaries."""
    minidump1 = """\
some header
Thread 1 (crashed)
 0  crashed_file + offset
    extra information
 1  some_file!A::B(function arguments) + offset
    extra information
 2  another_file!C::D + offset
    extra information
 3  unused_file + offset
    extra information
Thread 2
 0  separate_thread + offset
    extra information
"""
    minidump2 = """\
some header
Thread 0 (crashed)
 0  crashed_file!A::B(function arguments) + offset
    extra information
 1  some_file + offset
    extra information
 2  another_file + offset
    extra information
 3  unused_file!C::D + offset
    extra information
Thread 1
 0  separate_thread + offset
    extra_information
"""
    summaries = browser_backend._GetStackSummaries([minidump1, minidump2])
    self.assertEqual(
        summaries,
        [['crashed_file', 'some_file!A::B', 'another_file!C::D'],
         ['crashed_file!A::B', 'some_file', 'another_file']])

  def testNoCrashedThreadHeader(self):
    """Tests that no summary is generated without a crashed thread header."""
    minidump = """\
some header
Thread 1
 0  crashed_file + offset
    extra information
 1  some_file!A::B(function arguments) + offset
    extra information
 2  another_file!C::D + offset
    extra information
 3  unused_file + offset
    extra information
"""
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(summaries, [None])

  def testNoFrames(self):
    """Tests that no summary is generated without valid frames."""
    minidump = """\
some header
Thread 1 (crashed)
 a  crashed_file + offset
    extra information
 b  some_file!A::B(function arguments) + offset
    extra information
 c  another_file!C::D + offset
    extra information
 d  unused_file + offset
    extra information
"""
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(summaries, [None])

  def testLessThanMaxFrames(self):
    """Tests that everything works if there are fewer than the max frames."""
    minidump = """\
some header
Thread 1 (crashed)
 0 some_file!A::B(function arguments) + offset
   extra information
 1 some_file + offset
   extra information
"""
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(summaries, [['some_file!A::B', 'some_file']])

  def testOmittedFrames(self):
    """Tests that explicitly omitted frames are not included in the summary."""
    minidump = """\
some header
Thread 1 (crashed)
 0 libc.so.6 + offset
   extra information
 1 some_file!A::B(function arguments) + offset
   extra information
 2 libc.so.6 + offset
   extra information
 3 some_file + offset
   extra information
 4 another_file + offset
   extra information
"""
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(
        summaries, [['some_file!A::B', 'some_file', 'another_file']])

  def testLinuxStack(self):
    """Tests stack summary with a real Linux minidump sample."""
    # pylint: disable=line-too-long
    minidump = """\
Operating system: Linux
                  4.15.0 -161-generic #169-Ubuntu SMP Fri Oct 15 13:41:54 UTC 2021 x86_64
CPU: amd64
     family 6 model 158 stepping 13
     16 CPUs

GPU: UNKNOWN

Crash reason:  SIGSEGV /SEGV_MAPERR
Crash address: 0x4
Process uptime: 0 seconds

Thread 0 (crashed)
 0  libnvidia-glcore.so.440.100 + 0x106dce7
    rax = 0x000000003f800000   rdx = 0x000000000000001a
    rcx = 0x0000000000000000   rbx = 0x00000000409b8000
    rsi = 0x0000000000000000   rdi = 0x0000362401cac000
    rbp = 0x0000000000000000   rsp = 0x00007ffd2a591b08
     r8 = 0x0000000000000000    r9 = 0x0000000000000000
    r10 = 0x000000000000826b   r11 = 0x0000000000000009
    r12 = 0x0000000000000000   r13 = 0x0000000000000000
    r14 = 0x0000000000000000   r15 = 0x000000003f800000
    rip = 0x00007f37bd389ce7
    Found by: given as instruction pointer in context
 1  libnvidia-glcore.so.440.100 + 0xfcd638
    rsp = 0x00007ffd2a591b10   rip = 0x00007f37bd2e9638
    Found by: stack scanning
 2  libGLESv2.so!rx::RendererGL::RendererGL(std::Cr::unique_ptr<rx::FunctionsGL, std::Cr::default_delete<rx::FunctionsGL>>, egl::AttributeMap const&, rx::DisplayGL*) + 0x4de
    rsp = 0x00007ffd2a591b70   rip = 0x00007f37bf0cb0be
    Found by: stack scanning
 3  chrome!absl::base_internal::LowLevelAlloc::Alloc(unsigned long) + 0x937124
    rsp = 0x00007ffd2a591b88   rip = 0x0000564f0dafdc80
    Found by: stack scanning
 4  chrome!absl::base_internal::LowLevelAlloc::Alloc(unsigned long) + 0x937a94
    rsp = 0x00007ffd2a591bd0   rip = 0x0000564f0dafe5f0
    Found by: stack scanning
 5  chrome!partition_alloc::internal::CheckThatSlotOffsetIsZero(unsigned long) + 0x117
    rsp = 0x00007ffd2a591bf0   rip = 0x0000564f04c9e237
    Found by: stack scanning
 6  chrome!absl::base_internal::LowLevelAlloc::Alloc(unsigned long) + 0x937124
    rsp = 0x00007ffd2a591bf8   rip = 0x0000564f0dafdc80
    Found by: stack scanning
"""
    # pylint: enable=line-too-long
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(summaries, [['libnvidia-glcore.so.440.100',
                                  'libnvidia-glcore.so.440.100',
                                  'libGLESv2.so!rx::RendererGL::RendererGL']])

  def testWindowsStack(self):
    """Tests stack summary with a real Windows minidump sample."""
    # pylint: disable=line-too-long
    minidump = """\
Last event: 620.276c: Access violation - code c0000005 (first/second chance not available)
  debugger time: Mon Feb  6 10:21:09.298 2023 (UTC - 8:00)
RetAddr           : Args to Child                                                           : Call Site
00007ffe`02d79289 : 0000330a`901d345b 0000003e`1a1fd510 000056f8`007edd80 000056f8`00600000 : chrome!gl::Crash+0x6e
00007ffd`fe7c8096 : 0000330a`901d37eb 000056f8`007edd98 000056f8`004feac0 00000000`003255e0 : chrome!viz::GpuServiceImpl::Crash+0x39
00007ffe`01fbd96c : 00000000`00000002 0000003e`1a1fd530 0000003e`1a1fd5a0 0000003e`1a1fd548 : chrome!viz::mojom::GpuServiceStubDispatch::Accept+0x7ce
00007ffe`01fbd4e9 : 0000330a`901d332b 000056f8`0009b338 00000000`00000002 000056f8`0009b310 : chrome!mojo::InterfaceEndpointClient::HandleValidatedMessage+0x46a
00007ffe`03119020 : 0000330a`0000276c 0000003e`ffffffff 0000330a`901d32db 00000000`0000276c : chrome!mojo::InterfaceEndpointClient::HandleIncomingMessageThunk::Accept+0xb9
00007ffe`01fbf22c : 00000000`000030a8 00000000`00000b20 00000000`00000000 000056f8`00164000 : chrome!mojo::MessageDispatcher::Accept+0x230
00007ffe`01fc6114 : 000056f8`00167020 00007ffe`01e3d8fd 00000000`0000276c 00000000`ffffffff : chrome!mojo::InterfaceEndpointClient::HandleIncomingMessage+0x5e
00007ffe`01fc5b90 : 000056f8`000267c8 00007ffe`01e5bc3d 0000003e`1a1fdcb0 00000000`000c8bc0 : chrome!mojo::internal::MultiplexRouter::ProcessIncomingMessage+0x27a
00007ffe`03119020 : 000056f8`00027b20 00000000`00000000 00000000`00000003 00000000`7fffffff : chrome!mojo::internal::MultiplexRouter::Accept+0x170
00007ffe`01fbb9a5 : 00007ffe`0b0f4c00 00000000`00000080 00000000`00000018 00aaaaaa`aaaaaaaa : chrome!mojo::MessageDispatcher::Accept+0x230
00007ffe`01fbc245 : 00000000`00000000 00007ffd`fd2acf33 00000000`00000001 0000003e`1a1fe060 : chrome!mojo::Connector::DispatchMessageW+0x2b3
00007ffe`01fbc079 : 0000003e`1a1fe160 00007ffd`fd3cab74 00000000`00000000 00000000`00000000 : chrome!mojo::Connector::ReadAllAvailableMessages+0xa3
00007ffd`fe150fc8 : 0000003e`1a1fe110 000056f8`0005a6d0 00000000`00000000 00007ffd`fd390e1b : chrome!mojo::Connector::OnHandleReadyInternal+0x43
00007ffd`fd410321 : 00000000`00000000 000056f8`0005a720 0000003e`1a1fe170 00007ffe`01ddeebb : chrome!base::RepeatingCallback<void (media::CdmContext::Event)>::Run+0x5c
"""
    # pylint: enable=line-too-long
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(
        summaries, [['chrome!gl::Crash',
                     'chrome!viz::GpuServiceImpl::Crash',
                     'chrome!viz::mojom::GpuServiceStubDispatch::Accept']])

  def testAndroidStack(self):
    """Tests stack summary with a real Android minidump sample."""
    # pylint: disable=line-too-long
    minidump = """\
Operating system: Android
                  4.4.169 google/walleye/walleye:9/PQ3A.190801.002/bpastene08280824:userdebug/dev-keys -g09a041b17c60 #1 SMP PREEMPT Wed Jun 5 22:23:19 UTC 2019 armv8l
CPU: arm
     ARMv0
     8 CPUs

GPU: UNKNOWN

Crash reason:  SIGTRAP
Crash address: 0xc5f28654
Process uptime: 362 seconds

Thread 16 (crashed)
 0  libchrome.so!logging::LogMessage::~LogMessage() [immediate_crash.h : 144 + 0x0]
     r0 = 0x00000000    r1 = 0x00000000    r2 = 0xc902479c    r3 = 0xc2f2717c
     r4 = 0xc90247a0    r5 = 0xc9024788    r6 = 0xc1684128    r7 = 0x0000268a
     r8 = 0xc258ab5c    r9 = 0xc258ab58   r10 = 0xc258ab5c   r12 = 0xc8fa1cd8
     fp = 0xc258ab50    sp = 0xc16840f8    lr = 0xc5e77d83    pc = 0xc5ee58a0
    Found by: given as instruction pointer in context
 1  libchrome.so!logging::LogMessage::~LogMessage() [logging.cc : 724 + 0x3]
     r4 = 0xc16846a8    r5 = 0xc16846a8    r6 = 0x00000020    r7 = 0x00000036
     r8 = 0x00000000    r9 = 0x42525840   r10 = 0xe443e970    fp = 0xc8f8b0bc
     sp = 0xc1684658    pc = 0xc5ee59c7
    Found by: call frame info
 2  libchrome.so!logging::CheckError::~CheckError() [check.cc : 186 + 0x7]
     r4 = 0xc16846a8    r5 = 0xc16846a8    r6 = 0x00000020    r7 = 0x00000036
     r8 = 0x00000000    r9 = 0x42525840   r10 = 0xe443e970    fp = 0xc8f8b0bc
     sp = 0xc1684660    pc = 0xc5ecc929
    Found by: call frame info
 3  libchrome.so!blink::DevToolsSession::IOSession::DispatchProtocolCommand(int, WTF::String const&, base::span<unsigned char const, 4294967295u>) [devtools_session.cc : 104 + 0x5]
     r4 = 0xc16846f8    r5 = 0xc16846a8    r6 = 0x00000020    r7 = 0x00000036
     r8 = 0x00000000    r9 = 0x42525840   r10 = 0xe443e970    fp = 0xc8f8b0bc
     sp = 0xc1684668    pc = 0xc7e91a11
    Found by: call frame info
 4  libchrome.so!blink::mojom::blink::DevToolsSessionStubDispatch::Accept(blink::mojom::blink::DevToolsSession*, mojo::Message*) [devtools_agent.mojom-blink.cc : 1089 + 0x7]
     r4 = 0x42525840    r5 = 0xc7e919bd    r6 = 0x00000000    r7 = 0x00000036
     r8 = 0x00000000    r9 = 0xc2548e18   r10 = 0xc8fb78e4    fp = 0xc8f8b0bc
     sp = 0xc16846e0    pc = 0xc4c45a07
    Found by: call frame info
 5  libchrome.so!mojo::InterfaceEndpointClient::HandleValidatedMessage(mojo::Message*) [interface_endpoint_client.cc : 1007 + 0x7]
     r4 = 0xc1684948    r5 = 0xc25adf80    r6 = 0xc25ae0b8    r7 = 0xc3703cae
     r8 = 0x00000000    r9 = 0xc2548e18   r10 = 0xc8fb78e4    fp = 0xc8f8b0bc
     sp = 0xc1684718    pc = 0xc63a39cf
    Found by: call frame info
 6  libchrome.so!mojo::InterfaceEndpointClient::HandleIncomingMessageThunk::Accept(mojo::Message*) [interface_endpoint_client.cc : 357 + 0x7]
     r4 = 0xc1684948    r5 = 0xc25adf80    r6 = 0x00000000    r7 = 0xc25ae030
     r8 = 0x00000002    r9 = 0xc2548e18   r10 = 0xc25d8dc0    fp = 0xffffffff
     sp = 0xc1684838    pc = 0xc63a3747
    Found by: call frame info
"""
    # pylint: enable=line-too-long
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(
        summaries, [['libchrome.so!logging::LogMessage::~LogMessage',
                     'libchrome.so!logging::LogMessage::~LogMessage',
                     'libchrome.so!logging::CheckError::~CheckError']])

  def testMacStack(self):
    """Tests stack summary with a real Mac minidump sample."""
    # pylint: disable=line-too-long
    minidump = """\
Operating system: Mac OS X
                  12.1.0 21C52
CPU: amd64
     family 6 model 70 stepping 1
     8 CPUs

GPU: UNKNOWN

Crash reason:  EXC_BREAKPOINT / EXC_I386_BPT
Crash address: 0x138011913
Process uptime: 24 seconds

Thread 0 (crashed)
 0  Chromium Framework!v8::base::OS::Abort() + 0x13
    rax = 0x0000000000000000   rdx = 0x0000000000000000
    rcx = 0x00000001124226ac   rbx = 0x000000013ddf9f63
    rsi = 0x00000000000120a8   rdi = 0x00007ff851654538
    rbp = 0x00007ff7b673f580   rsp = 0x00007ff7b673f580
     r8 = 0x00007ff851654558    r9 = 0x0000000000000000
    r10 = 0x00000000ffffff00   r11 = 0x00007ff851654550
    r12 = 0x00007ff7b673f850   r13 = 0x00007ff8516798a0
    r14 = 0x0000000000000022   r15 = 0x000000013ddf9f37
    rip = 0x0000000138011913
    Found by: given as instruction pointer in context
 1  Chromium Framework!V8_Fatal(char const*, int, char const*, ...) + 0x16c
    rbp = 0x00007ff7b673f8a0   rsp = 0x00007ff7b673f590
    rip = 0x0000000137ffb63c
    Found by: previous frame's frame pointer
 2  Chromium Framework!cppgc::internal::VerificationState::VerifyMarked(void const*) const + 0x7b
    rbp = 0x00007ff7b673f8d0   rsp = 0x00007ff7b673f8b0
    rip = 0x000000013283645b
    Found by: previous frame's frame pointer
 3  Chromium Framework!void cppgc::TraceTrait<blink::HeapHashTableBacking<WTF::HashTable<cppgc::internal::BasicMember<blink::ResizeObservation, cppgc::internal::WeakMemberTag, cppgc::internal::DijkstraWriteBarrierPolicy, cppgc::internal::DisabledCheckingPolicy>, WTF::KeyValuePair<cppgc::internal::BasicMember<blink::ResizeObservation, cppgc::internal::WeakMemberTag, cppgc::internal::DijkstraWriteBarrierPolicy, cppgc::internal::DisabledCheckingPolicy>, unsigned int>, WTF::KeyValuePairExtractor, WTF::HashMapValueTraits<WTF::HashTraits<cppgc::internal::BasicMember<blink::ResizeObservation, cppgc::internal::WeakMemberTag, cppgc::internal::DijkstraWriteBarrierPolicy, cppgc::internal::DisabledCheckingPolicy>>, WTF::HashTraits<unsigned int>>, WTF::HashTraits<cppgc::internal::BasicMember<blink::ResizeObservation, cppgc::internal::WeakMemberTag, cppgc::internal::DijkstraWriteBarrierPolicy, cppgc::internal::DisabledCheckingPolicy>>, blink::HeapAllocator>>>::Trace<(WTF::WeakHandlingFlag)0>(cppgc::Visitor*, void const*) + 0x9e
    rbp = 0x00007ff7b673f920   rsp = 0x00007ff7b673f8e0
    rip = 0x000000013ad743ee
    Found by: previous frame's frame pointer
 4  Chromium Framework!cppgc::internal::HeapVisitor<cppgc::internal::MarkingVerifierBase>::Traverse(cppgc::internal::RawHeap&) + 0x2fa
    rbp = 0x00007ff7b673f9c0   rsp = 0x00007ff7b673f930
    rip = 0x000000013283688a
    Found by: previous frame's frame pointer
 5  Chromium Framework!cppgc::internal::MarkingVerifierBase::Run(cppgc::EmbedderStackState, std::optional<unsigned long>) + 0x20
    rbp = 0x00007ff7b673f9f0   rsp = 0x00007ff7b673f9d0
    rip = 0x0000000132836530
    Found by: previous frame's frame pointer
 6  Chromium Framework!v8::internal::CppHeap::TraceEpilogue() + 0x15f
    rbp = 0x00007ff7b673fae0   rsp = 0x00007ff7b673fa00
    rip = 0x0000000131913a1f
    Found by: previous frame's frame pointer
"""
    # pylint: enable=line-too-long
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(
        summaries, [['Chromium Framework!v8::base::OS::Abort',
                     'Chromium Framework!V8_Fatal',
                     'Chromium Framework!cppgc::internal::VerificationState::'
                     'VerifyMarked']])

  def testMacKernelLevel(self):
    """Tests stack summary with a real Mac minidump involving the kernel."""
    minidump = """\
Operating system: Mac OS X
                  10.14.6 18G103
CPU: amd64
     family 6 model 70 stepping 1
     8 CPUs

GPU: UNKNOWN

Crash reason:  EXC_BAD_ACCESS / KERN_PROTECTION_FAILURE
Crash address: 0x2f40000022b0
Process uptime: 354 seconds

Thread 0 (crashed)
 0  0x110e02563a2
    rax = 0x00002f4004f7f7e9   rdx = 0x0000000005a71d91
    rcx = 0x00002f400000325d   rbx = 0x0000000004f7ef45
    rsi = 0x00002f40000022a9   rdi = 0x00002f400143e91d
    rbp = 0x00007ffee372bf70   rsp = 0x00007ffee372bf10
     r8 = 0x00002f40000022e1    r9 = 0x00000000000005e8
    r10 = 0x0000000000000000   r11 = 0x00000000000005e8
    r12 = 0x0000000004f7ef45   r13 = 0x0000011c009cc000
    r14 = 0x00002f4000000000   r15 = 0x00002f4001184f61
    rip = 0x00000110e02563a2
    Found by: given as instruction pointer in context
 1  0x110e02d1e29
    rbp = 0x00007ffee372c108   rsp = 0x00007ffee372bf80
    rip = 0x00000110e02d1e29
    Found by: previous frame's frame pointer
 2  0x110e02b66c9
    rbp = 0x00007ffee372c1e8   rsp = 0x00007ffee372c118
    rip = 0x00000110e02b66c9
    Found by: previous frame's frame pointer
 3  0x110ffe0ee2c
    rbp = 0x00007ffee372c308   rsp = 0x00007ffee372c1f8
    rip = 0x00000110ffe0ee2c
    Found by: previous frame's frame pointer
 4  0x110e017aee6
    rbp = 0x00007ffee372c398   rsp = 0x00007ffee372c318
    rip = 0x00000110e017aee6
    Found by: previous frame's frame pointer
 5  0x110e009f8f9
    rbp = 0x00007ffee372c3e0   rsp = 0x00007ffee372c3a8
    rip = 0x00000110e009f8f9
    Found by: previous frame's frame pointer
 6  0x110ffe0cbdc
    rbp = 0x00007ffee372c408   rsp = 0x00007ffee372c3f0
    rip = 0x00000110ffe0cbdc
    Found by: previous frame's frame pointer
"""
    summaries = browser_backend._GetStackSummaries([minidump])
    self.assertEqual(
        summaries, [['0x110e02563a2', '0x110e02d1e29', '0x110e02b66c9']])
