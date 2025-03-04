from __future__ import absolute_import
import os, signal, subprocess, sys
import re
import platform
import tempfile

import lit.ShUtil as ShUtil
import lit.Test as Test
import lit.util
from lit.util import to_bytes, to_string

class InternalShellError(Exception):
    def __init__(self, command, message):
        self.command = command
        self.message = message

kIsWindows = platform.system() == 'Windows'

# Don't use close_fds on Windows.
kUseCloseFDs = not kIsWindows

# Use temporary files to replace /dev/null on Windows.
kAvoidDevNull = kIsWindows

class ShellEnvironment(object):

    """Mutable shell environment containing things like CWD and env vars.

    Environment variables are not implemented, but cwd tracking is.
    """

    def __init__(self, cwd, env):
        self.cwd = cwd
        self.env = env

def executeShCmd(cmd, shenv, results):
    if isinstance(cmd, ShUtil.Seq):
        if cmd.op == ';':
            res = executeShCmd(cmd.lhs, shenv, results)
            return executeShCmd(cmd.rhs, shenv, results)

        if cmd.op == '&':
            raise InternalShellError(cmd,"unsupported shell operator: '&'")

        if cmd.op == '||':
            res = executeShCmd(cmd.lhs, shenv, results)
            if res != 0:
                res = executeShCmd(cmd.rhs, shenv, results)
            return res

        if cmd.op == '&&':
            res = executeShCmd(cmd.lhs, shenv, results)
            if res is None:
                return res

            if res == 0:
                res = executeShCmd(cmd.rhs, shenv, results)
            return res

        raise ValueError('Unknown shell command: %r' % cmd.op)
    assert isinstance(cmd, ShUtil.Pipeline)

    # Handle shell builtins first.
    if cmd.commands[0].args[0] == 'cd':
        # Update the cwd in the environment.
        if len(cmd.commands[0].args) != 2:
            raise ValueError('cd supports only one argument')
        newdir = cmd.commands[0].args[1]
        if os.path.isabs(newdir):
            shenv.cwd = newdir
        else:
            shenv.cwd = os.path.join(shenv.cwd, newdir)
        return 0

    procs = []
    input = subprocess.PIPE
    stderrTempFiles = []
    opened_files = []
    named_temp_files = []
    # To avoid deadlock, we use a single stderr stream for piped
    # output. This is null until we have seen some output using
    # stderr.
    for i,j in enumerate(cmd.commands):
        # Apply the redirections, we use (N,) as a sentinel to indicate stdin,
        # stdout, stderr for N equal to 0, 1, or 2 respectively. Redirects to or
        # from a file are represented with a list [file, mode, file-object]
        # where file-object is initially None.
        redirects = [(0,), (1,), (2,)]
        for r in j.redirects:
            if r[0] == ('>',2):
                redirects[2] = [r[1], 'w', None]
            elif r[0] == ('>>',2):
                redirects[2] = [r[1], 'a', None]
            elif r[0] == ('>&',2) and r[1] in '012':
                redirects[2] = redirects[int(r[1])]
            elif r[0] == ('>&',) or r[0] == ('&>',):
                redirects[1] = redirects[2] = [r[1], 'w', None]
            elif r[0] == ('>',):
                redirects[1] = [r[1], 'w', None]
            elif r[0] == ('>>',):
                redirects[1] = [r[1], 'a', None]
            elif r[0] == ('<',):
                redirects[0] = [r[1], 'r', None]
            else:
                raise InternalShellError(j,"Unsupported redirect: %r" % (r,))

        # Map from the final redirections to something subprocess can handle.
        final_redirects = []
        for index,r in enumerate(redirects):
            if r == (0,):
                result = input
            elif r == (1,):
                if index == 0:
                    raise InternalShellError(j,"Unsupported redirect for stdin")
                elif index == 1:
                    result = subprocess.PIPE
                else:
                    result = subprocess.STDOUT
            elif r == (2,):
                if index != 2:
                    raise InternalShellError(j,"Unsupported redirect on stdout")
                result = subprocess.PIPE
            else:
                if r[2] is None:
                    if kAvoidDevNull and r[0] == '/dev/null':
                        r[2] = tempfile.TemporaryFile(mode=r[1])
                    else:
                        # Make sure relative paths are relative to the cwd.
                        redir_filename = os.path.join(shenv.cwd, r[0])
                        r[2] = open(redir_filename, r[1])
                    # Workaround a Win32 and/or subprocess bug when appending.
                    #
                    # FIXME: Actually, this is probably an instance of PR6753.
                    if r[1] == 'a':
                        r[2].seek(0, 2)
                    opened_files.append(r[2])
                result = r[2]
            final_redirects.append(result)

        stdin, stdout, stderr = final_redirects

        # If stderr wants to come from stdout, but stdout isn't a pipe, then put
        # stderr on a pipe and treat it as stdout.
        if (stderr == subprocess.STDOUT and stdout != subprocess.PIPE):
            stderr = subprocess.PIPE
            stderrIsStdout = True
        else:
            stderrIsStdout = False

            # Don't allow stderr on a PIPE except for the last
            # process, this could deadlock.
            #
            # FIXME: This is slow, but so is deadlock.
            if stderr == subprocess.PIPE and j != cmd.commands[-1]:
                stderr = tempfile.TemporaryFile(mode='w+b')
                stderrTempFiles.append((i, stderr))

        # Resolve the executable path ourselves.
        args = list(j.args)
        executable = lit.util.which(args[0], shenv.env['PATH'])
        if not executable:
            raise InternalShellError(j, '%r: command not found' % j.args[0])

        # Replace uses of /dev/null with temporary files.
        if kAvoidDevNull:
            for i,arg in enumerate(args):
                if arg == "/dev/null":
                    f = tempfile.NamedTemporaryFile(delete=False)
                    f.close()
                    named_temp_files.append(f.name)
                    args[i] = f.name

        try:
            procs.append(subprocess.Popen(args, cwd=shenv.cwd,
                                          executable = executable,
                                          stdin = stdin,
                                          stdout = stdout,
                                          stderr = stderr,
                                          env = shenv.env,
                                          close_fds = kUseCloseFDs))
        except OSError as e:
            raise InternalShellError(j, 'Could not create process due to {}'.format(e))

        # Immediately close stdin for any process taking stdin from us.
        if stdin == subprocess.PIPE:
            procs[-1].stdin.close()
            procs[-1].stdin = None

        # Update the current stdin source.
        if stdout == subprocess.PIPE:
            input = procs[-1].stdout
        elif stderrIsStdout:
            input = procs[-1].stderr
        else:
            input = subprocess.PIPE

    # Explicitly close any redirected files. We need to do this now because we
    # need to release any handles we may have on the temporary files (important
    # on Win32, for example). Since we have already spawned the subprocess, our
    # handles have already been transferred so we do not need them anymore.
    for f in opened_files:
        f.close()

    # FIXME: There is probably still deadlock potential here. Yawn.
    procData = [None] * len(procs)
    procData[-1] = procs[-1].communicate()

    for i in range(len(procs) - 1):
        if procs[i].stdout is not None:
            out = procs[i].stdout.read()
        else:
            out = ''
        if procs[i].stderr is not None:
            err = procs[i].stderr.read()
        else:
            err = ''
        procData[i] = (out,err)

    # Read stderr out of the temp files.
    for i,f in stderrTempFiles:
        f.seek(0, 0)
        procData[i] = (procData[i][0], f.read())

    def to_string(bytes):
        if isinstance(bytes, str):
            return bytes
        return bytes.encode('utf-8')

    exitCode = None
    for i,(out,err) in enumerate(procData):
        res = procs[i].wait()
        # Detect Ctrl-C in subprocess.
        if res == -signal.SIGINT:
            raise KeyboardInterrupt

        # Ensure the resulting output is always of string type.
        try:
            out = to_string(out.decode('utf-8'))
        except:
            out = str(out)
        try:
            err = to_string(err.decode('utf-8'))
        except:
            err = str(err)

        results.append((cmd.commands[i], out, err, res))
        if cmd.pipe_err:
            # Python treats the exit code as a signed char.
            if exitCode is None:
                exitCode = res
            elif res < 0:
                exitCode = min(exitCode, res)
            else:
                exitCode = max(exitCode, res)
        else:
            exitCode = res

    # Remove any named temporary files we created.
    for f in named_temp_files:
        try:
            os.remove(f)
        except OSError:
            pass

    if cmd.negate:
        exitCode = not exitCode

    return exitCode

