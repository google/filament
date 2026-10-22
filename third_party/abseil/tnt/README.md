This folder was last updated as follows to fix compile issues with Mac OS 14.5:

```shell
cd third_party
curl -L -O https://github.com/abseil/abseil-cpp/archive/refs/tags/20250127.1.zip
unzip 20250127.1.zip
mv abseil-cpp-20250127.1 abseil_new
rsync -r abseil_new/ abseil/ --delete --exclude tnt
rm -rf 20250127.1.zip abseil_new
rm abseil/.clang-format
git add abseil
```

This folder was last updated as follows to match the Dawn dependency:

    cd third_party
    curl -L -O https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp/+archive/cae4b6a3990e1431caa09c7b2ed1c76d0dfeab17.tar.gz
    mkdir abseil_new 
    tar -xzf cae4b6a3990e1431caa09c7b2ed1c76d0dfeab17.tar.gz -C abseil_new
    rsync -r abseil_new/ abseil/ --delete --exclude tnt
    rm -rf abseil_new/ cae4b6a3990e1431caa09c7b2ed1c76d0dfeab17.tar.gz
    git add abseil ; git status



This folder previously last updated as follows:

    cd third_party
    curl -L -O https://github.com/abseil/abseil-cpp/archive/refs/heads/master.zip
    unzip master.zip
    mv abseil-cpp-master abseil_new
    rsync -r abseil_new/ abseil/ --delete --exclude tnt
    rm -rf master.zip abseil_new
    git add abseil ; git status

## Custom Changes

A custom patch has been applied to fix compilation on ARM architectures when AddressSanitizer (ASAN) is enabled (specifically regarding `ABSL_HAVE_INTRINSIC_INT128`).

If you update the `abseil` folder, you may need to re-apply the patch. You can do so by running the following command from the repository root:

```shell
git apply third_party/abseil/tnt/asan_arm_int128.diff
```
