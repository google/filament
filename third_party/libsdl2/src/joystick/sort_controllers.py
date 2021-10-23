#!/usr/bin/env python
#
# Script to sort the game controller database entries in SDL_gamecontroller.c

import re


filename = "SDL_gamecontrollerdb.h"
input = open(filename)
output = open(filename + ".new", "w")
parsing_controllers = False
controllers = []
controller_guids = {}
conditionals = []
split_pattern = re.compile(r'([^"]*")([^,]*,)([^,]*,)([^"]*)(".*)')

def find_element(prefix, bindings):
    i=0
    for element in bindings:
        if element.startswith(prefix):
            return i
        i=(i + 1)

    return -1
       
def save_controller(line):
    global controllers
    match = split_pattern.match(line)
    entry = [ match.group(1), match.group(2), match.group(3) ]
    bindings = sorted(match.group(4).split(","))
    if (bindings[0] == ""):
        bindings.pop(0)

    pos=find_element("sdk", bindings)
    if pos >= 0:
        bindings.append(bindings.pop(pos))

    pos=find_element("hint:", bindings)
    if pos >= 0:
        bindings.append(bindings.pop(pos))

    entry.extend(",".join(bindings) + ",")
    entry.append(match.group(5))
    controllers.append(entry)

    if ',sdk' in line or ',hint:' in line:
        conditionals.append(entry[1])

def write_controllers():
    global controllers
    global controller_guids
    # Check for duplicates
    for entry in controllers:
        if (entry[1] in controller_guids and entry[1] not in conditionals):
            current_name = entry[2]
            existing_name = controller_guids[entry[1]][2]
            print("Warning: entry '%s' is duplicate of entry '%s'" % (current_name, existing_name))

            if (not current_name.startswith("(DUPE)")):
                entry[2] = "(DUPE) " + current_name

            if (not existing_name.startswith("(DUPE)")):
                controller_guids[entry[1]][2] = "(DUPE) " + existing_name

        controller_guids[entry[1]] = entry

    for entry in sorted(controllers, key=lambda entry: entry[2]+"-"+entry[1]):
        line = "".join(entry) + "\n"
        line = line.replace("\t", "    ")
        if not line.endswith(",\n") and not line.endswith("*/\n") and not line.endswith(",\r\n") and not line.endswith("*/\r\n"):
            print("Warning: '%s' is missing a comma at the end of the line" % (line))
        output.write(line)

    controllers = []
    controller_guids = {}

for line in input:
    if (parsing_controllers):
        if (line.startswith("{")):
            output.write(line)
        elif (line.startswith("    NULL")):
            parsing_controllers = False
            write_controllers()
            output.write(line)
        elif (line.startswith("#if")):
            print("Parsing " + line.strip())
            output.write(line)
        elif (line.startswith("#endif")):
            write_controllers()
            output.write(line)
        else:
            save_controller(line)
    else:
        if (line.startswith("static const char *s_ControllerMappings")):
            parsing_controllers = True

        output.write(line)

output.close()
print("Finished writing %s.new" % filename)