def executeScriptInternal(test, litConfig, tmpBase, commands, cwd):
    cmds = []
    for ln in commands:
        try:
            cmds.append(ShUtil.ShParser(ln, litConfig.isWindows,
                                        test.config.pipefail).parse())
        except:
            return lit.Test.Result(Test.FAIL, "shell parser error on: %r" % ln)

    cmd = cmds[0]
    for c in cmds[1:]:
        cmd = ShUtil.Seq(cmd, '&&', c)

    results = []
    try:
        shenv = ShellEnvironment(cwd, test.config.environment)
        exitCode = executeShCmd(cmd, shenv, results)
    except InternalShellError:
        e = sys.exc_info()[1]
        exitCode = 127
        results.append((e.command, '', e.message, exitCode))

    out = err = ''
    for i,(cmd, cmd_out,cmd_err,res) in enumerate(results):
        out += 'Command %d: %s\n' % (i, ' '.join('"%s"' % s for s in cmd.args))
        out += 'Command %d Result: %r\n' % (i, res)
        out += 'Command %d Output:\n%s\n\n' % (i, cmd_out)
        out += 'Command %d Stderr:\n%s\n\n' % (i, cmd_err)

    return out, err, exitCode

def executeScript(test, litConfig, tmpBase, commands, cwd):
    bashPath = litConfig.getBashPath();
    isWin32CMDEXE = (litConfig.isWindows and not bashPath)
    script = tmpBase + '.script'
    if isWin32CMDEXE:
        script += '.bat'

    # Write script file
    mode = 'w'
    if litConfig.isWindows and not isWin32CMDEXE:
      mode += 'b'  # Avoid CRLFs when writing bash scripts.
    f = open(script, mode)
    if isWin32CMDEXE:
        f.write('\nif %ERRORLEVEL% NEQ 0 EXIT\n'.join(commands))
    else:
        if test.config.pipefail:
            f.write('set -o pipefail;')
        f.write('{ ' + '; } &&\n{ '.join(commands) + '; }')
    f.write('\n')
    f.close()

    if isWin32CMDEXE:
        command = ['cmd','/c', script]
    else:
        if bashPath:
            command = [bashPath, script]
        else:
            command = ['/bin/sh', script]
        if litConfig.useValgrind:
            # FIXME: Running valgrind on sh is overkill. We probably could just
            # run on clang with no real loss.
            command = litConfig.valgrindArgs + command

    return lit.util.executeCommand(command, cwd=cwd,
                                   env=test.config.environment)

