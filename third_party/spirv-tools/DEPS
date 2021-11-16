use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'effcee_revision': 'ddf5e2bb92957dc8a12c5392f8495333d6844133',
  'googletest_revision': 'bf0701daa9f5b30e5882e2f8f9a5280bcba87e77',
  're2_revision': '4244cd1cb492fa1d10986ec67f862964c073f844',
  'spirv_headers_revision': '814e728b30ddd0f4509233099a3ad96fd4318c07',
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

