Contributing to libdrm
======================

Submitting Patches
------------------

Patches should be sent to dri-devel@lists.freedesktop.org, using git
send-email. For patches only touching driver specific code one of the driver
mailing lists (like amd-gfx@lists.freedesktop.org) is also appropriate. See git
documentation for help:

http://git-scm.com/documentation

Since dri-devel is a very busy mailing list please use --subject-prefix="PATCH
libdrm" to make it easier to find libdrm patches. This is best done by running

    git config --local format.subjectprefix "PATCH libdrm"

The first line of a commit message should contain a prefix indicating what part
is affected by the patch followed by one sentence that describes the change. For
examples:

    amdgpu: Use uint32_t i in amdgpu_find_bo_by_cpu_mapping

The body of the commit message should describe what the patch changes and why,
and also note any particular side effects. For a recommended reading on
writing commit messages, see:

http://who-t.blogspot.de/2009/12/on-commit-messages.html

Your patches should also include a Signed-off-by line with your name and email
address. If you're not the patch's original author, you should also gather
S-o-b's by them (and/or whomever gave the patch to you.) The significance of
this is that it certifies that you created the patch, that it was created under
an appropriate open source license, or provided to you under those terms.  This
lets us indicate a chain of responsibility for the copyright status of the code.
For more details:

https://developercertificate.org/

We won't reject patches that lack S-o-b, but it is strongly recommended.

Review and Merging
------------------

Patches should have at least one positive review (Reviewed-by: tag) or
indication of approval (Acked-by: tag) before merging. For any code shared
between drivers this is mandatory.

Please note that kernel/userspace API header files have special rules, see
include/drm/README.

Coding style in the project loosely follows the CodingStyle of the linux kernel:

https://www.kernel.org/doc/html/latest/process/coding-style.html?highlight=coding%20style

Commit Rights
-------------

Commit rights will be granted to anyone who requests them and fulfills the
below criteria:

- Submitted a few (5-10 as a rule of thumb) non-trivial (not just simple
  spelling fixes and whitespace adjustment) patches that have been merged
  already. Since libdrm is just a glue library between the kernel and userspace
  drivers, merged patches to those components also count towards the commit
  criteria.

- Are actively participating on discussions about their work (on the mailing
  list or IRC). This should not be interpreted as a requirement to review other
  peoples patches but just make sure that patch submission isn't one-way
  communication. Cross-review is still highly encouraged.

- Will be regularly contributing further patches. This includes regular
  contributors to other parts of the open source graphics stack who only
  do the oddball rare patch within libdrm itself.

- Agrees to use their commit rights in accordance with the documented merge
  criteria, tools, and processes.

To apply for commit rights ("Developer" role in gitlab) send a mail to
dri-devel@lists.freedesktop.org and please ping the maintainers if your request
is stuck.

Committers are encouraged to request their commit rights get removed when they
no longer contribute to the project. Commit rights will be reinstated when they
come back to the project.

Maintainers and committers should encourage contributors to request commit
rights, as especially junior contributors tend to underestimate their skills.

Code of Conduct
---------------

Please be aware the fd.o Code of Conduct also applies to libdrm:

https://www.freedesktop.org/wiki/CodeOfConduct/

See the gitlab project owners for contact details of the libdrm maintainers.

Abuse of commit rights, like engaging in commit fights or willfully pushing
patches that violate the documented merge criteria, will also be handled through
the Code of Conduct enforcement process.

Happy hacking!
