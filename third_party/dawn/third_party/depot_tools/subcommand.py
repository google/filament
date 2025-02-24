# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Manages subcommands in a script.

Each subcommand should look like this:
  @usage('[pet name]')
  def CMDpet(parser, args):
    '''Prints a pet.

    Many people likes pet. This command prints a pet for your pleasure.
    '''
    parser.add_option('--color', help='color of your pet')
    options, args = parser.parse_args(args)
    if len(args) != 1:
      parser.error('A pet name is required')
    pet = args[0]
    if options.color:
      print('Nice %s %d' % (options.color, pet))
    else:
      print('Nice %s' % pet)
    return 0

Explanation:
  - usage decorator alters the 'usage: %prog' line in the command's help.
  - docstring is used to both short help line and long help line.
  - parser can be augmented with arguments.
  - return the exit code.
  - Every function in the specified module with a name starting with 'CMD' will
    be a subcommand.
  - The module's docstring will be used in the default 'help' page.
  - If a command has no docstring, it will not be listed in the 'help' page.
    Useful to keep compatibility commands around or aliases.
  - If a command is an alias to another one, it won't be documented. E.g.:
      CMDoldname = CMDnewcmd
    will result in oldname not being documented but supported and redirecting to
    newcmd. Make it a real function that calls the old function if you want it
    to be documented.
  - CMDfoo_bar will be command 'foo-bar'.
