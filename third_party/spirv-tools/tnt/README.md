# Updating
```shell
cd third_party
curl -L https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools/+archive/33e02568181e3312f49a3cf33df470bf96ef293a.tar.gz > spirv-tools-src.tar.gz
mkdir spirv-tools-new
tar -xzf spirv-tools-src.tar.gz -C spirv-tools-new
rsync -r spirv-tools-new/ spirv-tools/ --delete
git restore spirv-tools/tnt/README.md
rm -rf spirv-tools/.github
rm -rf spirv-tools-new spirv-tools-src.tar.gz
git add spirv-tools
```