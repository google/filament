import os, shutil, argparse, typing

def match_sufffix(file_name: str, suffix: str, accepted_prefixes: typing.List[str]) -> str:
  """
  Check if the file name is one of the searched for ones with the given suffix and if so return it.
  :param accepted_prefixes: If None accepts any prefix
  :return: file_name with the suffix removed or "" if it doesn't match. This does mean a string that
  is just the suffix is considered to not match as it will return the empty string.
  """
  if file_name.endswith(suffix):
    prefix = file_name.removesuffix(suffix)
    if accepted_prefixes is None or prefix in accepted_prefixes:
      return prefix
  return ""


def replace_file_names(path: str, removed: str, replacement: str = "", output_path: str = "",
                       prefixes: typing.List[str] = None):
  if not output_path:
    output_path = path
  for file_name in os.listdir(path=path):
    prefix = match_sufffix(file_name, removed, prefixes)
    if prefix:
      # Remove the prefix from the list so that prefixes is the list of intended but not yet found
      # files.
      if prefixes is not None:
        prefixes.remove(prefix)
      new_file_name = prefix + replacement
      new_file_path = os.path.join(output_path, new_file_name)
      old_file_path = os.path.join(path, file_name)
      print(f'{old_file_path} to {new_file_path}')
      shutil.move(old_file_path, new_file_path)
  if prefixes is not None:
    for unfound_prefix in prefixes:
      print(f'Failed to find {unfound_prefix}_actual.png')

if __name__ == "__main__":
  parser = argparse.ArgumentParser(prog='Backend Test File Renamer',
                                   description='Moves actual generated test images to the expected '
                                               'images directory, to update the test requirements. '
                                               'test_cases accepts multiple arguments that should '
                                               'be the name of the expected image file without the '
                                               '.png suffix. Also --all can be passed to copy all '
                                               'images.\n'
                                               'Remember to sync CMake after running this to move '
                                               'the new expected images to the binary directory.')
  parser.add_argument('-i', '--input_path')
  parser.add_argument('-o', '--output_path', default="./expected_images")
  parser.add_argument('-t', '--test_cases', action='extend', nargs='*')
  parser.add_argument('-a', '--all', action='store_true')

  args = parser.parse_args()
  input_path = "."
  if args.input_path:
    input_path = args.input_path

  prefixes = args.test_cases
  if args.all:
    prefixes = None

  replace_file_names(path=input_path, output_path=args.output_path, removed="_actual.png",
                     replacement=".png", prefixes=prefixes)
