- [Ubershader Archive Files](#ubershader-archive-files)
- [Ubershader Spec Files](#ubershader-spec-files)

# Ubershader Archive Files

An ubershader archive provides a way to bundle up a set of `filamat` files along with some metadata
that conveys which glTF features each material can handle. It is a file that has been compressed
with `zstd` and has an `.uberz` file extension. In uncompressed form, it has the following layout
(little endian is assumed).

```
[u32] magic identifier: UBER
[u32] simple (unpartitioned) version number for the archive format
[u64] number of specs
[u64] byte offset to SPECS
SPECS:
foreach spec {
    [u8] shading model
    [u8] blending model
    [u16] number of flags
    [u32] size in bytes of the filamat blob
    [u64] byte offset to FLAGLIST for this spec
    [u64] byte offset to FILAMAT for this spec
}
foreach spec {
    FLAGLIST:
    foreach flag {
        [u64] byte offset to FLAGNAME for this spec/flag pair
        [u64] flag value: 0 = unsupported, 1 = optional, or 2 = required
    }
}
foreach spec {
    foreach flag {
        FLAGNAME:
        [u8...] flag name, including null terminator
    }
}
foreach spec {
    FILAMAT:
    [u8...] filamat blob
}
```

In the above specification, each "offset" is a number of bytes between the top of the file to the
given label. These offsets are 64 bits so that they can be replaced with pointers in a C struct,
which allows the file to be consumed without any parsing. On 32-bit architectures, this still works
because we can simply ignore the unused padding after every pointer.

# Ubershader Spec Files

An ubershader spec file is a simple text file with a `.spec` extension. It contains a list of
key-value pairs conforming to the following grammar. Each key-value pair is either a *feature flag*
or a *fundamental aspect*.

- Each feature flag can be **unsupported**, **required**, or **optional**.
- The fundamental aspect of the material cannot be changed, such as the blend mode.

```eBNF
spec = { [ comment | key_value_pair ] , "\n" } ;
comment = "#" , { any } ;
key_value_pair = ( fundamental_aspect | feature_flag ) ;
fundamental_aspect = ( blending | shading ) ;
feature_flag = identifier , equals , ("unsupported" | "required" | "optional") ;
blending = "BlendingMode" , equals ,
    ( "opaque" | "transparent" | "fade" | "add" | "masked" | "multiply" | "screen" ) ;
shading = "ShadingModel"  , equals ,
    ( "lit" | "subsurface" | "cloth" | "unlit" | "specularGlossiness") ;
equals = [ whitespace ] , "=" , [ whitespace ] ;
any = ? any character other than newline ? ;
whitespace = ? sequence of tabs and spaces ? ;
identifier = ? sequence of alphanumeric characters ? ;
```

If a fundamental aspect is missing from the spec, then the loader will assume that the spec can
handle all possible values for that aspect. For example, we may wish to override the glTF
blending mode in certain ubershader materials (e.g. materials that support `KHR_materials_volume`).
These materials should simply omit the `BlendingMode` line from the spec.

If any feature flag is missing from the spec, it implicitly has the value of `unsupported`. For an
up-to-date list of recognized feature flags, look at the source for `UbershaderProvider::getMaterial`.

If a particular feature flag is set to `required` for a particular material, then the glTF loader
will bind that material to a given glTF mesh only if that feature is enabled in the mesh.

Usually, features are either `unsupported` or `optional`. For example, if the ubershader user can
set `normalIndex` in the material to `-1` to signal that they do not have a normal map, then normal
mapping should be specified as an `optional` feature of the ubershader.
