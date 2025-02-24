# Validation for Chromium's Third Party Metadata Files

This directory contains the code to validate Chromium third party metadata
files, i.e. `README.chromium` files.

## Prerequisites
1. Have the Chromium source code
[checked out](https://chromium.googlesource.com/chromium/src/+/main/docs/#checking-out-and-building) on disk
1. Ensure you've run `gclient runhooks` on your source checkout

## Run
`metadata/scan.py` can be used to search for and validate all Chromium third
party metadata files within a repository. For example, if your `chromium/src`
checkout is at `~/my/path/to/chromium/src`, run the following command from the
root directory of `depot_tools`:
```
vpython3 --vpython-spec=.vpython3 metadata/scan.py ~/my/path/to/chromium/src
```
