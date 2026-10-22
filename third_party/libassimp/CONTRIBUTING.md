# How to contribute

If you want to contribute, follow these steps:

- First, create your own clone of assimp.
- When you want to fix a bug or add a new feature, create a branch on your own fork following [these instructions](https://help.github.com/articles/creating-a-pull-request-from-a-fork/).
- Push it to your fork of the repository and open a pull request.
- A pull request will start our continuous integration service, which checks if the build works for Linux and Windows.
  It will check for memory leaks, compiler warnings and memory alignment issues. If any of these tests fail, fix it and the tests will be restarted automatically.
  - At the end, we will perform a code review and merge your branch to the master branch.
