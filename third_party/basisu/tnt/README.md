This folder was last updated as follows:

    export ver=1.16.3
    cd third_party
    curl -L -O https://github.com/BinomialLLC/basis_universal/archive/refs/tags/${ver}.zip
    unzip ${ver}.zip
    mv basis_universal-* basis_new
    rsync -r basis_new/ basisu/ --delete --exclude tnt
    rm -rf ${ver}.zip basis_new
    git add basisu ; git status

Our CMakeLists differs from the one in basisu as follows.

(1)

Disable support for various texture formats that we don't need.

(2)

Remove the `CMAKE_RUNTIME_OUTPUT_DIRECTORY` setter and strip commands from their CMakeLists, because
we prefer to use the default location for the executable.

(3)

Add a library target called `basis_encoder` because we prefer to use it as a library, not an
executable.

(4)

Add a library target called `basis_transcoder` for use as a lean & mean reader-only library.

(5)

Simplify by removing support for OPENCL and requiring ZSTD to be enabled.
