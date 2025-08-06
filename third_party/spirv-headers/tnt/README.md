# Updating
```shell
cd third_party
curl -L https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers/+archive/2a611a970fdbc41ac2e3e328802aed9985352dca.tar.gz > spirv-headers-src.tar.gz
mkdir spirv-headers-new
tar -xzf spirv-headers-src.tar.gz -C spirv-headers-new
rsync -r spirv-headers-new/ spirv-headers/ --delete
git restore spirv-headers/tnt/README.md
rm -rf spirv-headers/.github
rm -rf spirv-headers-new spirv-headers-src.tar.gz
git add spirv-headers
```
