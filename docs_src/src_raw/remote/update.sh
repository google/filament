#!/bin/bash

pushd "$(dirname "$0")" > /dev/null

curl -OL https://nightly.link/google/filament/workflows/web-continuous/main/filament-web.zip
unzip -q filament-web.zip
tar -xvzf filament-release-web.tgz
rm filament-release-web.tgz
rm filament-web.zip
rm filament.d.ts
cp ../../web/samples/remote.html index.html

popd

git status
echo ""
echo "All done! Next, make a git commit that updates docs/remote."
