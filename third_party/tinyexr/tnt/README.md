To update to the tinyexr that's currently on GitHub master, do the following:

```
cd third_party
curl -L https://github.com/syoyo/tinyexr/archive/master.zip > master.zip
unzip master.zip
rsync -r tinyexr-master/ tinyexr/ --delete
git checkout tinyexr/tnt/
git checkout tinyexr/LICENSE
rm -rf tinyexr-master master.zip
```

Remove the following directories to save space:

```
rm -rf tinyexr/examples
rm -rf tinyexr/jni
rm -rf tinyexr/test
```

Commit the update:
```
git add tinyexr
git commit -m "Update tinyexr to <SHA>"
```

Make sure the LICENSE file remains and is up to date (check top of tinyexr.h).
