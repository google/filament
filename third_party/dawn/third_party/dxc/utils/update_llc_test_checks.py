#!/usr/bin/env python2.7

"""A test case update script.

This script is a utility to update LLVM X86 'llc' based test cases with new
FileCheck patterns. It can either update all of the tests in the file or
a single test function.
"""

import argparse
import itertools
import string
import subprocess
import sys
import tempfile
import re


def llc(args, cmd_args, ir):
  with open(ir) as ir_file:
    stdout = subprocess.check_output(args.llc_binary + ' ' + cmd_args,
                                     shell=True, stdin=ir_file)
  return stdout


ASM_SCRUB_WHITESPACE_RE = re.compile(r'(?!^(|  \w))[ \t]+', flags=re.M)
ASM_SCRUB_TRAILING_WHITESPACE_RE = re.compile(r'[ \t]+$', flags=re.M)
ASM_SCRUB_SHUFFLES_RE = (
    re.compile(
        r'^(\s*\w+) [^#\n]+#+ ((?:[xyz]mm\d+|mem) = .*)$',
        flags=re.M))
ASM_SCRUB_SP_RE = re.compile(r'\d+\(%(esp|rsp)\)')
ASM_SCRUB_RIP_RE = re.compile(r'[.\w]+\(%rip\)')
ASM_SCRUB_KILL_COMMENT_RE = re.compile(r'^ *#+ +kill:.*\n')