def parseIntegratedTestScriptCommands(source_path):
    """
    parseIntegratedTestScriptCommands(source_path) -> commands

    Parse the commands in an integrated test script file into a list of
    (line_number, command_type, line).
    """

    # This code is carefully written to be dual compatible with Python 2.5+ and
    # Python 3 without requiring input files to always have valid codings. The
    # trick we use is to open the file in binary mode and use the regular
    # expression library to find the commands, with it scanning strings in
    # Python2 and bytes in Python3.
    #
    # Once we find a match, we do require each script line to be decodable to
    # UTF-8, so we convert the outputs to UTF-8 before returning. This way the
    # remaining code can work with "strings" agnostic of the executing Python
    # version.

    keywords = ['RUN:', 'XFAIL:', 'REQUIRES:', 'UNSUPPORTED:', 'END.']
    keywords_re = re.compile(
        to_bytes("(%s)(.*)\n" % ("|".join(k for k in keywords),)))

    f = open(source_path, 'rb')
    try:
        # Read the entire file contents.
        data = f.read()

        # Ensure the data ends with a newline.
        if not data.endswith(to_bytes('\n')):
            data = data + to_bytes('\n')

        # Iterate over the matches.
        line_number = 1
        last_match_position = 0
        for match in keywords_re.finditer(data):
            # Compute the updated line number by counting the intervening
            # newlines.
            match_position = match.start()
            line_number += data.count(to_bytes('\n'), last_match_position,
                                      match_position)
            last_match_position = match_position

            # Convert the keyword and line to UTF-8 strings and yield the
            # command. Note that we take care to return regular strings in
            # Python 2, to avoid other code having to differentiate between the
            # str and unicode types.
            keyword,ln = match.groups()
            yield (line_number, to_string(keyword[:-1].decode('utf-8')),
                   to_string(ln.decode('utf-8')))
    finally:
        f.close()


