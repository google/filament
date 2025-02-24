Update binaries:

1. Download archives pre-compiled binaries.
2. Unzip archives.
3. Re-zip just the binary:
   `zip new.zip node-v10.14.1-linux-x64/bin/node`
4. Use the update script:
   `./dependency_manager/bin/update --config
   common/node_runner/node_runner/node_binaries.json --dependency node --path
   new.zip --platform linux_x86_64`
5. Mail out the automated change to `node_binaries.json` for review and CQ.