def scrub_asm(asm):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = ASM_SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Detect shuffle asm comments and hide the operands in favor of the comments.
  asm = ASM_SCRUB_SHUFFLES_RE.sub(r'\1 {{.*#+}} \2', asm)
  # Generically match the stack offset of a memory operand.
  asm = ASM_SCRUB_SP_RE.sub(r'{{[0-9]+}}(%\1)', asm)
  # Generically match a RIP-relative memory operand.
  asm = ASM_SCRUB_RIP_RE.sub(r'{{.*}}(%rip)', asm)
  # Strip kill operands inserted into the asm.
  asm = ASM_SCRUB_KILL_COMMENT_RE.sub('', asm)
  # Strip trailing whitespace.
  asm = ASM_SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-v', '--verbose', action='store_true',
                      help='Show verbose output')
  parser.add_argument('--llc-binary', default='llc',
                      help='The "llc" binary to use to generate the test case')
  parser.add_argument(
      '--function', help='The function in the test file to update')
  parser.add_argument('tests', nargs='+')
  args = parser.parse_args()

  run_line_re = re.compile('^\s*;\s*RUN:\s*(.*)$')
  ir_function_re = re.compile('^\s*define\s+(?:internal\s+)?[^@]*@(\w+)\s*\(')
  asm_function_re = re.compile(
      r'^_?(?P<f>[^:]+):[ \t]*#+[ \t]*@(?P=f)\n[^:]*?'
      r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
      r'^\s*(?:[^:\n]+?:\s*\n\s*\.size|\.cfi_endproc|\.globl|\.comm|\.(?:sub)?section)',
      flags=(re.M | re.S))
  check_prefix_re = re.compile('--check-prefix=(\S+)')
  check_re = re.compile(r'^\s*;\s*([^:]+?)(?:-NEXT|-NOT|-DAG|-LABEL)?:')

  for test in args.tests:
    if args.verbose:
      print >>sys.stderr, 'Scanning for RUN lines in test file: %s' % (test,)
    with open(test) as f:
      test_lines = [l.rstrip() for l in f]

    run_lines = [m.group(1)
                 for m in [run_line_re.match(l) for l in test_lines] if m]
    if args.verbose:
      print >>sys.stderr, 'Found %d RUN lines:' % (len(run_lines),)
      for l in run_lines:
        print >>sys.stderr, '  RUN: ' + l

    checks = []
    for l in run_lines:
      (llc_cmd, filecheck_cmd) = tuple([cmd.strip() for cmd in l.split('|', 1)])
      if not llc_cmd.startswith('llc '):
        print >>sys.stderr, 'WARNING: Skipping non-llc RUN line: ' + l
        continue

      if not filecheck_cmd.startswith('FileCheck '):
        print >>sys.stderr, 'WARNING: Skipping non-FileChecked RUN line: ' + l
        continue

      llc_cmd_args = llc_cmd[len('llc'):].strip()
      llc_cmd_args = llc_cmd_args.replace('< %s', '').replace('%s', '').strip()

      check_prefixes = [m.group(1)
                        for m in check_prefix_re.finditer(filecheck_cmd)]
      if not check_prefixes:
        check_prefixes = ['CHECK']

      # FIXME: We should use multiple check prefixes to common check lines. For
      # now, we just ignore all but the last.
      checks.append((check_prefixes, llc_cmd_args))

    asm = {}
    for prefixes, _ in checks:
      for prefix in prefixes:
        asm.update({prefix: dict()})
    for prefixes, llc_args in checks:
      if args.verbose:
        print >>sys.stderr, 'Extracted LLC cmd: llc ' + llc_args
        print >>sys.stderr, 'Extracted FileCheck prefixes: ' + str(prefixes)
      raw_asm = llc(args, llc_args, test)
      # Build up a dictionary of all the function bodies.
      for m in asm_function_re.finditer(raw_asm):
        if not m:
          continue
        f = m.group('f')
        f_asm = scrub_asm(m.group('body'))
        if f.startswith('stress'):
          # We only use the last line of the asm for stress tests.
          f_asm = '\n'.join(f_asm.splitlines()[-1:])
        if args.verbose:
          print >>sys.stderr, 'Processing asm for function: ' + f
          for l in f_asm.splitlines():
            print >>sys.stderr, '  ' + l
        for prefix in prefixes:
          if f in asm[prefix] and asm[prefix][f] != f_asm:
            if prefix == prefixes[-1]:
              print >>sys.stderr, ('WARNING: Found conflicting asm under the '
                                   'same prefix!')
            else:
              asm[prefix][f] = None
              continue

          asm[prefix][f] = f_asm

    is_in_function = False
    is_in_function_start = False
    prefix_set = set([prefix for prefixes, _ in checks for prefix in prefixes])
    if args.verbose:
      print >>sys.stderr, 'Rewriting FileCheck prefixes: %s' % (prefix_set,)
    fixed_lines = []
    for l in test_lines:
      if is_in_function_start:
        if l.lstrip().startswith(';'):
          m = check_re.match(l)
          if not m or m.group(1) not in prefix_set:
            fixed_lines.append(l)
            continue

        # Print out the various check lines here
        printed_prefixes = []
        for prefixes, _ in checks:
          for prefix in prefixes:
            if prefix in printed_prefixes:
              break
            if not asm[prefix][name]:
              continue
            if len(printed_prefixes) != 0:
              fixed_lines.append(';')
            printed_prefixes.append(prefix)
            fixed_lines.append('; %s-LABEL: %s:' % (prefix, name))
            asm_lines = asm[prefix][name].splitlines()
            fixed_lines.append('; %s:       %s' % (prefix, asm_lines[0]))
            for asm_line in asm_lines[1:]:
              fixed_lines.append('; %s-NEXT:  %s' % (prefix, asm_line))
            break
        is_in_function_start = False

      if is_in_function:
        # Skip any blank comment lines in the IR.
        if l.strip() == ';':
          continue
        # And skip any CHECK lines. We'll build our own.
        m = check_re.match(l)
        if m and m.group(1) in prefix_set:
          continue
        # Collect the remaining lines in the function body and look for the end
        # of the function.
        fixed_lines.append(l)
        if l.strip() == '}':
          is_in_function = False
        continue

      fixed_lines.append(l)

      m = ir_function_re.match(l)
      if not m:
        continue
      name = m.group(1)
      if args.function is not None and name != args.function:
        # When filtering on a specific function, skip all others.
        continue
      is_in_function = is_in_function_start = True

    if args.verbose:
      print>>sys.stderr, 'Writing %d fixed lines to %s...' % (
          len(fixed_lines), test)
    with open(test, 'w') as f:
      f.writelines([l + '\n' for l in fixed_lines])


if __name__ == '__main__':
  main()
