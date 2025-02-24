# How to Contribute to Vulkan Source Repositories

The source code for The Vulkan-ValidationLayer components is sponsored by Khronos and LunarG.
While there are often active and organized development efforts underway to improve their coverage,
opportunities always exist for anyone to help by contributing additional validation layer checks
and tests.

## Incomplete VUIDs

There are some [VUID](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/validation_overview.adoc#valid-usage-id-vuid) that are incomplete and need to be added. The following can be used to find them
* [Incomplete tagged issues](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues?q=is%3Aopen+is%3Aissue+label%3AIncomplete)
* The `Coverage - html` page at [the Vulkan SDK documentation page](https://vulkan.lunarg.com/doc/sdk/latest/windows/validation_error_database.html)
  * it lists all published Vulkan VUIDs and their status.
* Run `scripts/vk_validation_stats.py` with `-todo` to see a list of as-yet unimplemented validation checks.
  * ```bash
    # Get summary report
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -summary
    # Some VUIDs are handled in `spirv-val` and need to pass in the repo to check against
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -summary
    # Print out all the information to an HTML page (also has text and csv support)
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -html vuid.html
    # -todo filters out only VUID that are unimplemented
    python3 scripts/vk_validation_stats.py external/Vulkan-Headers/registry/validusage.json -spirvtools ~/path/to/SPIRV-Tools/ -todo -html todo.html
    ```

Of course, if you have your own work in mind, please open an issue to describe it and assign it to yourself.
Finally, please feel free to contact any of the developers that are actively contributing should you
wish to coordinate further.

It is the maintainers goal for all issues to be assigned or `triaged` within one business day of their submission.
If you choose to work on an issue that is assigned, simply coordinate with the current assignee.

> triaged = decide if real issue, label it, assign it

## **How to Submit Fixes**

* **Ensure that the bug was not already reported or fixed** by searching on GitHub under Issues
  and Pull Requests.
* Use the existing GitHub forking and pull request process.
  This will involve [forking the repository](https://help.github.com/articles/fork-a-repo/),
  creating a branch with your commits, and then [submitting a pull request](https://help.github.com/articles/using-pull-requests/).
* Please read and adhere to the style and process guidelines enumerated below. Some highlights:
  - Source code must follow the repo coding style guidelines, including a pass through a clang-format utility
  - Implemented VUID checks must be accompanied by relevant tests
  - Validation source code should be in a separate commit from the tests, unless there are interdependencies. The repo should compile and
    pass all tests after each commit.
* Please base your fixes on the `main` branch. SDK branches are generally not updated except for critical fixes needed to repair an SDK release.
* The resulting Pull Request will be assigned to a repository maintainer. It is the maintainer's responsibility to ensure the Pull Request
  passes the Google/LunarG internal CI processes. Once the Pull Request has been approved and is passing internal CI, a repository maintainer
  will merge the PR.

### **Coding Conventions and Formatting**

* Use the **[Google style guide](https://google.github.io/styleguide/cppguide.html)** for source code with the following exceptions:
    * The column limit is 132 (as opposed to the default value 80). The clang-format tool will handle this. See below.
    * The indent is 4 spaces instead of the default 2 spaces. Access modifier (e.g. `public:`) is indented 2 spaces instead of the
      default 1 space. Again, the clang-format tool will handle this.
    * The C++ file extension is `*.cpp` instead of the default `*.cc`.
    * If you can justify a reason for violating a rule in the guidelines, then you are free to do so. Be prepared to defend your
decision during code review. This should be used responsibly. An example of a bad reason is "I don't like that rule." An example of
a good reason is "This violates the style guide, but it improves type safety."

> New code should target the above Google style guide, avoid copying/pasting incorrectly formatted code.

* For the [python generated code scripts](docs/generated_code.md), please follow the [python coding style guide](docs/python_scripts_code_style.md)

* Run **clang-format** on your changes to maintain consistent formatting
    * There are `.clang-format` files present in the repository to define clang-format settings
      which are found and used automatically by clang-format.
    * **clang-format** binaries are available from the LLVM orginization, here: [LLVM](https://clang.llvm.org/). Our CI system
      currently uses clang-format version `14` to check that the lines of code you have changed are formatted properly. It is
      recommended that you use the same version to format your code prior to submission.
    * A sample git workflow may look like:

>        # Make changes to the source.
>        $ git add -u .
>        $ git clang-format --style=file
>        # Check to see if clang-format made any changes and if they are OK.
>        $ git add -u .
>        $ git commit

`NOTE`: `scripts/check_code_format.py` will run clang-format for you and is required for passing CI.

* **Commit Messages**
    * Limit the subject line to 64 characters -- this allows the information to display correctly in git/GitHub logs
    * Begin subject line with a one-word component description followed by a colon (e.g. build, docs, layers, tests, etc.)
    * Separate subject from body with a blank line
    * Wrap the body at 72 characters
    * Capitalize the subject line
    * Do not end the subject line with a period
    * Use the body to explain what and why vs. how
    * Use the imperative mode in the subject line. This just means to write it as a command (e.g. Fix the sprocket)

`NOTE`: `scripts/check_code_format.py` will check your commit for you and is required for passing CI.

Strive for commits that implement a single or related set of functionality, using as many commits as is necessary (more is better).
That said, please ensure that the repository compiles and passes tests without error for each commit in your pull request.  Note
that to be accepted into the repository, the pull request must [pass all tests](#testing your changes) on all supported platforms
-- the continuous integration features will assist in enforcing this requirement.

### **Writing good error messages**

When writing an error message for `LogError` it is important to

1. Print values related to the error message
2. Explain the logic that got to that error

Example of a good error message

```cpp
if (render_pass == VK_NULL_HANDLE) {
    // ...
} else if (value != 0 && HasDepthFlag(flag)) {
    // print Render Pass object
    // Value is not the expected one, log it
    // List flag users used
    skip |= LogError(render_pass, "value is %" PRIu32 " but flag (%s) is missing VK_FLAG_DEPTH.", value, string_VkFlag(flag));
}
```

#### **Testing Your Changes**

* Run the included layer validation tests (`vk_layer_validation_tests`) in the repository before and after each of your commits to check for any regressions.

* Write additional layer validation tests that explicitly exercise your changes.

* Feel free to subject your code changes to other tests as well!

* [How to setup tests to run](./tests) and [overview for creating tests](docs/creating_tests.md).

`TIP`: It's ideal to test your changes in a fork and let Github Actions verify your changes before making a PR.

#### **Special Considerations for Validation Layers**
* **Validation Checks:**  Validation checks are carried out by the Khronos Validation layer. The CoreChecks validation object
contains checks that require significant amounts of application state to carry out. In contrast, the stateless validation object contains
checks that require (mostly) no state at all. Please inquire if you are unsure of the location for your contribution. The other
validation objects (thread_safety, object lifetimes) are more special-purpose and are mostly code-generated from the specification.
* **Validation Error/Warning Messages:**  Strive to give specific information describing the particulars of the failure, including
output all of the applicable Vulkan Objects and related values. Also, ensure that when messages can give suggestions about _how_ to
fix the problem, they should do so to better assist the user. Note that Vulkan object handles must be output via the `FormatHandle()`
function, and that all object handles visible in a message should also be included in the callback data.  If more than a single object is
output, the LogObjectList structure should be used.
* **Generated Source Code:** The `layers/vulkan/generated` directory contains source code that is created by several
generator scripts in the `scripts` directory. All changes to these scripts _must_ be submitted with the
corresponding generated output to keep the repository self-consistent. [Here for more information](docs/generated_code.md).

### **Contributor License Agreement (CLA)**

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

### **License and Copyrights**

All contributions made to the Vulkan-ValidationLayers repository are Khronos branded and as such,
any new files need to have the Khronos license (Apache 2.0 style) and copyright included.
Please see an existing file in this repository for an example.

All contributions made to the LunarG repositories are to be made under the Apache 2.0 license
and any new files need to include this license and any applicable copyrights.

You can include your individual copyright after any existing copyrights.
