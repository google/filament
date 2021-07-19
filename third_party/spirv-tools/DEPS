use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'effcee_revision': '2ec8f8738118cc483b67c04a759fee53496c5659',
  'googletest_revision': 'b7d472f1225c5a64943821d8483fecb469d3f382',
  're2_revision': 'f8e389f3acdc2517562924239e2a188037393683',
  'spirv_headers_revision': 'ddf3230c14c71e81fc0eae9b781cc4bcc2d1f0f5',
}

deps = {
  'external/effcee':
      Var('github') + '/google/effcee.git@' + Var('effcee_revision'),

  'external/googletest':
      Var('github') + '/google/googletest.git@' + Var('googletest_revision'),

  'external/re2':
      Var('github') + '/google/re2.git@' + Var('re2_revision'),

  'external/spirv-headers':
      Var('github') +  '/KhronosGroup/SPIRV-Headers.git@' +
          Var('spirv_headers_revision'),
}

