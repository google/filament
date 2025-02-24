# How to Contribute to Vulkan Source Repositories

## **The Repository**

The source code for The Vulkan-Tools components is sponsored by Khronos and LunarG.
* [Khronos Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools)


Repository Issue labels:

* _Bug_:          These issues refer to invalid or broken functionality and are the highest priority.
* _Enhancement_:  These issues refer to ideas for extending or improving tools and utilities

It is the maintainers goal for all issues to be assigned within one business day of their submission. If you choose
to work on an issue that is assigned, simply coordinate with the current assignee.

### **How to Submit Fixes**

* **Ensure that the bug was not already reported or fixed** by searching on GitHub under Issues
  and Pull Requests.
* Use the existing GitHub forking and pull request process.
  This will involve [forking the repository](https://help.github.com/articles/fork-a-repo/),
  creating a branch with your commits, and then [submitting a pull request](https://help.github.com/articles/using-pull-requests/).
* Please read and adhere to the style and process [guidelines ](#coding-conventions-and-formatting) enumerated below.
* Please base your fixes on the main branch.  SDK branches are generally not updated except for critical fixes needed to repair an SDK release.
* The resulting Pull Request will be assigned to a repository maintainer. It is the maintainer's responsibility to ensure the Pull Request
  passes the Google/LunarG internal CI processes. Once the Pull Request has been approved and is passing internal CI, a repository maintainer
  will merge the PR.


#### **Coding Conventions and Formatting**
* Use the **[Google style guide](https://google.github.io/styleguide/cppguide.html)** for source code with the following exceptions:
    * The column limit is 132 (as opposed to the default value 80). The clang-format tool will handle this. See below.
    * The indent is 4 spaces instead of the default 2 spaces. Again, the clang-format tool will handle this.
    * If you can justify a reason for violating a rule in the guidelines, then you are free to do so. Be prepared to defend your
decision during code review. This should be used responsibly. An example of a bad reason is "I don't like that rule." An example of
a good reason is "This violates the style guide, but it improves type safety."

* Run **clang-format** on your changes to maintain consistent formatting
    * There are `.clang-format` files present in the repository to define clang-format settings
      which are found and used automatically by clang-format.
	* **clang-format** binaries are available from the LLVM orginization, here: [LLVM](https://clang.llvm.org/).
    * A sample git workflow may look like:

>        # Make changes to the source.
>        $ git add -u .
>        $ git clang-format --style=file
>        # Check to see if clang-format made any changes and if they are OK.
>        $ git add -u .
>        $ git commit

* **Commit Messages**
    * Limit the subject line to 50 characters -- this allows the information to display correctly in git/Github logs
    * Begin subject line with a one-word component description followed by a colon (e.g. loader, layers, tests, etc.)
    * Separate subject from body with a blank line
    * Wrap the body at 72 characters
    * Capitalize the subject line
    * Do not end the subject line with a period
    * Use the body to explain what and why vs. how
    * Use the imperative mode in the subject line. This just means to write it as a command (e.g. Fix the sprocket)

Strive for commits that implement a single or related set of functionality, using as many commits as is necessary (more is better).
That said, please ensure that the repository compiles and passes tests without error for each commit in your pull request.  Note
that to be accepted into the repository, the pull request must [pass all tests](#testing your changes) on all supported platforms

#### Generated Source Code

The `icd/generated` and `vulkaninfo/generated` directories contains source code that is created by several
generator scripts in the `scripts` directory. All changes to these scripts _must_ be submitted with the
corresponding generated output to keep the repository self-consistent. This requirement is enforced by CI.
Regenerate source files after modifying any of the generator scripts and before building and testing your changes.
More details can be found in [BUILD.md](BUILD.md).

#### **Testing Your Changes**
* Run the repository components with the Vulkan Validation Layers before and after each of your commits to check for any regressions.

  (These instructions are for Linux)
* In the `cube` directory, run:
>        vkcube
>        vkcube --validate
* In the `vulkaninfo` directory, run:
>        vulkaninfo
* If you are adding or changing JSON output, please read
  [Validating vulkaninfo JSON output](https://github.com/KhronosGroup/Vulkan-Tools/blob/main/vulkaninfo/json_validation_process.md).
  **Note:** When adding new output to vulkaninfo, do NOT add JSON output unless the formmatting is defined by a schema.
* Run tests that explicitly exercise your changes.
* Feel free to subject your code changes to other tests as well!

### **Contributor License Agreement (CLA)**

You will be prompted with a one-time "click-through" CLA dialog as part of submitting your pull request
or other contribution to GitHub.

### **License and Copyrights**

All contributions made to the Vulkan-Tools repository are Khronos branded and as such,
any new files need to have the Khronos license (Apache 2.0 style) and copyright included.
Please see an existing file in this repository for an example.

All contributions made to the LunarG repositories are to be made under the Apache 2.0 license
and any new files need to include this license and any applicable copyrights.

You can include your individual copyright after any existing copyrights.
