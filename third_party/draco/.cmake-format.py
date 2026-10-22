with section('parse'):
  # Specify structure for custom cmake functions
  additional_commands = {
      'draco_add_emscripten_executable': {
          'kwargs': {
              'NAME': '*',
              'SOURCES': '*',
              'OUTPUT_NAME': '*',
              'DEFINES': '*',
              'INCLUDES': '*',
              'COMPILE_FLAGS': '*',
              'LINK_FLAGS': '*',
              'OBJLIB_DEPS': '*',
              'LIB_DEPS': '*',
              'GLUE_PATH': '*',
              'PRE_LINK_JS_SOURCES': '*',
              'POST_LINK_JS_SOURCES': '*',
              'FEATURES': '*',
          },
          'pargs': 0,
      },
      'draco_add_executable': {
          'kwargs': {
              'NAME': '*',
              'SOURCES': '*',
              'OUTPUT_NAME': '*',
              'TEST': 0,
              'DEFINES': '*',
              'INCLUDES': '*',
              'COMPILE_FLAGS': '*',
              'LINK_FLAGS': '*',
              'OBJLIB_DEPS': '*',
              'LIB_DEPS': '*',
          },
          'pargs': 0,
      },
      'draco_add_library': {
          'kwargs': {
              'NAME': '*',
              'TYPE': '*',
              'SOURCES': '*',
              'TEST': 0,
              'OUTPUT_NAME': '*',
              'DEFINES': '*',
              'INCLUDES': '*',
              'COMPILE_FLAGS': '*',
              'LINK_FLAGS': '*',
              'OBJLIB_DEPS': '*',
              'LIB_DEPS': '*',
              'PUBLIC_INCLUDES': '*',
          },
          'pargs': 0,
      },
      'draco_generate_emscripten_glue': {
          'kwargs': {
              'INPUT_IDL': '*',
              'OUTPUT_PATH': '*',
          },
          'pargs': 0,
      },
      'draco_get_required_emscripten_flags': {
          'kwargs': {
              'FLAG_LIST_VAR_COMPILER': '*',
              'FLAG_LIST_VAR_LINKER': '*',
          },
          'pargs': 0,
      },
      'draco_option': {
          'kwargs': {
              'NAME': '*',
              'HELPSTRING': '*',
              'VALUE': '*',
          },
          'pargs': 0,
      },
      # Rules for built in CMake commands and those from dependencies.
      'list': {
          'kwargs': {
              'APPEND': '*',
              'FILTER': '*',
              'FIND': '*',
              'GET': '*',
              'INSERT': '*',
              'JOIN': '*',
              'LENGTH': '*',
              'POP_BACK': '*',
              'POP_FRONT': '*',
              'PREPEND': '*',
              'REMOVE_DUPLICATES': '*',
              'REMOVE_ITEM': '*',
              'REVERSE': '*',
              'SORT': '*',
              'SUBLIST': '*',
              'TRANSFORM': '*',
          },
      },
      'protobuf_generate': {
        'kwargs': {
            'IMPORT_DIRS': '*',
            'LANGUAGE': '*',
            'OUT_VAR': '*',
            'PROTOC_OUT_DIR': '*',
            'PROTOS': '*',
        },
      },
  }

with section('format'):
  # Formatting options.

  # How wide to allow formatted cmake files
  line_width = 80

  # How many spaces to tab for indent
  tab_size = 2

  # If true, separate flow control names from their parentheses with a space
  separate_ctrl_name_with_space = False

  # If true, separate function names from parentheses with a space
  separate_fn_name_with_space = False

  # If a statement is wrapped to more than one line, than dangle the closing
  # parenthesis on its own line.
  dangle_parens = False

  # Do not sort argument lists.
  enable_sort = False

  # What style line endings to use in the output.
  line_ending = 'unix'

  # Format command names consistently as 'lower' or 'upper' case
  command_case = 'canonical'

  # Format keywords consistently as 'lower' or 'upper' case
  keyword_case = 'upper'
