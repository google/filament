# Markdeep Rasterizer

This module can be used to convert [Markdeep](https://casual-effects.com/markdeep/) files
to static HTML.

# Usage

This module is meant to be executed directly with `npx`:

```bash
npx markdeep-rasterize <file1> <file2> ... <fileN> <outDirectory>
```

Where each `<fileX>` is a path to a Markdeep file ending in `.md.html`. The output `.html` files
will be generated in the specified output directory.

Here is a real-world example from [Filament](https://github.com/google/filament):

```bash
npx markdeep-rasterizer ../../docs/Filament.md.html ../../docs/Materials.md.html  ../../docs/
```

# License

Apache License
Version 2.0, January 2004
