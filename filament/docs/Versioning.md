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
