use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'effcee_revision': '35912e1b7778ec2ddcff7e7188177761539e59e0',
  'googletest_revision': 'd9bb8412d60b993365abb53f00b6dad9b2c01b62',
  're2_revision': 'd2836d1b1c34c4e330a85a1006201db474bf2c8a',
  'spirv_headers_revision': '34d04647d384e0aed037e7a2662a655fc39841bb',
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

