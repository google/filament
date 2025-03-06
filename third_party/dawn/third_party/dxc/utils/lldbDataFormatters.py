"""
LLDB Formatters for LLVM data types.

Load into LLDB with 'command script import /path/to/lldbDataFormatters.py'
"""

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand('type category define -e llvm -l c++')
    debugger.HandleCommand('type synthetic add -w llvm '
                           '-l lldbDataFormatters.SmallVectorSynthProvider '
                           '-x "^llvm::SmallVectorImpl<.+>$"')
    debugger.HandleCommand('type synthetic add -w llvm '
                           '-l lldbDataFormatters.SmallVectorSynthProvider '
                           '-x "^llvm::SmallVector<.+,.+>$"')
    debugger.HandleCommand('type synthetic add -w llvm '
                           '-l lldbDataFormatters.ArrayRefSynthProvider '
                           '-x "^llvm::ArrayRef<.+>$"')
    debugger.HandleCommand('type summary add -w llvm '
                           '-F lldbDataFormatters.OptionalSummaryProvider '
                           '-x "^llvm::Optional<.+>$"')

# Pretty printer for llvm::SmallVector/llvm::SmallVectorImpl
class SmallVectorSynthProvider:
    def __init__(self, valobj, dict):
        self.valobj = valobj;
        self.update() # initialize this provider

    def num_children(self):
        begin = self.begin.GetValueAsUnsigned(0)
        end = self.end.GetValueAsUnsigned(0)
        return (end - begin)/self.type_size

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1;

    def get_child_at_index(self, index):
        # Do bounds checking.
        if index < 0:
            return None
        if index >= self.num_children():
            return None;

        offset = index * self.type_size
        return self.begin.CreateChildAtOffset('['+str(index)+']',
                                              offset, self.data_type)

    def update(self):
        self.begin = self.valobj.GetChildMemberWithName('BeginX')
        self.end = self.valobj.GetChildMemberWithName('EndX')
        the_type = self.valobj.GetType()
        # If this is a reference type we have to dereference it to get to the
        # template parameter.
        if the_type.IsReferenceType():
            the_type = the_type.GetDereferencedType()

        self.data_type = the_type.GetTemplateArgumentType(0)
        self.type_size = self.data_type.GetByteSize()
        assert self.type_size != 0

class ArrayRefSynthProvider:
    """ Provider for llvm::ArrayRef """
    def __init__(self, valobj, dict):
        self.valobj = valobj;
        self.update() # initialize this provider

    def num_children(self):
        return self.length

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1;

    def get_child_at_index(self, index):
        if index < 0 or index >= self.num_children():
            return None;
        offset = index * self.type_size
        return self.data.CreateChildAtOffset('[' + str(index) + ']',
                                             offset, self.data_type)

    def update(self):
        self.data = self.valobj.GetChildMemberWithName('Data')
        length_obj = self.valobj.GetChildMemberWithName('Length')
        self.length = length_obj.GetValueAsUnsigned(0)
        self.data_type = self.data.GetType().GetPointeeType()
        self.type_size = self.data_type.GetByteSize()
        assert self.type_size != 0

def OptionalSummaryProvider(valobj, internal_dict):
    if not valobj.GetChildMemberWithName('hasVal').GetValueAsUnsigned(0):
        return 'None'
    underlying_type = valobj.GetType().GetTemplateArgumentType(0)
    storage = valobj.GetChildMemberWithName('storage')
    return str(storage.Cast(underlying_type))
