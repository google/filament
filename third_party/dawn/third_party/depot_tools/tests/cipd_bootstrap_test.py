#!/usr/bin/env vpython3
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import subprocess
import sys
import unittest
import tempfile

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# CIPD client version to use for self-update from an "old" checkout to the tip.
#
# This version is from Dec 2023. Digests were generated using:
#   cipd selfupdate-roll -version-file tmp \
#        -version git_revision:161d3029a2818c224db3154cf3e78fde51a1047f
#   cat tmp.digests
OLD_VERSION = 'git_revision:161d3029a2818c224db3154cf3e78fde51a1047f'
OLD_DIGESTS = """
aix-ppc64        sha256  2e947e55e7fe25d3b5ab5524f8603a3b15d13c209f18873704cfc2c4a95ee685
dragonfly-amd64  sha256  f0229d21ae5c19ddfd4689d2c93937d085a090b8ccadd4a177d897d7d884f5b9
freebsd-386      sha256  8c3681db91539fd8cad78684a798390351ef7c71b1394c9bc38c7deb337d7db3
freebsd-amd64    sha256  e085d373967a10a92c02f1df3c67cc949d20145bedd041bac268de42d2c4e7aa
freebsd-arm64    sha256  3b8d306ce501fc77896ca95b1a04fe5a1e1825843554517a13ad7fee70c72fc3
freebsd-riscv64  sha256  1656b824ce881ee4078998917ce6bbfadc98a881aa21f35bb8827e14f2746ce6
illumos-amd64    sha256  6d1e6c7f0f2e98fcbaaad849431fe7a886470b14a04d872daa804d9473a62682
linux-386        sha256  0658444410909d596a161412033c54459382e287b812af67afe03bf40124dc2b
linux-amd64      sha256  341314febc2b0e447914a20a3b845eb5052957451b30ed27b6221e8ddf9e0ed0
linux-arm64      sha256  3d2fcff18c46571a4bbf71623ddcb7229259bf289379b54ef221cbf7ceee3c98
linux-armv6l     sha256  0f690d2d2a653ea5e4ed11769307b705f2243f1f8cf6fc4ac8fe550084a730af
linux-loong64    sha256  0cdfe6ae538991c66b923828d2647accf52a2b2d56093520ab3efc45ecb61552
linux-mips       sha256  32fb27a6eea67b822bf7992b8ceb497490b1e2b101b6e5efb7d21f2544871b8f
linux-mips64     sha256  8cfcd51312ebe246ab45595e8bc83f0de28f2455e3a404a863141d439133ada7
linux-mips64le   sha256  66f129348448373c339d16920ce3bab53b8d740db4a580aed16a820bcc638721
linux-mipsle     sha256  342ac75930281310f9f32d636c5c67210346ce213c320edc5af12242ed42dc3b
linux-ppc64      sha256  709c3532542a7fd9768f9157503ac56c828ad423984e3467b4b635a945feea8d
linux-ppc64le    sha256  06410f5b1451f16137bde261b5c5dec78d017c36de7170db9000c0016c8eeebe
linux-riscv64    sha256  213764d2c36b012eb6c4d95dde46ebff5fcbddea69fdabef3c055e35bc857beb
linux-s390x      sha256  0040b832d04790457a5e7b39e495ee8baf2d3374310f6ea64d982b354ece9ac7
mac-amd64        sha256  5e6465c8396f9f8fc434782ab20e144e920a5a2552ed19c6f42704e630045059
mac-arm64        sha256  0ccf6dce0b382b9cd37ac72ee6a31943d97c544550b9cf78cdb22ed461c2696c
netbsd-386       sha256  add933cea38805e164c2d71b808f65c672168c51a5bb8002fabc75bcd80a72e4
netbsd-amd64     sha256  6a9fb662fbe98d1927c71d3b4045b82be106771ed43d4ddef0c37e4859626a00
netbsd-arm64     sha256  3b10b2fabbf5ce1fd74c8d517b72eadcf2d784126931125140ecfa6ebcb05c6c
netbsd-armv6l    sha256  40fb3148c3682d6b8151adcee3f93c828c781b0aaf2009d89484b9e913d2723c
openbsd-386      sha256  bb5b47f27cd5e1285a5dcae9b69c756d023611d18706964dfd7b765cdfedf225
openbsd-amd64    sha256  845447b68cab9d5fa96479440f0a692b66c4e1bfc73b92626bb355d25ed675aa
openbsd-arm64    sha256  713c6b2768ca8a12098ac57174e54245a0e5e8a58853426c4ffd34b8cdb06e5a
solaris-amd64    sha256  1745422a7dbaa1f53a36e325c02e908cf5bf68cdfb62343b6eeb1424cb597ceb
windows-386      sha256  cd2148e390a5eddc078d6dd2cdfcd9d4624515d24482a1fa255fdcbac9d5c4f8
windows-amd64    sha256  e88f9268c536bb88a7b45a73308a70cbc93438b771f2ad2b54e385e132dbc146
windows-arm64    sha256  45d2ae5ae31b9ac25277f25fde7eb26edf22458b89a55cffa5d9dccf6f117c44
"""


class CipdBootstrapTest(unittest.TestCase):
    """Tests that CIPD client can bootstrap from scratch and self-update from some
    old version to a most recent one.

    WARNING: This integration test touches real network and real CIPD backend and
    downloads several megabytes of stuff.
    """
    def setUp(self):
        self.tempdir = tempfile.mkdtemp('depot_tools_cipd')

    def tearDown(self):
        shutil.rmtree(self.tempdir)

    def stage_files(self, cipd_version=None, digests=None):
        """Copies files needed for cipd bootstrap into the temp dir.

        Args:
            cipd_version: if not None, a value to put into cipd_client_version file.
        """
        names = (
            '.cipd_impl.ps1',
            'cipd',
            'cipd.bat',
            'cipd_client_version',
            'cipd_client_version.digests',
        )
        for f in names:
            shutil.copy2(os.path.join(ROOT_DIR, f),
                         os.path.join(self.tempdir, f))
        if cipd_version is not None:
            with open(os.path.join(self.tempdir, 'cipd_client_version'),
                      'wt') as f:
                f.write(cipd_version + '\n')
        if digests is not None:
            p = os.path.join(self.tempdir, 'cipd_client_version.digests')
            with open(p, 'wt') as f:
                f.write(digests + '\n')

    def call_cipd_help(self):
        """Calls 'cipd help' bootstrapping the client in tempdir.

        Returns (exit code, merged stdout and stderr).
        """
        exe = 'cipd.bat' if sys.platform == 'win32' else 'cipd'
        p = subprocess.Popen([os.path.join(self.tempdir, exe), 'help'],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        out, _ = p.communicate()
        return p.returncode, out

    def test_new_bootstrap(self):
        """Bootstrapping the client from scratch."""
        self.stage_files()
        ret, out = self.call_cipd_help()
        if ret:
            self.fail('Bootstrap from scratch failed:\n%s' % out)

    def test_self_update(self):
        """Updating the existing client in-place."""
        self.stage_files(cipd_version=OLD_VERSION, digests=OLD_DIGESTS)
        ret, out = self.call_cipd_help()
        if ret:
            self.fail('Update to %s fails:\n%s' % (OLD_VERSION, out))
        self.stage_files()
        ret, out = self.call_cipd_help()
        if ret:
            self.fail('Update from %s to the tip fails:\n%s' %
                      (OLD_VERSION, out))


if __name__ == '__main__':
    unittest.main()
