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

    def handle_failed_image(self, failed_image):
        self.show_images(failed_image)
        print(f'Update {failed_image}\'s expected image? y/n')
        while True:
            user_input = input()
            if user_input == 'y':
                self.move_actual_to_source([failed_image])
                break
            elif user_input == 'n':
                break

    def handle_all_failed_images(self):
        for failed_image in self.get_latest_failed_images():
            self.handle_failed_image(failed_image)

    def show_images(self, failed_image):
        # TODO: Test more on non-mac systems
        open_command: str
        os_name = platform.system().lower()
        if 'windows' in os_name:
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

    def batch_move(self, prefixes: typing.Optional[typing.List[str]] = None):
        replace_file_names(path=self.actual_directory, removed=TestResults.ACTUAL_SUFFIX,
                           replacement=TestResults.EXPECTED_SUFFIX,
                           output_path=self.source_expected_directory, prefixes=prefixes)


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


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='Backend Test File Renamer',
                                     description='Moves actual generated test images to the '
                                                 'expected images directory, to update the test '
                                                 'requirements. test_cases accepts multiple '
                                                 'arguments that should be the name of the '
                                                 'expected image file without the .png suffix. '
                                                 'Also --all can be passed to copy all images.\n'
                                                 'Remember to sync CMake after running this to '
                                                 'move the new expected images to the binary '
                                                 'directory.')
    parser.add_argument('-r', '--results_path',
                        help='The path with the generated images directory, which should be where '
                             'the test binary was run.')
    parser.add_argument('-s', '--source_expected_path', default="./expected_images",
                        help='The directory that updated expected images should be written to, '
                             'which should be the source directory copy.')
    # The mutually exclusive options for how to process the actual images
    parser.add_argument('-b', '--batch', action='extend', nargs='*',
                        help='If true copy all actual images to the source expected image '
                             'directory.')
    parser.add_argument('-a', '--all', action='store_true',
                        help='If true, visually compare all generated images.')
    parser.add_argument('-t', '--tests', action='store_true',
                        help='If true use a test_detail.xml file that exists in the results_path '
                             'directory to visually compare all images that failed a test.')
    parser.add_argument('-c', '--compare', action='extend', nargs='*',
                        help='A list of image names to visually compare (without the .png suffix).')

    args = parser.parse_args()
    if not args.results_path:
        raise AssertionError("No result path provided")
    results_path = args.results_path

    results = TestResults(results_directory=results_path,
                          source_expected_directory=args.source_expected_path)

    if args.all:
        results.batch_move()
    elif args.tests:
        results.handle_all_failed_images()
    elif args.compare:
        for file_prefix in args.compare:
            results.show_images(file_prefix)
    else:
        results.batch_move(args.batch)
    print("--------------------------------------------------------")
    print("REMEMBER TO RESYNC CMAKE AND UPDATE HASHES IN TEST FILES")
    print("--------------------------------------------------------")
