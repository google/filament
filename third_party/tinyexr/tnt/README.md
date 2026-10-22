To update to the tinyexr that's currently on GitHub master, do the following:

```
cd third_party
curl -L https://github.com/syoyo/tinyexr/archive/master.zip > master.zip
curl -L https://github.com/syoyo/tinyexr/archive/refs/tags/v3.2.0.zip > tinyexr.zip
unzip tinyexr.zip
rsync -r tinyexr-3.2.0/ tinyexr/ --delete --exclude tnt
rm -rf tinyexr/{.github,experimental,benchmark,doc,web,archive,tools/resize/tests,sandbox/tocio/tests,tools/texcomp/test,tools/texpipe/test,tools/envmap/test,deps/miniz/examples}
git checkout tinyexr/tnt/
git checkout tinyexr/LICENSE
rm -rf tinyexr-3.2.0 tinyexr.zip
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
