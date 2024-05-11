# Filament for Web

Filament is a mobile-first library for physically based rendering. It has a lightweight C++ core
made available to web developers via a WebAssembly module. The WASM file is bundled with a
first-class JavaScript API.

See the [web docs](https://github.com/google/filament/tree/main/web/docs) for more information.

## Publishing to npm

See [Versioning.md](https://github.com/google/filament/blob/main/filament/docs/Versioning.md)
for a description of Filament's three-tier versioning scheme.

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

:bangbang: | If there is a material change, be sure to update the live demos!
:---: | :---

```
cd out/cmake-webgl-release/web/filament-js
npm publish --dry-run
```
8. If the output of the dry run looks okay to you (double check the version number!), then finally
do `npm publish`.

[yarn]: https://yarnpkg.com
[unpkg]: https://unpkg.com
[jsdelivr]: https://www.jsdelivr.com/

9. Update the live drag-and-drop viewer as follows:

   1. Edit the pinned Filament version in the `<script>` tag in `docs/viewer/index.html`.
   2. Push the change to GitHub and test the site: `https://google.github.io/filament/viewer/`.
