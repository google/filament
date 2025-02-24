#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# These licenses are used to verify that code imported to Android complies with
# their licensing requirements. Do not add entries to this list without approval.
# SPDX Identifiers are preferred when available. For the full list of
# identifiers; see https://spdx.org/licenses/.
# Licenses are grouped by their classification (restrictiveness level) and then alphabetically.
#
# The classifications are based on the license classifier tool available at:
# https://github.com/google/licenseclassifier/blob/main/license_type.go
# Unfortunately, this open source version is no longer maintained.
# These are the differrent classifications we identify, ordered by restrictiveness level:
# * unencumbered, permissive, notice, reciprocal, restricted, by_exception_only, forbidden.
#
# 'by_exception_only' and 'forbidden' should never enter Chromium, reach out to
# product counsel if the need arises.
#
# REVIEW INSTRUCTIONS FOR chromium-third-party@google.com (and a guide to contributing to this file):
# 1. Paste the contents of the license to be classified into
#   https://opensource.corp.google.com/license/analyze. This will provide the ID
#   and the classification. Command line alternatives are documented at
#   go/license-classifier, but work on entire files only.
#   1.1 'unencumbered', 'permissive', or 'notice' are allowed ✅.
#   1.2 'reciprocal' are allowed, but only in open source projects e.g. Chromium.
#       See OPEN_SOURCE_SPDX_LICENSES below.
#   1.3 >='restricted' are handled on a case-by-case basis and require individual approval
#       from opensource-licensing@google.com and chromium-third-party@google.com. Be sure to include
#       the license and relevant details in the email. It can be helpful to
#       identify existing dependencies that have already been approved.
#
# 2. Check spdx.org/licenses to see if the license has an SPDX identifier.
#   2.1 If it does: Use this value instead of the license classifier output,
#       and add it to ALLOWED_SPDX_LICENSES.
#   2.2 If does not: Add the id provided by the license classifier
#       to EXTENDED_LICENSE_CLASSIFIERS.
#
# 3. Ensure that it is added under the correct classification
#   e.g. '# notice', and then sorted alphabetically asscending.
#
# 4. If you are uncertain whether a given third-party library can be included in
#   Chromium, please email opensource-licensing@google.com with the library's
#   license documentation, and explain where and how the component is going to
#   be used.
ALLOWED_SPDX_LICENSES = frozenset([
    # unencumbered.
    # go/keep-sorted start case=no
    "blessing",
    "CC0-1.0",
    "LZMA-SDK-9.22",
    "Unlicense",
    # go/keep-sorted end
    # permissive.
    # go/keep-sorted start case=no
    "GPL-2.0-with-autoconf-exception",
    "GPL-2.0-with-classpath-exception",
    "GPL-3.0-with-autoconf-exception",
    "MIT-0",
    # go/keep-sorted end
    # notice.
    # go/keep-sorted start case=no
    "AML",
    "Apache-2.0",
    "Artistic-2.0",
    "Beerware",
    "BSD-2-Clause",
    "BSD-2-Clause-FreeBSD",
    "BSD-3-Clause",
    "BSD-3-Clause-Attribution",
    "BSD-4-Clause",
    "BSD-4-Clause-UC",
    "BSD-4.3TAHOE",
    "BSD-Source-Code",
    "BSL-1.0",
    "CC-BY-3.0",
    "CC-BY-4.0",
    "FTL",
    "HPND",
    "HPND-sell-variant",
    "ICU",
    "IJG",
    "ISC",
    "JSON",
    "Libpng",
    "libtiff",
    "MIT",
    "MIT-Modern-Variant",
    "MS-PL",
    "NAIST-2003",
    "NCSA",
    "OFL-1.1",
    "OpenSSL",
    "SGI-B-2.0",
    "SunPro",
    "Unicode-3.0",
    "Unicode-DFS-2015",
    "Unicode-DFS-2016",
    "X11",
    "Zlib",
    # go/keep-sorted end
])

# These are licenses that are not in the SPDX license list, but are identified
# by the license classifier.
EXTENDED_LICENSE_CLASSIFIERS = frozenset([
    # unencumbered.
    # go/keep-sorted start case=no
    "AhemFont",
    "LZMA",
    "public-domain-md5",
    "SPL-SQRT-FLOOR",
    # go/keep-sorted end
    # permissive.
    # go/keep-sorted start case=no
    "LicenseRef-AMSFonts-2.2",
    "SolarDesigner",
    "test_fonts",
    # go/keep-sorted end
    # notice.
    # go/keep-sorted start case=no
    "Apache-with-LLVM-Exception",
    "Apache-with-Runtime-Exception",
    "Bitstream",
    "BLAS",
    "BSD-2-Clause-Flex",
    "BSD-3-Clause-OpenMPI",
    "BSD-4-Clause-Wasabi",
    "Caffe",
    "CERN",
    "cURL",
    "dso",
    "Entenssa",
    "FFT2D",
    "getopt",
    "GIF-Encoder",
    "GNU-All-permissive-Copying-License",
    "IBM-DHCP",
    "JsonCPP",
    "Khronos",
    "Libpng-2.0",
    "LicenseRef-base64",
    "LicenseRef-base64-cpp",
    "LicenseRef-OpenGLUT",
    "LicenseRef-takuya-ooura",
    "pffft",
    "Punycode",
    "SSLeay",
    "WebM-Project-Patent",
    "X11-Lucent",
    "zxing",
    # go/keep-sorted end

    # The Android Software Development Kit License is a special case.
    # It can introduce licensing complexities due to the potentially extensive
    # transitive dependency chain. Developers should carefully review the
    # licenses of all dependencies.
    "Android Software Development Kit License",
])

# These licenses are only allowed in open source projects due to their
# reciprocal requirements.
OPEN_SOURCE_SPDX_LICENSES = frozenset([
    # reciprocal.
    # go/keep-sorted start case=no
    "APSL-2.0",
    "CDDL-1.0",
    "CDDL-1.1",
    "CPL-1.0",
    "EPL-1.0",
    "MPL-1.1",
    "MPL-2.0",
    # go/keep-sorted end
])

# TODO(b/388620886): Implement warning when changing to or from these licenses
# (but not every time the README.chromium file is modified).
WITH_PERMISSION_ONLY = frozenset([
    # restricted.
    # go/keep-sorted start case=no
    "CC-BY-SA-3.0",
    "GPL-2.0",
    "GPL-3.0",
    "LGPL-2.0",
    "LGPL-2.1",
    "LGPL-3.0",
    "NPL-1.1",
    # go/keep-sorted end
    # by_exception_only.
    # go/keep-sorted start case=no
    "Commercial",
    "LicenseRef-Play-Core-SDK-TOS",
    "LicenseRef-Unity-Companion-License-1.3",
    "Opus-Patent-BSD-3-Clause",
    "UnRAR",
    # go/keep-sorted end
])

ALLOWED_LICENSES = ALLOWED_SPDX_LICENSES | EXTENDED_LICENSE_CLASSIFIERS
ALLOWED_OPEN_SOURCE_LICENSES = ALLOWED_LICENSES | OPEN_SOURCE_SPDX_LICENSES
ALL_LICENSES = ALLOWED_OPEN_SOURCE_LICENSES | WITH_PERMISSION_ONLY