"""

import difflib
import sys
import textwrap


def usage(more):
    """Adds a 'usage_more' property to a CMD function."""
    def hook(fn):
        fn.usage_more = more
        return fn

    return hook


def epilog(text):
    """Adds an 'epilog' property to a CMD function.

    It will be shown in the epilog. Usually useful for examples.
    """
    def hook(fn):
        fn.epilog = text
        return fn

    return hook


def CMDhelp(parser, args):
    """Prints list of commands or help for a specific command."""
    # This is the default help implementation. It can be disabled or overridden
    # if wanted.
    if not any(i in ('-h', '--help') for i in args):
        args = args + ['--help']
    parser.parse_args(args)
    # Never gets there.
    assert False


def _get_color_module():
    """Returns the colorama module if available.

    If so, assumes colors are supported and return the module handle.
    """
    return sys.modules.get('colorama') or sys.modules.get(
        'third_party.colorama')


def _function_to_name(name):
    """Returns the name of a CMD function."""
    return name[3:].replace('_', '-')


class CommandDispatcher(object):
    def __init__(self, module):
        """module is the name of the main python module where to look for
        commands.

        The python builtin variable __name__ MUST be used for |module|. If the
        script is executed in the form 'python script.py',
        __name__ == '__main__' and sys.modules['script'] doesn't exist. On the
        other hand if it is unit tested, __main__ will be the unit test's
        module so it has to reference to itself with 'script'. __name__ always
        match the right value.
        """
        self.module = sys.modules[module]

    def enumerate_commands(self):
        """Returns a dict of command and their handling function.

        The commands must be in the '__main__' modules. To import a command
        from a submodule, use:
            from mysubcommand import CMDfoo

        Automatically adds 'help' if not already defined.

        Normalizes '_' in the commands to '-'.

        A command can be effectively disabled by defining a global variable to
        None, e.g.:
            CMDhelp = None
        """
        cmds = dict((_function_to_name(name), getattr(self.module, name))
                    for name in dir(self.module) if name.startswith('CMD'))
        cmds.setdefault('help', CMDhelp)
        return cmds

    def find_nearest_command(self, name_asked):
        """Retrieves the function to handle a command as supplied by the user.

        It automatically tries to guess the _intended command_ by handling typos
        and/or incomplete names.
        """
        commands = self.enumerate_commands()
        name_to_dash = name_asked.replace('_', '-')
        if name_to_dash in commands:
            return commands[name_to_dash]

        # An exact match was not found. Try to be smart and look if there's
        # something similar.
        commands_with_prefix = [c for c in commands if c.startswith(name_asked)]
        if len(commands_with_prefix) == 1:
            return commands[commands_with_prefix[0]]

        # A #closeenough approximation of levenshtein distance.
        def close_enough(a, b):
            return difflib.SequenceMatcher(a=a, b=b).ratio()

        hamming_commands = sorted(
            ((close_enough(c, name_asked), c) for c in commands), reverse=True)
        if (hamming_commands[0][0] - hamming_commands[1][0]) < 0.3:
            # Too ambiguous.
            return None

        if hamming_commands[0][0] < 0.8:
            # Not similar enough. Don't be a fool and run a random command.
            return None

        return commands[hamming_commands[0][1]]

    def _gen_commands_list(self):
        """Generates the short list of supported commands."""
        commands = self.enumerate_commands()
        docs = sorted(
            (cmd_name, self._create_command_summary(cmd_name, handler))
            for cmd_name, handler in commands.items())
        # Skip commands without a docstring.
        docs = [i for i in docs if i[1]]
        # Then calculate maximum length for alignment:
        length = max(len(c) for c in commands)

        # Look if color is supported.
        colors = _get_color_module()
        green = reset = ''
        if colors:
            green = colors.Fore.GREEN
            reset = colors.Fore.RESET
        return ('Commands are:\n' +
                ''.join('  %s%-*s%s %s\n' %
                        (green, length, cmd_name, reset, doc)
                        for cmd_name, doc in docs))

    def _add_command_usage(self, parser, command):
        """Modifies an OptionParser object with the function's documentation."""
        cmd_name = _function_to_name(command.__name__)
        if cmd_name == 'help':
            cmd_name = '<command>'
            # Use the module's docstring as the description for the 'help'
            # command if available.
            parser.description = (self.module.__doc__ or '').rstrip()
            if parser.description:
                parser.description += '\n\n'
            parser.description += self._gen_commands_list()
            # Do not touch epilog.
        else:
            # Use the command's docstring if available. For commands, unlike
            # module docstring, realign.
            lines = (command.__doc__ or '').rstrip().splitlines()
            if lines[:1]:
                rest = textwrap.dedent('\n'.join(lines[1:]))
                parser.description = '\n'.join((lines[0], rest))
            else:
                parser.description = lines[0] if lines else ''
            if parser.description:
                parser.description += '\n'
            parser.epilog = getattr(command, 'epilog', None)
            if parser.epilog:
                parser.epilog = '\n' + parser.epilog.strip() + '\n'

        more = getattr(command, 'usage_more', '')
        extra = '' if not more else ' ' + more
        parser.set_usage('usage: %%prog %s [options]%s' % (cmd_name, extra))

    @staticmethod
    def _create_command_summary(cmd_name, command):
        """Creates a oneliner summary from the command's docstring."""
        if cmd_name != _function_to_name(command.__name__):
            # Skip aliases. For example using at module level:
            # CMDfoo = CMDbar
            return ''
        doc = command.__doc__ or ''
        line = doc.split('\n', 1)[0].rstrip('.')
        if not line:
            return line
        return (line[0].lower() + line[1:]).strip()

    def execute(self, parser, args):
        """Dispatches execution to the right command.

        Fallbacks to 'help' if not disabled.
        """
        # Unconditionally disable format_description() and format_epilog().
        # Technically, a formatter should be used but it's not worth (yet) the
        # trouble.
        parser.format_description = lambda _: parser.description or ''
        parser.format_epilog = lambda _: parser.epilog or ''

        if args:
            if args[0] in ('-h', '--help') and len(args) > 1:
                # Reverse the argument order so 'tool --help cmd' is rewritten
                # to 'tool cmd --help'.
                args = [args[1], args[0]] + args[2:]
            command = self.find_nearest_command(args[0])
            if command:
                if command.__name__ == 'CMDhelp' and len(args) > 1:
                    # Reverse the argument order so 'tool help cmd' is rewritten
                    # to 'tool cmd --help'. Do it here since we want 'tool help
                    # cmd' to work too.
                    args = [args[1], '--help'] + args[2:]
                    command = self.find_nearest_command(args[0]) or command

                # "fix" the usage and the description now that we know the
                # subcommand.
                self._add_command_usage(parser, command)
                return command(parser, args[1:])

        cmdhelp = self.enumerate_commands().get('help')
        if cmdhelp:
            # Not a known command. Default to help.
            self._add_command_usage(parser, cmdhelp)
            # Don't pass list of arguments as those may not be supported by
            # cmdhelp. See: https://crbug.com/1352093
            return cmdhelp(parser, [])

        # Nothing can be done.
        return 2
