import os
import shutil

from utils import execute, ArgParseImpl

GOLDENS_DIR = 'renderdiff'

class GoldenManager:
  def __init__(self, working_dir, access_token=None):
    self.working_dir_ = working_dir
    self.access_token_ = access_token
    assert os.path.isdir(self.working_dir_),\
        f"working directory {self.working_dir_} does not exist"
    self._prepare()

  def _assets_dir(self):
    return os.path.join(self.working_dir_, "filament-assets")

  def _prepare(self):
    assets_dir = self._assets_dir()
    if not os.path.exists(assets_dir):
      access_token_part = ''
      if self.access_token_:
        access_token_part = f'x-access-token:{self.access_token_}@'
      execute(
          f'git clone --depth=1 https://{access_token_part}github.com/google/filament-assets.git',
          cwd=self.working_dir_)
    else:
      self.update()

  def update(self):
    self._git_exec('fetch')
    self._git_exec('checkout main')
    self._git_exec('rebase')

  def _git_exec(self, cmd):
    execute(f'git {cmd}', cwd=self._assets_dir(), capture_output=False)

  def merge_to_main(self, branch, push_to_remote=False):
    self.update()
    assets_dir = self._assets_dir()
    self._git_exec(f'checkout main')
    self._git_exec(f'merge --no-ff {branch}')
    if push_to_remote and self.access_token_:
      self._git_exec(f'push origin main')

  def source_from_and_commit(self, src_dir, commit_msg, branch, push_to_remote=False):
    assets_dir = self._assets_dir()
    self._git_exec(f'checkout main')
    # Force create the branch (note will overwrite the old branch)
    self._git_exec(f'switch -C {branch}')
    rdiff_dir = os.path.join(assets_dir, GOLDENS_DIR)
    execute(f'rm -rf {rdiff_dir}')
    execute(f'mkdir -p {rdiff_dir}')
    shutil.copytree(src_dir, rdiff_dir, dirs_exist_ok=True)
    self._git_exec(f'add {GOLDENS_DIR}')

    TMP_GOLDEN_COMMIT_FILE = '/tmp/golden_commit.txt'

    with open(TMP_GOLDEN_COMMIT_FILE, 'w') as f:
      f.write(commit_msg)
    self._git_exec(f'commit -F {TMP_GOLDEN_COMMIT_FILE}')
    if push_to_remote and self.access_token_:
      self._git_exec(f'push -f origin ${branch}')

  def download_to(self, dest_dir, branch='main'):
    assets_dir = self._assets_dir()
    execute(f'mkdir -p {dest_dir}')
    rdiff_dir = os.path.join(assets_dir, GOLDENS_DIR)
    shutil.copytree(rdiff_dir, dest_dir, dirs_exist_ok=True)

# For testing only
if __name__ == "__main__":
  golden_manager = GoldenManager(os.getcwd())
  # golden_manager.source_from_and_commit(
  #     os.path.join(os.getcwd(), 'out/renderdiff_tests'),
  #     'First commit (local)',
  #     branch='branch-test')
  # golden_manager.merge_to_main('branch-test', push_to_remote=True)
  # golden_manager.download_to(os.path.join(os.getcwd(), 'tmp/goldens'))
