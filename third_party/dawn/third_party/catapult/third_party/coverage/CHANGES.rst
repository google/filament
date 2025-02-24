.. Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
.. For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

==============================
Change history for Coverage.py
==============================


Version 4.0.3, 24 November 2015
-------------------------------

- Fixed a mysterious problem that manifested in different ways: sometimes
  hanging the process (`issue 420`_), sometimes making database connections
  fail (`issue 445`_).

- The XML report now has correct ``<source>`` elements when using a
  ``--source=`` option somewhere besides the current directory.  This fixes
  `issue 439`_. Thanks, Arcady Ivanov.

- Fixed an unusual edge case of detecting source encodings, described in
  `issue 443`_.

- Help messages that mention the command to use now properly use the actual
  command name, which might be different than "coverage".  Thanks to Ben Finney,
  this closes `issue 438`_.

.. _issue 420: https://bitbucket.org/ned/coveragepy/issues/420/coverage-40-hangs-indefinitely-on-python27
.. _issue 438: https://bitbucket.org/ned/coveragepy/issues/438/parameterise-coverage-command-name
.. _issue 439: https://bitbucket.org/ned/coveragepy/issues/439/incorrect-cobertura-file-sources-generated
.. _issue 443: https://bitbucket.org/ned/coveragepy/issues/443/coverage-gets-confused-when-encoding
.. _issue 445: https://bitbucket.org/ned/coveragepy/issues/445/django-app-cannot-connect-to-cassandra


Version 4.0.2 --- 4 November 2015
---------------------------------

- More work on supporting unusually encoded source. Fixed `issue 431`_.

- Files or directories with non-ASCII characters are now handled properly,
  fixing `issue 432`_.

- Setting a trace function with sys.settrace was broken by a change in 4.0.1,
  as reported in `issue 436`_.  This is now fixed.

- Officially support PyPy 4.0, which required no changes, just updates to the
  docs.

.. _issue 431: https://bitbucket.org/ned/coveragepy/issues/431/couldnt-parse-python-file-with-cp1252
.. _issue 432: https://bitbucket.org/ned/coveragepy/issues/432/path-with-unicode-characters-various
.. _issue 436: https://bitbucket.org/ned/coveragepy/issues/436/disabled-coverage-ctracer-may-rise-from


Version 4.0.1 --- 13 October 2015
---------------------------------

- When combining data files, unreadable files will now generate a warning
  instead of failing the command.  This is more in line with the older
  coverage.py v3.7.1 behavior, which silently ignored unreadable files.
  Prompted by `issue 418`_.

- The --skip-covered option would skip reporting on 100% covered files, but
  also skipped them when calculating total coverage.  This was wrong, it should
  only remove lines from the report, not change the final answer.  This is now
  fixed, closing `issue 423`_.

- In 4.0, the data file recorded a summary of the system on which it was run.
  Combined data files would keep all of those summaries.  This could lead to
  enormous data files consisting of mostly repetitive useless information. That
  summary is now gone, fixing `issue 415`_.  If you want summary information,
  get in touch, and we'll figure out a better way to do it.

- Test suites that mocked os.path.exists would experience strange failures, due
  to coverage.py using their mock inadvertently.  This is now fixed, closing
  `issue 416`_.

- Importing a ``__init__`` module explicitly would lead to an error:
  ``AttributeError: 'module' object has no attribute '__path__'``, as reported
  in `issue 410`_.  This is now fixed.

- Code that uses ``sys.settrace(sys.gettrace())`` used to incur a more than 2x
  speed penalty.  Now there's no penalty at all. Fixes `issue 397`_.

- Pyexpat C code will no longer be recorded as a source file, fixing
  `issue 419`_.

- The source kit now contains all of the files needed to have a complete source
  tree, re-fixing `issue 137`_ and closing `issue 281`_.

.. _issue 281: https://bitbucket.org/ned/coveragepy/issues/281/supply-scripts-for-testing-in-the
.. _issue 397: https://bitbucket.org/ned/coveragepy/issues/397/stopping-and-resuming-coverage-with
.. _issue 410: https://bitbucket.org/ned/coveragepy/issues/410/attributeerror-module-object-has-no
.. _issue 415: https://bitbucket.org/ned/coveragepy/issues/415/repeated-coveragedataupdates-cause
.. _issue 416: https://bitbucket.org/ned/coveragepy/issues/416/mocking-ospathexists-causes-failures
.. _issue 418: https://bitbucket.org/ned/coveragepy/issues/418/json-parse-error
.. _issue 419: https://bitbucket.org/ned/coveragepy/issues/419/nosource-no-source-for-code-path-to-c
.. _issue 423: https://bitbucket.org/ned/coveragepy/issues/423/skip_covered-changes-reported-total


Version 4.0 --- 20 September 2015
---------------------------------

No changes from 4.0b3


Version 4.0b3 --- 7 September 2015
----------------------------------

- Reporting on an unmeasured file would fail with a traceback.  This is now
  fixed, closing `issue 403`_.

- The Jenkins ShiningPanda plugin looks for an obsolete file name to find the
  HTML reports to publish, so it was failing under coverage.py 4.0.  Now we
  create that file if we are running under Jenkins, to keep things working
  smoothly. `issue 404`_.

- Kits used to include tests and docs, but didn't install them anywhere, or
  provide all of the supporting tools to make them useful.  Kits no longer
  include tests and docs.  If you were using them from the older packages, get
  in touch and help me understand how.

.. _issue 403: https://bitbucket.org/ned/coveragepy/issues/403/hasherupdate-fails-with-typeerror-nonetype
.. _issue 404: https://bitbucket.org/ned/coveragepy/issues/404/shiningpanda-jenkins-plugin-cant-find-html



Version 4.0b2 --- 22 August 2015
--------------------------------

- 4.0b1 broke --append creating new data files.  This is now fixed, closing
  `issue 392`_.

- ``py.test --cov`` can write empty data, then touch files due to ``--source``,
  which made coverage.py mistakenly force the data file to record lines instead
  of arcs.  This would lead to a "Can't combine line data with arc data" error
  message.  This is now fixed, and changed some method names in the
  CoverageData interface.  Fixes `issue 399`_.

- `CoverageData.read_fileobj` and `CoverageData.write_fileobj` replace the
  `.read` and `.write` methods, and are now properly inverses of each other.

- When using ``report --skip-covered``, a message will now be included in the
  report output indicating how many files were skipped, and if all files are
  skipped, coverage.py won't accidentally scold you for having no data to
  report.  Thanks, Krystian Kichewko.

- A new conversion utility has been added:  ``python -m coverage.pickle2json``
  will convert v3.x pickle data files to v4.x JSON data files.  Thanks,
  Alexander Todorov.  Closes `issue 395`_.

- A new version identifier is available, `coverage.version_info`, a plain tuple
  of values similar to `sys.version_info`_.

.. _issue 392: https://bitbucket.org/ned/coveragepy/issues/392/run-append-doesnt-create-coverage-file
.. _issue 395: https://bitbucket.org/ned/coveragepy/issues/395/rfe-read-pickled-files-as-well-for
.. _issue 399: https://bitbucket.org/ned/coveragepy/issues/399/coverageexception-cant-combine-line-data
.. _sys.version_info: https://docs.python.org/3/library/sys.html#sys.version_info


Version 4.0b1 --- 2 August 2015
-------------------------------

- Coverage.py is now licensed under the Apache 2.0 license.  See NOTICE.txt for
  details.  Closes `issue 313`_.

- The data storage has been completely revamped.  The data file is now
  JSON-based instead of a pickle, closing `issue 236`_.  The `CoverageData`
  class is now a public supported documented API to the data file.

- A new configuration option, ``[run] note``, lets you set a note that will be
  stored in the `runs` section of the data file.  You can use this to annotate
  the data file with any information you like.

- Unrecognized configuration options will now print an error message and stop
  coverage.py.  This should help prevent configuration mistakes from passing
  silently.  Finishes `issue 386`_.

- In parallel mode, ``coverage erase`` will now delete all of the data files,
  fixing `issue 262`_.

- Coverage.py now accepts a directory name for ``coverage run`` and will run a
  ``__main__.py`` found there, just like Python will.  Fixes `issue 252`_.
  Thanks, Dmitry Trofimov.

