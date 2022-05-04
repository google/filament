use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'effcee_revision': 'ddf5e2bb92957dc8a12c5392f8495333d6844133',
  'googletest_revision': '25dcdc7e8bfac8967f20fb2c0a628f5cf442188d',
  're2_revision': '0c5616df9c0aaa44c9440d87422012423d91c7d1',
  'spirv_headers_revision': '4995a2f2723c401eb0ea3e10c81298906bf1422b',
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

