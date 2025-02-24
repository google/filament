from pdb import set_trace
class Parser(object):

    def __init__(self):
        self.in_code = False
        self.code = []

    def parse(self, x):
        for line in x:
            self.parse_line(line)

    def parse_line(self, line):
        line = line[:-1]
        is_code = False
        if self.in_code:
            if line.strip() and not line.startswith(" "):
                self.in_code = False
            else:
                is_code = True
        elif line.strip().endswith("::"):
            self.in_code = True

        if is_code:
            self.code.append(line[1:])
            
parser = Parser()
parser.parse(open("index.rst").readlines())
print("\n".join(parser.code))
