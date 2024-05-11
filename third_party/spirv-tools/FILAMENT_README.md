When updating SPIRV-Tools to a new version, run the following from *this* directory

```
bash filament-update.sh [git commit hash]
```

This will pull in the updated source for SPRIV-Tools and pull in the right version of SPIRV-Headers.
The script will also try to apply Filament specific changes to the `CMakeLists.txt`.  It could be
that the diff application will fail, in which case, the updater will need to resolve the difference
manually and update `filament-specific-changes.patch`.

The above script will bring in the changes, but you would still need to add it to a git commit
(i.e. pull request) by doing

```
git add -u third_party/spriv-tools third_party/spirv-headers
```

from the Filament source root.

