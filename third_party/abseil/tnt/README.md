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
    curl -L -O https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp/+archive/f81f6c011baf9b0132a5594c034fe0060820711d.tar.gz
    mkdir abseil_new 
    tar -xzf f81f6c011baf9b0132a5594c034fe0060820711d.tar.gz -C abseil_new
    rsync -r abseil_new/ abseil/ --delete --exclude tnt
    rm -rf abseil_new/ f81f6c011baf9b0132a5594c034fe0060820711d.tar.gz
    git add abseil ; git status



This folder previously last updated as follows:

    cd third_party
    curl -L -O https://github.com/abseil/abseil-cpp/archive/refs/heads/master.zip
    unzip master.zip
    mv abseil-cpp-master abseil_new
    rsync -r abseil_new/ abseil/ --delete --exclude tnt
    rm -rf master.zip abseil_new
    git add abseil ; git status