def parseIntegratedTestScript(test, normalize_slashes=False,
                              extra_substitutions=[], require_script=True):
    """parseIntegratedTestScript - Scan an LLVM/Clang style integrated test
    script and extract the lines to 'RUN' as well as 'XFAIL' and 'REQUIRES'
    and 'UNSUPPORTED' information. The RUN lines also will have variable
    substitution performed. If 'require_script' is False an empty script may be
    returned. This can be used for test formats where the actual script is
    optional or ignored.
    """

    # Get the temporary location, this is always relative to the test suite
    # root, not test source root.
    #
    # FIXME: This should not be here?
    sourcepath = test.getSourcePath()
    sourcedir = os.path.dirname(sourcepath)
    execpath = test.getExecPath()
    execdir,execbase = os.path.split(execpath)
    tmpDir = os.path.join(execdir, 'Output')
    tmpBase = os.path.join(tmpDir, execbase)

    # Normalize slashes, if requested.
    if normalize_slashes:
        sourcepath = sourcepath.replace('\\', '/')
        sourcedir = sourcedir.replace('\\', '/')
        tmpDir = tmpDir.replace('\\', '/')
        tmpBase = tmpBase.replace('\\', '/')

    # We use #_MARKER_# to hide %% while we do the other substitutions.
    substitutions = list(extra_substitutions)
    substitutions.extend([('%%', '#_MARKER_#')])
    substitutions.extend(test.config.substitutions)
    substitutions.extend([('%s', sourcepath),
                          ('%S', sourcedir),
                          ('%p', sourcedir),
                          ('%{pathsep}', os.pathsep),
                          ('%t', tmpBase + '.tmp'),
                          ('%T', tmpDir),
                          ('#_MARKER_#', '%')])

    # "%/[STpst]" should be normalized.
    substitutions.extend([
            ('%/s', sourcepath.replace('\\', '/')),
            ('%/S', sourcedir.replace('\\', '/')),
            ('%/p', sourcedir.replace('\\', '/')),
            ('%/t', tmpBase.replace('\\', '/') + '.tmp'),
            ('%/T', tmpDir.replace('\\', '/')),
            ])

    # re for %if
    re_cond_end = re.compile('%{')
    re_if = re.compile('(.*?)(?:%if)')
    re_nested_if = re.compile('(.*?)(?:%if|%})')
    re_else = re.compile('^\s*%else\s*(%{)?')


    # Collect the test lines from the script.
    script = []
    requires = []
    unsupported = []
    for line_number, command_type, ln in \
            parseIntegratedTestScriptCommands(sourcepath):
        if command_type == 'RUN':
            # Trim trailing whitespace.
            ln = ln.rstrip()

            # Substitute line number expressions
            ln = re.sub('%\(line\)', str(line_number), ln)
            def replace_line_number(match):
                if match.group(1) == '+':
                    return str(line_number + int(match.group(2)))
                if match.group(1) == '-':
                    return str(line_number - int(match.group(2)))
            ln = re.sub('%\(line *([\+-]) *(\d+)\)', replace_line_number, ln)

            # Collapse lines with trailing '\\'.
            if script and script[-1][-1] == '\\':
                script[-1] = script[-1][:-1] + ln
            else:
                script.append(ln)
        elif command_type == 'XFAIL':
            test.xfails.extend([s.strip() for s in ln.split(',')])
        elif command_type == 'REQUIRES':
            requires.extend([s.strip() for s in ln.split(',')])
        elif command_type == 'UNSUPPORTED':
            unsupported.extend([s.strip() for s in ln.split(',')])
        elif command_type == 'END':
            # END commands are only honored if the rest of the line is empty.
            if not ln.strip():
                break
        else:
            raise ValueError("unknown script command type: %r" % (
                    command_type,))


    def substituteIfElse(ln):
        # early exit to avoid wasting time on lines without
        # conditional substitutions
        if ln.find('%if ') == -1:
            return ln

        def tryParseIfCond(ln):
            # space is important to not conflict with other (possible)
            # substitutions
            if not ln.startswith('%if '):
                return None, ln
            ln = ln[4:]

            # stop at '%{'
            match = re_cond_end.search(ln)
            if not match:
                raise ValueError("'%{' is missing for %if substitution")
            cond = ln[:match.start()]

            # eat '%{' as well
            ln = ln[match.end():]
            return cond, ln

        def tryParseElse(ln):
            match = re_else.search(ln)
            if not match:
                return False, ln
            if not match.group(1):
                raise ValueError("'%{' is missing for %else substitution")
            return True, ln[match.end():]

        def tryParseEnd(ln):
            if ln.startswith('%}'):
                return True, ln[2:]
            return False, ln

        def parseText(ln, isNested):
            # parse everything until %if, or %} if we're parsing a
            # nested expression.
            re_pat = re_nested_if if isNested else re_if
            match = re_pat.search(ln)
            if not match:
                # there is no terminating pattern, so treat the whole
                # line as text
                return ln, ''
            text_end = match.end(1)
            return ln[:text_end], ln[text_end:]

        def parseRecursive(ln, isNested):
            result = ''
            while len(ln):
                if isNested:
                    found_end, _ = tryParseEnd(ln)
                    if found_end:
                        break

                # %if cond %{ branch_if %} %else %{ branch_else %}
                cond, ln = tryParseIfCond(ln)
                if cond:
                    branch_if, ln = parseRecursive(ln, isNested=True)
                    found_end, ln = tryParseEnd(ln)
                    if not found_end:
                        raise ValueError("'%}' is missing for %if substitution")

                    branch_else = ''
                    found_else, ln = tryParseElse(ln)
                    if found_else:
                        branch_else, ln = parseRecursive(ln, isNested=True)
                        found_end, ln = tryParseEnd(ln)
                        if not found_end:
                            raise ValueError("'%}' is missing for %else substitution")

                    cond = cond.strip()

                    if cond in test.config.available_features:
                        result += branch_if
                    else:
                        result += branch_else
                    continue

                # The rest is handled as plain text.
                text, ln = parseText(ln, isNested)
                result += text

            return result, ln

        result, ln = parseRecursive(ln, isNested=False)
        assert len(ln) == 0
        return result

    # Apply substitutions to the script.  Allow full regular
    # expression syntax.  Replace each matching occurrence of regular
    # expression pattern a with substitution b in line ln.
    def processLine(ln):
        # Apply substitutions
        ln = substituteIfElse(ln)

        for a,b in substitutions:
            if kIsWindows:
                b = b.replace("\\","\\\\")
            ln = re.sub(a, b, ln)

        # Strip the trailing newline and any extra whitespace.
        return ln.strip()
    script = [processLine(ln)
              for ln in script]

    # Verify the script contains a run line.
    if require_script and not script:
        return lit.Test.Result(Test.UNRESOLVED, "Test has no run line!")

    # Check for unterminated run lines.
    if script and script[-1][-1] == '\\':
        return lit.Test.Result(Test.UNRESOLVED,
                               "Test has unterminated run lines (with '\\')")

    # Check that we have the required features:
    missing_required_features = [f for f in requires
                                 if f not in test.config.available_features]
    if missing_required_features:
        msg = ', '.join(missing_required_features)
        return lit.Test.Result(Test.UNSUPPORTED,
                               "Test requires the following features: %s" % msg)
    unsupported_features = [f for f in unsupported
                            if f in test.config.available_features]
    if unsupported_features:
        msg = ', '.join(unsupported_features)
        return lit.Test.Result(Test.UNSUPPORTED,
                    "Test is unsupported with the following features: %s" % msg)

    if test.config.limit_to_features:
        # Check that we have one of the limit_to_features features in requires.
        limit_to_features_tests = [f for f in test.config.limit_to_features
                                   if f in requires]
        if not limit_to_features_tests:
            msg = ', '.join(test.config.limit_to_features)
            return lit.Test.Result(Test.UNSUPPORTED,
                 "Test requires one of the limit_to_features features %s" % msg)

    return script,tmpBase,execdir

