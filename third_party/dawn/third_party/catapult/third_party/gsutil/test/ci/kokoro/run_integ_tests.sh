#!/bin/bash

# This shell script is used for setting up our Kokoro Ubuntu environment
# with necessary dependencies for running integration tests, and then
# running tests when PRs are submitted or code is pushed.
#
# This script is intentionally a bit verbose with what it writes to stdout.
# Since its output is only visible in test run logs, and those logs are only
# likely to be viewed in the event of an error, I decided it would be beneficial
# to leave some settings like `set -x` and `cat`s and `echo`s in. This should
# help future engineers debug what went wrong, and assert info about the test
# environment at the cost of a small preamble in the logs.

# -x : Display commands being run
# -u : Disallow unset variables
# Doc: https://www.gnu.org/software/bash/manual/html_node/The-Set-Builtin.html#The-Set-Builtin
set -xu

HELPER_PATH="/tmpfs/src/github/src/gsutil/test/ci/kokoro/run_integ_tests_helper.sh"

# Set the locale to utf-8 for macos b/154863917 and linux
# https://github.com/GoogleCloudPlatform/gsutil/pull/1692
if [[ $KOKORO_JOB_NAME =~ "linux" ]]; then
  export LANG=C.UTF-8
  export LC_ALL=C.UTF-8
  # Kokoro for linux runs in a Docker container as root, which causes
  # few tests to fail. See b/281868063.
  # Add a new user.
  # -s: Set the login shell.
  useradd -s /bin/bash tester

  # Make this user the owner of /tmpfs/src and /root dirs so that restricted
  # files can be accessed.
  chown -hR tester: /tmpfs/src/
  chown -hR tester: /root/

  # Call the script as the new user.
  # -E: Preserve the environment variables.
  sudo -E -u tester bash "$HELPER_PATH"
elif [[ $KOKORO_JOB_NAME =~ "macos" ]]; then
  export LANG=en_US.UTF-8
  export LC_ALL=en_US.UTF-8
  bash "$HELPER_PATH"
fi

