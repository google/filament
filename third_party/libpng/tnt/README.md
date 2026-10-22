## Updating

```shell
cd third_party
mkdir libpng_copy && cd libpng_copy
git init
git fetch --depth=1 https://github.com/pnggroup/libpng.git v1.6.47
git reset --hard FETCH_HEAD
find . -name .git -type d -print0 | xargs -0 rm -r
rm -rf .github
rm -rf ci
cp -r ../libpng/tnt .
cp scripts/pnglibconf.h.prebuilt pnglibconf.h
cp scripts/makefile.system makefile
make test
make install
cd ..
rm -rf libpng
mv libpng_copy libpng
git add libpng
```
