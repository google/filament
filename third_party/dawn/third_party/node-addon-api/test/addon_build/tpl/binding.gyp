{
  'target_defaults': {
    'include_dirs': [
        "<!(node -p \"require('node-addon-api').include_dir\")"
    ],
    'variables': {
      'NAPI_VERSION%': "<!(node -p \"process.versions.napi\")",
      'disable_deprecated': "<!(node -p \"process.env['npm_config_disable_deprecated']\")"
    },
    'conditions': [
      ['NAPI_VERSION!=""', { 'defines': ['NAPI_VERSION=<@(NAPI_VERSION)'] } ],
      ['disable_deprecated=="true"', {
        'defines': ['NODE_ADDON_API_DISABLE_DEPRECATED']
      }],
      ['OS=="mac"', {
        'cflags+': ['-fvisibility=hidden'],
        'xcode_settings': {
          'OTHER_CFLAGS': ['-fvisibility=hidden']
        }
      }]
    ],
    'sources': [
      'addon.cc',
    ],
  },
  'targets': [
    {
      'target_name': 'addon',
      'defines': [ 'NAPI_CPP_EXCEPTIONS' ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'ExceptionHandling': 1,
          'EnablePREfast': 'true',
        },
      },
      'xcode_settings': {
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
      },
    },
    {
      'target_name': 'addon_noexcept',
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'cflags': [ '-fno-exceptions' ],
      'cflags_cc': [ '-fno-exceptions' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'ExceptionHandling': 0,
          'EnablePREfast': 'true',
        },
      },
      'xcode_settings': {
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'NO',
      },
    },
  ],
}
