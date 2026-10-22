import os, shutil, argparse, typing, xml.etree.ElementTree, subprocess, platform


class TestResults(object):
    ACTUAL_SUFFIX = '_actual.png'
    EXPECTED_SUFFIX = '.png'

    def __init__(self, results_directory: str, source_expected_directory: str):
        self.results_directory = results_directory
        self.actual_directory = os.path.join(self.results_directory, 'images', 'actual_images')
        self.expected_directory = os.path.join(self.results_directory, 'images', 'expected_images')
        self.source_expected_directory = source_expected_directory

    def get_latest_failed_images(self) -> typing.List[str]:
        failed_images = []
        xml_tree = xml.etree.ElementTree.parse(
            os.path.join(self.results_directory, 'test_detail.xml'))
        testsuites = xml_tree.getroot()
        for testsuite in testsuites.findall('testsuite'):
            for testcase in testsuite.findall('testcase'):
                for properties in testcase.findall('properties'):
                    for property in properties.findall('property'):
                        if property.get('name') == 'FailedImages':
                            failed_images.extend(property.get('value').split(','))

        return failed_images

    def process(self, test_prefixes: typing.List[str], compare: bool, move: bool,
                compare_program: str = ''):
        for test in test_prefixes:
            self.handle_image(test, compare, move, compare_program)

    def handle_image(self, failed_image: str, compare: bool, move: bool, compare_program: str):
        if compare:
            self.show_images(failed_image, compare_program)

            # If the image can be moved, prompt the user after the comparison. But always wait for
            # user input to not spam them with images.
            if move:
                print(f'Update {failed_image}\'s expected image? y/n')
                while True:
                    user_input = input()
                    if user_input == 'y':
                        break
                    elif user_input == 'n':
                        move = False
                        break
            else:
                print(f'Move to next?')
                input()

        if move:
            self.move_actual_to_source([failed_image])

    def show_images(self, failed_image: str, command: str):
        # TODO: Test more on non-mac systems
        open_command: str
        os_name = platform.system().lower()
        if command:
            open_command = command
        elif 'windows' in os_name:
            open_command = 'start'
        elif 'osx' in os_name or 'darwin' in os_name:
            open_command = 'open'
        else:
            open_command = 'xdg-open'

        subprocess.run(
            [open_command,
             os.path.join(self.actual_directory, failed_image + TestResults.ACTUAL_SUFFIX)])
        subprocess.run(
            [open_command,
             os.path.join(self.expected_directory, failed_image + TestResults.EXPECTED_SUFFIX)])

    def move_actual_to_source(self, file_prefixes: typing.List[str]):
        replace_file_names(path=self.actual_directory, removed=TestResults.ACTUAL_SUFFIX,
                           replacement=TestResults.EXPECTED_SUFFIX,
                           output_path=self.source_expected_directory, prefixes=file_prefixes)

def match_suffix(file_name: str, suffix: str, accepted_prefixes: typing.List[str]) -> str:
    """
    Check if the file name is one of the searched for ones with the given suffix and if so return
    it.
    :param accepted_prefixes: If None accepts any prefix
    :return: file_name with the suffix removed or "" if it doesn't match. This does mean a string
    that is just the suffix is considered to not match as it will return the empty string.
    """
    if file_name.endswith(suffix):
        prefix = file_name.removesuffix(suffix)
        if accepted_prefixes is None or prefix in accepted_prefixes:
            return prefix
    return ''


def replace_file_names(path: str, removed: str, replacement: str = '', output_path: str = '',
                       prefixes: typing.Optional[typing.List[str]] = None):
    if not output_path:
        output_path = path
    for file_name in os.listdir(path=path):
        prefix = match_suffix(file_name, removed, prefixes)
        if prefix:
            # Remove the prefix from the list so that prefixes is the list of intended but not yet
            # found files.
            if prefixes is not None:
                prefixes.remove(prefix)
            new_file_name = prefix + replacement
            new_file_path = os.path.join(output_path, new_file_name)
            old_file_path = os.path.join(path, file_name)
            print(f'{old_file_path} to {new_file_path}')
            shutil.copyfile(old_file_path, new_file_path)
    if prefixes is not None:
        for unfound_prefix in prefixes:
            print(f'Failed to find {unfound_prefix}_actual.png')


def main():
    parser = argparse.ArgumentParser(prog='Backend Test File Renamer',
                                     description=
        'Compares and moves actual generated test images to the expected images directory to '
        'update the test requirements. You need to use -r to specify the directory with the test '
        'binary.\nYou likely want the flags -cmx to compare and prompt to move every failed image'
        '\nRemember to sync CMake after running this to move the new expected images to the binary '
        'directory.')
    parser.add_argument('-r', '--results_path',
                        help='The path with the generated images directory, which should be where '
                             'the test binary was run.')
    parser.add_argument('-s', '--source_expected_path', default="./expected_images",
                        help='The directory that updated expected images should be written to, '
                             'which should be the source directory copy.')
    parser.add_argument('-c', '--compare', action='store_true',
                        help='If true, actual and expected images will be displayed to the user '
                             'for comparison.')
    parser.add_argument('-p', '--compare_program', default=None,
                        help='The terminal program that should be used to open images when '
                             'comparing them')
    parser.add_argument('-m', '--move_files', action='store_true',
                        help='If true, the actual images will be copied to overwrite the source '
                             'tree\'s expected image for that test. If the --compare flag is also '
                             'present the user will be prompted for each copy.')
    # The mutually exclusive options for what images to compare.
    parser.add_argument('-t', '--tests', action='extend', nargs='*',
                        help='The list of test images to compare, provided as the name of the '
                             'actual image file without the .png suffix.')
    parser.add_argument('-x', '--xml', action='store_true',
                        help='If true use a test_detail.xml file generated by the test binary to '
                             'compare all images that failed a test. Remember to pass '
                             '`--gtest_output=xml` to the test binary to generate this file.')

    args = parser.parse_args()
    if not args.results_path:
        raise AssertionError("No result path provided")

    # Catch argument mistakes
    if not args.move_files and not args.compare:
        print("Neither instructed to move or compare images")
        return
    if args.compare_program and not args.compare:
        print("Compare program provided but not comparing images")
        return

    results_path = args.results_path
    results = TestResults(results_directory=results_path,
                          source_expected_directory=args.source_expected_path)

    test_prefixes = args.tests
    if args.xml:
        test_prefixes = results.get_latest_failed_images()

    results.process(test_prefixes, compare=args.compare, move=args.move_files,
                    compare_program=args.compare_program)

    if args.move_files:
        print("--------------------------------------------------------")
        print("REMEMBER TO RESYNC CMAKE AND UPDATE HASHES IN TEST FILES")
        print("--------------------------------------------------------")


if __name__ == "__main__":
    main()
