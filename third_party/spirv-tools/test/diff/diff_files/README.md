# Diff tests

This directory contains files used to ensure correctness of the `spirv-diff` implementation.  The
`generate_tests.py` script takes `name_src.spvasm` and `name_dst.spvasm` (for each `name`) and
produces unit test files in the form of `name_autogen.cpp`.

The unit test files test the diff between the src and dst inputs, as well as between debug-stripped
versions of those.  Additionally, based on the `{variant}_TESTS` lists defined in
`generate_tests.py`, extra unit tests are added to exercise different options of spirv-diff.

New tests are added simply by placing a new `name_src.spvasm` and `name_dst.spvasm` pair in this
directory and running `generate_tests.py`.  Note that this script needs the path to the spirv-diff
executable that is built.

The `generate_tests.py` script additionally expects `name_src.spvasm` to include a heading where the
purpose of the test is explained.  This heading is parsed as a block of lines starting with `;;` at
the top of the file.