def _runShTest(test, litConfig, useExternalSh,
                   script, tmpBase, execdir):
    # Create the output directory if it does not already exist.
    lit.util.mkdir_p(os.path.dirname(tmpBase))

    if useExternalSh:
        res = executeScript(test, litConfig, tmpBase, script, execdir)
    else:
        res = executeScriptInternal(test, litConfig, tmpBase, script, execdir)
    if isinstance(res, lit.Test.Result):
        return res

    out,err,exitCode = res
    if exitCode == 0:
        status = Test.PASS
    else:
        status = Test.FAIL

    # Form the output log.
    output = """Script:\n--\n%s\n--\nExit Code: %d\n\n""" % (
        '\n'.join(script), exitCode)

    # Append the outputs, if present.
    if out:
        output += """Command Output (stdout):\n--\n%s\n--\n""" % (out,)
    if err:
        output += """Command Output (stderr):\n--\n%s\n--\n""" % (err,)

    return lit.Test.Result(status, output)


def executeShTest(test, litConfig, useExternalSh,
                  extra_substitutions=[]):
    if test.config.unsupported:
        return (Test.UNSUPPORTED, 'Test is unsupported')

    res = parseIntegratedTestScript(test, useExternalSh, extra_substitutions)
    if isinstance(res, lit.Test.Result):
        return res
    if litConfig.noExecute:
        return lit.Test.Result(Test.PASS)

    script, tmpBase, execdir = res
    return _runShTest(test, litConfig, useExternalSh, script, tmpBase, execdir)

