import os
import sys
import platform
import itertools
import lit.util

OldPy = sys.version_info[0] == 2 and sys.version_info[1] < 7

def strip_dxil_validator_path(env_path):
    dxil_name, separator = ('dxil.dll', ';') if platform.system() == 'Windows' else ('dxil.so', ';')
    return separator.join([
        p for p in env_path.split(separator)
        if not os.path.isfile(os.path.join(p, dxil_name))
        ])

def _find_git_windows_unix_tools(tools_needed):
    assert(sys.platform == 'win32')
    if sys.version_info.major >= 3:
        import winreg
    else:
        import _winreg as winreg

    # Search both the 64 and 32-bit hives, as well as HKLM + HKCU
    masks = [0, winreg.KEY_WOW64_64KEY]
    hives = [winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER]
    for mask, hive in itertools.product(masks, hives):
        try:
            with winreg.OpenKey(hive, r"SOFTWARE\GitForWindows", 0,
                                winreg.KEY_READ | mask) as key:
                install_root, _ = winreg.QueryValueEx(key, 'InstallPath')
                if not install_root:
                    continue
                candidate_path = os.path.join(install_root, 'usr', 'bin')
                if not lit.util.checkToolsPath(
                            candidate_path, tools_needed):
                    continue

                # We found it, stop enumerating.
                return lit.util.to_string(candidate_path)
        except:
            continue
    raise(f"fail to find {tools_needed} which are required for DXC tests")

class TestingConfig:
    """"
    TestingConfig - Information on the tests inside a suite.
    """


    @staticmethod
    def fromdefaults(litConfig):
        """
        fromdefaults(litConfig) -> TestingConfig

        Create a TestingConfig object with default values.
        """
        # Set the environment based on the command line arguments.

        # strip dxil validator dir if not need it.
        all_path = os.pathsep.join(litConfig.path +
                                     [os.environ.get('PATH','')])

        all_path = strip_dxil_validator_path(all_path)
        if sys.platform == 'win32':
            required_tools = [
                'cmp.exe', 'grep.exe', 'sed.exe', 'diff.exe', 'echo.exe', 'ls.exe']
            path = lit.util.whichTools(required_tools, all_path)
            if path is None:
                path = _find_git_windows_unix_tools(required_tools)

            all_path = f"{path};{all_path}"
        environment = {
            'PATH' : all_path,
            'LLVM_DISABLE_CRASH_REPORT' : '1',
            }

        pass_vars = ['LIBRARY_PATH', 'LD_LIBRARY_PATH', 'SYSTEMROOT', 'TERM',
                     'LD_PRELOAD', 'ASAN_OPTIONS', 'UBSAN_OPTIONS',
                     'LSAN_OPTIONS', 'ADB', 'ADB_SERIAL']
        for var in pass_vars:
            val = os.environ.get(var, '')
            # Check for empty string as some variables such as LD_PRELOAD cannot be empty
            # ('') for OS's such as OpenBSD.
            if val:
                environment[var] = val

        if sys.platform == 'win32':
            environment.update({
                    'INCLUDE' : os.environ.get('INCLUDE',''),
                    'PATHEXT' : os.environ.get('PATHEXT',''),
                    'PYTHONUNBUFFERED' : '1',
                    'USERPROFILE' : os.environ.get('USERPROFILE',''),
                    'TEMP' : os.environ.get('TEMP',''),
                    'TMP' : os.environ.get('TMP',''),
                    })

        # The option to preserve TEMP, TMP, and TMPDIR.
        # This is intended to check how many temporary files would be generated
        # (and be not cleaned up) in automated builders.
        if 'LIT_PRESERVES_TMP' in os.environ:
            environment.update({
                    'TEMP' : os.environ.get('TEMP',''),
                    'TMP' : os.environ.get('TMP',''),
                    'TMPDIR' : os.environ.get('TMPDIR',''),
                    })

        # Set the default available features based on the LitConfig.
        available_features = []
        if litConfig.useValgrind:
            available_features.append('valgrind')
            if litConfig.valgrindLeakCheck:
                available_features.append('vg_leak')

        return TestingConfig(None,
                             name = '<unnamed>',
                             suffixes = set(),
                             test_format = None,
                             environment = environment,
                             substitutions = [],
                             unsupported = False,
                             test_exec_root = None,
                             test_source_root = None,
                             excludes = [],
                             available_features = available_features,
                             pipefail = True)

    def load_from_path(self, path, litConfig):
        """
        load_from_path(path, litConfig)

        Load the configuration module at the provided path into the given config
        object.
        """

        # Load the config script data.
        data = None
        if not OldPy:
            f = open(path)
            try:
                data = f.read()
            except:
                litConfig.fatal('unable to load config file: %r' % (path,))
            f.close()

        # Execute the config script to initialize the object.
        cfg_globals = dict(globals())
        cfg_globals['config'] = self
        cfg_globals['lit_config'] = litConfig
        cfg_globals['__file__'] = path
        try:
            if OldPy:
                execfile(path, cfg_globals)
            else:
                exec(compile(data, path, 'exec'), cfg_globals, None)
            if litConfig.debug:
                litConfig.note('... loaded config %r' % path)
        except SystemExit:
            e = sys.exc_info()[1]
            # We allow normal system exit inside a config file to just
            # return control without error.
            if e.args:
                raise
        except:
            import traceback
            litConfig.fatal(
                'unable to parse config file %r, traceback: %s' % (
                    path, traceback.format_exc()))

        self.finish(litConfig)

    def __init__(self, parent, name, suffixes, test_format,
                 environment, substitutions, unsupported,
                 test_exec_root, test_source_root, excludes,
                 available_features, pipefail, limit_to_features = []):
        self.parent = parent
        self.name = str(name)
        self.suffixes = set(suffixes)
        self.test_format = test_format
        self.environment = dict(environment)
        self.substitutions = list(substitutions)
        self.unsupported = unsupported
        self.test_exec_root = test_exec_root
        self.test_source_root = test_source_root
        self.excludes = set(excludes)
        self.available_features = set(available_features)
        self.pipefail = pipefail
        # This list is used by TestRunner.py to restrict running only tests that
        # require one of the features in this list if this list is non-empty.
        # Configurations can set this list to restrict the set of tests to run.
        self.limit_to_features = set(limit_to_features)

    def finish(self, litConfig):
        """finish() - Finish this config object, after loading is complete."""

        self.name = str(self.name)
        self.suffixes = set(self.suffixes)
        self.environment = dict(self.environment)
        self.substitutions = list(self.substitutions)
        if self.test_exec_root is not None:
            # FIXME: This should really only be suite in test suite config
            # files. Should we distinguish them?
            self.test_exec_root = str(self.test_exec_root)
        if self.test_source_root is not None:
            # FIXME: This should really only be suite in test suite config
            # files. Should we distinguish them?
            self.test_source_root = str(self.test_source_root)
        self.excludes = set(self.excludes)

    @property
    def root(self):
        """root attribute - The root configuration for the test suite."""
        if self.parent is None:
            return self
        else:
            return self.parent.root

