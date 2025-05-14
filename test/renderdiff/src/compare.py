import glob
import os
import sys
import pprint
import json

from utils import execute, ArgParseImpl, important_print
from image_diff import same_image
from results import RESULT_OK, RESULT_FAILED, RESULT_MISSING

def _compare_goldens(base_dir, comparison_dir):
  render_results = {}
  base_files = glob.glob(os.path.join(base_dir, "./**/*.tif"))
  for golden_file in base_files:
    base_fname = os.path.abspath(golden_file)
    test_case = base_fname.replace(f'{os.path.abspath(base_dir)}/', '')
    comp_fname = os.path.abspath(os.path.join(comparison_dir, test_case))
    if not os.path.exists(comp_fname):
      print(f'file name not found: {comp_fname}')
      render_results[test_case] = RESULT_MISSING
      continue
    if not same_image(base_fname, comp_fname):
      render_results[test_case] = RESULT_FAILED
    else:
      render_results[test_case] = RESULT_OK
  return render_results

if __name__ == '__main__':
  parser = ArgParseImpl()
  parser.add_argument('--src', help='Directory of the base of the diff.', required=True)
  parser.add_argument('--dest', help='Directory of the comparison of the diff.')
  parser.add_argument('--out', help='Directory of output for the result of the diff.')

  args, _ = parser.parse_known_args(sys.argv[1:])

  dest = args.dest
  if not dest:
    print('Assume the default renderdiff output folder')
    dest = os.path.join(os.getcwd(), './out/renderdiff_tests')
  assert os.path.exists(dest), f"Destination folder={dest} does not exist."

  results = _compare_goldens(args.src, dest)

  if args.out:
    assert os.path.exists(arg.out), f"Output folder={dest} does not exist."
    with open(os.path.join(args.out, "compare_results.json", 'w')) as f:
      f.write(json.dumps(results))

  failed = [f"   {k}" for k in results.keys() if results[k] != RESULT_OK]
  success_count = len(results) - len(failed)
  important_print(f'Successfully compared {success_count} / {len(results)} images' +
                    ('\nFailed:\n' + ('\n'.join(failed)) if len(failed) > 0 else ''))
  if len(failed) > 0:
    exit(1)
