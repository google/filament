## Updating

```shell
cd third_party
mkdir libz_copy && cd libz_copy
git init
git fetch --depth=1 https://github.com/madler/zlib.git v1.3.1
git reset --hard FETCH_HEAD
find . -name .git -type d -print0 | xargs -0 rm -r
rm -rf .github
cp -r ../libz/tnt .
cd ..
rm -rf libz
mv libz_copy libz
git add libz
```
