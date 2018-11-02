See also `Versioning.md` for a description of our three-tier versioning scheme.

## Publishing to npm

By pushing Filament's WebAssembly build to the npm registry, we can simplify the workflow for web
developers due to tools like [yarn]. This also has the benefit of making Filament available on CDN
servers like [unpkg] and [jsdelivr].

To publish a new package to npm, do the following.

1. Follow the instructions in the toplevel README for installing Emscripten.
2. Make sure you have an npm account and that your npm account belongs to the Google team.
3. Edit the version number in `package.json`. You cannot publish the same version more than once.
4. From the root folder in your Filament repo, do:
```
./build.sh -ap webgl release
./build.sh -ap all release
```
5. Make a commit and a tag whose label is the 3-tiered version number prefixed with a `v`:
```
git commit && git tag -a v{XX}.{YY}.{ZZ}
```
6. Push your change to GitHub, then make a GitHub Release associated with your new tag.
7. Perform a "dry run" of the npm packaging process:

```
cd out/cmake-webgl-release/web/filament-js
npm publish --dry-run
```
8. If the output of the dry run looks okay to you (double check the version number!), then finally
do `npm publish`.

[yarn]: https://yarnpkg.com
[unpkg]: https://unpkg.com
[jsdelivr]: https://www.jsdelivr.com/
