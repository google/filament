from __future__ import absolute_import
import os
import sys
import signal
import subprocess

import lit.Test
import lit.TestRunner
import lit.util
from .base import TestFormat

# TAEF must be run with custom command line string and shell=True
# because of the way it manually processes quoted arguments in a
# non-standard way.
def executeCommandForTaef(command, cwd=None, env=None):
    p = subprocess.Popen(command, cwd=cwd,
                        shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        env=env,
                        # Close extra file handles on UNIX (on Windows this cannot be done while
                        # also redirecting input). Taef only run on windows.
                        close_fds=False)
    out,err = p.communicate()
    exitCode = p.wait()

    # Detect Ctrl-C in subprocess.
    if exitCode == -signal.SIGINT:
        raise KeyboardInterrupt

    # Ensure the resulting output is always of string type.
    out = lit.util.convert_string(out)
    err = lit.util.convert_string(err)

    return out, err, exitCode

class TaefTest(TestFormat):
    def __init__(self, te_path, test_dll, test_path, select_filter, extra_params):
        self.te = te_path
        self.test_dll = test_dll
        self.test_path = test_path
        self.select_filter = select_filter
        self.extra_params = extra_params
        # NOTE: when search test, always running on test_dll,
        #       use test_searched to make sure only add test once.
        #       If TaeftTest is created in directory with sub directory,
        #       getTestsInDirectory will be called more than once.
        self.test_searched = False

    def getTaefTests(self, dll_path, litConfig, localConfig):
        """getTaefTests()

        Return the tests available in taef test dll.

        Args:
          litConfig: LitConfig instance
          localConfig: TestingConfig instance"""

        # TE:F:\repos\DxcGitHub\hlsl.bin\TAEF\x64\te.exe
        # test dll : F:\repos\DxcGitHub\hlsl.bin\Debug\test\ClangHLSLTests.dll
        # /list

        if litConfig.debug:
            litConfig.note('searching taef test in %r' % dll_path)
        
        cmd = [self.te, dll_path, "/list", "/select:", self.select_filter]

        try:
            lines,err,exitCode = executeCommandForTaef(cmd)
            # this is for windows
            lines = lines.replace('\r', '')
            lines = lines.split('\n')

        except:
            litConfig.error("unable to discover taef tests in %r, using %s. exeption encountered." % (dll_path, self.te))
            raise StopIteration

        if exitCode:
            litConfig.error("unable to discover taef tests in %r, using %s. error: %s." % (dll_path, self.te, err))
            raise StopIteration

        for ln in lines:
            # The test name is like VerifierTest::RunUnboundedResourceArrays.
            if ln.find('::') == -1:
                continue

            yield ln.strip()

    # Note: path_in_suite should not include the executable name.
    def getTestsInExecutable(self, testSuite, path_in_suite, execpath,
                             litConfig, localConfig):

        # taef test should be dll.
        if not execpath.endswith('dll'):
            return

        (dirname, basename) = os.path.split(execpath)
        # Discover the tests in this executable.
        for testname in self.getTaefTests(execpath, litConfig, localConfig):
            testPath = path_in_suite + (basename, testname)
            yield lit.Test.Test(testSuite, testPath, localConfig, file_path=execpath)

    def getTestsInDirectory(self, testSuite, path_in_suite,
                            litConfig, localConfig):
        # Make sure self.test_dll only search once.
        if self.test_searched:
            return

        self.test_searched = True

        filepath = self.test_dll
        for test in self.getTestsInExecutable(
                testSuite, path_in_suite, filepath,
                litConfig, localConfig):
            yield test

    def execute(self, test, litConfig):
        test_dll = test.getFilePath()

        testPath,testName = os.path.split(test.getSourcePath())

        select_filter = str.format("@Name='{}'", testName)

        if self.select_filter != "":
            select_filter = str.format("{} AND {}", select_filter, self.select_filter)

        cmd = [self.te, test_dll, '/inproc',
                '/select:', select_filter,
                '/miniDumpOnCrash', '/unicodeOutput:false',
                str.format('/outputFolder:{}', self.test_path)]
        cmd.extend(self.extra_params)

        if litConfig.useValgrind:
            cmd = litConfig.valgrindArgs + cmd

        if litConfig.noExecute:
            return lit.Test.PASS, ''

        out, err, exitCode = executeCommandForTaef(
            cmd, env = test.config.environment)

        if exitCode:
            skipped = 'Failed=0, Blocked=0, Not Run=0, Skipped=1'
            if skipped in out:
                return lit.Test.UNSUPPORTED, ''

            unselected = 'The selection criteria did not match any tests.'
            if unselected in out:
                return lit.Test.UNSUPPORTED, ''

            return lit.Test.FAIL, out + err

        summary = 'Summary: Total='
        if summary not in out:
            msg = ('Unable to find %r in taef output:\n\n%s%s' %
                   (summary, out, err))
            return lit.Test.UNRESOLVED, msg
        no_fail = 'Failed=0, Blocked=0, Not Run=0, Skipped=0'
        if no_fail not in out == -1:
            msg = ('Unable to find %r in taef output:\n\n%s%s' %
                   (no_fail, out, err))
            return lit.Test.UNRESOLVED, msg

        return lit.Test.PASS,''

