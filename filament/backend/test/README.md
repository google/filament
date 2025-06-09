# Backend Unit Tests

These are tests that ensure the Filament backend is working properly on various operating systems
and graphics backends.

The majority of these tests generate images that are then compared against a known golden image.

## Running with a specific graphics library

Run with `-a<backend>` to run with a specific backend, such as `-avulkan` for vulkan or `-ametal`
for metal.

## Image comparisons

The expected images are stored as PNG files in the `expected_images` subdirectory of the test source
code. When cmake is run it will copy this directory to the build output creating
`images/expected_images` inside the same directory as the unit test binary.

The unit tests will then write their resulting images into `images/actual_images` in order to make
the tests easier to debug.

### Python utility for updating golden images and comparing results

Inside the unit test source code directory there is a python script called
`move_actual_images_to_expected.py`.
It will display the actual and expected images side-by-side and copy the actual image into the
source tree's `expected_images` directory if the user approves the change.

#### Directories

The `-r` flag is required and should be the directory where the test binary is.

If not running the script from the directory it's in, `-s` should be the source code's
`expected_images` directory.

#### Configuring compare/move behavior

The `-c` flag has the tool display the actual and expected images side-by-side. By default it
does this by opening both with the OS's default behavior. `-p` can be used to specify a different
terminal program.

The `-m` flag has the tool move the actual image to overwrite the expected image in the source
tree. If the images are also being compared then the move will only happen if the user approves the
change.

After updating the expected images, cmake will need to be run again to copy them to the binary's
directory. Also, currently some platforms can't load images to compare and so have the hash
hardcoded into the test source code, so for now those need to be updated by running the test and
copying the value manually.

#### Picking which tests to compare

The `-x` flag causes the tool to check the `test_detail.xml` file and only process the images who
failed a comparison. `--gtest_output=xml` needs to be passed to the test binary to generate this
file.

The `-t` argument takes a list of images to compare, provided as the file name without the `.png`
suffix.

## Unsupported tests

If a test depends on a feature that is unsupported by the current environment, it will start with
the `SKIP_IF` macro. This will cause the test to not be run.