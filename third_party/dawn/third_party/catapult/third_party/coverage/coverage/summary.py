# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

"""Summary reporting"""

import sys

from coverage import env
from coverage.report import Reporter
from coverage.results import Numbers
from coverage.misc import NotPython, CoverageException, output_encoding


class SummaryReporter(Reporter):
    """A reporter for writing the summary report."""

    def __init__(self, coverage, config):
        super(SummaryReporter, self).__init__(coverage, config)
        self.branches = coverage.data.has_arcs()

    def report(self, morfs, outfile=None):
        """Writes a report summarizing coverage statistics per module.

        `outfile` is a file object to write the summary to. It must be opened
        for native strings (bytes on Python 2, Unicode on Python 3).

        """
        self.find_file_reporters(morfs)

        # Prepare the formatting strings
        max_name = max([len(fr.relative_filename()) for fr in self.file_reporters] + [5])
        fmt_name = u"%%- %ds  " % max_name
        fmt_err = u"%s   %s: %s\n"
        fmt_skip_covered = u"\n%s file%s skipped due to complete coverage.\n"

        header = (fmt_name % "Name") + u" Stmts   Miss"
        fmt_coverage = fmt_name + u"%6d %6d"
        if self.branches:
            header += u" Branch BrPart"
            fmt_coverage += u" %6d %6d"
        width100 = Numbers.pc_str_width()
        header += u"%*s" % (width100+4, "Cover")
        fmt_coverage += u"%%%ds%%%%" % (width100+3,)
        if self.config.show_missing:
            header += u"   Missing"
            fmt_coverage += u"   %s"
        rule = u"-" * len(header) + u"\n"
        header += u"\n"
        fmt_coverage += u"\n"

        if outfile is None:
            outfile = sys.stdout

        if env.PY2:
            writeout = lambda u: outfile.write(u.encode(output_encoding()))
        else:
            writeout = outfile.write

        # Write the header
        writeout(header)
        writeout(rule)

        total = Numbers()
        skipped_count = 0

        for fr in self.file_reporters:
            try:
                analysis = self.coverage._analyze(fr)
                nums = analysis.numbers
                total += nums

                if self.config.skip_covered:
                    # Don't report on 100% files.
                    no_missing_lines = (nums.n_missing == 0)
                    no_missing_branches = (nums.n_partial_branches == 0)
                    if no_missing_lines and no_missing_branches:
                        skipped_count += 1
                        continue

                args = (fr.relative_filename(), nums.n_statements, nums.n_missing)
                if self.branches:
                    args += (nums.n_branches, nums.n_partial_branches)
                args += (nums.pc_covered_str,)
                if self.config.show_missing:
                    missing_fmtd = analysis.missing_formatted()
                    if self.branches:
                        branches_fmtd = analysis.arcs_missing_formatted()
                        if branches_fmtd:
                            if missing_fmtd:
                                missing_fmtd += ", "
                            missing_fmtd += branches_fmtd
                    args += (missing_fmtd,)
                writeout(fmt_coverage % args)
            except Exception:
                report_it = not self.config.ignore_errors
                if report_it:
                    typ, msg = sys.exc_info()[:2]
                    # NotPython is only raised by PythonFileReporter, which has a
                    # should_be_python() method.
                    if typ is NotPython and not fr.should_be_python():
                        report_it = False
                if report_it:
                    writeout(fmt_err % (fr.relative_filename(), typ.__name__, msg))

        if total.n_files > 1:
            writeout(rule)
            args = ("TOTAL", total.n_statements, total.n_missing)
            if self.branches:
                args += (total.n_branches, total.n_partial_branches)
            args += (total.pc_covered_str,)
            if self.config.show_missing:
                args += ("",)
            writeout(fmt_coverage % args)

        if not total.n_files and not skipped_count:
            raise CoverageException("No data to report.")

        if self.config.skip_covered and skipped_count:
            writeout(fmt_skip_covered % (skipped_count, 's' if skipped_count > 1 else ''))

        return total.n_statements and total.pc_covered
