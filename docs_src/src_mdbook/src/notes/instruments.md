# Using Instruments on macOS

When running a binary under Instruments on macOS, you may run into the following issue when
launching or attaching to an executable:

```
Failed to gain authorization
Recovery Suggestion: Target binary needs to be debuggable and signed with 'get-task-allow'
```

This is a security precaution; the solution is to code sign the binary with the
`com.apple.security.get-task-allow` entitlement.

1. Create an `entitlements.plist` file with the following contents:

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.get-task-allow</key>
    <true/>
</dict>
</plist>
```

2. Run the following command:

```
codesign -s - --entitlements entitlements.plist <binary>
```

Replace `<binary>` with the name of the binary, for example: `out/cmake-debug/samples/gltf_viewer`.

Afterwards, you should be able to successfully launch and attach to the executable using
Instruments.
