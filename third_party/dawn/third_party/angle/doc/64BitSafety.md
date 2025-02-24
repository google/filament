# 64-bit Safety In the Compiler

An issue that has arisen recently for contributors making changes to the GLSL ES
grammar files has been that certain versions of flex, the lexer on which ANGLE
relies, produce outputs which are not safe in 64-bit builds.

To address this issue, ANGLE has added a step to its generation scripts to apply
64-bit safety fixes to newly regenerated outputs. This should be unnoticeable to
developers invoking flex via the generate\_parser.sh scripts in the relevant
compiler directories, as the fixes will be applied by the patch utility as part
of that script.

When making code contributions that affect the grammar files, please ensure that
you've generated the outputs using the script, to make certain that the 64-bit
safety fixes are applied.
