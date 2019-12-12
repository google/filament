## Updating

To update to the Google Test that's currently on GitHub master, do the following.

```
curl -L https://github.com/google/googletest/archive/master.zip > master.zip
unzip master.zip
rsync -r googletest-master/ libgtest/ --delete
git checkout libgtest/tnt
rm -rf googletest-master master.zip
git add libgtest
```

Please be sure to run unit tests before uploading your PR:

```
./build.sh -u
```
