Contributing to CivetWeb
====

Contributions to CivetWeb are welcome, provided all contributions carry the MIT license.

- Please report issues on GitHub. If the issue you want to report is already reported there, add a note with your specific details to that issue. In case of doubt, please create a new issue.
- If you know how to fix the issue, please create a pull request on GitHub. Please take care your modifications pass the continuous integration checks. These checks are performed automatically when you create a pull request, but it may take some hours until all tests are completed. Please provide a description for every pull request.
- Alternatively, you can post a patch or describe the required modifications in a GitHub issue.
However, a pull request would be preferred.
- Contributor names are listed in CREDITS.md, unless you explicitly state you don't want your name to be listed there. This file is occasionally updated, adding new contributors, using author names from git commits and GitHub comments.


- In case your modifications either
  1. modify or extend the API,
  2. affect multi-threading,
  3. imply structural changes,
  or
  4. have significant influence on maintenance,
  
  please first create an issue on GitHub or create a thread on the CivetWeb discussion group, to discuss the planned changed.

- In case you think you found a security issue that should be evaluated and fixed before public disclosure, feel free to write an email.  Although CivetWeb is a fork from Mongoose from 2013, the code bases are different now, so security vulnerabilities of Mongoose usually do not affect CivetWeb.  Open an issue for Mongoose vulnerabilities you want to have checked if CivetWeb is affected.


Why does a pull request need a description?
---

I'm asking for this, because I usually review all pull requests.
The first thing I check is: "What is the intention of the fix **according to the description**?" and "Does the code really fix it?".
Second: "Do I except side effects?".
Third: "Is there a better way to fix the issue **explained in the description**?"
I don't like to "reverse engineer" the intention of the fix from the diff (although it may be obvious to the author of the PR, sometimes it's not for others). But you should also do it for yourself: You will get early feedback if your changes are not doing what you expect, or if there is a much more effective way to reach the same goal. Finally it will help all other users, since it helps writing better release notes.
