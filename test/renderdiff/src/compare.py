import glob
import os
import sys
import pprint
import json

from utils import execute, ArgParseImpl, important_print, mkdir_p
from image_diff import same_image, output_image_diff
from results import RESULT_OK, RESULT_FAILED, RESULT_MISSING

def _compare_goldens(base_dir, comparison_dir, out_dir=None):
  all_files = glob.glob(os.path.join(base_dir, "./**/*.tif"), recursive=True)
  test_dirs = set(os.path.abspath(os.path.dirname(f)).replace(os.path.abspath(base_dir) + '/', '') \
                  for f in all_files)
  all_results = []
  for test_dir in test_dirs:
    results = []
    output_test_dir = None if not out_dir else os.path.abspath(os.path.join(out_dir, test_dir))
    if output_test_dir:
      mkdir_p(output_test_dir)
    base_test_dir = os.path.abspath(os.path.join(base_dir, test_dir))
    comp_test_dir = os.path.abspath(os.path.join(comparison_dir, test_dir))
    for golden_file in \
        glob.glob(os.path.join(base_test_dir, "*.tif")):
      base_fname = os.path.abspath(golden_file)
      test_case = base_fname.replace(f'{base_test_dir}/', '')
      comp_fname = os.path.join(comp_test_dir, test_case)
      result = {
        'name': test_case,
      }
      if not os.path.exists(comp_fname):
        print(f'file name not found: {comp_fname}')
        result['result'] = RESULT_MISSING
      elif not same_image(base_fname, comp_fname):
        result['result'] = RESULT_FAILED
        if output_test_dir:
          # just the file name
          diff_fname = f"{test_case.replace('.tif', '_diff.tif')}"
          output_image_diff(base_fname, comp_fname, os.path.join(output_test_dir, diff_fname))
          result['diff'] = diff_fname
      else:
        result['result'] = RESULT_OK
      results.append(result)
    if output_test_dir:
      output_fname = os.path.join(output_test_dir, "compare_results.json")
      results_meta = {
        'results': results,
        'base_dir': os.path.relpath(output_fname, base_test_dir),
        'comparison_dir': os.path.relpath(output_fname, comp_test_dir),
      }
      with open(output_fname, 'w') as f:
        f.write(json.dumps(results_meta, indent=2))
      important_print(f'Written comparison results for {test_dir} to \n    {output_fname}')
    all_results += results
  return all_results

if __name__ == '__main__':
  parser = ArgParseImpl()
  parser.add_argument('--src', help='Directory of the base of the diff.', required=True)
  parser.add_argument('--dest', help='Directory of the comparison of the diff.')
  parser.add_argument('--out', help='Directory of output for the result of the diff.')

  args, _ = parser.parse_known_args(sys.argv[1:])

  dest = args.dest
  if not dest:
    print('Assume the default renderdiff output folder')
    dest = os.path.join(os.getcwd(), './out/renderdiff')
  assert os.path.exists(dest), f"Destination folder={dest} does not exist."

  results = _compare_goldens(args.src, dest, out_dir=args.out)

  failed = [f"   {k['name']}" for k in results if k['result'] != RESULT_OK]
  success_count = len(results) - len(failed)
  important_print(f'Successfully compared {success_count} / {len(results)} images' +
                    ('\nFailed:\n' + ('\n'.join(failed)) if len(failed) > 0 else ''))
  if len(failed) > 0:
    exit(1)