- The XML report now includes a ``missing-branches`` attribute.  Thanks, Steve
  Peak.  This is not a part of the Cobertura DTD, so the XML report no longer
  references the DTD.

- Missing branches in the HTML report now have a bit more information in the
  right-hand annotations.  Hopefully this will make their meaning clearer.

- All the reporting functions now behave the same if no data had been
  collected, exiting with a status code of 1.  Fixed ``fail_under`` to be
  applied even when the report is empty.  Thanks, Ionel Cristian Mărieș.

- Plugins are now initialized differently.  Instead of looking for a class
  called ``Plugin``, coverage.py looks for a function called ``coverage_init``.

- A file-tracing plugin can now ask to have built-in Python reporting by
  returning `"python"` from its `file_reporter()` method.

- Code that was executed with `exec` would be mis-attributed to the file that
  called it.  This is now fixed, closing `issue 380`_.

- The ability to use item access on `Coverage.config` (introduced in 4.0a2) has
  been changed to a more explicit `Coverage.get_option` and
  `Coverage.set_option` API.

- The ``Coverage.use_cache`` method is no longer supported.

- The private method ``Coverage._harvest_data`` is now called
  ``Coverage.get_data``, and returns the ``CoverageData`` containing the
  collected data.

- The project is consistently referred to as "coverage.py" throughout the code
  and the documentation, closing `issue 275`_.

- Combining data files with an explicit configuration file was broken in 4.0a6,
  but now works again, closing `issue 385`_.

- ``coverage combine`` now accepts files as well as directories.

- The speed is back to 3.7.1 levels, after having slowed down due to plugin
  support, finishing up `issue 387`_.

.. _issue 236: https://bitbucket.org/ned/coveragepy/issues/236/pickles-are-bad-and-you-should-feel-bad
.. _issue 252: https://bitbucket.org/ned/coveragepy/issues/252/coverage-wont-run-a-program-with
.. _issue 262: https://bitbucket.org/ned/coveragepy/issues/262/when-parallel-true-erase-should-erase-all
.. _issue 275: https://bitbucket.org/ned/coveragepy/issues/275/refer-consistently-to-project-as-coverage
.. _issue 313: https://bitbucket.org/ned/coveragepy/issues/313/add-license-file-containing-2-3-or-4
.. _issue 380: https://bitbucket.org/ned/coveragepy/issues/380/code-executed-by-exec-excluded-from
.. _issue 385: https://bitbucket.org/ned/coveragepy/issues/385/coverage-combine-doesnt-work-with-rcfile
.. _issue 386: https://bitbucket.org/ned/coveragepy/issues/386/error-on-unrecognised-configuration
.. _issue 387: https://bitbucket.org/ned/coveragepy/issues/387/performance-degradation-from-371-to-40

.. 40 issues closed in 4.0 below here


Version 4.0a6 --- 21 June 2015
------------------------------

- Python 3.5b2 and PyPy 2.6.0 are supported.

- The original module-level function interface to coverage.py is no longer
  supported.  You must now create a ``coverage.Coverage`` object, and use
  methods on it.

- The ``coverage combine`` command now accepts any number of directories as
  arguments, and will combine all the data files from those directories.  This
  means you don't have to copy the files to one directory before combining.
  Thanks, Christine Lytwynec.  Finishes `issue 354`_.

- Branch coverage couldn't properly handle certain extremely long files. This
  is now fixed (`issue 359`_).

- Branch coverage didn't understand yield statements properly.  Mickie Betz
  persisted in pursuing this despite Ned's pessimism.  Fixes `issue 308`_ and
  `issue 324`_.

- The COVERAGE_DEBUG environment variable can be used to set the ``[run] debug``
  configuration option to control what internal operations are logged.

- HTML reports were truncated at formfeed characters.  This is now fixed
  (`issue 360`_).  It's always fun when the problem is due to a `bug in the
  Python standard library <http://bugs.python.org/issue19035>`_.

- Files with incorrect encoding declaration comments are no longer ignored by
  the reporting commands, fixing `issue 351`_.

- HTML reports now include a timestamp in the footer, closing `issue 299`_.
  Thanks, Conrad Ho.

- HTML reports now begrudgingly use double-quotes rather than single quotes,
  because there are "software engineers" out there writing tools that read HTML
  and somehow have no idea that single quotes exist.  Capitulates to the absurd
  `issue 361`_.  Thanks, Jon Chappell.

- The ``coverage annotate`` command now handles non-ASCII characters properly,
  closing `issue 363`_.  Thanks, Leonardo Pistone.

- Drive letters on Windows were not normalized correctly, now they are. Thanks,
  Ionel Cristian Mărieș.

- Plugin support had some bugs fixed, closing `issue 374`_ and `issue 375`_.
  Thanks, Stefan Behnel.

.. _issue 299: https://bitbucket.org/ned/coveragepy/issue/299/inserted-created-on-yyyy-mm-dd-hh-mm-in
.. _issue 308: https://bitbucket.org/ned/coveragepy/issue/308/yield-lambda-branch-coverage
.. _issue 324: https://bitbucket.org/ned/coveragepy/issue/324/yield-in-loop-confuses-branch-coverage
.. _issue 351: https://bitbucket.org/ned/coveragepy/issue/351/files-with-incorrect-encoding-are-ignored
.. _issue 354: https://bitbucket.org/ned/coveragepy/issue/354/coverage-combine-should-take-a-list-of
.. _issue 359: https://bitbucket.org/ned/coveragepy/issue/359/xml-report-chunk-error
.. _issue 360: https://bitbucket.org/ned/coveragepy/issue/360/html-reports-get-confused-by-l-in-the-code
.. _issue 361: https://bitbucket.org/ned/coveragepy/issue/361/use-double-quotes-in-html-output-to
.. _issue 363: https://bitbucket.org/ned/coveragepy/issue/363/annotate-command-hits-unicode-happy-fun
.. _issue 374: https://bitbucket.org/ned/coveragepy/issue/374/c-tracer-lookups-fail-in
.. _issue 375: https://bitbucket.org/ned/coveragepy/issue/375/ctracer_handle_return-reads-byte-code


Version 4.0a5 --- 16 February 2015
----------------------------------

- Plugin support is now implemented in the C tracer instead of the Python
  tracer. This greatly improves the speed of tracing projects using plugins.

- Coverage.py now always adds the current directory to sys.path, so that
  plugins can import files in the current directory (`issue 358`_).

- If the `config_file` argument to the Coverage constructor is specified as
  ".coveragerc", it is treated as if it were True.  This means setup.cfg is
  also examined, and a missing file is not considered an error (`issue 357`_).

- Wildly experimental: support for measuring processes started by the
  multiprocessing module.  To use, set ``--concurrency=multiprocessing``,
  either on the command line or in the .coveragerc file (`issue 117`_). Thanks,
  Eduardo Schettino.  Currently, this does not work on Windows.

- A new warning is possible, if a desired file isn't measured because it was
  imported before coverage.py was started (`issue 353`_).

- The `coverage.process_startup` function now will start coverage measurement
  only once, no matter how many times it is called.  This fixes problems due
  to unusual virtualenv configurations (`issue 340`_).

- Added 3.5.0a1 to the list of supported CPython versions.

.. _issue 117: https://bitbucket.org/ned/coveragepy/issue/117/enable-coverage-measurement-of-code-run-by
.. _issue 340: https://bitbucket.org/ned/coveragepy/issue/340/keyerror-subpy
.. _issue 353: https://bitbucket.org/ned/coveragepy/issue/353/40a3-introduces-an-unexpected-third-case
.. _issue 357: https://bitbucket.org/ned/coveragepy/issue/357/behavior-changed-when-coveragerc-is
.. _issue 358: https://bitbucket.org/ned/coveragepy/issue/358/all-coverage-commands-should-adjust


Version 4.0a4 --- 25 January 2015
---------------------------------

- Plugins can now provide sys_info for debugging output.

- Started plugins documentation.

- Prepared to move the docs to readthedocs.org.


Version 4.0a3 --- 20 January 2015
---------------------------------

