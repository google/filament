# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

function cipd_bin_setup {
    local MYPATH="${DEPOT_TOOLS_DIR:-$(dirname "${BASH_SOURCE[0]}")}"
    local ENSURE="$MYPATH/cipd_manifest.txt"
    local ROOT="$MYPATH/.cipd_bin"

    UNAME="${DEPOT_TOOLS_UNAME_S:-$(uname -s | tr '[:upper:]' '[:lower:]')}"
    case $UNAME in
      cygwin*)
        ENSURE="$(cygpath -w $ENSURE)"
        ROOT="$(cygpath -w $ROOT)"
        ;;
    esac

    # value in .cipd_client_root file overrides the default root.
    CIPD_ROOT_OVERRIDE_FILE="${MYPATH}/.cipd_client_root"
    if [ -f "${CIPD_ROOT_OVERRIDE_FILE}" ]; then
        ROOT=$(<"${CIPD_ROOT_OVERRIDE_FILE}")
    fi

    (
    source "$MYPATH/cipd" ensure \
        -log-level warning \
        -ensure-file "$ENSURE" \
        -root "$ROOT"
    )

    echo $ROOT
}
