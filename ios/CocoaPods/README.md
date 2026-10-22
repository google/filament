# Filament CocoaPods

[CocoaPods](https://cocoapods.org/) is the dependency manager Filament uses for Apple platforms
(macOS and iOS).

## Installation

On macOS, CocoaPods requires a modern Ruby environment. Avoid using the system Ruby; instead,
install the latest Ruby version using `brew`, `port,` or use a manager like `rbenv` or `rvm`. Once
Ruby is configured:

```bash
gem install cocoapods
```

## Linting the Podspec

The Filament podspec is configured to fetch releases directly from GitHub. Before deployment,
CocoaPods must **lint** the spec to validate that the library compiles and links correctly across
all supported architectures.

To run the linter locally:

```bash
cd ios/CocoaPods/
pod spec lint
```

### Debugging Failures

If the lint fails, use the `--verbose` flag to inspect the underlying `xcodebuild` output:

```bash
pod spec lint --verbose
```

## Isolating Subspecs

Filament is partitioned into multiple **subspecs**. CocoaPods lints these individually. To
accelerate debugging or isolate a specific build failure (e.g., `Filament/filament`), use the
following command:

```bash
pod spec lint --no-clean --verbose --subspec=Filament/filament 2>&1 | tee lint_output.log
```

* `--no-clean`: Preserves the temporary Xcode workspace and build artifacts in `/var/folders/...`
  for inspection.
* `--verbose`: Prints the full compiler and linker invocations.
* `--subspec`: Limits the scope of the lint to a specific component.

> **Tip:** To view all defined subspecs, grep the podspec file:
> `grep "spec.subspec" ios/CocoaPods/Filament.podspec`


## Testing a Podspec Locally

Before pushing a new version to the CocoaPods trunk, you should verify the podspec in a sample project.

### Using the Sample Project

Filament includes a dedicated sample project located in `ios/samples/HelloCocoaPods`. To test your
local changes:

1. **Modify the Podfile**: Add a `:podspec` directive pointing to your local `.podspec` file.

```ruby
platform :ios, '13.0'

target 'HelloCocoaPods' do
    # Use the local development podspec
    pod 'Filament', :podspec => '../../CocoaPods/Filament.podspec'
end
```

2. **Sync Dependencies**: Run the installation command from the sample directory:

```bash
pod install
```

3. **Verify the Build**: Open the generated `HelloCocoaPods.xcworkspace` in Xcode. Build and run the
   target to ensure the headers are found and the binaries link correctly.
