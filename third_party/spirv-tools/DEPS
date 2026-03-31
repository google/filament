use_relative_paths = True

vars = {
  'github': 'https://github.com',

  'abseil_revision': '889ddc99e10c02c7110223e6d0dc41d83f0c0b60',

  'effcee_revision': '514b52ec61609744d7e587d93a7ef9b60407ab45',

  'googletest_revision': '85087857ad10bd407cd6ed2f52f7ea9752db621f',

  # Use protobufs before they gained the dependency on abseil
  'protobuf_revision': 'v21.12',

  're2_revision': 'e7aec5985072c1dbe735add802653ef4b36c231a',

  'spirv_headers_revision': '04f10f650d514df88b76d25e83db360142c7b174',

  'mimalloc_revision': 'b69f9cb39f22b95971a4b9196a9c265a7b474718',
}

deps = {
  'external/abseil_cpp':
      Var('github') + '/abseil/abseil-cpp.git@' + Var('abseil_revision'),

  'external/effcee':
      Var('github') + '/google/effcee.git@' + Var('effcee_revision'),

  'external/googletest':
      Var('github') + '/google/googletest.git@' + Var('googletest_revision'),

  'external/protobuf':
      Var('github') + '/protocolbuffers/protobuf.git@' + Var('protobuf_revision'),

  'external/re2':
      Var('github') + '/google/re2.git@' + Var('re2_revision'),

  'external/spirv-headers':
      Var('github') +  '/KhronosGroup/SPIRV-Headers.git@' +
          Var('spirv_headers_revision'),

  'external/mimalloc':
      Var('github') + '/microsoft/mimalloc.git@' + Var('mimalloc_revision'),
}

