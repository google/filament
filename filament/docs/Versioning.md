## Versioning

Filament uses a 3-number versioning scheme that superficially resembles a [semantic
version](https://semver.org/) but is actually more interesting because of our material system. Here
are the guidelines:

- Increment the **most significant** number only when making a non-backwards compatible API change,
  or when introducing a major new API.
- Increment the **middle number** only when making a non-backwards compatible change to the material
  system. When this number gets bumped, users need to rebuild their mat files. Reset the middle
  number to zero when the most significant number has been incremented.
- Increment the **least significant** number each time a new release is published. Reset this number
  to zero if one of the other two numbers have been incremented.

## Material Versioning

Additionally, the Filament renderer and material compiler internally contain a standalone integer
called `MATERIAL_VERSION`, defined in `MaterialEnums.h`. This should be incremented every time we
change the middle number in the public-facing version.

When a material version mismatch is detected at run time, a panic is triggered, even in release
builds. Therefore we should increment this only when making a serious breaking change to the
material system (e.g. changing the size of a uniform block). Cosmetic shader changes usually do
not merit a change to the material version number.

Currently our material archives have two version chunks, one for "normal" materials and one for
post-process materials. However for now these two numbers must be set to the same value.
