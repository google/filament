This directory is no longer used. Test specs now entirely handled via Starlark
in //infra/config/global/. This directory still exists because:

1. A default (non-builder-specific) test spec directory needs to be defined for a
   Chromium config.
2. The Starlark-generated GN args need to be pointed to by an entry in
   `mb_config.pyl`