- Reports now use file names with extensions.  Previously, a report would
  describe a/b/c.py as "a/b/c".  Now it is shown as "a/b/c.py".  This allows
  for better support of non-Python files, and also fixed `issue 69`_.

- The XML report now reports each directory as a package again.  This was a bad
  regression, I apologize.  This was reported in `issue 235`_, which is now
  fixed.

- A new configuration option for the XML report: ``[xml] package_depth``
  controls which directories are identified as packages in the report.
  Directories deeper than this depth are not reported as packages.
  The default is that all directories are reported as packages.
  Thanks, Lex Berezhny.

- When looking for the source for a frame, check if the file exists. On
  Windows, .pyw files are no longer recorded as .py files. Along the way, this
  fixed `issue 290`_.

- Empty files are now reported as 100% covered in the XML report, not 0%
  covered (`issue 345`_).

- Regexes in the configuration file are now compiled as soon as they are read,
  to provide error messages earlier (`issue 349`_).

.. _issue 69: https://bitbucket.org/ned/coveragepy/issue/69/coverage-html-overwrite-files-that-doesnt
.. _issue 235: https://bitbucket.org/ned/coveragepy/issue/235/package-name-is-missing-in-xml-report
.. _issue 290: https://bitbucket.org/ned/coveragepy/issue/290/running-programmatically-with-pyw-files
.. _issue 345: https://bitbucket.org/ned/coveragepy/issue/345/xml-reports-line-rate-0-for-empty-files
.. _issue 349: https://bitbucket.org/ned/coveragepy/issue/349/bad-regex-in-config-should-get-an-earlier


Version 4.0a2 --- 14 January 2015
---------------------------------

- Officially support PyPy 2.4, and PyPy3 2.4.  Drop support for
  CPython 3.2 and older versions of PyPy.  The code won't work on CPython 3.2.
  It will probably still work on older versions of PyPy, but I'm not testing
  against them.

- Plugins!

- The original command line switches (`-x` to run a program, etc) are no
  longer supported.

- A new option: `coverage report --skip-covered` will reduce the number of
  files reported by skipping files with 100% coverage.  Thanks, Krystian
  Kichewko.  This means that empty `__init__.py` files will be skipped, since
  they are 100% covered, closing `issue 315`_.

- You can now specify the ``--fail-under`` option in the ``.coveragerc`` file
  as the ``[report] fail_under`` option.  This closes `issue 314`_.

- The ``COVERAGE_OPTIONS`` environment variable is no longer supported.  It was
  a hack for ``--timid`` before configuration files were available.

- The HTML report now has filtering.  Type text into the Filter box on the
  index page, and only modules with that text in the name will be shown.
  Thanks, Danny Allen.

- The textual report and the HTML report used to report partial branches
  differently for no good reason.  Now the text report's "missing branches"
  column is a "partial branches" column so that both reports show the same
  numbers.  This closes `issue 342`_.

- If you specify a ``--rcfile`` that cannot be read, you will get an error
  message.  Fixes `issue 343`_.

- The ``--debug`` switch can now be used on any command.

- You can now programmatically adjust the configuration of coverage.py by
  setting items on `Coverage.config` after construction.

- A module run with ``-m`` can be used as the argument to ``--source``, fixing
  `issue 328`_.  Thanks, Buck Evan.

- The regex for matching exclusion pragmas has been fixed to allow more kinds
  of whitespace, fixing `issue 334`_.

- Made some PyPy-specific tweaks to improve speed under PyPy.  Thanks, Alex
  Gaynor.

- In some cases, with a source file missing a final newline, coverage.py would
  count statements incorrectly.  This is now fixed, closing `issue 293`_.

- The status.dat file that HTML reports use to avoid re-creating files that
  haven't changed is now a JSON file instead of a pickle file.  This obviates
  `issue 287`_ and `issue 237`_.

.. _issue 237: https://bitbucket.org/ned/coveragepy/issue/237/htmlcov-with-corrupt-statusdat
.. _issue 287: https://bitbucket.org/ned/coveragepy/issue/287/htmlpy-doesnt-specify-pickle-protocol
.. _issue 293: https://bitbucket.org/ned/coveragepy/issue/293/number-of-statement-detection-wrong-if-no
.. _issue 314: https://bitbucket.org/ned/coveragepy/issue/314/fail_under-param-not-working-in-coveragerc
.. _issue 315: https://bitbucket.org/ned/coveragepy/issue/315/option-to-omit-empty-files-eg-__init__py
.. _issue 328: https://bitbucket.org/ned/coveragepy/issue/328/misbehavior-in-run-source
.. _issue 334: https://bitbucket.org/ned/coveragepy/issue/334/pragma-not-recognized-if-tab-character
.. _issue 342: https://bitbucket.org/ned/coveragepy/issue/342/console-and-html-coverage-reports-differ
.. _issue 343: https://bitbucket.org/ned/coveragepy/issue/343/an-explicitly-named-non-existent-config


Version 4.0a1 --- 27 September 2014
-----------------------------------

- Python versions supported are now CPython 2.6, 2.7, 3.2, 3.3, and 3.4, and
  PyPy 2.2.

- Gevent, eventlet, and greenlet are now supported, closing `issue 149`_.
  The ``concurrency`` setting specifies the concurrency library in use.  Huge
  thanks to Peter Portante for initial implementation, and to Joe Jevnik for
  the final insight that completed the work.

- Options are now also read from a setup.cfg file, if any.  Sections are
  prefixed with "coverage:", so the ``[run]`` options will be read from the
  ``[coverage:run]`` section of setup.cfg.  Finishes `issue 304`_.

- The ``report -m`` command can now show missing branches when reporting on
  branch coverage.  Thanks, Steve Leonard. Closes `issue 230`_.

- The XML report now contains a <source> element, fixing `issue 94`_.  Thanks
  Stan Hu.

- The class defined in the coverage module is now called ``Coverage`` instead
  of ``coverage``, though the old name still works, for backward compatibility.

- The ``fail-under`` value is now rounded the same as reported results,
  preventing paradoxical results, fixing `issue 284`_.

- The XML report will now create the output directory if need be, fixing
  `issue 285`_.  Thanks, Chris Rose.

- HTML reports no longer raise UnicodeDecodeError if a Python file has
  undecodable characters, fixing `issue 303`_ and `issue 331`_.

- The annotate command will now annotate all files, not just ones relative to
  the current directory, fixing `issue 57`_.

- The coverage module no longer causes deprecation warnings on Python 3.4 by
  importing the imp module, fixing `issue 305`_.

- Encoding declarations in source files are only considered if they are truly
  comments.  Thanks, Anthony Sottile.

.. _issue 57: https://bitbucket.org/ned/coveragepy/issue/57/annotate-command-fails-to-annotate-many
.. _issue 94: https://bitbucket.org/ned/coveragepy/issue/94/coverage-xml-doesnt-produce-sources
.. _issue 149: https://bitbucket.org/ned/coveragepy/issue/149/coverage-gevent-looks-broken
.. _issue 230: https://bitbucket.org/ned/coveragepy/issue/230/show-line-no-for-missing-branches-in
.. _issue 284: https://bitbucket.org/ned/coveragepy/issue/284/fail-under-should-show-more-precision
.. _issue 285: https://bitbucket.org/ned/coveragepy/issue/285/xml-report-fails-if-output-file-directory
.. _issue 303: https://bitbucket.org/ned/coveragepy/issue/303/unicodedecodeerror
.. _issue 304: https://bitbucket.org/ned/coveragepy/issue/304/attempt-to-get-configuration-from-setupcfg
.. _issue 305: https://bitbucket.org/ned/coveragepy/issue/305/pendingdeprecationwarning-the-imp-module
.. _issue 331: https://bitbucket.org/ned/coveragepy/issue/331/failure-of-encoding-detection-on-python2


Version 3.7.1 --- 13 December 2013
----------------------------------

- Improved the speed of HTML report generation by about 20%.

- Fixed the mechanism for finding OS-installed static files for the HTML report
  so that it will actually find OS-installed static files.


Version 3.7 --- 6 October 2013
------------------------------

- Added the ``--debug`` switch to ``coverage run``.  It accepts a list of
  options indicating the type of internal activity to log to stderr.

