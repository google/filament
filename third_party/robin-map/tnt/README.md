## Updating

To update to the robin-map that's currently on GitHub master, do the following from the third_party/ directory:

```
curl -L https://github.com/Tessil/robin-map/archive/master.zip > master.zip
unzip master.zip
rsync -r robin-map-master/include/tsl/ robin-map/tsl/ --delete
rsync -r robin-map-master/tests/ robin-map/tests/ --delete
cp robin-map-master/{CMakeLists.txt,LICENSE,README.md,appveyor.yml,doxygen.conf,.travis.yml} robin-map/
rm -rf master.zip robin-map-master/
git add robin-map
```
