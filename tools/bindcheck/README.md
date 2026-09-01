# bindcheck

Detects drift between Filament's C++ public API and its JavaScript and Java bindings.

Most of Filament's bindings are hand-written (`tools/beamsplitter` generates only the option
structs in `Options.h`). Nothing keeps the rest in sync with the headers, so methods and enum
values quietly go missing. This tool compares the surfaces and reports what has drifted.

    pip3 install libclang
    python3 tools/bindcheck/bindcheck.py

Runs in well under a second; no build, no `compile_commands.json`. Exit code is 1 when any
finding survives the exclusions, so it can gate a presubmit.

## Surfaces

| surface | files | role |
| --- | --- | --- |
| `cpp` | `filament/include/filament/*.h` | source of truth, parsed with libclang |
| `js` | `web/filament-js/jsbindings.cpp`, `jsenums*.cpp`, `extensions*.js` | what embind registers plus the hand-written wrappers |
| `dts` | `web/filament-js/filament.d.ts` | what TypeScript users are told exists |
| `java` | `android/filament-android/src/main/java/com/google/android/filament/*.java` | the Java API |

All four name nested types differently, so `Outer$Inner` -- embind's registered name -- is the
canonical key throughout.

## Checks

    python3 tools/bindcheck/bindcheck.py --list-checks

| check | what it means |
| --- | --- |
| `unbound-js` / `unbound-java` | public C++ method with no counterpart in that binding |
| `unbound-class-js` / `-java` | class bound in one language but entirely absent from the other |
| `undeclared-ts` | reachable from JavaScript but missing from `filament.d.ts` |
| `phantom-ts` | declared in `filament.d.ts` but bound nowhere; fails at runtime |
| `nonpublic-java` | bound in Java, but declared without `public`, so no app can call it |
| `enum-values-js` / `-java` | C++ enumerator missing from a binding |
| `enum-order-java` | **Java ordinal does not match the C++ value** |
| `enum-name-java` | Java declares a constant C++ does not have |

`nonpublic-java` is the one that found real bugs fastest. `View.setShadowingEnabled` is
public while `isShadowingEnabled` is not, so an Android app can turn shadowing on and never
read it back; `setChannelDepthClearEnabled` and its getter are both package-private, so the
feature is unreachable in either direction. All of them carry javadoc with `@return` tags,
which is written for public API -- these read as an omitted keyword, not a decision. A
member it reports is skipped by `unbound-java`: it is bound, just not reachable, which is a
different bug with a different fix.

`enum-order-java` is the one worth wiring into CI first. Java passes `enum.ordinal()` across JNI
and the C++ side casts the int straight back to the enum:

```java
nBuilderFormat(mNativeBuilder, format.ordinal());          // Texture.java
```
```cpp
builder->format((Texture::InternalFormat) format);         // cpp/Texture.cpp
```

So inserting, dropping or reordering a Java constant still compiles, still runs, and silently
selects a different C++ value. Nothing else in the build catches that.

The comparison is against each C++ constant's *value*, never its declaration position --
`AttachmentPoint`'s `COLOR = COLOR0` is declared last but is worth 0, and Java constants may
carry an explicit value of their own (`LINE_STRIP(3)`).

## Exclusions

Deliberate divergence is normal here: the JS API is not meant to mirror C++ one-to-one. Add an
entry to `exclusions.json`; a reason is required and `--explain` prints it, which keeps the file
a record of decisions rather than a pile of silenced noise.

```json
{
  "check": "enum-order-java",
  "key": "Fence$FenceStatus",
  "members": ["ERROR"],
  "reason": "Java maps the native int through an explicit switch, never ordinal()."
}
```

Omit `members` to exclude a whole class, omit `key` to disable a check, omit `check` (or use
`"*"`) to apply the entry to every check. `"key": "*"` covers a member name wherever it appears,
which is how members inherited by many classes -- `Builder::name`, `Builder::async` -- are
recorded once instead of once per class, and `key` also takes a list, because a divergence is
usually a convention rather than a one-off.

## What is compared

Methods and enumerators. Public C++ *fields* are not compared: the 350-odd of them are almost all
in the option structs, which `tools/beamsplitter` generates into both `Options.h` and
`extensions_generated.js` from one description, so they cannot drift the way hand-written members
do. `filament.d.ts` fields are read, because embind's `.property()` bindings (`KtxInfo.glType`)
are declared as fields there and would otherwise look undeclared.

## Adding a check

Write a function taking the four surfaces and returning a list of `Finding`, then append it to
`CHECKS`. Nothing else changes.

## Before trusting a new finding

Every false alarm during development came from the text extractors, not the checks -- a single
dropped enum constant shifts every ordinal after it and looks exactly like a real JNI bug. Run
`--self-test`, which pins the cases that actually bit, and confirm a finding against the source
before filing it.