- Improved the branch coverage facility, fixing `issue 92`_ and `issue 175`_.

- Running code with ``coverage run -m`` now behaves more like Python does,
  setting sys.path properly, which fixes `issue 207`_ and `issue 242`_.

- Coverage.py can now run .pyc files directly, closing `issue 264`_.

- Coverage.py properly supports .pyw files, fixing `issue 261`_.

- Omitting files within a tree specified with the ``source`` option would
  cause them to be incorrectly marked as unexecuted, as described in
  `issue 218`_.  This is now fixed.

- When specifying paths to alias together during data combining, you can now
  specify relative paths, fixing `issue 267`_.

- Most file paths can now be specified with username expansion (``~/src``, or
  ``~build/src``, for example), and with environment variable expansion
  (``build/$BUILDNUM/src``).

- Trying to create an XML report with no files to report on, would cause a
  ZeroDivideError, but no longer does, fixing `issue 250`_.

- When running a threaded program under the Python tracer, coverage.py no
  longer issues a spurious warning about the trace function changing: "Trace
  function changed, measurement is likely wrong: None."  This fixes `issue
  164`_.

- Static files necessary for HTML reports are found in system-installed places,
  to ease OS-level packaging of coverage.py.  Closes `issue 259`_.

- Source files with encoding declarations, but a blank first line, were not
  decoded properly.  Now they are.  Thanks, Roger Hu.

- The source kit now includes the ``__main__.py`` file in the root coverage
  directory, fixing `issue 255`_.

.. _issue 92: https://bitbucket.org/ned/coveragepy/issue/92/finally-clauses-arent-treated-properly-in
.. _issue 164: https://bitbucket.org/ned/coveragepy/issue/164/trace-function-changed-warning-when-using
.. _issue 175: https://bitbucket.org/ned/coveragepy/issue/175/branch-coverage-gets-confused-in-certain
.. _issue 207: https://bitbucket.org/ned/coveragepy/issue/207/run-m-cannot-find-module-or-package-in
.. _issue 242: https://bitbucket.org/ned/coveragepy/issue/242/running-a-two-level-package-doesnt-work
.. _issue 218: https://bitbucket.org/ned/coveragepy/issue/218/run-command-does-not-respect-the-omit-flag
.. _issue 250: https://bitbucket.org/ned/coveragepy/issue/250/uncaught-zerodivisionerror-when-generating
.. _issue 255: https://bitbucket.org/ned/coveragepy/issue/255/directory-level-__main__py-not-included-in
.. _issue 259: https://bitbucket.org/ned/coveragepy/issue/259/allow-use-of-system-installed-third-party
.. _issue 261: https://bitbucket.org/ned/coveragepy/issue/261/pyw-files-arent-reported-properly
.. _issue 264: https://bitbucket.org/ned/coveragepy/issue/264/coverage-wont-run-pyc-files
.. _issue 267: https://bitbucket.org/ned/coveragepy/issue/267/relative-path-aliases-dont-work


Version 3.6 --- 5 January 2013
------------------------------

- Added a page to the docs about troublesome situations, closing `issue 226`_,
  and added some info to the TODO file, closing `issue 227`_.

.. _issue 226: https://bitbucket.org/ned/coveragepy/issue/226/make-readme-section-to-describe-when
.. _issue 227: https://bitbucket.org/ned/coveragepy/issue/227/update-todo


Version 3.6b3 --- 29 December 2012
----------------------------------

- Beta 2 broke the nose plugin. It's fixed again, closing `issue 224`_.

.. _issue 224: https://bitbucket.org/ned/coveragepy/issue/224/36b2-breaks-nosexcover


Version 3.6b2 --- 23 December 2012
----------------------------------

- Coverage.py runs on Python 2.3 and 2.4 again. It was broken in 3.6b1.

- The C extension is optionally compiled using a different more widely-used
  technique, taking another stab at fixing `issue 80`_ once and for all.

- Combining data files would create entries for phantom files if used with
  ``source`` and path aliases.  It no longer does.

- ``debug sys`` now shows the configuration file path that was read.

- If an oddly-behaved package claims that code came from an empty-string
  file name, coverage.py no longer associates it with the directory name,
  fixing `issue 221`_.

.. _issue 221: https://bitbucket.org/ned/coveragepy/issue/221/coveragepy-incompatible-with-pyratemp


Version 3.6b1 --- 28 November 2012
----------------------------------

- Wildcards in ``include=`` and ``omit=`` arguments were not handled properly
  in reporting functions, though they were when running.  Now they are handled
  uniformly, closing `issue 143`_ and `issue 163`_.  **NOTE**: it is possible
  that your configurations may now be incorrect.  If you use ``include`` or
  ``omit`` during reporting, whether on the command line, through the API, or
  in a configuration file, please check carefully that you were not relying on
  the old broken behavior.

- The **report**, **html**, and **xml** commands now accept a ``--fail-under``
  switch that indicates in the exit status whether the coverage percentage was
  less than a particular value.  Closes `issue 139`_.

- The reporting functions coverage.report(), coverage.html_report(), and
  coverage.xml_report() now all return a float, the total percentage covered
  measurement.

- The HTML report's title can now be set in the configuration file, with the
  ``--title`` switch on the command line, or via the API.

- Configuration files now support substitution of environment variables, using
  syntax like ``${WORD}``.  Closes `issue 97`_.

- Embarrassingly, the ``[xml] output=`` setting in the .coveragerc file simply
  didn't work.  Now it does.

- The XML report now consistently uses file names for the file name attribute,
  rather than sometimes using module names.  Fixes `issue 67`_.
  Thanks, Marcus Cobden.

- Coverage percentage metrics are now computed slightly differently under
  branch coverage.  This means that completely unexecuted files will now
  correctly have 0% coverage, fixing `issue 156`_.  This also means that your
  total coverage numbers will generally now be lower if you are measuring
  branch coverage.

- When installing, now in addition to creating a "coverage" command, two new
  aliases are also installed.  A "coverage2" or "coverage3" command will be
  created, depending on whether you are installing in Python 2.x or 3.x.
  A "coverage-X.Y" command will also be created corresponding to your specific
  version of Python.  Closes `issue 111`_.

- The coverage.py installer no longer tries to bootstrap setuptools or
  Distribute.  You must have one of them installed first, as `issue 202`_
  recommended.

- The coverage.py kit now includes docs (closing `issue 137`_) and tests.

- On Windows, files are now reported in their correct case, fixing `issue 89`_
  and `issue 203`_.

- If a file is missing during reporting, the path shown in the error message
  is now correct, rather than an incorrect path in the current directory.
  Fixes `issue 60`_.

- Running an HTML report in Python 3 in the same directory as an old Python 2
  HTML report would fail with a UnicodeDecodeError. This issue (`issue 193`_)
  is now fixed.

- Fixed yet another error trying to parse non-Python files as Python, this
  time an IndentationError, closing `issue 82`_ for the fourth time...

- If `coverage xml` fails because there is no data to report, it used to
  create a zero-length XML file.  Now it doesn't, fixing `issue 210`_.

- Jython files now work with the ``--source`` option, fixing `issue 100`_.

- Running coverage.py under a debugger is unlikely to work, but it shouldn't
  fail with "TypeError: 'NoneType' object is not iterable".  Fixes `issue
  201`_.

- On some Linux distributions, when installed with the OS package manager,
  coverage.py would report its own code as part of the results.  Now it won't,
  fixing `issue 214`_, though this will take some time to be repackaged by the
  operating systems.

- Docstrings for the legacy singleton methods are more helpful.  Thanks Marius
  Gedminas.  Closes `issue 205`_.

- The pydoc tool can now show documentation for the class `coverage.coverage`.
  Closes `issue 206`_.

- Added a page to the docs about contributing to coverage.py, closing
  `issue 171`_.

- When coverage.py ended unsuccessfully, it may have reported odd errors like
  ``'NoneType' object has no attribute 'isabs'``.  It no longer does,
  so kiss `issue 153`_ goodbye.

.. _issue 60: https://bitbucket.org/ned/coveragepy/issue/60/incorrect-path-to-orphaned-pyc-files
.. _issue 67: https://bitbucket.org/ned/coveragepy/issue/67/xml-report-filenames-may-be-generated
.. _issue 89: https://bitbucket.org/ned/coveragepy/issue/89/on-windows-all-packages-are-reported-in
.. _issue 97: https://bitbucket.org/ned/coveragepy/issue/97/allow-environment-variables-to-be
.. _issue 100: https://bitbucket.org/ned/coveragepy/issue/100/source-directive-doesnt-work-for-packages
.. _issue 111: https://bitbucket.org/ned/coveragepy/issue/111/when-installing-coverage-with-pip-not
.. _issue 137: https://bitbucket.org/ned/coveragepy/issue/137/provide-docs-with-source-distribution
.. _issue 139: https://bitbucket.org/ned/coveragepy/issue/139/easy-check-for-a-certain-coverage-in-tests
.. _issue 143: https://bitbucket.org/ned/coveragepy/issue/143/omit-doesnt-seem-to-work-in-coverage
.. _issue 153: https://bitbucket.org/ned/coveragepy/issue/153/non-existent-filename-triggers
.. _issue 156: https://bitbucket.org/ned/coveragepy/issue/156/a-completely-unexecuted-file-shows-14
.. _issue 163: https://bitbucket.org/ned/coveragepy/issue/163/problem-with-include-and-omit-filename
.. _issue 171: https://bitbucket.org/ned/coveragepy/issue/171/how-to-contribute-and-run-tests
.. _issue 193: https://bitbucket.org/ned/coveragepy/issue/193/unicodedecodeerror-on-htmlpy
.. _issue 201: https://bitbucket.org/ned/coveragepy/issue/201/coverage-using-django-14-with-pydb-on
.. _issue 202: https://bitbucket.org/ned/coveragepy/issue/202/get-rid-of-ez_setuppy-and
.. _issue 203: https://bitbucket.org/ned/coveragepy/issue/203/duplicate-filenames-reported-when-filename
.. _issue 205: https://bitbucket.org/ned/coveragepy/issue/205/make-pydoc-coverage-more-friendly
.. _issue 206: https://bitbucket.org/ned/coveragepy/issue/206/pydoc-coveragecoverage-fails-with-an-error
.. _issue 210: https://bitbucket.org/ned/coveragepy/issue/210/if-theres-no-coverage-data-coverage-xml
.. _issue 214: https://bitbucket.org/ned/coveragepy/issue/214/coveragepy-measures-itself-on-precise


Version 3.5.3 --- 29 September 2012
-----------------------------------

- Line numbers in the HTML report line up better with the source lines, fixing
  `issue 197`_, thanks Marius Gedminas.

- When specifying a directory as the source= option, the directory itself no
  longer needs to have a ``__init__.py`` file, though its sub-directories do,
  to be considered as source files.

- Files encoded as UTF-8 with a BOM are now properly handled, fixing
  `issue 179`_.  Thanks, Pablo Carballo.

- Fixed more cases of non-Python files being reported as Python source, and
  then not being able to parse them as Python.  Closes `issue 82`_ (again).
  Thanks, Julian Berman.

- Fixed memory leaks under Python 3, thanks, Brett Cannon. Closes `issue 147`_.

- Optimized .pyo files may not have been handled correctly, `issue 195`_.
  Thanks, Marius Gedminas.

- Certain unusually named file paths could have been mangled during reporting,
  `issue 194`_.  Thanks, Marius Gedminas.

- Try to do a better job of the impossible task of detecting when we can't
  build the C extension, fixing `issue 183`_.

- Testing is now done with `tox`_, thanks, Marc Abramowitz.

.. _issue 147: https://bitbucket.org/ned/coveragepy/issue/147/massive-memory-usage-by-ctracer
.. _issue 179: https://bitbucket.org/ned/coveragepy/issue/179/htmlreporter-fails-when-source-file-is
.. _issue 183: https://bitbucket.org/ned/coveragepy/issue/183/install-fails-for-python-23
.. _issue 194: https://bitbucket.org/ned/coveragepy/issue/194/filelocatorrelative_filename-could-mangle
.. _issue 195: https://bitbucket.org/ned/coveragepy/issue/195/pyo-file-handling-in-codeunit
.. _issue 197: https://bitbucket.org/ned/coveragepy/issue/197/line-numbers-in-html-report-do-not-align
.. _tox: http://tox.readthedocs.org/


Version 3.5.2 --- 4 May 2012
----------------------------

No changes since 3.5.2.b1


Version 3.5.2b1 --- 29 April 2012
---------------------------------

- The HTML report has slightly tweaked controls: the buttons at the top of
  the page are color-coded to the source lines they affect.

- Custom CSS can be applied to the HTML report by specifying a CSS file as
  the ``extra_css`` configuration value in the ``[html]`` section.

- Source files with custom encodings declared in a comment at the top are now
  properly handled during reporting on Python 2.  Python 3 always handled them
  properly.  This fixes `issue 157`_.

- Backup files left behind by editors are no longer collected by the source=
  option, fixing `issue 168`_.

- If a file doesn't parse properly as Python, we don't report it as an error
  if the file name seems like maybe it wasn't meant to be Python.  This is a
  pragmatic fix for `issue 82`_.

- The ``-m`` switch on ``coverage report``, which includes missing line numbers
  in the summary report, can now be specified as ``show_missing`` in the
  config file.  Closes `issue 173`_.

- When running a module with ``coverage run -m <modulename>``, certain details
  of the execution environment weren't the same as for
  ``python -m <modulename>``.  This had the unfortunate side-effect of making
  ``coverage run -m unittest discover`` not work if you had tests in a
  directory named "test".  This fixes `issue 155`_ and `issue 142`_.

- Now the exit status of your product code is properly used as the process
  status when running ``python -m coverage run ...``.  Thanks, JT Olds.

- When installing into pypy, we no longer attempt (and fail) to compile
  the C tracer function, closing `issue 166`_.

.. _issue 142: https://bitbucket.org/ned/coveragepy/issue/142/executing-python-file-syspath-is-replaced
.. _issue 155: https://bitbucket.org/ned/coveragepy/issue/155/cant-use-coverage-run-m-unittest-discover
.. _issue 157: https://bitbucket.org/ned/coveragepy/issue/157/chokes-on-source-files-with-non-utf-8
.. _issue 166: https://bitbucket.org/ned/coveragepy/issue/166/dont-try-to-compile-c-extension-on-pypy
.. _issue 168: https://bitbucket.org/ned/coveragepy/issue/168/dont-be-alarmed-by-emacs-droppings
.. _issue 173: https://bitbucket.org/ned/coveragepy/issue/173/theres-no-way-to-specify-show-missing-in


Version 3.5.1 --- 23 September 2011
-----------------------------------

- The ``[paths]`` feature unfortunately didn't work in real world situations
  where you wanted to, you know, report on the combined data.  Now all paths
  stored in the combined file are canonicalized properly.


Version 3.5.1b1 --- 28 August 2011
----------------------------------

- When combining data files from parallel runs, you can now instruct
  coverage.py about which directories are equivalent on different machines.  A
  ``[paths]`` section in the configuration file lists paths that are to be
  considered equivalent.  Finishes `issue 17`_.

- for-else constructs are understood better, and don't cause erroneous partial
  branch warnings.  Fixes `issue 122`_.

- Branch coverage for ``with`` statements is improved, fixing `issue 128`_.

- The number of partial branches reported on the HTML summary page was
  different than the number reported on the individual file pages.  This is
  now fixed.

- An explicit include directive to measure files in the Python installation
  wouldn't work because of the standard library exclusion.  Now the include
  directive takes precedence, and the files will be measured.  Fixes
  `issue 138`_.

- The HTML report now handles Unicode characters in Python source files
  properly.  This fixes `issue 124`_ and `issue 144`_. Thanks, Devin
  Jeanpierre.

- In order to help the core developers measure the test coverage of the
  standard library, Brandon Rhodes devised an aggressive hack to trick Python
  into running some coverage.py code before anything else in the process.
  See the coverage/fullcoverage directory if you are interested.

.. _issue 17: http://bitbucket.org/ned/coveragepy/issue/17/support-combining-coverage-data-from
.. _issue 122: http://bitbucket.org/ned/coveragepy/issue/122/for-else-always-reports-missing-branch
.. _issue 124: http://bitbucket.org/ned/coveragepy/issue/124/no-arbitrary-unicode-in-html-reports-in
.. _issue 128: http://bitbucket.org/ned/coveragepy/issue/128/branch-coverage-of-with-statement-in-27
.. _issue 138: http://bitbucket.org/ned/coveragepy/issue/138/include-should-take-precedence-over-is
.. _issue 144: http://bitbucket.org/ned/coveragepy/issue/144/failure-generating-html-output-for


Version 3.5 --- 29 June 2011
----------------------------

- The HTML report hotkeys now behave slightly differently when the current
  chunk isn't visible at all:  a chunk on the screen will be selected,
  instead of the old behavior of jumping to the literal next chunk.
  The hotkeys now work in Google Chrome.  Thanks, Guido van Rossum.


Version 3.5b1 --- 5 June 2011
-----------------------------

- The HTML report now has hotkeys.  Try ``n``, ``s``, ``m``, ``x``, ``b``,
  ``p``, and ``c`` on the overview page to change the column sorting.
  On a file page, ``r``, ``m``, ``x``, and ``p`` toggle the run, missing,
  excluded, and partial line markings.  You can navigate the highlighted
  sections of code by using the ``j`` and ``k`` keys for next and previous.
  The ``1`` (one) key jumps to the first highlighted section in the file,
  and ``0`` (zero) scrolls to the top of the file.

- The ``--omit`` and ``--include`` switches now interpret their values more
  usefully.  If the value starts with a wildcard character, it is used as-is.
  If it does not, it is interpreted relative to the current directory.
  Closes `issue 121`_.

- Partial branch warnings can now be pragma'd away.  The configuration option
  ``partial_branches`` is a list of regular expressions.  Lines matching any of
  those expressions will never be marked as a partial branch.  In addition,
  there's a built-in list of regular expressions marking statements which should
  never be marked as partial.  This list includes ``while True:``, ``while 1:``,
  ``if 1:``, and ``if 0:``.

- The ``coverage()`` constructor accepts single strings for the ``omit=`` and
  ``include=`` arguments, adapting to a common error in programmatic use.

- Modules can now be run directly using ``coverage run -m modulename``, to
  mirror Python's ``-m`` flag.  Closes `issue 95`_, thanks, Brandon Rhodes.

- ``coverage run`` didn't emulate Python accurately in one small detail: the
  current directory inserted into ``sys.path`` was relative rather than
  absolute. This is now fixed.

- HTML reporting is now incremental: a record is kept of the data that
  produced the HTML reports, and only files whose data has changed will
  be generated.  This should make most HTML reporting faster.

- Pathological code execution could disable the trace function behind our
  backs, leading to incorrect code measurement.  Now if this happens,
  coverage.py will issue a warning, at least alerting you to the problem.
  Closes `issue 93`_.  Thanks to Marius Gedminas for the idea.

- The C-based trace function now behaves properly when saved and restored
  with ``sys.gettrace()`` and ``sys.settrace()``.  This fixes `issue 125`_
  and `issue 123`_.  Thanks, Devin Jeanpierre.

- Source files are now opened with Python 3.2's ``tokenize.open()`` where
  possible, to get the best handling of Python source files with encodings.
  Closes `issue 107`_, thanks, Brett Cannon.

- Syntax errors in supposed Python files can now be ignored during reporting
  with the ``-i`` switch just like other source errors.  Closes `issue 115`_.

- Installation from source now succeeds on machines without a C compiler,
  closing `issue 80`_.

- Coverage.py can now be run directly from a working tree by specifying
  the directory name to python:  ``python coverage_py_working_dir run ...``.
  Thanks, Brett Cannon.

- A little bit of Jython support: `coverage run` can now measure Jython
  execution by adapting when $py.class files are traced. Thanks, Adi Roiban.
  Jython still doesn't provide the Python libraries needed to make
  coverage reporting work, unfortunately.

- Internally, files are now closed explicitly, fixing `issue 104`_.  Thanks,
  Brett Cannon.

.. _issue 80: https://bitbucket.org/ned/coveragepy/issue/80/is-there-a-duck-typing-way-to-know-we-cant
.. _issue 93: http://bitbucket.org/ned/coveragepy/issue/93/copying-a-mock-object-breaks-coverage
.. _issue 95: https://bitbucket.org/ned/coveragepy/issue/95/run-subcommand-should-take-a-module-name
.. _issue 104: https://bitbucket.org/ned/coveragepy/issue/104/explicitly-close-files
.. _issue 107: https://bitbucket.org/ned/coveragepy/issue/107/codeparser-not-opening-source-files-with
.. _issue 115: https://bitbucket.org/ned/coveragepy/issue/115/fail-gracefully-when-reporting-on-file
.. _issue 121: https://bitbucket.org/ned/coveragepy/issue/121/filename-patterns-are-applied-stupidly
.. _issue 123: https://bitbucket.org/ned/coveragepy/issue/123/pyeval_settrace-used-in-way-that-breaks
.. _issue 125: https://bitbucket.org/ned/coveragepy/issue/125/coverage-removes-decoratortoolss-tracing


Version 3.4 --- 19 September 2010
---------------------------------

- The XML report is now sorted by package name, fixing `issue 88`_.

- Programs that exited with ``sys.exit()`` with no argument weren't handled
  properly, producing a coverage.py stack trace.  That is now fixed.

.. _issue 88: http://bitbucket.org/ned/coveragepy/issue/88/xml-report-lists-packages-in-random-order


Version 3.4b2 --- 6 September 2010
----------------------------------

- Completely unexecuted files can now be included in coverage results, reported
  as 0% covered.  This only happens if the --source option is specified, since
  coverage.py needs guidance about where to look for source files.

- The XML report output now properly includes a percentage for branch coverage,
  fixing `issue 65`_ and `issue 81`_.

- Coverage percentages are now displayed uniformly across reporting methods.
  Previously, different reports could round percentages differently.  Also,
  percentages are only reported as 0% or 100% if they are truly 0 or 100, and
  are rounded otherwise.  Fixes `issue 41`_ and `issue 70`_.

- The precision of reported coverage percentages can be set with the
  ``[report] precision`` config file setting.  Completes `issue 16`_.

- Threads derived from ``threading.Thread`` with an overridden `run` method
  would report no coverage for the `run` method.  This is now fixed, closing
  `issue 85`_.

.. _issue 16: http://bitbucket.org/ned/coveragepy/issue/16/allow-configuration-of-accuracy-of-percentage-totals
.. _issue 41: http://bitbucket.org/ned/coveragepy/issue/41/report-says-100-when-it-isnt-quite-there
.. _issue 65: http://bitbucket.org/ned/coveragepy/issue/65/branch-option-not-reported-in-cobertura
.. _issue 70: http://bitbucket.org/ned/coveragepy/issue/70/text-report-and-html-report-disagree-on-coverage
.. _issue 81: http://bitbucket.org/ned/coveragepy/issue/81/xml-report-does-not-have-condition-coverage-attribute-for-lines-with-a
.. _issue 85: http://bitbucket.org/ned/coveragepy/issue/85/threadrun-isnt-measured


Version 3.4b1 --- 21 August 2010
--------------------------------

- BACKWARD INCOMPATIBILITY: the ``--omit`` and ``--include`` switches now take
  file patterns rather than file prefixes, closing `issue 34`_ and `issue 36`_.

- BACKWARD INCOMPATIBILITY: the `omit_prefixes` argument is gone throughout
  coverage.py, replaced with `omit`, a list of file name patterns suitable for
  `fnmatch`.  A parallel argument `include` controls what files are included.

- The run command now has a ``--source`` switch, a list of directories or
  module names.  If provided, coverage.py will only measure execution in those
  source files.

- Various warnings are printed to stderr for problems encountered during data
  measurement: if a ``--source`` module has no Python source to measure, or is
  never encountered at all, or if no data is collected.

- The reporting commands (report, annotate, html, and xml) now have an
  ``--include`` switch to restrict reporting to modules matching those file
  patterns, similar to the existing ``--omit`` switch. Thanks, Zooko.

- The run command now supports ``--include`` and ``--omit`` to control what
  modules it measures. This can speed execution and reduce the amount of data
  during reporting. Thanks Zooko.

- Since coverage.py 3.1, using the Python trace function has been slower than
  it needs to be.  A cache of tracing decisions was broken, but has now been
  fixed.

- Python 2.7 and 3.2 have introduced new opcodes that are now supported.

- Python files with no statements, for example, empty ``__init__.py`` files,
  are now reported as having zero statements instead of one.  Fixes `issue 1`_.

- Reports now have a column of missed line counts rather than executed line
  counts, since developers should focus on reducing the missed lines to zero,
  rather than increasing the executed lines to varying targets.  Once
  suggested, this seemed blindingly obvious.

- Line numbers in HTML source pages are clickable, linking directly to that
  line, which is highlighted on arrival.  Added a link back to the index page
  at the bottom of each HTML page.

- Programs that call ``os.fork`` will properly collect data from both the child
  and parent processes.  Use ``coverage run -p`` to get two data files that can
  be combined with ``coverage combine``.  Fixes `issue 56`_.

- Coverage.py is now runnable as a module: ``python -m coverage``.  Thanks,
  Brett Cannon.

- When measuring code running in a virtualenv, most of the system library was
  being measured when it shouldn't have been.  This is now fixed.

- Doctest text files are no longer recorded in the coverage data, since they
  can't be reported anyway.  Fixes `issue 52`_ and `issue 61`_.

- Jinja HTML templates compile into Python code using the HTML file name,
  which confused coverage.py.  Now these files are no longer traced, fixing
  `issue 82`_.

- Source files can have more than one dot in them (foo.test.py), and will be
  treated properly while reporting.  Fixes `issue 46`_.

- Source files with DOS line endings are now properly tokenized for syntax
  coloring on non-DOS machines.  Fixes `issue 53`_.

- Unusual code structure that confused exits from methods with exits from
  classes is now properly analyzed.  See `issue 62`_.

- Asking for an HTML report with no files now shows a nice error message rather
  than a cryptic failure ('int' object is unsubscriptable). Fixes `issue 59`_.

.. _issue 1:  http://bitbucket.org/ned/coveragepy/issue/1/empty-__init__py-files-are-reported-as-1-executable
.. _issue 34: http://bitbucket.org/ned/coveragepy/issue/34/enhanced-omit-globbing-handling
.. _issue 36: http://bitbucket.org/ned/coveragepy/issue/36/provide-regex-style-omit
.. _issue 46: http://bitbucket.org/ned/coveragepy/issue/46
.. _issue 53: http://bitbucket.org/ned/coveragepy/issue/53
.. _issue 52: http://bitbucket.org/ned/coveragepy/issue/52/doctesttestfile-confuses-source-detection
.. _issue 56: http://bitbucket.org/ned/coveragepy/issue/56
.. _issue 61: http://bitbucket.org/ned/coveragepy/issue/61/annotate-i-doesnt-work
.. _issue 62: http://bitbucket.org/ned/coveragepy/issue/62
.. _issue 59: http://bitbucket.org/ned/coveragepy/issue/59/html-report-fails-with-int-object-is
.. _issue 82: http://bitbucket.org/ned/coveragepy/issue/82/tokenerror-when-generating-html-report


Version 3.3.1 --- 6 March 2010
------------------------------

- Using `parallel=True` in .coveragerc file prevented reporting, but now does
  not, fixing `issue 49`_.

- When running your code with "coverage run", if you call `sys.exit()`,
  coverage.py will exit with that status code, fixing `issue 50`_.

.. _issue 49: http://bitbucket.org/ned/coveragepy/issue/49
.. _issue 50: http://bitbucket.org/ned/coveragepy/issue/50


Version 3.3 --- 24 February 2010
--------------------------------

- Settings are now read from a .coveragerc file.  A specific file can be
  specified on the command line with --rcfile=FILE.  The name of the file can
  be programmatically set with the `config_file` argument to the coverage()
  constructor, or reading a config file can be disabled with
  `config_file=False`.

- Fixed a problem with nested loops having their branch possibilities
  mischaracterized: `issue 39`_.

- Added coverage.process_start to enable coverage measurement when Python
  starts.

- Parallel data file names now have a random number appended to them in
  addition to the machine name and process id.

- Parallel data files combined with "coverage combine" are deleted after
  they're combined, to clean up unneeded files.  Fixes `issue 40`_.

- Exceptions thrown from product code run with "coverage run" are now displayed
  without internal coverage.py frames, so the output is the same as when the
  code is run without coverage.py.

- The `data_suffix` argument to the coverage constructor is now appended with
  an added dot rather than simply appended, so that .coveragerc files will not
  be confused for data files.

- Python source files that don't end with a newline can now be executed, fixing
  `issue 47`_.

- Added an AUTHORS.txt file.

.. _issue 39: http://bitbucket.org/ned/coveragepy/issue/39
.. _issue 40: http://bitbucket.org/ned/coveragepy/issue/40
.. _issue 47: http://bitbucket.org/ned/coveragepy/issue/47


Version 3.2 --- 5 December 2009
-------------------------------

- Added a ``--version`` option on the command line.


Version 3.2b4 --- 1 December 2009
---------------------------------

- Branch coverage improvements:

  - The XML report now includes branch information.

- Click-to-sort HTML report columns are now persisted in a cookie.  Viewing
  a report will sort it first the way you last had a coverage report sorted.
  Thanks, `Chris Adams`_.

- On Python 3.x, setuptools has been replaced by `Distribute`_.

.. _Distribute: http://packages.python.org/distribute/


Version 3.2b3 --- 23 November 2009
----------------------------------

- Fixed a memory leak in the C tracer that was introduced in 3.2b1.

- Branch coverage improvements:

  - Branches to excluded code are ignored.

- The table of contents in the HTML report is now sortable: click the headers
  on any column.  Thanks, `Chris Adams`_.

.. _Chris Adams: http://improbable.org/chris/


Version 3.2b2 --- 19 November 2009
----------------------------------

- Branch coverage improvements:

  - Classes are no longer incorrectly marked as branches: `issue 32`_.

  - "except" clauses with types are no longer incorrectly marked as branches:
    `issue 35`_.

- Fixed some problems syntax coloring sources with line continuations and
  source with tabs: `issue 30`_ and `issue 31`_.

- The --omit option now works much better than before, fixing `issue 14`_ and
  `issue 33`_.  Thanks, Danek Duvall.

.. _issue 14: http://bitbucket.org/ned/coveragepy/issue/14
.. _issue 30: http://bitbucket.org/ned/coveragepy/issue/30
.. _issue 31: http://bitbucket.org/ned/coveragepy/issue/31
.. _issue 32: http://bitbucket.org/ned/coveragepy/issue/32
.. _issue 33: http://bitbucket.org/ned/coveragepy/issue/33
.. _issue 35: http://bitbucket.org/ned/coveragepy/issue/35


Version 3.2b1 --- 10 November 2009
----------------------------------

- Branch coverage!

- XML reporting has file paths that let Cobertura find the source code.

- The tracer code has changed, it's a few percent faster.

- Some exceptions reported by the command line interface have been cleaned up
  so that tracebacks inside coverage.py aren't shown.  Fixes `issue 23`_.

.. _issue 23: http://bitbucket.org/ned/coveragepy/issue/23


Version 3.1 --- 4 October 2009
------------------------------

- Source code can now be read from eggs.  Thanks, Ross Lawley.  Fixes
  `issue 25`_.

.. _issue 25: http://bitbucket.org/ned/coveragepy/issue/25


Version 3.1b1 --- 27 September 2009
-----------------------------------

- Python 3.1 is now supported.

- Coverage.py has a new command line syntax with sub-commands.  This expands
  the possibilities for adding features and options in the future.  The old
  syntax is still supported.  Try "coverage help" to see the new commands.
  Thanks to Ben Finney for early help.

- Added an experimental "coverage xml" command for producing coverage reports
  in a Cobertura-compatible XML format.  Thanks, Bill Hart.

- Added the --timid option to enable a simpler slower trace function that works
  for DecoratorTools projects, including TurboGears.  Fixed `issue 12`_ and
  `issue 13`_.

- HTML reports show modules from other directories.  Fixed `issue 11`_.

- HTML reports now display syntax-colored Python source.

- Programs that change directory will still write .coverage files in the
  directory where execution started.  Fixed `issue 24`_.

- Added a "coverage debug" command for getting diagnostic information about the
  coverage.py installation.

.. _issue 11: http://bitbucket.org/ned/coveragepy/issue/11
.. _issue 12: http://bitbucket.org/ned/coveragepy/issue/12
.. _issue 13: http://bitbucket.org/ned/coveragepy/issue/13
.. _issue 24: http://bitbucket.org/ned/coveragepy/issue/24


Version 3.0.1 --- 7 July 2009
-----------------------------

- Removed the recursion limit in the tracer function.  Previously, code that
  ran more than 500 frames deep would crash. Fixed `issue 9`_.

- Fixed a bizarre problem involving pyexpat, whereby lines following XML parser
  invocations could be overlooked.  Fixed `issue 10`_.

- On Python 2.3, coverage.py could mis-measure code with exceptions being
  raised.  This is now fixed.

- The coverage.py code itself will now not be measured by coverage.py, and no
  coverage.py modules will be mentioned in the nose --with-cover plug-in.
  Fixed `issue 8`_.

- When running source files, coverage.py now opens them in universal newline
  mode just like Python does.  This lets it run Windows files on Mac, for
  example.

.. _issue 9: http://bitbucket.org/ned/coveragepy/issue/9
.. _issue 10: http://bitbucket.org/ned/coveragepy/issue/10
.. _issue 8: http://bitbucket.org/ned/coveragepy/issue/8


Version 3.0 --- 13 June 2009
----------------------------

- Fixed the way the Python library was ignored.  Too much code was being
  excluded the old way.

- Tabs are now properly converted in HTML reports.  Previously indentation was
  lost.  Fixed `issue 6`_.

- Nested modules now get a proper flat_rootname.  Thanks, Christian Heimes.

.. _issue 6: http://bitbucket.org/ned/coveragepy/issue/6


Version 3.0b3 --- 16 May 2009
-----------------------------

- Added parameters to coverage.__init__ for options that had been set on the
  coverage object itself.

- Added clear_exclude() and get_exclude_list() methods for programmatic
  manipulation of the exclude regexes.

- Added coverage.load() to read previously-saved data from the data file.

- Improved the finding of code files.  For example, .pyc files that have been
  installed after compiling are now located correctly.  Thanks, Detlev
  Offenbach.

- When using the object API (that is, constructing a coverage() object), data
  is no longer saved automatically on process exit.  You can re-enable it with
  the auto_data=True parameter on the coverage() constructor. The module-level
  interface still uses automatic saving.


Version 3.0b --- 30 April 2009
------------------------------

HTML reporting, and continued refactoring.

- HTML reports and annotation of source files: use the new -b (browser) switch.
  Thanks to George Song for code, inspiration and guidance.

- Code in the Python standard library is not measured by default.  If you need
  to measure standard library code, use the -L command-line switch during
  execution, or the cover_pylib=True argument to the coverage() constructor.

- Source annotation into a directory (-a -d) behaves differently.  The
  annotated files are named with their hierarchy flattened so that same-named
  files from different directories no longer collide.  Also, only files in the
  current tree are included.

- coverage.annotate_file is no longer available.

- Programs executed with -x now behave more as they should, for example,
  __file__ has the correct value.

- .coverage data files have a new pickle-based format designed for better
  extensibility.

- Removed the undocumented cache_file argument to coverage.usecache().


Version 3.0b1 --- 7 March 2009
------------------------------

Major overhaul.

- Coverage.py is now a package rather than a module.  Functionality has been
  split into classes.

- The trace function is implemented in C for speed.  Coverage.py runs are now
  much faster.  Thanks to David Christian for productive micro-sprints and
  other encouragement.

- Executable lines are identified by reading the line number tables in the
  compiled code, removing a great deal of complicated analysis code.

- Precisely which lines are considered executable has changed in some cases.
  Therefore, your coverage stats may also change slightly.

- The singleton coverage object is only created if the module-level functions
  are used.  This maintains the old interface while allowing better
  programmatic use of Coverage.py.

- The minimum supported Python version is 2.3.


Version 2.85 --- 14 September 2008
----------------------------------

- Add support for finding source files in eggs. Don't check for
  morf's being instances of ModuleType, instead use duck typing so that
  pseudo-modules can participate. Thanks, Imri Goldberg.

- Use os.realpath as part of the fixing of file names so that symlinks won't
  confuse things. Thanks, Patrick Mezard.


Version 2.80 --- 25 May 2008
----------------------------

- Open files in rU mode to avoid line ending craziness. Thanks, Edward Loper.


Version 2.78 --- 30 September 2007
----------------------------------

- Don't try to predict whether a file is Python source based on the extension.
  Extension-less files are often Pythons scripts. Instead, simply parse the file
  and catch the syntax errors. Hat tip to Ben Finney.


Version 2.77 --- 29 July 2007
-----------------------------

- Better packaging.


Version 2.76 --- 23 July 2007
-----------------------------

- Now Python 2.5 is *really* fully supported: the body of the new with
  statement is counted as executable.


Version 2.75 --- 22 July 2007
-----------------------------

- Python 2.5 now fully supported. The method of dealing with multi-line
  statements is now less sensitive to the exact line that Python reports during
  execution. Pass statements are handled specially so that their disappearance
  during execution won't throw off the measurement.


Version 2.7 --- 21 July 2007
----------------------------

- "#pragma: nocover" is excluded by default.

- Properly ignore docstrings and other constant expressions that appear in the
  middle of a function, a problem reported by Tim Leslie.

- coverage.erase() shouldn't clobber the exclude regex. Change how parallel
  mode is invoked, and fix erase() so that it erases the cache when called
  programmatically.

- In reports, ignore code executed from strings, since we can't do anything
  useful with it anyway.

- Better file handling on Linux, thanks Guillaume Chazarain.

- Better shell support on Windows, thanks Noel O'Boyle.

- Python 2.2 support maintained, thanks Catherine Proulx.

- Minor changes to avoid lint warnings.


Version 2.6 --- 23 August 2006
------------------------------

- Applied Joseph Tate's patch for function decorators.

- Applied Sigve Tjora and Mark van der Wal's fixes for argument handling.

- Applied Geoff Bache's parallel mode patch.

- Refactorings to improve testability. Fixes to command-line logic for parallel
  mode and collect.


Version 2.5 --- 4 December 2005
-------------------------------

- Call threading.settrace so that all threads are measured. Thanks Martin
  Fuzzey.

- Add a file argument to report so that reports can be captured to a different
  destination.

- Coverage.py can now measure itself.

- Adapted Greg Rogers' patch for using relative file names, and sorting and
  omitting files to report on.


Version 2.2 --- 31 December 2004
--------------------------------

- Allow for keyword arguments in the module global functions. Thanks, Allen.


Version 2.1 --- 14 December 2004
--------------------------------

- Return 'analysis' to its original behavior and add 'analysis2'. Add a global
  for 'annotate', and factor it, adding 'annotate_file'.


Version 2.0 --- 12 December 2004
--------------------------------

Significant code changes.

- Finding executable statements has been rewritten so that docstrings and
  other quirks of Python execution aren't mistakenly identified as missing
  lines.

- Lines can be excluded from consideration, even entire suites of lines.

- The file system cache of covered lines can be disabled programmatically.

- Modernized the code.


Earlier History
---------------

2001-12-04 GDR Created.

2001-12-06 GDR Added command-line interface and source code annotation.

2001-12-09 GDR Moved design and interface to separate documents.

2001-12-10 GDR Open cache file as binary on Windows. Allow simultaneous -e and
-x, or -a and -r.

2001-12-12 GDR Added command-line help. Cache analysis so that it only needs to
be done once when you specify -a and -r.

2001-12-13 GDR Improved speed while recording. Portable between Python 1.5.2
and 2.1.1.

2002-01-03 GDR Module-level functions work correctly.

2002-01-07 GDR Update sys.path when running a file with the -x option, so that
it matches the value the program would get if it were run on its own.
