# Running clang-tidy

* Add `"checkout_clang_tidy": True` to `.gclient` file in the `custom_vars`.
  ```
  {
    "custom_vars": {
      "checkout_clang_tidy": True,
    }
  }
  ```
* `gclient sync`

There should now be `third_party/llvm-build/Release+Asserts/bin/clang-tidy`

* `cd out`
* `git clone https://chromium.googlesource.com/chromium/tools/build`

The Chromium build folder contains the `tricium` files used to run `clang-tidy`

Running clang-tidy over all the source can be done with:

```
cd ..
out/build/recipes/recipe_modules/tricium_clang_tidy/resources/tricium_clang_tidy_script.py \
--base_path $PWD \
--out_dir out/Debug \
--findings_file all_findings.json \
--clang_tidy_binary $PWD/third_party/llvm-build/Release+Asserts/bin/clang-tidy \
--all
```

`--all` can be replaced by specific files if desired to run on individual source
files.


## References
* https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/clang_tidy.md
