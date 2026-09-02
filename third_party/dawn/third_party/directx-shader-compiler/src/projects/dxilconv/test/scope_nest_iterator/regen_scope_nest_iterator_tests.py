import argparse
import os
import glob
import subprocess
import difflib
import sys

class Regen:
    def __init__(self, path, options):
        self.path = path
        self.opt  = options.opt
        self.checkout = options.sd
        self.dryrun = options.dryrun
    
    def run(self):
        old,new = self.get_diff()
        if old != new:
            print("+++    check lines changed {}".format(self.path))
            self.print_diff(old, new)
            if not self.dryrun:
                self.update_test(new)
        else:
            print("--- no check lines changed {}".format(self.path))

    def update_test(self, new):
        if self.checkout and not os.access(self.path, os.W_OK):
            self.sd("edit", self.path)
        
        if not os.access(self.path, os.W_OK):
            print("ERROR: unable to write to file '{}''".format(self.path))
            sys.exit(1)

        with open(self.path, "w") as f:
            f.write(new)

    def sd(self, *args):
        cmd = ["sd"] + list(args)
        subprocess.run(cmd, check=True)

    def print_diff(self, old, new):
        for line in difflib.unified_diff(old.splitlines(keepends=True), new.splitlines(keepends=True), fromfile='old', tofile='new'):
            sys.stdout.write(line) 

    def get_diff(self):
        old = self.get_test_content()
        old_check = self.get_old_check_lines()
        new_check = self.get_new_check_lines()
        new = old.replace(old_check, new_check)
        
        return (old, new)

    def get_test_content(self):
        with open(self.path) as f:
            return f.read() 
   
    def get_run_line(self):
        for line in self.get_test_content().splitlines():
            if "RUN:" in line:
                start = line.find(":") + 1
                end   = line.find("|")
                return line[start:end]
    
        raise RuntimeError("Could not find run line")

    def get_run_command(self):
        run_line = self.get_run_line()
        run_line = run_line.replace("%opt-exe", self.opt)
        run_line = run_line.replace("%s", self.path)
        return run_line.split()

    def get_new_output(self):
        cmd  = self.get_run_command()
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, check=True, universal_newlines=True)
        return proc.stdout
    
    def get_new_check_lines(self):
        output = self.get_new_output()
        check_lines = []
        grab_lines = False
        for line in output.splitlines():
            if not line.strip():
                continue
            if line.startswith("ScopeNestInfo:"):
                grab_lines = True                
            if grab_lines:
                check = "; CHECK: " + line
                check_lines.append(check)
            

        return "\n".join(check_lines) 
    
    def get_old_check_lines(self):
        check_lines = []
        for line in self.get_test_content().splitlines():
            if "; CHECK:" in line:
                check_lines.append(line)
        return "\n".join(check_lines)

def parse_args():
    default_opt = os.path.join(os.environ["OBJECT_ROOT"], "onecoreuap\\windows\\directx\\dxg\\opensource\\llvm\\src\\tools\\opt", os.environ["_BuildAlt"], "opt.exe")

    parser = argparse.ArgumentParser(description="regenerate check lines in test")
    parser.add_argument("--opt", default=default_opt,
                        help="path to opt")
    parser.add_argument("--sd", action='store_true', help="sd edit the files")                    
    parser.add_argument("tests", nargs="*", help="glob patten of tests to run", default=["*.ll"])
    parser.add_argument("--dryrun", action="store_true")

    args = parser.parse_args()
    return args

def get_tests(options):
    test_files = []
    for pattern in options.tests:
        test_files.extend(glob.glob(pattern))
    return test_files

def main():
    options = parse_args()
    tests = get_tests(options)
    
    for test in tests:
        Regen(test, options).run()

if __name__ == "__main__":
    main()