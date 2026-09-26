# Filament Release Notes log

**If you are merging a PR into main**: please add the release note below, under the *Release notes

**If you are cherry-picking a commit into an rc/ branch**: add the release note under the
appropriate header in [RELEASE_NOTES.md](./RELEASE_NOTES.md).

## Release notes for next branch cut

- engine: `Texture::setImage()`, `Texture::setImageAsync()` and `Texture::generateMipmaps()` are
  now non-const; the const overloads are deprecated
- engine: `Material::getDefaultInstance() const` is deprecated; use the non-const overload
- engine: `FrameHistoryStream::getNewFrames()` now consumes the new frames when it returns, rather
  than as the range is iterated: frames that aren't iterated are no longer returned by the next
  call, and a range can be iterated more than once. Missing frame IDs just before a PENDING frame
  are now returned with that frame [⚠️ **API Change**]
- engine: `Engine::Builder::build()` now also validates the `Engine::Config` when the engine is
  created asynchronously
- engine: fix the const `DebugRegistry::getPropertyAddress()` templates, which didn't compile
- engine: fix a dangling reference to the caller's string in `DebugRegistry::getDataSource()`
- libs: the `IBLPrefilterContext` filters (`EquirectangularToCubemap`, `IrradianceFilter` and
  `SpecularFilter`) now take a non-const input `Texture*`; the overloads taking a
  `Texture const*` are deprecated
