<!--
Copyright 2023 The Khronos Group Inc.
Copyright 2023 Valve Corporation
Copyright 2023 LunarG, Inc.

SPDX-License-Identifier: Apache-2.0
-->

# How to generate the code

- Linux:
```bash
scripts/generate_source.py external/Vulkan-Headers/registry/
```

- Windows Powershell:
```powershell
pwsh -Command { python3 scripts/generate_source.py external/Vulkan-Headers/registry/ }
```

- Windows Command:
```cmd
cmd /C "python3 scripts/generate_source.py external/Vulkan-Headers/registry/"
```

If only dealing with a single file,  run `scripts/generate_source.py` with `--target`

```bash
# Example - only generates chassis.h
scripts/generate_source.py external/Vulkan-Headers/registry/ --target vk_dispatch_table.h
```

When making change to the `scripts/` folder, make sure to run `generate_source.py`
(Code generation does **not** happen automatically at build time.)
