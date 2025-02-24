# Synchronization Validation Development

This document contains miscellaneous information related to development of synchronization validation.

## Error messages

### How to diff-compare error messages before and after the change

The syncval error reporting code uses helper routines to generate the parts of the error messages. One potential issues with this shared code is when modifying a specific error message it's possible to accidentally break another one. The easiest way to validate the scope of the changes is to compare the output of all negative tests before and after the change.

The following components of the error messages vary between the runs:
* gtest timing
* dispatchable handles

`--gtest_print_time=0` removes the timing information

Use [this patch](https://github.com/user-attachments/files/18318045/remove-dispatchable-handles.patch) to replace all dispatchable handles with zero values. It should be applied only locally to `layers/error_messages/logging.cpp`.

Run the negative tests before and after the change:
```
--gtest_filter=NegativeSyncVal*.* --print-vu --gtest_print_time=0 >syncval_{before/after}.txt
```

Diff-compare output files.
